"""The pre-deployment check must reject foreign module CRC tables."""

import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest


SCRIPT = Path(__file__).with_name("frozen_module_versions.py")


def elf32_module(path, versions):
    names = b"\0.shstrtab\0__versions\0"
    body = b"".join(
        struct.pack("<I", crc) + name.encode() + b"\0" * (60 - len(name))
        for name, crc in versions
    )
    version_offset = (52 + len(names) + 3) & ~3
    section_offset = version_offset + len(body)
    header = struct.pack(
        "<16sHHIIIIIHHHHHH",
        b"\x7fELF\x01\x01\x01" + b"\0" * 9,
        1, 3, 1, 0, 0, section_offset, 0, 52, 0, 0, 40, 3, 1,
    )
    section = struct.Struct("<IIIIIIIIII")
    headers = b"\0" * 40
    headers += section.pack(1, 3, 0, 0, 52, len(names), 0, 0, 1, 0)
    headers += section.pack(11, 1, 0, 0, version_offset, len(body), 0, 0, 4, 64)
    path.write_bytes(header + names + b"\0" * (version_offset - 52 - len(names)) + body + headers)


class FrozenModuleVersionsTest(unittest.TestCase):
    def run_check(self, reference, candidate):
        return subprocess.run(
            [sys.executable, str(SCRIPT), str(reference), str(candidate)],
            text=True, capture_output=True, check=False,
        )

    def run_symvers_check(self, reference, table):
        return subprocess.run(
            [sys.executable, str(SCRIPT), "--check-symvers", str(reference), str(table)],
            text=True, capture_output=True, check=False,
        )

    def test_rejects_module_layout_crc_mismatch(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            candidate = Path(directory) / "candidate.ko"
            elf32_module(reference, [("module_layout", 0xB84EFB99)])
            elf32_module(candidate, [("module_layout", 0x995E9910)])
            result = self.run_check(reference, candidate)
            self.assertEqual(result.returncode, 1, result.stderr)
            report = json.loads(result.stdout)
            self.assertEqual(report["module_layout"], {
                "reference": "0xb84efb99", "candidate": "0x995e9910"
            })
            self.assertEqual(report["mismatched_count"], 1)
            self.assertEqual(report["classification"], "REJECT")

    def test_rejects_candidate_import_not_covered_by_reference(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            candidate = Path(directory) / "candidate.ko"
            elf32_module(reference, [("module_layout", 0xB84EFB99)])
            elf32_module(candidate, [
                ("module_layout", 0xB84EFB99), ("new_symbol", 0x12345678)
            ])
            result = self.run_check(reference, candidate)
            self.assertEqual(result.returncode, 1, result.stderr)
            report = json.loads(result.stdout)
            self.assertEqual(report["mismatched_count"], 0)
            self.assertEqual(report["unverified_candidate_imports"], ["new_symbol"])

    def test_matching_complete_import_set_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            candidate = Path(directory) / "candidate.ko"
            elf32_module(reference, [
                ("module_layout", 0xB84EFB99), ("shared", 0xAABBCCDD)
            ])
            elf32_module(candidate, [
                ("module_layout", 0xB84EFB99), ("shared", 0xAABBCCDD)
            ])
            result = self.run_check(reference, candidate)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(json.loads(result.stdout)["classification"], "PASS")

    def test_rejects_build_table_with_module_layout_mismatch(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            table = Path(directory) / "Module.symvers"
            elf32_module(reference, [
                ("module_layout", 0xB84EFB99), ("shared", 0xAABBCCDD)
            ])
            table.write_text(
                "0x995e9910\tmodule_layout\tvmlinux\tEXPORT_SYMBOL\t\n"
                "0xaabbccdd\tshared\tvmlinux\tEXPORT_SYMBOL\t\n"
            )
            result = self.run_symvers_check(reference, table)
            self.assertEqual(result.returncode, 1, result.stderr)
            report = json.loads(result.stdout)
            self.assertEqual(report["classification"], "REJECT")
            self.assertEqual(report["mismatched_count"], 1)
            self.assertEqual(report["module_layout"], {
                "reference": "0xb84efb99", "build_table": "0x995e9910"
            })

    def test_rejects_build_table_missing_an_import(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            table = Path(directory) / "Module.symvers"
            elf32_module(reference, [
                ("module_layout", 0xB84EFB99), ("shared", 0xAABBCCDD)
            ])
            table.write_text("0xb84efb99\tmodule_layout\tvmlinux\tEXPORT_SYMBOL\t\n")
            result = self.run_symvers_check(reference, table)
            self.assertEqual(result.returncode, 1, result.stderr)
            self.assertEqual(json.loads(result.stdout)["missing_imports"], ["shared"])

    def test_rejects_other_import_crc_even_when_module_layout_matches(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            candidate = Path(directory) / "candidate.ko"
            table = Path(directory) / "Module.symvers"
            elf32_module(reference, [
                ("module_layout", 0xB84EFB99), ("shared", 0xAABBCCDD)
            ])
            elf32_module(candidate, [
                ("module_layout", 0xB84EFB99), ("shared", 0x11223344)
            ])
            table.write_text(
                "0xb84efb99\tmodule_layout\tvmlinux\tEXPORT_SYMBOL\t\n"
                "0x11223344\tshared\tvmlinux\tEXPORT_SYMBOL\t\n"
            )
            for result in (self.run_check(reference, candidate),
                           self.run_symvers_check(reference, table)):
                self.assertEqual(result.returncode, 1, result.stderr)
                self.assertEqual(list(json.loads(result.stdout)["mismatches"]), ["shared"])

    def test_matching_build_table_passes_import_coverage(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            table = Path(directory) / "Module.symvers"
            elf32_module(reference, [
                ("module_layout", 0xB84EFB99), ("shared", 0xAABBCCDD)
            ])
            table.write_text(
                "0xb84efb99\tmodule_layout\tvmlinux\tEXPORT_SYMBOL\t\n"
                "0xaabbccdd\tshared\tvmlinux\tEXPORT_SYMBOL\t\n"
            )
            result = self.run_symvers_check(reference, table)
            self.assertEqual(result.returncode, 0, result.stderr)
            report = json.loads(result.stdout)
            self.assertEqual(report["classification"], "PASS")
            self.assertEqual(report["scope"], "REFERENCE_IMPORTS_ONLY")
            self.assertFalse(report["candidate_imports_verified"])

    def test_captured_target_table_covers_original_and_all_new_imports(self):
        # This is independently captured installed-header evidence, not the
        # public source-package table that passed vermagic but failed loading.
        root = SCRIPT.resolve().parents[2]
        capture = root / 'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08'
        reference = root / 'docs/hardware-evidence/MINI12-20260927-H0/raw/installed-gma500_gfx.ko'
        table = capture / 'artifacts/build_Module_symvers'
        result = self.run_symvers_check(reference, table)
        self.assertEqual(result.returncode, 0, result.stderr)
        report = json.loads(result.stdout)
        self.assertEqual(report['reference_import_count'], 222)
        self.assertEqual(report['module_layout']['build_table'], '0xb84efb99')
        self.assertEqual(report['build_table_sha256'],
                         'faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca')
        # Reference-only PASS must not become an implicit candidate PASS.
        self.assertFalse(report['candidate_imports_verified'])
        import frozen_module_versions as versions
        entries, _ = versions.read_symvers(table)
        new_imports = {'drm_clflush_pages', 'memcmp', 'memcpy', 'module_put',
                       'request_resource', 'try_module_get', 'usleep_range',
                       'vmap', 'vunmap'}
        self.assertTrue(new_imports <= entries.keys())
        stdout = (capture / 'stdout.txt').read_text()
        self.assertIn('READONLY_CAPTURE_PASS', stdout)
        self.assertIn('linux-headers-5.10.240-antix.1-486-smp: '
                      '/usr/src/linux-headers-5.10.240-antix.1-486-smp/Module.symvers', stdout)

    def test_captured_kernel_config_and_generated_headers_agree(self):
        import hashlib
        import re
        root = SCRIPT.resolve().parents[2]
        artifacts = root / 'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts'
        config_bytes = (artifacts / 'build__config').read_bytes()
        self.assertEqual(config_bytes, (artifacts / 'boot-config').read_bytes())
        self.assertEqual(hashlib.sha256(config_bytes).hexdigest(),
                         '93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9')
        config = dict(line.split('=', 1) for line in config_bytes.decode().splitlines()
                      if line.startswith('CONFIG_'))
        auto = dict(line.split('=', 1) for line in
                    (artifacts / 'build_include_config_auto_conf').read_text().splitlines()
                    if line.startswith('CONFIG_'))
        self.assertEqual(config, auto)
        generated = dict(re.findall(r'^#define (CONFIG_\w+) (.+)$',
                         (artifacts / 'build_include_generated_autoconf_h').read_text(), re.M))
        expected = {(k + '_MODULE' if v == 'm' else k):
                    ('1' if v in ('y', 'm') else v) for k, v in config.items()}
        self.assertEqual(expected.keys(), generated.keys())
        for key, value in expected.items():
            actual = generated[key]
            if value != actual:
                # Kconfig writes a hex-typed zero as 0 in .config and 0x0
                # in autoconf.h. Compare its numeric value, not typography.
                self.assertTrue(re.fullmatch(r'(?:0x[0-9a-fA-F]+|[0-9]+)', value))
                self.assertTrue(re.fullmatch(r'(?:0x[0-9a-fA-F]+|[0-9]+)', actual))
                self.assertEqual(int(value, 0), int(actual, 0))
        self.assertEqual(config['CONFIG_MODVERSIONS'], 'y')
        self.assertEqual(config['CONFIG_X86_32'], 'y')
        self.assertEqual(config['CONFIG_CC_IS_GCC'], 'y')
        self.assertEqual(config['CONFIG_GCC_VERSION'], '140200')

    def test_rejects_duplicate_build_table_symbol(self):
        with tempfile.TemporaryDirectory() as directory:
            reference = Path(directory) / "reference.ko"
            table = Path(directory) / "Module.symvers"
            elf32_module(reference, [("module_layout", 0xB84EFB99)])
            table.write_text(
                "0xb84efb99\tmodule_layout\tvmlinux\tEXPORT_SYMBOL\t\n"
                "0xb84efb99\tmodule_layout\tvmlinux\tEXPORT_SYMBOL\t\n"
            )
            result = self.run_symvers_check(reference, table)
            self.assertEqual(result.returncode, 2)
            self.assertIn("duplicate", result.stderr)


if __name__ == "__main__":
    unittest.main()
