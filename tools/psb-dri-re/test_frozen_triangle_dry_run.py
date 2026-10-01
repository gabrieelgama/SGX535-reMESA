"""Offline integration checks with synthetic GPU addresses; no device access."""
import json
import unittest

import frozen_triangle_dry_run as dry_run


class DryRunTests(unittest.TestCase):
    def test_selected_bytes_and_launch_words_are_exact(self):
        report = dry_run.build_report()
        self.assertEqual(report['address_source'], 'SYNTHETIC_TEST_ONLY')
        self.assertEqual(
            report['objects']['primary_pds']['bytes_hex'],
            '01000000' + '00000000' * 7 + '20000000' + '00000000' * 3
            + '45030007000000af',
        )
        self.assertEqual(report['objects']['secondary_pds']['bytes_hex'], '000000af')
        self.assertEqual(report['objects']['fragment_use']['bytes_hex'], '00000000400104f8')
        self.assertEqual(report['primary_program_bytes_hex'], '45030007000000af')
        self.assertEqual(report['launch_words'], [0x420, 0x30000, 0x0c000016])
        self.assertEqual(report['objects']['primary_pds']['offset'], 0x160)
        self.assertEqual(report['objects']['primary_pds']['size'], 56)
        self.assertEqual(report['objects']['scene_arg']['bytes_hex'],
                         '0000000000000000200000002000000004000000')
        self.assertEqual(len(report['objects']['submit_command']['bytes_hex']), 288)
        self.assertTrue(report['objects']['submit_command']['bytes_hex'].startswith('00100000'))

    def test_report_covers_every_object_and_remains_non_executable(self):
        report = dry_run.build_report()
        self.assertEqual(len(report['objects']), 51)
        self.assertEqual(len(report['bos']), 10)
        self.assertEqual(len(report['relocations']), 49)
        self.assertEqual(report['objects']['scene_hw']['bytes_hex'], None)
        self.assertEqual(report['objects']['ta_page_table']['bytes_hex'], None)
        self.assertEqual(report['source_containment'], 'UNPROVED')
        self.assertEqual(report['gate_b'], 'BLOCKED')
        self.assertEqual(report['whitelist'], [])
        self.assertFalse(report['hardware_ready'])
        self.assertEqual(report['publication_order'][-1], 'first_consumer')
        self.assertEqual(report['submission_template']['status'], 'SYNTHETIC_NON_EXECUTABLE')
        self.assertEqual(report['bootstrap_cpu_model']['cpu_branch_code'], 121)
        self.assertEqual(report['bootstrap_cpu_model']['branch_selection'], 'ZEROED_DEFAULT_OPTIONS')
        self.assertEqual(report['bootstrap_cpu_model']['option_dword_indices'], [])
        self.assertFalse(report['bootstrap_cpu_model']['ready'])

    def test_repeated_dry_runs_are_byte_identical(self):
        first = json.dumps(dry_run.build_report(), sort_keys=True, separators=(',', ':'))
        second = json.dumps(dry_run.build_report(), sort_keys=True, separators=(',', ':'))
        self.assertEqual(first, second)


if __name__ == '__main__':
    unittest.main()
