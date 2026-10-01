"""Exercise retained PCI/module source; no target boot or hardware emulation."""
import fnmatch
import hashlib
import json
import pathlib
import resource
import subprocess
import tempfile
import unittest

from test_frozen_irq_lifecycle import extract_function

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[1]
EVIDENCE = ROOT / 'docs/phase8/artifacts/first-load-review-20261001'
DRIVER = ROOT / ('docs/phase8/artifacts/lifecycle-gate-review-20261001/'
                 'candidate-02-lifecycle-integrated-psb_drv.c')
ORIGINAL = ROOT / ('docs/hardware-evidence/MINI12-20260927-H0/raw/'
                   'installed-gma500_gfx.ko')
DERIVATIVE = ROOT / ('docs/phase8/artifacts/candidate-01-build-20260930/'
                     'candidate-02-lifecycle/gma500_gfx.ko')


class FirstLoadTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        manifest = json.loads((EVIDENCE / 'source-contracts.json').read_text())
        for name, digest in manifest['sha256'].items():
            data = (EVIDENCE / 'source-contracts' / name).read_bytes()
            if hashlib.sha256(data).hexdigest() != digest:
                raise AssertionError('retained source drift: ' + name)
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.directory = pathlib.Path(cls.tmp.name)
        pci = (EVIDENCE / 'source-contracts/drivers/pci/pci-driver.c').read_text()
        header = (EVIDENCE / 'source-contracts/drivers/pci/pci.h').read_text()
        module = (EVIDENCE / 'source-contracts/kernel/module.c').read_text()
        driver = DRIVER.read_text()
        start = driver.index('static const struct pci_device_id pciidlist[]')
        table = driver[start:driver.index('\n};', start) + 3]
        start = pci.index('struct drv_dev_and_id {')
        arguments = pci[start:pci.index('\n};', start) + 3]
        cls.fragments = (
            table, arguments,
            extract_function(header, 'static inline const struct pci_device_id *\npci_match_one_device('),
            extract_function(pci, 'const struct pci_device_id *pci_match_id('),
            extract_function(pci, 'static long local_pci_probe('),
            # NUMA/CPU dispatch and dynamic PCI IDs are kernel boundaries, not
            # part of this fixed static-ID ownership test.
            'static const struct pci_device_id *pci_match_device(struct pci_driver *d, '
            'struct pci_dev *p) { return pci_match_id(pciidlist, p); }',
            'static int pci_call_probe(struct pci_driver *d, struct pci_dev *p, '
            'const struct pci_device_id *id) { struct drv_dev_and_id args = '
            '{d, p, id}; return (int)local_pci_probe(&args); }',
            extract_function(pci, 'static int __pci_device_probe('),
            'static char *module_blacklist;',
            extract_function(module, 'static bool blacklisted('),
        )
        cls.binary = cls.build('first-load', '\n'.join(cls.fragments))

    @classmethod
    def build(cls, name, source):
        (cls.directory / (name + '.inc')).write_text(source + '\n')
        binary = cls.directory / name
        result = subprocess.run([
            '/usr/bin/gcc-14', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
            '-Wno-unused-parameter', '-Wno-missing-field-initializers',
            '-fsanitize=undefined', '-fno-sanitize-recover=all',
            '-I', str(cls.directory), '-DACTUAL_SOURCE="' + name + '.inc"',
            str(HERE / 'test_frozen_first_load.c'), '-o', str(binary)],
            capture_output=True, text=True)
        if result.returncode:
            raise AssertionError(result.stderr)
        return binary

    def run_case(self, case, binary=None):
        return subprocess.run([str(binary or self.binary), str(case)],
                              capture_output=True, text=True,
                              preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))

    def assert_case(self, case):
        result = self.run_case(case)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_actual_pci_table_selects_poulsbo_and_rejects_other_device(self):
        self.assert_case(0)

    def test_already_owned_device_is_not_replaced(self):
        self.assert_case(1)

    def test_unbound_match_assigns_owner_before_probe(self):
        self.assert_case(2)

    def test_negative_probe_result_clears_core_owner_and_pm_reference(self):
        # Driver-internal cleanup is separately covered by IRQ lifecycle tests.
        self.assert_case(3)

    def test_kernel_blacklist_blocks_both_same_named_artifacts(self):
        self.assert_case(4)

    def test_original_and_derivative_share_the_retained_target_alias_and_name(self):
        modalias = 'pci:v00008086d00008108sv00001028sd000002B1bc03sc00i00'
        for module in (ORIGINAL, DERIVATIVE):
            name = subprocess.check_output(['/usr/sbin/modinfo', '-F', 'name', str(module)], text=True)
            self.assertEqual(name.strip(), 'gma500_gfx')
            aliases = subprocess.check_output(['/usr/sbin/modinfo', '-F', 'alias', str(module)], text=True)
            self.assertTrue(any(fnmatch.fnmatchcase(modalias, alias) for alias in aliases.splitlines()))
            softdeps = subprocess.check_output(['/usr/sbin/modinfo', '-F', 'softdep', str(module)], text=True)
            self.assertEqual(softdeps.strip(), '')  # Does not inspect external modprobe policy.

    def test_deliberate_ownership_and_blacklist_mutations_are_detected(self):
        source = '\n'.join(self.fragments)
        changes = (
            ('steal-owner', 'if (!pci_dev->driver && drv->probe)', 'if (drv->probe)', 1),
            ('ignore-blacklist', 'return true;', 'return false;', 4),
        )
        for name, original, replacement, case in changes:
            self.assertEqual(source.count(original), 1)
            binary = self.build(name, source.replace(original, replacement))
            result = self.run_case(case, binary)
            self.assertNotEqual(result.returncode, 0, name + ' escaped detection')
            self.assertIn('Assertion', result.stderr)


if __name__ == '__main__':
    unittest.main()
