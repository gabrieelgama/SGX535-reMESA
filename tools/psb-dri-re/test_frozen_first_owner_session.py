"""Synthetic supplied-record tests; never hardware/first-owner evidence."""
import importlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

HERE = Path(__file__).resolve().parent


def fixture():
    boot = '22222222-2222-4222-8222-222222222222'
    target = '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-314e2f3b37195dc56df7c57dd938d78377a5ea8e'
    records = [
        dict(phase='unprivileged-boot-identity', mode='experimental',
             boot_id=boot, kernel='5.10.240-antix.1-486-smp',
             architecture='i686', uptime_start=250),
        dict(boot_id=boot, kernel='5.10.240-antix.1-486-smp', architecture='i686',
             guards=[dict(guard='synthetic test guard', **{'pass': True})],
             euid=0, machine='Inspiron 1210', module_state='live',
             loaded_note_sha256='edb1e3e59cfd70c48d400bec89f2a30c1a436b05bf115bc1a05581abc153bdc9',
             pci_driver='/sys/bus/pci/drivers/gma500', driver_module='/sys/module/gma500_gfx', drm_bdf='0000:00:02.0',
             pci=dict(vendor='0x8086', device='0x8108', subsystem_vendor='0x1028',
                      subsystem_device='0x02b1', irq='16'),
             framebuffer='gma500drmfb', framebuffer_dimensions='1280,800',
             vtcon0=0, vtcon1=1, taint=12289,
             interrupts=' 16: 0 0 IO-APIC gma500\n',
             cmdline='BOOT_IMAGE=/boot/vmlinuz-5.10.240-antix.1-486-smp root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0',
             kernel_log='[ 0.000000] Linux version 5.10.240-antix.1-486-smp\n',
             hook_log='SGX535-FIRSTLOAD BEGIN\nSGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE\nSGX535-FIRSTLOAD PASS: derivative first owner; boot may continue',
             slimski_status='run: /etc/runit/runsvdir/default/slimski: (pid 42) 20s\n',
             xorg_processes='43 /usr/lib/xorg/Xorg -nolisten tcp\n',
             grub_env='saved_entry=gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1\n',
             destinations={target: dict(path=target, size=50805231,
                 sha256='fe64b3dcfe74b78a6d96edcd4fd7c118c3901fcff8631afec6c14647da292e3c',
                 type='regular', symlink=False, uid=0, gid=0, mode=0o644, nlink=1)}),
        dict(phase='passive-capture-end', mode='experimental', boot_id=boot,
             uptime_end=252, duration_seconds=2, same_boot_timing_pass=True)]
    witness = dict(expected_boot_id=boot, prior_boot_id='11111111-1111-4111-8111-111111111111',
                   capture_status=0, capture_stderr='', host_duration_seconds=3,
                   selection_photo=True, stock_entry_visible_before_selection=True,
                   experimental_selected_once=True, userspace_reached=True,
                   display_normal=True, local_sudo_succeeded=True,
                   userspace_seen_before_boot_watch=True, operator_recovery_confirmed=True,
                   experimental_boots=1, sgx_actions=0, hot_module_actions=0)
    return records, witness


class SessionTests(unittest.TestCase):
    def setUp(self):
        self.assertIsNotNone(importlib.util.find_spec('frozen_first_owner_session'),
                             'candidate-specific supplied-record checker missing')
        self.tool = importlib.import_module('frozen_first_owner_session')

    def test_complete_synthetic_records_grant_no_execution_or_gpu_claim(self):
        records, witness = fixture()
        report = self.tool.validate(records, witness)
        self.assertEqual(report['classification'], 'CONSISTENT SUPPLIED FIRST-OWNER RECORDS')
        self.assertFalse(report['boot_authorized'])
        self.assertFalse(report['sgx_authorized'])
        self.assertFalse(report['triangle_established'])
        self.assertEqual(report['live_provenance'], 'INDEPENDENT VERIFICATION REQUIRED')

    def test_native_schema_rejects_aliases_and_old_destination(self):
        for key, value in [('pci_driver', 'gma500'), ('driver_module', 'gma500_gfx'),
                           ('vtcon0', False), ('vtcon1', True), ('vtcon1', '1'),
                           ('framebuffer_dimensions', '1280x800')]:
            records, witness = fixture(); records[1][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)
        records, witness = fixture()
        row = next(iter(records[1]['destinations'].values()))
        records[1]['destinations'] = {'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-diagnostic-01': row}
        with self.assertRaises(ValueError): self.tool.validate(records, witness)

    def test_old_note_is_rejected_even_when_all_capture_guards_pass(self):
        records, witness = fixture()
        records[1]['loaded_note_sha256'] = 'a74fb4f5b624980dfb717cb4c5c6fc400da4a9aa0dde264cd2365b5c7c3e4f8e'
        with self.assertRaisesRegex(ValueError, 'loaded_note'):
            self.tool.validate(records, witness)

    def test_incomplete_or_malformed_capture_command_receipt_rejects(self):
        cases = [('capture_status', None), ('capture_status', False),
                 ('capture_status', 0.0), ('capture_stderr', None),
                 ('capture_stderr', False), ('capture_stderr', 0),
                 ('capture_stderr', [])]
        for key, value in cases:
            records, witness = fixture(); witness[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.tool.validate(records, witness)
        for key in ['capture_status', 'capture_stderr']:
            records, witness = fixture(); del witness[key]
            with self.subTest(missing=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)

    def test_file_and_privilege_metadata_types_reject_boolean_aliases(self):
        records, witness = fixture(); records[1]['euid'] = False
        with self.assertRaises(ValueError): self.tool.validate(records, witness)
        for key, value in [('uid', False), ('gid', False), ('nlink', True),
                           ('symlink', 0), ('mode', 420.0)]:
            records, witness = fixture()
            next(iter(records[1]['destinations'].values()))[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)

    def test_old_staged_image_is_rejected_even_with_new_loaded_note(self):
        records, witness = fixture()
        row = next(iter(records[1]['destinations'].values()))
        row['sha256'] = '376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae'
        with self.assertRaisesRegex(ValueError, 'image'):
            self.tool.validate(records, witness)

    def test_identity_ownership_health_and_trace_drift_reject(self):
        cases = [('module_state', 'coming'), ('driver_module', 'other'),
                 ('interrupts', '17: 0 IO-APIC gma500\n'), ('taint', 12305),
                 ('kernel_log', '[1] BUG: synthetic fault\n'),
                 ('hook_log', ''), ('vtcon1', '0'), ('framebuffer_dimensions', '800x600'),
                 ('slimski_status', 'down: slimski'), ('xorg_processes', ''),
                 ('cmdline', 'quiet'), ('grub_env', 'saved_entry=experimental\n')]
        for key, value in cases:
            with self.subTest(key=key):
                records, witness = fixture(); records[1][key] = value
                with self.assertRaises(ValueError): self.tool.validate(records, witness)

    def test_duplicate_trace_and_changed_pci_reject(self):
        for duplicate in [True, False]:
            records, witness = fixture()
            if duplicate: records[1]['hook_log'] += '\nSGX535-FIRSTLOAD BEGIN'
            else: records[1]['pci']['device'] = '0x1234'
            with self.assertRaises(ValueError): self.tool.validate(records, witness)

    def test_missing_physical_observation_never_becomes_true(self):
        for key in ['selection_photo', 'display_normal', 'local_sudo_succeeded',
                    'userspace_seen_before_boot_watch', 'operator_recovery_confirmed']:
            records, witness = fixture(); del witness[key]
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)

    def test_boot_drift_reuse_failed_capture_and_window_reject(self):
        for key, value in [('expected_boot_id', 'other'), ('prior_boot_id', fixture()[0][0]['boot_id']),
                           ('capture_status', 1), ('host_duration_seconds', 41)]:
            records, witness = fixture(); witness[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)
        records, witness = fixture(); records[2]['uptime_end'] = 1201
        with self.assertRaises(ValueError): self.tool.validate(records, witness)

    def test_same_boot_uuid_alias_cannot_be_a_prior_boot(self):
        for prior in ['22222222222242228222222222222222',
                      'urn:uuid:22222222-2222-4222-8222-222222222222']:
            records,witness=fixture();witness['prior_boot_id']=prior
            with self.subTest(prior=prior),self.assertRaises(ValueError):
                self.tool.validate(records,witness)

    def test_sgx_hot_actions_second_boot_and_boolean_counts_reject(self):
        for key, value in [('sgx_actions', 1), ('hot_module_actions', 1),
                           ('experimental_boots', 2), ('experimental_boots', True),
                           ('sgx_actions', False)]:
            records, witness = fixture(); witness[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)

    def test_partial_records_and_unsafe_image_inode_reject(self):
        records, witness = fixture()
        with self.assertRaises(ValueError): self.tool.validate(records[:-1], witness)
        for key, value in [('symlink', True), ('nlink', 2), ('uid', 1000), ('size', 4096)]:
            records, witness = fixture(); next(iter(records[1]['destinations'].values()))[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.tool.validate(records, witness)

    def test_historical_capture_is_rejected_unchanged(self):
        path = HERE.parents[1] / 'docs/hardware-evidence/MINI12-20261002T063600Z-DIAGNOSTIC-FIRSTOWNER-02/passive/decoded-records.json'
        records = json.loads(path.read_text()); _, witness = fixture()
        witness['expected_boot_id'] = records[0]['boot_id']
        with self.assertRaises(ValueError): self.tool.validate(records, witness)

    def test_default_cli_is_preparation_only(self):
        run = subprocess.run([sys.executable, '-B', str(HERE/'frozen_first_owner_session.py')],
                             capture_output=True, text=True)
        self.assertEqual(run.returncode, 0, run.stderr)
        report = json.loads(run.stdout)
        self.assertEqual(report['mode'], 'OFFLINE PREPARATION ONLY')
        self.assertFalse(report['sgx_authorized'])
        self.assertIn('unique_image_target', report)

    def test_cli_preserves_inputs_and_rejects_fifo(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); records, witness = fixture()
            rp, wp = root/'records.json', root/'witness.json'
            rp.write_text(json.dumps(records)); wp.write_text(json.dumps(witness))
            original = (rp.read_bytes(), wp.read_bytes())
            args = [sys.executable, '-B', str(HERE/'frozen_first_owner_session.py'),
                    '--records', str(rp), '--witness', str(wp)]
            run = subprocess.run(args, capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual((rp.read_bytes(), wp.read_bytes()), original)
            fifo = root/'fifo'; os.mkfifo(fifo); args[-1] = str(fifo)
            run = subprocess.run(args, capture_output=True, text=True, timeout=2)
            self.assertEqual(run.returncode, 2, run.stderr)
            self.assertFalse(json.loads(run.stdout)['sgx_authorized'])


if __name__ == '__main__': unittest.main()
