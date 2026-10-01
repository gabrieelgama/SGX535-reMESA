"""Exercise the offline owner rollback contract with injected backend failures."""
import pathlib
import subprocess
import tempfile
import unittest


HERE = pathlib.Path(__file__).resolve().parent


class FrozenSceneOwnerTests(unittest.TestCase):
    def test_all_acquisition_failures_unwind_and_submit_holds_resources(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / 'owner-test'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', str(HERE / 'frozen_kernel_contract.c'),
                str(HERE / 'frozen_scene_owner.c'),
                str(HERE / 'frozen_va_pool.c'),
                str(HERE / 'test_frozen_scene_owner.c'), '-o', str(binary),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
