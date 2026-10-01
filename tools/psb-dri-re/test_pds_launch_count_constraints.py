"""CPU formula constraints; synthetic cases are not GPU observations."""
import unittest
import pds_launch_count_constraints as count


class CountTests(unittest.TestCase):
    def test_selected_fields_are_distinct(self):
        r = count.model(0, 0, 0, 12)
        self.assertEqual((r['q'], r['q_encoded'], r['middle'], r['data_field']),
                         (4, 3, 0x30000, 0x0c000000))

    def test_temp_budget_controls_q_not_prefix_length(self):
        self.assertEqual([count.model(0,t,0,12)['q_encoded'] for t in (0,13,17,25)],
                         [3,2,1,0])
        self.assertEqual({count.model(0,t,0,12)['data_field'] for t in (0,13,17,25)},
                         {0x0c000000})

    def test_primary_attribute_budget_also_controls_q(self):
        self.assertEqual([count.model(a,0,0,12)['q_encoded'] for a in (0,32,40,52)],
                         [3,2,1,0])

    def test_data_size_does_not_control_q(self):
        self.assertEqual({count.model(0,0,0,d)['q_encoded'] for d in (0,4,8,12,16)}, {3})

    def test_false_preload_interpretation_rejected(self):
        r=count.model(0,13,0,12)
        with self.assertRaisesRegex(ValueError, 'q_encoded'):
            count.verify_case(dict(r,q_encoded=12//4))

    def test_contradictory_count_layout_pair_rejected(self):
        r=count.model(0,0,0,12)
        with self.assertRaisesRegex(ValueError, 'data_field'):
            count.verify_case(dict(r,data_field=0x08000000))

    def test_encoding_four_is_not_representable_q_term(self):
        with self.assertRaises(ValueError):
            count.verify_case(dict(count.model(0,0,0,12),q_encoded=4))

    def test_bounded_model_does_not_accept_unproved_ranges(self):
        for args in [(0,-1,0,12),(0,0,0,13),(0,0,0,256),(65536,0,0,12)]:
            with self.assertRaises(ValueError):count.model(*args)

    def test_table_matches_evidence_scope(self):
        count.check_table()

    def test_unusual_mask_not_replaced_by_alignment_intuition(self):
        r=count.model(0,0,32,12)
        self.assertEqual(r['budget'],394)
        with self.assertRaisesRegex(ValueError,'budget'):
            count.verify_case(dict(r,budget=384))


if __name__=='__main__':
    unittest.main()
