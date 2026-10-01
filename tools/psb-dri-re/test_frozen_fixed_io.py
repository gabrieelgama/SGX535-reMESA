import pathlib
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parent


class FixedIoTests(unittest.TestCase):
    def test_bounded_action_execution(self):
        with tempfile.TemporaryDirectory() as temp:
            binary = pathlib.Path(temp) / "fixed-io"
            subprocess.run(
                ["cc", "-std=c11", "-Wall", "-Wextra", "-Werror",
                 "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
                 str(ROOT / "frozen_fixed_io.c"),
                 str(ROOT / "test_frozen_fixed_io.c"), "-o", str(binary)],
                check=True, capture_output=True, text=True,
            )
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)


if __name__ == "__main__":
    unittest.main()
