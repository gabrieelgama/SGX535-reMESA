"""Check retained /run handoff source, without claiming live log delivery."""
import unittest
from pathlib import Path

import frozen_first_load_image as image

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / 'docs/phase8/artifacts/experimental-first-load-01-20261001/build-03'
MOVE_RUN = b'mount -n -o move /run ${rootmnt}/run\n'


def handoff_order(init, experimental=False):
    """Inspect exact selected shell commands; this does not execute a mount."""
    commands = [
        b'mount -t tmpfs -o "nodev,noexec,nosuid,size=${RUNSIZE:-10%},mode=0755" tmpfs /run\n',
        b'mkdir -m 0700 /run/initramfs\n',
    ]
    if experimental:
        commands.append(b'! /bin/sh /scripts/sgx535-first-load; then\n')
    commands.extend([
        b'run_scripts /scripts/init-top\n',
        b'run_scripts /scripts/init-bottom\n',
        MOVE_RUN,
        b'exec run-init ${drop_caps} "${rootmnt}" "${init}" "$@"',
    ])
    if any(init.count(command) != 1 for command in commands):
        raise ValueError('missing or duplicate selected /run handoff command')
    positions = [init.index(command) for command in commands]
    if positions != sorted(positions):
        raise ValueError('selected /run handoff order changed')


class TraceHandoffSourceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        inputs = image.load_inputs(ROOT)
        cls.stock = next(row['data'] for row in inputs['rows'] if row['name'] == 'init')
        cls.experimental = (BUILD / 'modified-init').read_bytes()

    def test_selected_source_is_actual_captured_stock_init(self):
        selected = ROOT / image.CAPTURE / 'offline-analysis/initramfs-analysis/selected-files/init'
        self.assertEqual(selected.read_bytes(), self.stock)
        self.assertEqual(image.sha(self.stock),
                         '0a9bb34973c78987922b57f010e99c36ddf102e8a5a2fd198385566539f5d6d5')
        handoff_order(self.stock)

    def test_experimental_init_preserves_handoff_after_preload(self):
        hook = (BUILD / 'sgx535-first-load.sh').read_bytes()
        self.assertEqual(self.experimental, image.patched_init(self.stock, hook))
        handoff_order(self.experimental, experimental=True)

    def test_missing_duplicate_or_early_run_move_is_rejected(self):
        without = self.experimental.replace(MOVE_RUN, b'')
        mutated = [without,
                   self.experimental.replace(MOVE_RUN, MOVE_RUN * 2),
                   MOVE_RUN + without]
        for init in mutated:
            with self.subTest(), self.assertRaises(ValueError):
                handoff_order(init, experimental=True)


if __name__ == '__main__':
    unittest.main()
