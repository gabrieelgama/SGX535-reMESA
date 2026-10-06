"""Synthetic retained-record consistency only; never opens a device."""
import unittest
import subprocess
import tempfile
from pathlib import Path
from frozen_source_guard_evidence import evaluate

GOOD = b'''SGXSOURCE2
state=0
reasons=0
producers_active=0
driver_attached=1
capsule_state=0
boot=3
prepared=1
asserted_reset=127
released_reset=0
startup_pending=0
startup_reads=0
startup_fault=0
startup_autonomous=0
delayed_exclusion=STARTUP_LIFECYCLE
'''


class SourceEvidenceTests(unittest.TestCase):
    def test_closed_value_matches_actual_compiled_producer_enum(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / 'enum.c'
            binary = Path(directory) / 'enum'
            source.write_text('#include "frozen_capsule.h"\n#include <stdio.h>\n'
                              'int main(void) { printf("%u", SGX535_CAP_CLOSED); }\n')
            subprocess.run(['gcc', '-std=c11', '-Wall', '-Wextra', '-Werror',
                            '-I', str(Path(__file__).parent), str(source), '-o', str(binary)], check=True)
            value = subprocess.check_output([str(binary)])
            closed = GOOD.replace(b'state=0\n', b'state=3\n', 1).replace(
                b'capsule_state=0', b'capsule_state=' + value)
            self.assertFalse(evaluate(closed, closed=True)['errors'])
            self.assertTrue(evaluate(closed, closed=True)['hardware_attribution'].startswith('UNKNOWN'))

    def test_prepared_synthetic_record_never_establishes_hardware_or_triangle(self):
        result = evaluate(GOOD)
        self.assertEqual(result['supplied_record_consistency'], 'CONFIRMED')
        self.assertTrue(result['hardware_attribution'].startswith('UNKNOWN'))
        self.assertEqual(result['triangle'], 'NOT ESTABLISHED BY SOURCE WITNESS')

    def test_closed_record_requires_both_intervals_closed(self):
        closed = GOOD.replace(b'state=0\n', b'state=3\n', 1).replace(b'capsule_state=0', b'capsule_state=4')
        self.assertFalse(evaluate(closed, closed=True)['errors'])
        self.assertTrue(evaluate(GOOD, closed=True)['errors'])
        self.assertTrue(evaluate(closed)['errors'])
        for state in (0, 1, 2, 3, 5, 6):
            self.assertTrue(evaluate(closed.replace(b'capsule_state=4',
                            ('capsule_state=' + str(state)).encode()), closed=True)['errors'])

    def test_each_proof_field_is_required(self):
        for line in GOOD.splitlines()[1:]:
            with self.subTest(line=line):
                self.assertTrue(evaluate(GOOD.replace(line + b'\n', b''))['errors'])

    def test_mixed_stale_pending_lost_or_unprepared_records_fail(self):
        for old, new in [(b'boot=3', b'boot=0'), (b'prepared=1', b'prepared=0'),
                         (b'asserted_reset=127', b'asserted_reset=63'),
                         (b'released_reset=0', b'released_reset=1'),
                         (b'startup_reads=0', b'startup_reads=1'),
                         (b'startup_autonomous=0', b'startup_autonomous=16777216'),
                         (b'startup_pending=0', b'startup_pending=8192'),
                         (b'startup_pending=0', b'startup_pending=1'),
                         (b'reasons=0', b'reasons=32'),
                         (b'reasons=0', b'reasons=64'),
                         (b'producers_active=0', b'producers_active=1'),
                         (b'capsule_state=0', b'capsule_state=6'),
                         (b'STARTUP_LIFECYCLE', b'UNPROVEN')]:
            with self.subTest(new=new):
                self.assertTrue(evaluate(GOOD.replace(old, new))['errors'])

    def test_truncated_duplicate_extra_and_malformed_records_fail(self):
        for raw in [b'', GOOD[:-1], GOOD + b'reasons=0\n', GOOD + b'extra=1\n',
                    GOOD.replace(b'boot=3', b'boot=-1'),
                    GOOD.replace(b'boot=3', b'boot=4294967296'), b'\xff\n',
                    b'x' * 4097 + b'\n']:
            with self.subTest(raw=raw[:40]):
                self.assertTrue(evaluate(raw)['errors'])


if __name__ == '__main__':
    unittest.main(verbosity=2)
