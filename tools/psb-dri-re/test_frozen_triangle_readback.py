"""Position-sensitive offline checks; fixtures are CPU references, not GPU evidence."""
import importlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest


HERE = Path(__file__).resolve().parent


def reference_fixture(include_diagonal=False):
    # Hand-derived scanlines for the three frozen vertices. The 16 pixel
    # centers on x+y=32 are the only boundary difference between references.
    widths = ((16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
              if include_diagonal else
              (15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
    data = bytearray(4096)
    for y, width in enumerate(widths, 8):
        data[y * 128 + 8 * 4:y * 128 + (8 + width) * 4] = b'\xff' * (width * 4)
    return data


def put(data, x, y, word):
    struct.pack_into('<I', data, y * 128 + x * 4, word)


class FrozenReadbackTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(importlib.util.find_spec('frozen_triangle_readback'),
                             'position-sensitive readback validator is missing')
        self.validator = importlib.import_module('frozen_triangle_readback')

    def test_exact_scanlines_match_each_complete_edge_reference(self):
        for diagonal, rule, count in [(False, 'exclude', 120),
                                      (True, 'include', 136)]:
            with self.subTest(rule=rule):
                report = self.validator.validate_readback(reference_fixture(diagonal))
                self.assertTrue(report['image_matches_reference'])
                self.assertEqual(report['matching_edge_rules'], [rule])
                self.assertEqual(report['references'][rule]['mismatch_count'], 0)
                self.assertEqual(report['references'][rule]['expected_white_pixels'], count)
                self.assertEqual(report['references'][{'include': 'exclude',
                                                      'exclude': 'include'}[rule]]
                                 ['mismatch_count'], 16)

    def test_matching_image_never_establishes_hardware_completion(self):
        report = self.validator.validate_readback(reference_fixture())
        self.assertEqual(report['completion_attribution'], 'UNKNOWN')
        self.assertFalse(report['triangle_established'])
        self.assertIn('pixel_center', report['reference_assumptions'])
        self.assertIn('unqualified', report['reference_assumptions'])

    def test_same_nonzero_count_at_wrong_positions_is_rejected(self):
        data = reference_fixture()
        put(data, 8, 8, 0)
        put(data, 0, 0, 0xffffffff)
        report = self.validator.validate_readback(data)
        self.assertFalse(report['image_matches_reference'])
        self.assertEqual(report['references']['exclude']['mismatch_count'], 2)
        errors = report['references']['exclude']['mismatches']
        self.assertEqual([(e['x'], e['y'], e['kind']) for e in errors],
                         [(0, 0, 'unexpected_foreground'), (8, 8, 'missing_foreground')])

    def test_shifted_triangle_is_rejected(self):
        data = reference_fixture()
        shifted = bytearray(4096)
        for y in range(32):
            shifted[y * 128 + 4:(y + 1) * 128] = data[y * 128:(y + 1) * 128 - 4]
        self.assertFalse(self.validator.validate_readback(shifted)['image_matches_reference'])

    def test_vertical_flip_is_not_silently_accepted(self):
        data = reference_fixture()
        flipped = b''.join(data[y * 128:(y + 1) * 128] for y in reversed(range(32)))
        self.assertFalse(self.validator.validate_readback(flipped)['image_matches_reference'])

    def test_white_rgb_with_wrong_alpha_is_rejected(self):
        data = reference_fixture()
        put(data, 8, 8, 0x00ffffff)
        report = self.validator.validate_readback(data)
        self.assertFalse(report['image_matches_reference'])
        self.assertEqual(report['references']['exclude']['mismatches'][0]['kind'],
                         'wrong_color')

    def test_nonzero_background_alpha_is_rejected(self):
        data = reference_fixture()
        put(data, 31, 31, 0xff000000)
        report = self.validator.validate_readback(data)
        self.assertFalse(report['image_matches_reference'])
        error = report['references']['exclude']['mismatches'][0]
        self.assertEqual((error['x'], error['y'], error['byte_offset']), (31, 31, 4092))
        self.assertEqual(error['expected_argb'], '0x00000000')
        self.assertEqual(error['actual_argb'], '0xff000000')

    def test_mixed_diagonal_coverage_matches_neither_reference(self):
        data = reference_fixture(True)
        put(data, 23, 8, 0)
        report = self.validator.validate_readback(data)
        self.assertFalse(report['image_matches_reference'])
        self.assertEqual(report['matching_edge_rules'], [])
        self.assertEqual(report['references']['include']['mismatch_count'], 1)
        self.assertEqual(report['references']['exclude']['mismatch_count'], 15)

    def test_empty_and_solid_images_are_rejected(self):
        for data in [bytes(4096), b'\xff' * 4096]:
            with self.subTest(first=data[0]):
                report = self.validator.validate_readback(data)
                self.assertFalse(report['image_matches_reference'])
                self.assertFalse(report['triangle_established'])

    def test_wrong_size_is_rejected_instead_of_truncated_or_padded(self):
        for size in [0, 4095, 4097]:
            with self.subTest(size=size):
                with self.assertRaisesRegex(ValueError, '4096'):
                    self.validator.validate_readback(bytes(size))

    def test_diagnostic_cap_does_not_skip_pixel_comparisons(self):
        report = self.validator.validate_readback(b'\xff' * 4096, max_details=2)
        self.assertEqual(report['references']['exclude']['mismatch_count'], 904)
        self.assertEqual(report['references']['include']['mismatch_count'], 888)
        self.assertEqual(len(report['references']['exclude']['mismatches']), 2)
        self.assertEqual(report['references']['exclude']['details_omitted'], 902)

    def test_cli_preserves_original_and_reports_only_reference_agreement(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'color.bin'
            original = bytes(reference_fixture())
            path.write_bytes(original)
            run = subprocess.run([sys.executable, '-B', str(HERE / 'frozen_triangle_readback.py'),
                                  str(path)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            report = json.loads(run.stdout)
            self.assertTrue(report['image_matches_reference'])
            self.assertFalse(report['triangle_established'])
            self.assertEqual(report['completion_attribution'], 'UNKNOWN')
            self.assertEqual(path.read_bytes(), original)

    def test_cli_rejects_bad_size_and_missing_file(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'bad.bin'
            path.write_bytes(bytes(4097))
            for supplied in [path, Path(directory) / 'absent.bin']:
                with self.subTest(path=supplied):
                    run = subprocess.run([sys.executable, '-B',
                                          str(HERE / 'frozen_triangle_readback.py'),
                                          str(supplied)], capture_output=True, text=True)
                    self.assertEqual(run.returncode, 2)
                    report = json.loads(run.stdout)
                    self.assertFalse(report['triangle_established'])
                    self.assertEqual(report['completion_attribution'], 'UNKNOWN')

    def test_cli_mismatch_returns_pixel_diagnostics(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'color.bin'
            data = reference_fixture()
            put(data, 10, 10, 0xffff0000)
            path.write_bytes(data)
            run = subprocess.run([sys.executable, '-B', str(HERE / 'frozen_triangle_readback.py'),
                                  str(path)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 1, run.stderr)
            error = json.loads(run.stdout)['references']['exclude']['mismatches'][0]
            self.assertEqual((error['x'], error['y'], error['kind']), (10, 10, 'wrong_color'))

    def test_cli_rejects_fifo_without_waiting_for_a_writer(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'color.fifo'
            os.mkfifo(path)
            run = subprocess.run([sys.executable, '-B',
                                  str(HERE / 'frozen_triangle_readback.py'), str(path)],
                                 capture_output=True, text=True, timeout=2)
            self.assertEqual(run.returncode, 2, run.stderr)
            report = json.loads(run.stdout)
            self.assertIn('regular file', report['error'])
            self.assertFalse(report['triangle_established'])
            self.assertEqual(report['completion_attribution'], 'UNKNOWN')

    def test_cli_rejects_stdout_alias_without_changing_valid_or_invalid_input(self):
        for hardlink in (False, True):
            for valid in (False, True):
                with self.subTest(hardlink=hardlink, valid=valid):
                    with tempfile.TemporaryDirectory() as directory:
                        path = Path(directory) / 'color.bin'
                        original = bytes(reference_fixture()) if valid else bytes(4095)
                        path.write_bytes(original)
                        output = path
                        if hardlink:
                            output = Path(directory) / 'report.json'
                            os.link(path, output)
                        # Nontruncating open isolates writes made by the validator.
                        with output.open('r+b') as stream:
                            run = subprocess.run(
                                [sys.executable, '-B', str(HERE / 'frozen_triangle_readback.py'),
                                 str(path)], stdout=stream, stderr=subprocess.PIPE, timeout=2)
                        self.assertEqual(path.read_bytes(), original)
                        self.assertEqual(run.returncode, 2, run.stderr)
                        self.assertIn(b'stdout', run.stderr)
                        self.assertIn(b'readback', run.stderr)

    def test_cli_does_not_report_alias_error_to_aliased_stderr(self):
        for valid in (False, True):
            with self.subTest(valid=valid):
                with tempfile.TemporaryDirectory() as directory:
                    path = Path(directory) / 'color.bin'
                    original = bytes(reference_fixture()) if valid else bytes(4095)
                    path.write_bytes(original)
                    with path.open('r+b') as stream:
                        run = subprocess.run(
                            [sys.executable, '-B', str(HERE / 'frozen_triangle_readback.py'),
                             str(path)], stdout=stream, stderr=stream, timeout=2)
                    self.assertEqual(path.read_bytes(), original)
                    self.assertEqual(run.returncode, 2)

    def test_cli_missing_or_closed_stdout_does_not_traceback_to_readback(self):
        for missing in (False, True):
            for valid in (False, True):
                with self.subTest(missing=missing, valid=valid):
                    with tempfile.TemporaryDirectory() as directory:
                        path = Path(directory) / 'color.bin'
                        original = bytes(reference_fixture()) if valid else bytes(4095)
                        path.write_bytes(original)
                        # Exercise actual Python streams/descriptors, without mocks.
                        setup = 'sys.stdout = None' if missing else 'os.close(1)'
                        code = ('import os, runpy, sys; ' + setup + '; '
                                'sys.argv = sys.argv[1:]; '
                                'runpy.run_path(sys.argv[0], run_name="__main__")')
                        with path.open('r+b') as stream:
                            run = subprocess.run(
                                [sys.executable, '-B', '-c', code,
                                 str(HERE / 'frozen_triangle_readback.py'), str(path)],
                                stdout=subprocess.PIPE, stderr=stream, timeout=2)
                        self.assertEqual(path.read_bytes(), original)
                        self.assertEqual(run.returncode, 2)
                        self.assertEqual(run.stdout, b'')


if __name__ == '__main__':
    unittest.main()
