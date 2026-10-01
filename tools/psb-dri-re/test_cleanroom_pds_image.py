"""Static Route C regressions. No historical code or device access."""
import dataclasses
import csv
from pathlib import Path
import tempfile
import unittest

import cleanroom_pds_image as pds


class ImageTests(unittest.TestCase):
    # Independent fixed little-endian oracle, U is a test parameter, not a GPU VA.
    expected = bytes.fromhex(
        '78563412 00000000 00000000 00000000'
        '00000000 00000000 00000000 00000000'
        '20000000 00000000 00000000 00000000'
        '45030007 000000af')

    def test_exact_image_and_every_byte_defined(self):
        image = pds.build_image(0x12345678)
        self.assertEqual(image.data, self.expected)
        self.assertEqual(len(image.data), 56)
        self.assertTrue(all(image.defined))
        self.assertEqual(len(image.origin), 56)
        pds.verify_image(image, 0x12345678)

    def test_nine_holes_have_explicit_initialization_provenance(self):
        image = pds.build_image(0)
        for offset in (8, 12, 16, 20, 24, 28, 36, 40, 44):
            with self.subTest(offset=offset):
                self.assertEqual(image.data[offset:offset+4], bytes(4))
                self.assertEqual(image.origin[offset:offset+4], ('ZERO_INIT',)*4)

    def test_dirty_input_is_overwritten(self):
        for fill in (0x00, 0x5a, 0xff):
            image = pds.build_image(0x12345678, initial=bytes([fill])*56)
            self.assertEqual(image.data, self.expected)
            pds.verify_image(image, 0x12345678)

    def test_omitted_initialization_rejected_even_if_bytes_happen_to_be_zero(self):
        for fill in (0, 0xa5):
            image = pds.build_image(0x12345678, initial=bytes([fill])*56,
                                    initialize=False)
            with self.assertRaisesRegex(ValueError, 'undefined'):
                pds.verify_image(image, 0x12345678)

    def test_corruption_of_each_known_word_and_each_hole_rejected(self):
        image = pds.build_image(0x12345678)
        for offset in range(0, 56, 4):
            with self.subTest(offset=offset):
                damaged = bytearray(image.data)
                damaged[offset] ^= 1
                with self.assertRaises(ValueError):
                    pds.verify_image(dataclasses.replace(image, data=bytes(damaged)),
                                     0x12345678)

    def test_forged_hole_provenance_rejected(self):
        image = pds.build_image(0)
        origin = list(image.origin)
        origin[8] = 'INHERITED'
        with self.assertRaisesRegex(ValueError, 'provenance'):
            pds.verify_image(dataclasses.replace(image, origin=tuple(origin)), 0)

    def test_relocation_is_explicit_parameter_and_only_changes_first_dword(self):
        for word in (0, 0xffffffff, 0x67676767):
            image = pds.build_image(word)
            self.assertEqual(int.from_bytes(image.data[:4], 'little'), word)
            self.assertEqual(image.data[4:], self.expected[4:])
        for bad in (None, -1, 1 << 32, True):
            with self.assertRaises(ValueError):
                pds.build_image(bad)

    def test_length_and_provenance_vectors_cannot_hide_host_padding(self):
        image = pds.build_image(0)
        for bad in (dataclasses.replace(image, data=image.data+b'\x00'),
                    dataclasses.replace(image, defined=image.defined[:-1]),
                    dataclasses.replace(image, origin=image.origin[:-1])):
            with self.assertRaises(ValueError):
                pds.verify_image(bad, 0)
        with self.assertRaises(ValueError):
            pds.build_image(0, initial=bytes(55))


class ContractTests(unittest.TestCase):
    def test_full_payload_zeroing_includes_slot_and_pool_padding(self):
        for slot_size in (0x20000, 0x40000):
            payload = bytearray([0xa5]) * (30 * slot_size)
            pds.initialize_payload(payload)
            self.assertFalse(any(payload))
            for selected in (0x160, 0x1c0):
                self.assertEqual(payload[selected:selected+56], bytes(56))

    def test_image_table_and_wrong_field_regression(self):
        table = Path(__file__).resolve().parents[2] / (
            'docs/phase7/psb-dri-re/cleanroom-pds-image.csv')
        pds.verify_table(table)
        with table.open(newline='') as stream:
            rows = list(csv.DictReader(stream))
        rows[8]['value'] = '0x00000021'
        with tempfile.TemporaryDirectory(prefix='pds-image-negative-') as tmp:
            bad = Path(tmp) / 'wrong.csv'
            with bad.open('w', newline='') as stream:
                writer = csv.DictWriter(stream, list(rows[0]), lineterminator='\n')
                writer.writeheader()
                writer.writerows(rows)
            with self.assertRaisesRegex(ValueError, 'formula/provenance'):
                pds.verify_table(bad)

    def test_complete_copy_preserves_full_backing_snapshot(self):
        original = bytes(range(256))*2
        pds.verify_preserved_snapshot(original, bytes(bytearray(original)))

    def test_partial_copy_and_changed_padding_rejected(self):
        original = bytes(range(256))*2
        for damaged in (original[:56], original[:56]+bytes(456),
                        original[:-1]+bytes([original[-1]^1])):
            with self.assertRaises(ValueError):
                pds.verify_preserved_snapshot(original, damaged)

    def test_finite_in_image_models_are_covered(self):
        # These are named hypotheses, NOT hardware-decoded source operands.
        for reads in (((0, 4), (4, 4), (32, 4)),
                      ((0, 4), (4, 4), (32, 4), (8, 4)),
                      ((0, 48),)):
            pds.require_contained(reads, 56)

    def test_unknown_extent_cannot_be_promoted_to_a_bounded_read_set(self):
        with self.assertRaisesRegex(ValueError, 'UNKNOWN'):
            pds.require_contained(None, 56)

    def test_larger_zero_backing_does_not_resolve_unknown_extent(self):
        for size in (56, 400, 0x20000, 30*0x20000, 30*0x40000):
            with self.subTest(size=size):
                with self.assertRaisesRegex(ValueError, 'UNKNOWN'):
                    pds.require_contained(None, size)

    def test_outside_range_is_a_contract_counterexample_not_an_isa_claim(self):
        for reads in (((56, 4),), ((52, 8),), ((-4, 4),)):
            with self.assertRaises(ValueError):
                pds.require_contained(reads, 56)


if __name__ == '__main__':
    unittest.main()
