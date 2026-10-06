"""Offline synthetic archive/guard tests; no transport, client execution or ioctl."""
import copy
import hashlib
import importlib
import importlib.util
import json
import os
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
GATE = json.loads((REPO / 'docs/phase8/gate-b-maintainer-readiness-20261004-314e2f3b.json').read_text())
APPROVAL = json.loads((REPO / 'docs/phase8/prospective-client-substitution-maintainer-20261004-314e2f3b.json').read_text())
TX = '11111111-1111-4111-8111-111111111111'


def synthetic_image():
    # Hand-derived strict-center reference; this fixture never proves rendering.
    return b''.join(struct.pack('<I', 0xffffffff if x >= 8 and y >= 8 and x + y <= 30 else 0)
                    for y in range(32) for x in range(32))


class BundleTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(importlib.util.find_spec('frozen_evidence_bundle'),
                             'offline original-first evidence component missing')
        self.tool = importlib.import_module('frozen_evidence_bundle')
        self.tmp = tempfile.TemporaryDirectory(prefix='sgx535-offline-bundle-')
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.sources = self.root / 'sources'; self.sources.mkdir(mode=0o700)
        self.destination = self.root / 'archives'; self.destination.mkdir(mode=0o700)
        self.image = synthetic_image()
        header = struct.pack('<4Ii6I', 1, 1, 0, 0, 0, 2, 9, 7, 1, 0, 0)
        self.raw = header + bytes(128) + self.image
        self.inputs = {}
        for role, data in {'response': self.raw, 'image': self.image,
                           'stdout': b'synthetic client stdout\n', 'stderr': b'',
                           'kernel_before': b'kernel before\n',
                           'kernel_after': b'kernel before\nsynthetic ledger events=7\n'}.items():
            path = self.sources / role; path.write_bytes(data); self.inputs[role] = path

    def archive(self, **kwargs):
        return self.tool.archive(self.destination, TX, GATE, APPROVAL, self.inputs, **kwargs)

    def test_perfect_synthetic_bundle_preserves_bytes_without_hardware_claim(self):
        path = self.archive()
        manifest = json.loads((path / 'manifest.json').read_text())
        self.assertEqual((path / 'response.original.bin').read_bytes(), self.raw)
        self.assertEqual((path / 'color.original.bin').read_bytes(), self.image)
        self.assertEqual((path / 'color.validation-copy.bin').read_bytes(), self.image)
        self.assertEqual(manifest['artifacts']['response']['sha256'], hashlib.sha256(self.raw).hexdigest())
        self.assertTrue(manifest['consistency']['necessary_service_tuple'])
        self.assertTrue(manifest['consistency']['image_equals_response'])
        self.assertTrue(manifest['image_analysis']['image_matches_reference'])
        self.assertFalse(manifest['triangle_established'])
        self.assertFalse(manifest['sgx_execution_authorized'])
        self.assertEqual(manifest['completion_attribution'], 'UNKNOWN')
        for source in self.inputs.values():
            self.assertTrue(source.exists())

    def test_declared_producer_must_match_approved_client_before_archive_creation(self):
        producer = copy.deepcopy(APPROVAL['approved_successor']['binary'])
        producer['sha256'] = '0' * 64
        with self.assertRaisesRegex(ValueError, 'producer identity'):
            self.archive(producer_identity=producer)
        self.assertEqual(list(self.destination.iterdir()), [])

    def test_matching_declared_producer_is_bound_without_live_attribution(self):
        producer = copy.deepcopy(APPROVAL['approved_successor']['binary'])
        path = self.archive(producer_identity=producer)
        manifest = json.loads((path/'manifest.json').read_text())
        self.assertEqual(manifest['declared_producer_identity'],producer)
        self.assertEqual(manifest['producer_identity_consistency'],'CONFIRMED')
        producer['sha256'] = '0' * 64
        self.assertNotEqual(manifest['declared_producer_identity'],producer)
        verified = self.tool.verify_archive(path,GATE,APPROVAL)
        self.assertEqual(verified['completion_attribution'],'UNKNOWN')
        self.assertFalse(verified['triangle_established'])

    def test_legacy_inputs_without_producer_identity_remain_explicitly_unknown(self):
        manifest = json.loads((self.archive()/'manifest.json').read_text())
        self.assertIsNone(manifest['declared_producer_identity'])
        self.assertEqual(manifest['producer_identity_consistency'],'UNKNOWN')

    def test_duplicate_transaction_never_overwrites_or_retries(self):
        first = self.archive()
        old = (first / 'manifest.json').read_bytes()
        with self.assertRaises(FileExistsError): self.archive()
        self.assertEqual((first / 'manifest.json').read_bytes(), old)

    def test_partial_response_is_preserved_and_remains_unknown(self):
        self.inputs['response'].write_bytes(self.raw[:300])
        path = self.archive()
        m = json.loads((path / 'manifest.json').read_text())
        self.assertEqual((path / 'response.original.bin').read_bytes(), self.raw[:300])
        self.assertFalse(m['consistency']['response_complete'])
        self.assertEqual(m['required_action'], 'STOP / NO RETRY IF IOCTL MAY HAVE OCCURRED')
        self.assertFalse(m['triangle_established'])

    def test_mismatched_image_preserved_without_analysis_copy(self):
        self.inputs['image'].write_bytes(bytes(4096))
        path = self.archive()
        m = json.loads((path / 'manifest.json').read_text())
        self.assertFalse(m['consistency']['image_equals_response'])
        self.assertEqual((path / 'color.original.bin').read_bytes(), bytes(4096))
        self.assertFalse((path / 'color.validation-copy.bin').exists())

    def test_missing_response_does_not_synthesize_one_from_stdout(self):
        del self.inputs['response']
        path = self.archive()
        m = json.loads((path / 'manifest.json').read_text())
        self.assertIn('response', m['missing_artifacts'])
        self.assertFalse((path / 'response.original.bin').exists())
        self.assertEqual(m['completion_attribution'], 'UNKNOWN')

    def test_reordered_or_truncated_kernel_evidence_cannot_be_attributed(self):
        self.inputs['kernel_after'].write_bytes(b'synthetic ledger events=7\n')
        m = json.loads((self.archive() / 'manifest.json').read_text())
        self.assertFalse(m['consistency']['kernel_prefix_continuity'])
        self.assertEqual(m['completion_attribution'], 'UNKNOWN')

    def test_symlink_hardlink_fifo_sources_fail_without_blocking(self):
        original = self.inputs['image']
        for kind in ('symlink', 'hardlink', 'fifo'):
            with self.subTest(kind=kind):
                bad = self.sources / kind
                if kind == 'symlink': bad.symlink_to(original)
                elif kind == 'hardlink': os.link(original, bad)
                else: os.mkfifo(bad)
                inputs = dict(self.inputs, image=bad)
                tx = {'symlink': '22222222', 'hardlink': '33333333', 'fifo': '44444444'}[kind] + TX[8:]
                with self.assertRaises(ValueError):
                    self.tool.archive(self.destination, tx, GATE, APPROVAL, inputs)
                bad.unlink()

    def test_symlink_or_writable_archive_root_refused(self):
        alias = self.root / 'alias'; alias.symlink_to(self.destination)
        with self.assertRaises((OSError, ValueError)):
            self.tool.archive(alias, TX, GATE, APPROVAL, self.inputs)
        self.destination.chmod(0o777)
        with self.assertRaises(ValueError): self.archive()
        self.destination.chmod(0o700)

    def test_failed_copy_leaves_originals_and_no_complete_manifest(self):
        real_write = self.tool.write_exclusive
        def fail_copy(directory, name, data, **kwargs):
            if name == 'color.validation-copy.bin': raise OSError('synthetic copy failure')
            return real_write(directory, name, data, **kwargs)
        with patch.object(self.tool, 'write_exclusive', side_effect=fail_copy):
            with self.assertRaises(OSError): self.archive()
        directories = list(self.destination.iterdir()); self.assertEqual(len(directories), 1)
        self.assertEqual((directories[0] / 'color.original.bin').read_bytes(), self.image)
        self.assertEqual((directories[0] / 'response.original.bin').read_bytes(), self.raw)
        self.assertFalse((directories[0] / 'manifest.json').exists())
        self.assertTrue((directories[0] / 'incomplete.json').exists())

    def test_approval_or_context_mismatch_refused_before_archive_creation(self):
        bad = copy.deepcopy(APPROVAL); bad['exact_context']['boot_id'] = TX
        with self.assertRaises(ValueError):
            self.tool.archive(self.destination, TX, GATE, bad, self.inputs)
        self.assertEqual(list(self.destination.iterdir()), [])

    def test_independent_archive_verifier_detects_tampered_original_and_seal(self):
        path = self.archive()
        self.assertIsNotNone(getattr(self.tool, 'verify_archive', None), 'archive verifier missing')
        result = self.tool.verify_archive(path, GATE, APPROVAL)
        self.assertEqual(result['archive_integrity'], 'CONFIRMED')
        self.assertEqual(result['completion_attribution'], 'UNKNOWN')
        self.assertFalse(result['triangle_established'])
        original = path / 'color.original.bin'
        original.chmod(0o600); original.write_bytes(bytes(4096))
        with self.assertRaises(ValueError): self.tool.verify_archive(path, GATE, APPROVAL)
        original.write_bytes(self.image); original.chmod(0o444)
        seal = path / 'sealed.json'; seal.chmod(0o600)
        seal.write_text(json.dumps({'manifest_sha256': '0' * 64}))
        with self.assertRaises(ValueError): self.tool.verify_archive(path, GATE, APPROVAL)

    def test_write_failure_retains_partial_bytes_without_seal(self):
        actual = self.tool.os.write
        calls = []
        def fail_second(fd, data):
            calls.append(len(data))
            if len(calls) == 1: return actual(fd, data[:50])
            raise OSError('synthetic storage failure')
        with patch.object(self.tool.os, 'write', side_effect=fail_second):
            with self.assertRaises(OSError): self.archive()
        path, = self.destination.iterdir()
        self.assertEqual((path / 'response.original.bin').read_bytes(), self.raw[:50])
        self.assertFalse((path / 'sealed.json').exists())
        self.assertFalse((path / 'color.validation-copy.bin').exists())

    def test_original_sync_failure_retains_response_and_prevents_analysis(self):
        actual = self.tool.os.fsync
        def fail_original(fd):
            if os.readlink('/proc/self/fd/' + str(fd)).endswith('/response.original.bin'):
                raise OSError('synthetic original sync failure')
            return actual(fd)
        with patch.object(self.tool.os, 'fsync', side_effect=fail_original):
            with self.assertRaises(OSError): self.archive()
        path, = self.destination.iterdir()
        self.assertEqual((path / 'response.original.bin').read_bytes(), self.raw)
        self.assertFalse((path / 'color.validation-copy.bin').exists())
        self.assertFalse((path / 'sealed.json').exists())
        self.assertTrue((path / 'incomplete.json').exists())

    def test_original_directory_sync_precedes_copy_and_copy_is_validator_input(self):
        real_sync = self.tool.os.fsync
        seen_original_sync = []
        actual_validator = self.tool.validate_readback
        def sync(fd):
            result = real_sync(fd)
            paths = list(self.destination.iterdir())
            if paths and (paths[0] / 'color.original.bin').exists() and not (paths[0] / 'color.validation-copy.bin').exists():
                seen_original_sync.append(fd)
            return result
        def validate(data):
            path, = self.destination.iterdir()
            self.assertTrue(seen_original_sync)
            self.assertTrue((path / 'color.validation-copy.bin').exists())
            self.assertEqual(data, (path / 'color.validation-copy.bin').read_bytes())
            return actual_validator(data)
        with patch.object(self.tool.os, 'fsync', side_effect=sync), \
                patch.object(self.tool, 'validate_readback', side_effect=validate):
            self.archive()


class GuardTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(importlib.util.find_spec('frozen_evidence_bundle'),
                             'offline guard component missing')
        self.tool = importlib.import_module('frozen_evidence_bundle')
        from test_frozen_first_owner_session import fixture
        records, _ = fixture()
        self.root = records[1]
        self.root['boot_id'] = GATE['identity_bindings']['boot_id']
        self.root['drm_node'] = dict(mode=0o20660, major=226, minor=0)
        self.witness = dict(boot_start=self.root['boot_id'], boot_end=self.root['boot_id'],
                            invocation_count=0, history_complete=True,
                            hot_module_actions=0, operator_available=True, recovery_ready=True)

    def check(self):
        return self.tool.check_guard_snapshot(GATE, APPROVAL, self.root, self.witness)

    def test_matching_snapshot_is_not_live_fresh_readiness_or_permission(self):
        report = self.check()
        self.assertTrue(report['snapshot_consistent'])
        self.assertEqual(report['live_freshness'], 'UNKNOWN')
        self.assertEqual(report['unused_one_shot_provenance'], 'UNKNOWN')
        self.assertFalse(report['ready_for_execution_authorization'])
        self.assertIn('protected_destinations', report['outstanding'])
        self.assertFalse(report['sgx_execution_authorized'])

    def test_identity_ownership_health_and_bool_aliases_fail_closed(self):
        cases = [('module_state', 'loading'), ('loaded_note_sha256', '0' * 64),
                 ('pci_driver', '/sys/bus/pci/drivers/other'), ('euid', False),
                 ('taint', 2), ('vtcon1', True),
                 ('kernel_log', 'WARNING: synthetic fault\n'), ('slimski_status', 'down: slimski')]
        for key, value in cases:
            with self.subTest(key=key):
                root = dict(self.root, **{key: value})
                report = self.tool.check_guard_snapshot(GATE, APPROVAL, root, self.witness)
                self.assertFalse(report['snapshot_consistent'])
                self.assertFalse(report['ready_for_execution_authorization'])

    def test_boot_drift_consumed_or_incomplete_history_and_recovery_fail_closed(self):
        for key, value in [('boot_end', TX), ('invocation_count', 1), ('invocation_count', False),
                           ('history_complete', False), ('operator_available', False),
                           ('recovery_ready', False)]:
            with self.subTest(key=key):
                witness = dict(self.witness, **{key: value})
                self.assertFalse(self.tool.check_guard_snapshot(GATE, APPROVAL, self.root, witness)['snapshot_consistent'])

    def test_preserved_actual_capture_is_consistent_but_not_fresh(self):
        path = REPO / 'docs/hardware-evidence/MINI12-20261003T002627Z-FROZEN-FIRSTOWNER-314e2f3b-01/candidate-first-owner/decoded-records.json'
        root = json.loads(path.read_text())[1]
        report = self.tool.check_guard_snapshot(GATE, APPROVAL, root, self.witness)
        self.assertTrue(report['snapshot_consistent'])
        self.assertEqual(report['live_freshness'], 'UNKNOWN')
        self.assertFalse(report['ready_for_execution_authorization'])


if __name__ == '__main__':
    unittest.main()
