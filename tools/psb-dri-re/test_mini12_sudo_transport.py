"""Credential routing for the reviewed Mini 12 root script transport."""

import subprocess
import unittest

from mini12_sudo_transport import AuthError, run_root_script


class SudoTransportTests(unittest.TestCase):
    def test_credential_and_script_use_separate_ssh_processes(self):
        # Whether sudo consumes the credential is irrelevant to the second
        # process: that process receives only the script.
        calls = []

        def fake_run(argv, *, input, capture_output, timeout):
            calls.append((argv, input))
            return subprocess.CompletedProcess(argv, 0, b"ok\n" if len(calls) == 2 else b"", b"")

        result = run_root_script(
            ["ssh", "pinned-target"], b"id -u\n", "example-secret",
            runner=fake_run,
        )
        self.assertEqual(result.stdout, b"ok\n")
        self.assertEqual(len(calls), 2)
        self.assertEqual(calls[0][0][-1], "sudo -S -p '' -v")
        self.assertEqual(calls[0][1], b"example-secret\n")
        self.assertNotIn("example-secret", " ".join(calls[0][0]))
        self.assertEqual(calls[1][0][-1], "sudo -n -p '' sh -s")
        self.assertEqual(calls[1][1], b"id -u\n")
        self.assertNotIn(b"example-secret", calls[1][1])
        self.assertNotIn("example-secret", " ".join(calls[1][0]))

    def test_failed_auth_does_not_start_script(self):
        calls = []

        def fake_run(argv, *, input, capture_output, timeout):
            calls.append((argv, input))
            return subprocess.CompletedProcess(argv, 1, b"", b"denied\n")

        with self.assertRaises(AuthError):
            run_root_script(["ssh", "pinned-target"], b"id -u\n",
                            "example-secret", runner=fake_run)
        self.assertEqual(len(calls), 1)

    def test_credential_newline_is_rejected_before_remote_call(self):
        def forbidden_run(*args, **kwargs):
            self.fail("remote call must not start")

        with self.assertRaises(ValueError):
            run_root_script(["ssh", "pinned-target"], b"id -u\n",
                            "example\nsecret", runner=forbidden_run)


if __name__ == "__main__":
    unittest.main()
