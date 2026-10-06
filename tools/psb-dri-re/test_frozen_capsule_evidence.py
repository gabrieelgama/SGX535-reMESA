"""Synthetic-only capsule/response agreement and protected materialization."""
import importlib
import hashlib
import json
import os
import re
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch


class CapsuleEvidenceTests(unittest.TestCase):
    def test_kernel_producer_pin_matches_exact_maintainer_decision(self):
        repo = Path(__file__).resolve().parents[2]
        decision = json.loads((repo / 'docs/phase8/response-export-client-substitution-maintainer-20261004-314e2f3b.json').read_text())
        approved = decision['approved_successor']['binary']
        source = (repo / 'kernel/sgx535_frozen/gma500_capsule_observer.c').read_text()
        initializer = source.split('static const u8 expected[32] = {',1)[1].split('};',1)[0]
        compiled = bytes(int(v,16) for v in re.findall(r'0x([0-9a-f]{2})',initializer))
        self.assertEqual(compiled.hex(), approved['sha256'])
        self.assertIn('i_size_read(file_inode(file)) != '+str(approved['size']), source)

    def setUp(self):
        self.tool = importlib.import_module('frozen_capsule_evidence')
        self.color = bytes(range(256)) * 16
        fnv, count, rows = self.tool.summary(self.color)
        self.response = struct.pack('<43I', 1, 1, 0, 0, 0, 2, 9, 7, 1,
                                    fnv, count, *rows) + self.color
        self.capsule = struct.pack('<8s12I', b'SGXCAPB1', 1, 4152, 4, 0,
                                   1, 1, 0, 9, 7, 1, 0, 0) + self.color

    def test_complete_synthetic_agreement_never_establishes_hardware_or_triangle(self):
        result = self.tool.check(self.capsule, self.response, self.color)
        self.assertEqual(result['agreement'], 'CONFIRMED')
        self.assertEqual(result['hardware_attribution'], 'UNKNOWN')
        self.assertFalse(result['triangle_established'])

    def test_wrong_length_and_format(self):
        for data in (self.capsule[:-1], self.capsule + b'x', b'BADMAGIC' + self.capsule[8:]):
            with self.assertRaises(ValueError): self.tool.check(data, self.response, self.color)

    def test_incomplete_ambiguous_crash_and_copyout_failure(self):
        for index, value in ((4, 1), (3, 2), (5, 0), (6, 0), (7, 1),
                             (8, 8), (9, 3), (10, 0), (11, 0xfffffff2)):
            values = list(struct.unpack('<8s12I', self.capsule[:56]))
            values[index] = value
            data = struct.pack('<8s12I', *values) + self.color
            result = self.tool.check(data, self.response, self.color)
            self.assertNotEqual(result['agreement'], 'CONFIRMED')
            self.assertEqual(result['instruction'], 'STOP / NO RETRY')

    def test_cross_operation_color_and_state_rejected(self):
        for offset in (16, 20, 24, 28, 32, 36, 40, 44, 172, 4267):
            wrong = bytearray(self.response); wrong[offset] ^= 1
            self.assertNotEqual(self.tool.check(self.capsule, bytes(wrong), self.color)['agreement'], 'CONFIRMED')
        wrong = bytearray(self.color); wrong[-1] ^= 1
        self.assertNotEqual(self.tool.check(self.capsule, self.response, bytes(wrong))['agreement'], 'CONFIRMED')

    def test_old_numeric_tuple_does_not_supply_channel_provenance(self):
        result = self.tool.check(self.capsule, self.response, self.color)
        self.assertEqual(result['producer_origin'], 'NOT ESTABLISHED BY SAVED BYTES ALONE')

    def test_exclusive_materialization_readback_and_no_overwrite(self):
        with tempfile.TemporaryDirectory() as d:
            os.chmod(d, 0o700)
            self.tool.preserve(d, self.capsule)
            self.assertEqual((Path(d) / 'capsule.original.bin').read_bytes(), self.capsule)
            with self.assertRaises(FileExistsError): self.tool.preserve(d, self.capsule)
            self.assertEqual((Path(d) / 'capsule.original.bin').read_bytes(), self.capsule)

    def test_partial_retained_after_preservation_failure(self):
        with tempfile.TemporaryDirectory() as d:
            os.chmod(d, 0o700)
            real = os.write; calls = 0
            def broken(fd, data):
                nonlocal calls
                calls += 1
                if calls == 1: return real(fd, data[:23])
                raise OSError('injected write loss')
            with patch('os.write', side_effect=broken):
                with self.assertRaises(OSError): self.tool.preserve(d, self.capsule)
            self.assertEqual((Path(d) / 'capsule.original.bin').read_bytes(), self.capsule[:23])
            self.assertFalse((Path(d) / 'capsule.receipt.json').exists())

    def test_sync_failure_retains_original_without_receipt(self):
        with tempfile.TemporaryDirectory() as d:
            os.chmod(d, 0o700)
            with patch('os.fsync', side_effect=OSError('sync loss')):
                with self.assertRaises(OSError): self.tool.preserve(d, self.capsule)
            self.assertEqual((Path(d) / 'capsule.original.bin').read_bytes(), self.capsule)
            self.assertFalse((Path(d) / 'capsule.receipt.json').exists())

    def test_link_destination_and_wrong_parent_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            os.chmod(d, 0o700)
            old = Path(d) / 'old'; old.write_bytes(b'old')
            (Path(d) / 'capsule.original.bin').symlink_to(old)
            with self.assertRaises(FileExistsError): self.tool.preserve(d, self.capsule)
            self.assertEqual(old.read_bytes(), b'old')
            os.chmod(d, 0o755)
            with self.assertRaises(ValueError): self.tool.preserve(d, self.capsule)

    def make_bundle(self, root):
        import frozen_evidence_bundle as bundle
        from test_frozen_evidence_bundle import GATE, REPO, TX
        approval = json.loads((REPO / 'docs/phase8/response-export-client-substitution-maintainer-20261004-314e2f3b.json').read_text())
        sources = Path(root) / 'sources'; sources.mkdir(mode=0o700)
        inputs = {}
        for role, data in {'response': self.response, 'image': self.color,
                           'stdout': b'synthetic', 'stderr': b'',
                           'kernel_before': b'synthetic before',
                           'kernel_after': b'synthetic before after'}.items():
            p = sources / role; p.write_bytes(data); inputs[role] = p
        path = bundle.archive(root, TX, GATE, approval, inputs,
                              producer_identity=approval['approved_successor']['binary'])
        return path, GATE, approval

    def test_capsule_binding_verifies_actual_archive_and_preserves_originals(self):
        with tempfile.TemporaryDirectory() as d:
            os.chmod(d, 0o700)
            path, gate, approval = self.make_bundle(d)
            original = (path / 'color.original.bin').read_bytes()
            self.tool.bind_archive(path, gate, approval, self.capsule)
            result = self.tool.verify_binding(path, gate, approval)
            self.assertEqual(result['agreement'], 'CONFIRMED')
            self.assertEqual(result['hardware_attribution'], 'UNKNOWN')
            self.assertEqual((path / 'color.original.bin').read_bytes(), original)
            with self.assertRaises(FileExistsError): self.tool.bind_archive(path, gate, approval, self.capsule)

    def test_capsule_binding_detects_tampered_original_or_binding(self):
        for name in ('capsule.original.bin', 'response.original.bin', 'capsule.binding.json'):
            with tempfile.TemporaryDirectory() as d:
                os.chmod(d, 0o700)
                path, gate, approval = self.make_bundle(d)
                self.tool.bind_archive(path, gate, approval, self.capsule)
                p = path / name; os.chmod(p, 0o600)
                raw = bytearray(p.read_bytes()); raw[-1] ^= 1; p.write_bytes(raw)
                with self.assertRaises(ValueError): self.tool.verify_binding(path, gate, approval)

    def test_independent_verifier_rejects_other_approved_client(self):
        import frozen_evidence_bundle as bundle
        from test_frozen_evidence_bundle import GATE, APPROVAL, TX
        with tempfile.TemporaryDirectory() as d:
            os.chmod(d, 0o700)
            inputs = {}
            for role, data in {'response': self.response, 'image': self.color}.items():
                p = Path(d) / role; p.write_bytes(data); inputs[role] = p
            path = bundle.archive(d, TX, GATE, APPROVAL, inputs,
                                  producer_identity=APPROVAL['approved_successor']['binary'])
            verified = bundle.verify_archive(path, GATE, APPROVAL)
            self.tool.preserve(path, self.capsule)
            binding = {'scope': 'OFFLINE COMMON SAVED-BYTE BINDING; LIVE ORIGIN SEPARATE',
                       'context': verified['context'],
                       'archive_manifest_sha256': hashlib.sha256((path/'manifest.json').read_bytes()).hexdigest(),
                       'capsule_sha256': hashlib.sha256(self.capsule).hexdigest(),
                       'agreement': self.tool.check(self.capsule, self.response, self.color)}
            encoded = bundle.json_bytes(binding)
            (path/'capsule.binding.json').write_bytes(encoded)
            (path/'capsule.binding.sealed.json').write_bytes(bundle.json_bytes({
                'binding_sha256': hashlib.sha256(encoded).hexdigest(),
                'scope': 'SAVED-BYTE INTEGRITY ONLY'}))
            with self.assertRaisesRegex(ValueError, 'exact approved response client'):
                self.tool.verify_binding(path, GATE, APPROVAL)


if __name__ == '__main__': unittest.main()
