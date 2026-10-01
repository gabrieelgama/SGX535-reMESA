"""Build and exercise the fixed one-scene service without hardware."""
import pathlib
import subprocess
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent


class FixedServiceTests(unittest.TestCase):
    def test_one_shot_and_failures(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / 'fixed-service'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', str(HERE / 'frozen_kernel_contract.c'),
                str(HERE / 'frozen_fixed_service.c'),
                str(HERE / 'test_frozen_fixed_service.c'),
                '-o', str(binary),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
