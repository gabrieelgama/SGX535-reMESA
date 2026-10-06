"""Offline regression for a changed hook and its enclosing init integrity gate."""
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

import frozen_first_load_image as image


def run_gate(init, hook, hook_result=0, hash_failure=False):
    """Execute only the real gate; substitute hook execution and HOLD sleeping."""
    block = re.search(rb'# SGX535-FIRSTLOAD-BEGIN:.*?# SGX535-FIRSTLOAD-END',
                      init, re.S).group().decode()
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        packaged = root / 'hook'
        packaged.write_bytes(hook)
        outcome = root / 'outcome'
        outcome.write_text(f"printf 'HOOK_RAN\\n'\nexit {hook_result}\n")
        console = root / 'console'
        block = block.replace('/scripts/sgx535-first-load', str(packaged))
        block = block.replace('/bin/sh ' + str(packaged), '/bin/sh ' + str(outcome))
        block = block.replace('/dev/console', str(console))
        block = block.replace('while :; do /bin/sleep 3600; done', 'exit 77')
        if hash_failure:
            block = block.replace('/bin/sha256sum', '/bin/false')
        result = subprocess.run(['/bin/sh', '-c', block + "\nprintf 'UDEV_NEXT\\n'\n"],
                                capture_output=True, text=True, timeout=5)
        return result, console.read_text() if console.exists() else ''


class HookBindingTests(unittest.TestCase):
    def binding(self):
        self.assertTrue(callable(getattr(image, 'bind_init_hook', None)),
                        'existing init must be rebound to the newly packaged hook')
        return image.bind_init_hook

    def checker(self):
        self.assertTrue(callable(getattr(image, 'verify_init_hook', None)),
                        'finished-image hook binding verification missing')
        return image.verify_init_hook

    def fixture(self):
        return image.patched_init(b'\nrun_scripts /scripts/init-top\n', b'old hook'), b'changed hook'

    def test_replaced_hook_passes_real_integrity_gate_and_can_continue(self):
        old, hook = self.fixture()
        fixed = self.binding()(old, hook)
        self.assertIn(hashlib.sha256(hook).hexdigest().encode(), fixed)
        self.assertEqual(self.checker()(fixed, hook), hashlib.sha256(hook).hexdigest())
        result, console = run_gate(fixed, hook)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, 'HOOK_RAN\nUDEV_NEXT\n')
        self.assertEqual(console, '')

    def test_stale_finished_image_binding_rejected_before_hook_execution(self):
        old, hook = self.fixture()
        with self.assertRaises(ValueError):
            self.checker()(old, hook)
        result, console = run_gate(old, hook)
        self.assertEqual(result.returncode, 77)
        self.assertNotIn('HOOK_RAN', result.stdout)
        self.assertNotIn('UDEV_NEXT', result.stdout)
        self.assertIn('HOLD', console)

    def test_hook_failure_still_holds_without_continuation(self):
        old, hook = self.fixture()
        result, console = run_gate(self.binding()(old, hook), hook, hook_result=1)
        self.assertEqual(result.returncode, 77)
        self.assertEqual(result.stdout, 'HOOK_RAN\n')
        self.assertIn('HOLD', console)

    def test_hash_command_failure_still_holds_without_hook(self):
        old, hook = self.fixture()
        result, console = run_gate(self.binding()(old, hook), hook, hash_failure=True)
        self.assertEqual(result.returncode, 77)
        self.assertEqual(result.stdout, '')
        self.assertIn('HOLD', console)

    def test_missing_or_duplicate_binding_anchor_rejected(self):
        old, hook = self.fixture()
        for value in (b'no gate', old + old):
            with self.subTest(value=value[:12]), self.assertRaises(ValueError):
                self.binding()(value, hook)


if __name__ == '__main__':
    unittest.main()
