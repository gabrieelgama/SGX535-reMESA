"""Run the portable SGX VA reservation ledger boundaries."""
import pathlib
import subprocess
import tempfile
import unittest


HERE = pathlib.Path(__file__).resolve().parent


class FrozenVaPoolTests(unittest.TestCase):
    def test_domains_external_collisions_and_release(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / 'va-test'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', str(HERE / 'frozen_kernel_contract.c'),
                str(HERE / 'frozen_va_pool.c'),
                str(HERE / 'test_frozen_va_pool.c'), '-o', str(binary),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
