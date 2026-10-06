#!/usr/bin/env python3
"""Offline preparation and supplied-record checks for the frozen Phase 8 pair.

No transport, file staging, boot, SGX invocation, reset or recovery action.
Default: print preparation requirements. With --records and --witness, check
saved passive records and operator observations, without rewriting either.
Consistency is not live provenance, a staging approval or an opening decision.
"""
import argparse
import json
import os
from pathlib import Path
import re
import stat
import uuid

import frozen_first_load_capture_v2 as capture
import frozen_first_load_procedure as identity
from frozen_kernel_health import classify_kernel_log


WORK = '/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z'
BUILD_ID = '314e2f3b37195dc56df7c57dd938d78377a5ea8e'
MODULE_SHA = '4141a07c1f2d2ffc9ab2770b7b4cf654f68c396275dad15f0ddc68192529d8e6'
IMAGE_SHA = 'fe64b3dcfe74b78a6d96edcd4fd7c118c3901fcff8631afec6c14647da292e3c'
NOTE_SHA = 'edb1e3e59cfd70c48d400bec89f2a30c1a436b05bf115bc1a05581abc153bdc9'
# Unique candidate staging; the historical diagnostic-01 file is preserved.
PROPOSED_IMAGE = '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-' + BUILD_ID
CMDLINE = ('BOOT_IMAGE=/boot/vmlinuz-5.10.240-antix.1-486-smp '
           'root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0')
TRACE = ['SGX535-FIRSTLOAD BEGIN',
         'SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE',
         'SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue']
OBSERVATIONS = ['selection_photo', 'stock_entry_visible_before_selection',
                'experimental_selected_once', 'userspace_reached', 'display_normal',
                'local_sudo_succeeded', 'userspace_seen_before_boot_watch',
                'operator_recovery_confirmed']


def require(condition, reason):
    if not condition:
        raise ValueError(reason)


def preparation():
    return {
        'mode': 'OFFLINE PREPARATION ONLY',
        'module': {'path': WORK + '/module/gma500_gfx.ko', 'size': 244960,
                   'build_id': BUILD_ID, 'sha256': MODULE_SHA,
                   'loaded_note_sha256': NOTE_SHA},
        'image': {'path': WORK + '/image/build-01/initrd.img-sgx535-firstload-diagnostic-01',
                  'size': 50805231, 'sha256': IMAGE_SHA},
        'entry_proposal': WORK + '/image/build-01/proposed-custom.cfg',
        'unique_image_target': PROPOSED_IMAGE,
        'staging_tool': 'tools/psb-dri-re/frozen_candidate_stage.py',
        'operator_observations': OBSERVATIONS,
        'witness_fields': ['expected_boot_id', 'prior_boot_id', 'capture_status',
                          'capture_stderr', 'host_duration_seconds',
                          'experimental_boots', 'sgx_actions', 'hot_module_actions'],
        'bounds': {'boot_watch_seconds': capture.BOOT_WATCH_SECONDS,
                   'capture_max_kernel_boot_seconds': capture.CYCLE_CAPTURE_MAX_BOOT_SECONDS,
                   'connection_seconds': capture.CONNECTION_SECONDS,
                   'child_seconds': capture.CHILD_SECONDS},
        'client': {'path': 'docs/phase8/artifacts/frozen-triangle-one-shot-i386',
                   'sha256': '758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf',
                   'target_client_and_output_paths': 'NOT ESTABLISHED BY AVAILABLE PROCEDURE',
                   'requirements': 'protected exclusive/no-follow staging, destination '
                                   'readback, ownership/type/link checks, absent outputs'},
        'evidence': 'Preserve original capture stdout/stderr, command/timing receipts, '
                    'operator observations and staging receipts. After a permitted '
                    'invocation preserve the complete client/kernel tuple and original '
                    '4096-byte readback; analyze a copy.',
        'validator': 'python3 -B tools/psb-dri-re/frozen_triangle_readback.py <copy>',
        'stop': 'Failed guard, identity/ownership/health drift, missing attribution or '
                'possible ioctl issuance: STOP; no retry or broadened experiment.',
        'recovery': 'Reviewed operator-controlled machine boundary and STOCK selection '
                    'only; no automatic reset, unbind or hot module replacement.',
        'boot_authorized': False, 'sgx_authorized': False,
        'opening_decision': 'NOT EVALUATED',
    }


def _validate_runtime(records, witness):
    """Technical supplied-record consistency only; no menu/physical qualification."""
    require(isinstance(witness, dict), 'operator observation object required')
    require(type(witness.get('capture_status')) is int
            and witness['capture_status'] == 0, 'explicit integer capture status zero required')
    require(type(witness.get('capture_stderr')) is str
            and witness['capture_stderr'] == '', 'explicit empty capture stderr required')
    timing = capture.validate_receipt(records, witness.get('capture_status'),
                                     witness.get('capture_stderr'),
                                     witness.get('host_duration_seconds'), 'experimental')
    boot = timing['boot_id']
    require(witness.get('expected_boot_id') == boot, 'expected boot drift')
    prior = witness.get('prior_boot_id')
    require(isinstance(prior, str) and isinstance(boot, str)
            and str(uuid.UUID(prior)) == prior and str(uuid.UUID(boot)) == boot
            and prior != boot,
            'distinct prior/first-owner boot identities required')
    for key, expected in [('experimental_boots', 1), ('sgx_actions', 0),
                          ('hot_module_actions', 0)]:
        require(type(witness.get(key)) is int and witness[key] == expected,
                'history/bound: ' + key)
    root = records[1]
    require(type(root.get('euid')) is int and root['euid'] == 0,
            'privileged capture required')
    pci = root.get('pci')
    require(isinstance(pci, dict) and all(pci.get(k) == v for k, v in {
        'vendor': '0x8086', 'device': '0x8108', 'subsystem_vendor': '0x1028',
        'subsystem_device': '0x02b1', 'irq': '16'}.items()), 'PCI identity drift')
    log = root.get('kernel_log')
    require(isinstance(log, str) and bool(log), 'complete kernel log required')
    require(root.get('pci_driver') == '/sys/bus/pci/drivers/gma500'
            and root.get('driver_module') == '/sys/module/gma500_gfx',
            'PCI owner symlink target drift')
    normalized = dict(root)
    normalized.update(pci_driver='gma500', driver_module='gma500_gfx')
    # These structural flags describe only the supplied record, never its
    # authenticity. Explicit identity/timing checks precede the shared guard.
    normalized.update(pci_irq=16, cmdline_exact=root.get('cmdline') == CMDLINE,
                      evidence_complete=True,
                      new_kernel_fault=classify_kernel_log(log)['classification'] == 'REJECT',
                      sgx_actions=witness['sgx_actions'],
                      hot_module_actions=witness['hot_module_actions'])
    identity.bound_identity({}, normalized, NOTE_SHA)
    require(isinstance(root.get('hook_log'), str) and root['hook_log'].splitlines() == TRACE,
            'missing/duplicate/reordered hook trace')
    require(root.get('framebuffer_dimensions') == '1280,800'
            and type(root.get('vtcon0')) is int and root['vtcon0'] == 0
            and type(root.get('vtcon1')) is int and root['vtcon1'] == 1,
            'framebuffer/VT ownership drift')
    require(isinstance(root.get('slimski_status'), str)
            and root['slimski_status'].startswith('run: '), 'slimski not running')
    require(isinstance(root.get('xorg_processes'), str)
            and re.search(r'^\d+ /\S*/Xorg(?:\s|$)', root['xorg_processes'], re.M),
            'Xorg not recorded running')
    require(root.get('grub_env') == 'saved_entry=' + identity.STOCK_ID + '\n',
            'stock saved/default state drift')
    destinations = root.get('destinations')
    row = destinations.get(PROPOSED_IMAGE) if isinstance(destinations, dict) else None
    require(isinstance(row, dict), 'selected staged image receipt absent')
    require(all(type(row.get(k)) is type(v) and row[k] == v for k, v in {
        'path': PROPOSED_IMAGE, 'size': 50805231, 'sha256': IMAGE_SHA,
        'type': 'regular', 'symlink': False, 'uid': 0, 'gid': 0,
        'mode': 0o644, 'nlink': 1}.items()), 'selected image content/inode mismatch')
    return {'classification': 'CONSISTENT SUPPLIED RUNTIME RECORDS ONLY',
            'boot_id': boot, 'build_id': BUILD_ID,
            'live_provenance': 'INDEPENDENT VERIFICATION REQUIRED',
            'staging_procedure': 'NOT APPROVED BY THIS CHECK',
            'boot_authorized': False, 'sgx_authorized': False,
            'opening_decision': 'NOT EVALUATED', 'triangle_established': False}


def validate(records, witness):
    """Original photographic path; its observation requirements stay unchanged."""
    require(isinstance(witness, dict), 'operator observation object required')
    for key in OBSERVATIONS:
        require(witness.get(key) is True, 'missing/failed operator observation: ' + key)
    report = _validate_runtime(records, witness)
    report['classification'] = 'CONSISTENT SUPPLIED FIRST-OWNER RECORDS'
    return report


def read_json(path):
    handle = os.open(path, os.O_PATH | os.O_CLOEXEC)
    try:
        require(stat.S_ISREG(os.fstat(handle).st_mode), 'input must be a regular file')
        with open(f'/proc/self/fd/{handle}', 'rb') as stream:
            data = stream.read(16 * 1024 * 1024 + 1)
        require(len(data) <= 16 * 1024 * 1024, 'record exceeds 16 MiB')
        return json.loads(data)
    finally:
        os.close(handle)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--records', type=Path, help='saved three-record passive capture JSON')
    parser.add_argument('--witness', type=Path, help='actual operator observations/timing/history JSON')
    args = parser.parse_args(argv)
    try:
        require(bool(args.records) == bool(args.witness), 'records and witness required together')
        report = (validate(read_json(args.records), read_json(args.witness))
                  if args.records else preparation())
    except (OSError, ValueError, TypeError) as error:
        print(json.dumps({'classification': 'STOP', 'reason': str(error),
                          'boot_authorized': False, 'sgx_authorized': False,
                          'triangle_established': False, 'retry': False}, sort_keys=True))
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
