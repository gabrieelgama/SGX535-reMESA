"""Model-integrity regressions; these are not GPU/ISA observations."""
import copy
import unittest

import pds_launch_state_check as launch
from cleanroom_pds_image import build_image


class LaunchStateTests(unittest.TestCase):
    def setUp(self):
        self.rows = launch.read_inventory()

    def test_inventory_is_valid_but_coverage_is_open(self):
        result = launch.check_inventory(self.rows, build_image(0x12345678), 0x12345678)
        self.assertFalse(result['coverage_closed'])
        self.assertEqual(result['missing_launch_rule'], 'L12')
        self.assertEqual(result['possible_inputs'], 'PARTIALLY BOUNDED')
        with self.assertRaisesRegex(ValueError, 'L12'):
            launch.require_closed(result)

    def test_omitted_state_rejected(self):
        for i in range(len(self.rows)):
            with self.subTest(i=i), self.assertRaises(ValueError):
                launch.check_inventory(self.rows[:i] + self.rows[i+1:], build_image(1), 1)

    def test_duplicate_rejected(self):
        with self.assertRaises(ValueError):
            launch.check_inventory(self.rows + [self.rows[0]], build_image(1), 1)

    def test_new_uncovered_input_rejected(self):
        row = dict(self.rows[-1], state_id='new_external_input')
        with self.assertRaisesRegex(ValueError, 'inventory'):
            launch.check_inventory(self.rows + [row], build_image(1), 1)

    def test_status_promotion_is_not_a_proof(self):
        rows = copy.deepcopy(self.rows)
        for row in rows:
            if row['state_id'] == 'ds1':
                row['determinism'] = 'CPU_DEFINED'
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            launch.check_inventory(rows, build_image(1), 1)

    def test_missing_provenance_rejected(self):
        rows = copy.deepcopy(self.rows)
        rows[0]['producer'] = ''
        with self.assertRaises(ValueError):
            launch.check_inventory(rows, build_image(1), 1)

    def test_wrong_initialization_order_rejected(self):
        rows = copy.deepcopy(self.rows)
        rows[0]['initialization'] = 'after_first_use'
        with self.assertRaises(ValueError):
            launch.check_inventory(rows, build_image(1), 1)

    def test_all_56_bytes_still_required(self):
        with self.assertRaisesRegex(ValueError, 'undefined'):
            launch.check_inventory(self.rows, build_image(1, initialize=False), 1)

    def test_cpu_layout_not_hardware_alias(self):
        self.assertEqual([launch.cpu_slot(0, n) for n in range(8)], list(range(0, 32, 4)))
        self.assertEqual([launch.cpu_slot(1, n) for n in range(4)], list(range(32, 48, 4)))
        self.assertEqual(launch.cpu_slot(1, 4), 48)
        self.assertEqual(launch.cpu_slot(0, 8), 64)
        self.assertEqual(launch.cpu_extent(2, 1), (9, 12))
        self.assertEqual(launch.cpu_extent(0, 0), (0, 0))
        # Extending the CPU placement formula grants no hardware read permission.
        self.assertFalse(launch.check_inventory(self.rows, build_image(1), 1)['coverage_closed'])

    def test_selected_packed_words(self):
        # Synthetic addresses exercise CPU masking only; they are not real mappings.
        self.assertEqual(launch.selected_control_words(0x20000160, 0x200001a0),
                         (0x0000001a, 0x00030000, 0x0c000016))
        with self.assertRaises(ValueError):
            launch.selected_control_words(0x20000161, 0x200001a0)

    def test_unknown_readable_state_blocks_coverage(self):
        rows = copy.deepcopy(self.rows)
        extra = dict(next(r for r in rows if r['state_id']=='temp0'),
                     state_id='synthetic_readable', selected_reference='YES',
                     range_status='BOUNDED', first_read_defined='NO',
                     availability_evidence='SYNTHETIC_REACHABLE_RANGE')
        result = launch.assess_availability(rows+[extra])
        self.assertIn('synthetic_readable',result['source_blockers'])

    def test_control_unknown_is_not_invented_source_hazard(self):
        result=launch.assess_availability(self.rows)
        self.assertNotIn('dout',result['source_blockers'])
        self.assertNotIn('entry',result['source_blockers'])
        self.assertIn('entry',result['control_obligations'])

    def test_exclusion_without_evidence_fails(self):
        rows=copy.deepcopy(self.rows)
        next(r for r in rows if r['state_id']=='temp0')['selected_reference']='EXCLUDED'
        with self.assertRaisesRegex(ValueError,'exclusion'):
            launch.assess_availability(rows)

    def test_source_cannot_be_relabelled_as_control(self):
        rows=copy.deepcopy(self.rows)
        next(r for r in rows if r['state_id']=='temp0')['role']='CONTROL'
        with self.assertRaisesRegex(ValueError,'role'):
            launch.check_inventory(rows,build_image(1),1)

    def test_complete_list_does_not_prove_complete_envelope(self):
        # Synthetic model fixture: no assertion about hardware is introduced.
        rows=copy.deepcopy(self.rows)
        for r in rows:
            if r['role']=='SOURCE':
                r.update(range_status='BOUNDED',first_read_defined='YES',
                         availability_evidence='SYNTHETIC_AXIOM')
        result=launch.assess_availability(rows)
        self.assertTrue(result['listed_sources_covered'])
        self.assertFalse(result['envelope_proven'])
        self.assertFalse(result['coverage_closed'])

    def test_missing_availability_provenance_fails(self):
        rows=copy.deepcopy(self.rows)
        next(r for r in rows if r['state_id']=='ds0')['availability_evidence']=''
        with self.assertRaises(ValueError):
            launch.assess_availability(rows)


if __name__ == '__main__':
    unittest.main()
