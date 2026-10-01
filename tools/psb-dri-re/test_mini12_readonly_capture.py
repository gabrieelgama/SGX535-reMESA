"""Exercise only the read-only capture's identity guards against local fixtures."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / 'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-05/capture.sh'


class ReadonlyCaptureIdentityTests(unittest.TestCase):
    def run_identity(self, *, product='Inspiron 1210   ', taint=12289,
                     release='5.10.240-antix.1-486-smp', arch='i686'):
        prefix = SCRIPT.read_text().split('stage=module_identity\n', 1)[0]
        self.assertNotEqual(prefix, SCRIPT.read_text())
        with tempfile.TemporaryDirectory() as temp:
            fixture = Path(temp)
            files = {
                'proc/sys/kernel/random/boot_id': 'fixture-boot',
                'proc/sys/kernel/tainted': str(taint),
                'proc/version': 'fixture kernel version',
                'sys/class/dmi/id/product_name': product,
            }
            for name, value in files.items():
                path = fixture / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(value + '\n')
            prefix = prefix.replace('/proc/', str(fixture / 'proc') + '/')
            prefix = prefix.replace('/sys/class/', str(fixture / 'sys/class') + '/')
            # Nothing outside the fixture is read; uname is a local shell stub.
            stub = ("uname() { case \"$1\" in -r) printf '%s\\n' '" + release +
                    "';; -m) printf '%s\\n' '" + arch +
                    "';; -v) printf '%s\\n' fixture-version;; *) return 1;; esac; }\n")
            return subprocess.run(['sh', '-s'], input=stub + prefix,
                                  capture_output=True, text=True)

    def test_exact_retained_identity_is_accepted(self):
        result = self.run_identity()
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_different_dmi_bytes_are_rejected(self):
        for product in ['Inspiron 1210', 'Inspiron 1210  ', 'Inspiron 1211   ']:
            with self.subTest(product=product):
                self.assertNotEqual(self.run_identity(product=product).returncode, 0)

    def test_unqualified_taint_bits_are_rejected(self):
        for extra in [2, 8, 128, 512, 16384]:
            with self.subTest(extra=extra):
                self.assertNotEqual(self.run_identity(taint=12289 | extra).returncode, 0)

    def test_wrong_kernel_or_arch_is_rejected(self):
        self.assertNotEqual(self.run_identity(release='5.10.241-antix.1-486-smp').returncode, 0)
        self.assertNotEqual(self.run_identity(arch='x86_64').returncode, 0)


if __name__ == '__main__':
    unittest.main()
