"""The exact-action client is inert without its reviewed operation flag."""
import pathlib
import hashlib
import struct
import subprocess
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent


class OneShotClientTests(unittest.TestCase):
    def test_retained_i386_artifact_matches_reviewed_inputs(self):
        root = HERE.parents[1]
        inputs = {
            root / 'docs/phase8/artifacts/frozen-triangle-one-shot-i386':
                '758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf',
            HERE / 'frozen_triangle_one_shot.c':
                '4271a62846a0f02694e574161658920987ee89ad69e3bf5b2be79fc83f5634d2',
            root / 'kernel/sgx535_frozen/gma500_fixed_uapi.h':
                '04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420',
        }
        for path, expected in inputs.items():
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(),
                             expected, str(path))
        data = next(iter(inputs)).read_bytes()
        self.assertEqual(data[:7], b'\x7fELF\x01\x01\x01')
        self.assertEqual(struct.unpack_from('<H', data, 18)[0], 3)  # EM_386
        phoff = struct.unpack_from('<I', data, 28)[0]
        phsize, phcount = struct.unpack_from('<HH', data, 42)
        self.assertEqual(phsize, 32)
        self.assertLessEqual(phoff + phsize * phcount, len(data))
        types = [struct.unpack_from('<I', data, phoff + i * phsize)[0]
                 for i in range(phcount)]
        self.assertNotIn(2, types)  # PT_DYNAMIC
        self.assertNotIn(3, types)  # PT_INTERP

    def test_build_and_default_refusal(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / "one-shot"
            build = subprocess.run([
                "cc", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                "-pedantic", str(HERE / "frozen_triangle_one_shot.c"),
                "-o", str(binary),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 2)
            self.assertIn("No action", run.stderr)


if __name__ == "__main__":
    unittest.main()
