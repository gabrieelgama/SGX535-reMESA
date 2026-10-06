"""Offline preservation of ALREADY CAPTURED artifacts and supplied guard checks.

No transport, device access, ioctl, client execution, staging, boot or recovery.
This is not a replacement for the unavailable reviewed invocation wrapper.
archive(root, transaction_uuid, gate, approval, inputs) requires a caller-owned
private existing root and a role->regular-file mapping. Optional producer_identity
must equal the supplied approved binary pin before any archive is created. This
checks a declaration, not live process provenance; absent declaration is UNKNOWN. Destinations are unique
and never reused, even following failure. Original files and their directory are
synced before any derived image. A sealed receipt binds the finished manifest.
Sync is a checked filesystem operation, not a proof of survival after power loss.

Metadata binds supplied bytes; it does not authenticate their hardware origin.
No report from this component grants execution permission or establishes a
triangle. check_guard_snapshot() checks supplied values, not live freshness.
"""
import copy
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import struct
import uuid

from frozen_first_load_procedure import bound_identity
from frozen_first_owner_session import CMDLINE
from frozen_triangle_readback import validate_readback


FILES = {'response': 'response.original.bin', 'image': 'color.original.bin',
         'stdout': 'client.stdout.original.txt', 'stderr': 'client.stderr.original.txt',
         'kernel_before': 'kernel.before.original.txt', 'kernel_after': 'kernel.after.original.txt'}
MAX_INPUT_BYTES = 8 * 1024 * 1024


def require(condition, reason):
    if not condition:
        raise ValueError(reason)


def bind_context(gate, approval):
    """Validate record consistency; do not reissue either maintainer decision."""
    require(gate['gate_b'] == 'PASS FOR EXACT OFFSCREEN READINESS', 'readiness record mismatch')
    require(approval['decision'] == 'APPROVE_EXACT_CLIENT_SUBSTITUTION', 'substitution not approved')
    require(gate['sgx_execution_authorized'] is False and approval['sgx_execution_authorized'] is False,
            'this offline component expects execution-unauthorized records')
    identity = gate['identity_bindings']
    for key in ('boot_id', 'module_build_id', 'action'):
        require(identity[key] == approval['exact_context'][key], 'approval context drift: ' + key)
    require(gate['whitelist'] == approval['whitelist'] == [identity['action']], 'whitelist drift')
    successor = approval['approved_successor']
    for key in ('source', 'binary', 'uapi'):
        require(successor[key] == approval['approved_forward_client_binding'][key], 'mixed successor: ' + key)
    for key in ('source', 'binary'):
        require(identity['selected_client'][key] == approval['historical_selected_client_to_preserve'][key],
                'historical client drift: ' + key)
    for key in ('path', 'size', 'sha256'):
        require(identity['uapi'][key] == successor['uapi'][key], 'UAPI drift')
    for key in ('module', 'diagnostic_image'):
        require(identity[key] == approval['unchanged_context_bindings'][key], 'artifact context drift')
    return {key: identity[key] for key in ('boot_id', 'module_build_id', 'action')} | {
        'module_sha256': identity['module']['sha256'],
        'image_sha256': identity['diagnostic_image']['sha256'],
        'client': copy.deepcopy(successor['binary']),
        'client_source': copy.deepcopy(successor['source']), 'uapi': copy.deepcopy(successor['uapi'])}


def read_regular(path, *, dir_fd=None):
    """Pin before reading; reject devices, links and FIFOs without data-open."""
    handle = os.open(path, os.O_PATH | os.O_NOFOLLOW | os.O_CLOEXEC, dir_fd=dir_fd)
    try:
        before = os.fstat(handle)
        require(stat.S_ISREG(before.st_mode) and before.st_nlink == 1, 'single-link regular source required')
        require(before.st_size <= MAX_INPUT_BYTES, 'source exceeds bounded offline input size')
        with open('/proc/self/fd/' + str(handle), 'rb') as stream:
            data = stream.read(MAX_INPUT_BYTES + 1)
        after = os.fstat(handle)
        signature = lambda s: (s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns, s.st_ctime_ns, s.st_nlink)
        require(signature(before) == signature(after) and len(data) == before.st_size,
                'source changed or read was incomplete')
        return data
    finally:
        os.close(handle)


def open_private_directory(path):
    """Anchor every directory component; never follow a parent symlink."""
    parts = Path(os.path.abspath(path)).parts
    fd = os.open('/', os.O_RDONLY | os.O_DIRECTORY | os.O_CLOEXEC)
    try:
        for part in parts[1:]:
            child = os.open(part, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC,
                            dir_fd=fd)
            os.close(fd); fd = child
        info = os.fstat(fd)
        require(info.st_uid == os.geteuid() and stat.S_IMODE(info.st_mode) == 0o700,
                'archive root must be caller-owned mode 0700')
        return fd
    except BaseException:
        os.close(fd)
        raise


def write_exclusive(directory, name, data, *, mode=0o444):
    """Create once, complete file writes, sync and check close; retain failures."""
    fd = os.open(name, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW | os.O_CLOEXEC,
                 0o600, dir_fd=directory)
    try:
        offset = 0
        while offset < len(data):
            try:
                count = os.write(fd, data[offset:])
            except InterruptedError:
                continue                 # File I/O only; no SGX call exists here.
            if count <= 0:
                raise OSError('zero-progress evidence write')
            offset += count
        os.fchmod(fd, mode)
        os.fsync(fd)
    finally:
        os.close(fd)                     # Never retry close or delete partial bytes.


def json_bytes(value):
    return (json.dumps(value, sort_keys=True, indent=2) + '\n').encode()


def archive(root, transaction, gate, approval, inputs, *, producer_identity=None):
    context = bind_context(gate, approval)
    # Supplied identity equality is not authentication of the producing process.
    if producer_identity is not None:
        require(producer_identity == context['client'], 'producer identity does not match approved client')
    producer = copy.deepcopy(producer_identity)
    require(str(uuid.UUID(transaction)) == transaction, 'canonical transaction UUID required')
    require(isinstance(inputs, dict) and set(inputs) <= set(FILES), 'unknown artifact role')
    name = 'frozen-' + context['boot_id'] + '-' + context['module_build_id'][:8] + '-' + transaction
    parent = open_private_directory(root)
    directory = None
    data = {}; artifacts = {}
    try:
        os.mkdir(name, 0o700, dir_fd=parent)    # Collision, including partial attempt: STOP.
        os.fsync(parent)
        directory = os.open(name, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW | os.O_CLOEXEC,
                            dir_fd=parent)
        for role in FILES:
            if role not in inputs:
                continue
            raw = read_regular(inputs[role])
            write_exclusive(directory, FILES[role], raw)
            data[role] = raw
            artifacts[role] = {'path': FILES[role], 'size_bytes': len(raw),
                               'sha256': hashlib.sha256(raw).hexdigest()}
        os.fsync(directory)                  # All originals precede derived evidence.
        response = data.get('response', b'')
        complete = len(response) == 4268
        consistent = complete and len(data.get('image', b'')) == 4096 and response[172:] == data['image']
        fields = struct.unpack('<4Ii6I', response[:44]) if complete else None
        tuple_ok = fields is not None and fields[:4] == (1, 1, 0, 0) and fields[4:9] == (0, 2, 9, 7, 1)
        kernel_continuity = bool(data.get('kernel_before')) and data.get('kernel_after', b'').startswith(data['kernel_before'])
        analysis = None
        if consistent:
            write_exclusive(directory, 'color.validation-copy.bin', data['image'])
            analysis = validate_readback(read_regular('color.validation-copy.bin', dir_fd=directory))
            write_exclusive(directory, 'image-analysis.json', json_bytes(analysis))
        manifest = {
            'scope': 'OFFLINE ALREADY-CAPTURED ARTIFACT ARCHIVE ONLY', 'transaction_id': transaction,
            'context': context, 'artifacts': artifacts,
            'declared_producer_identity': producer,
            'producer_identity_consistency': 'CONFIRMED' if producer is not None else 'UNKNOWN', 'missing_artifacts': sorted(set(FILES) - set(inputs)),
            'consistency': {'response_complete': complete, 'necessary_service_tuple': bool(tuple_ok),
                            'image_equals_response': consistent, 'kernel_prefix_continuity': bool(kernel_continuity)},
            'image_analysis': analysis,
            'byte_preservation': 'CONFIRMED BY CHECKED WRITES/SYNC; NOT A POWER-LOSS SURVIVAL CLAIM',
            'context_provenance': 'INFERRED FROM SUPPLIED DECISION RECORDS; LIVE ATTRIBUTION UNKNOWN',
            'completion_attribution': 'UNKNOWN', 'triangle_established': False,
            'invocation_identity_provenance': 'UNKNOWN; transaction UUID binds this archive only',
            'sgx_execution_authorized': False,
            'required_action': 'STOP / NO RETRY IF IOCTL MAY HAVE OCCURRED',
            'limits': 'Digests, matching context, prefix continuity, service tuple and image match do not authenticate hardware execution. Missing/partial/unauthenticated evidence remains UNKNOWN.',
        }
        encoded = json_bytes(manifest)
        write_exclusive(directory, 'manifest.json', encoded)
        os.fsync(directory)
        write_exclusive(directory, 'sealed.json', json_bytes({'manifest_sha256': hashlib.sha256(encoded).hexdigest(),
                                                             'scope': 'ARCHIVE COMPLETENESS ONLY'}))
        os.fsync(directory)
        return Path(root) / name
    except BaseException as error:
        if directory is not None:
            try:
                write_exclusive(directory, 'incomplete.json', json_bytes({
                    'scope': 'PARTIAL OFFLINE ARCHIVE; RETAIN ALL BYTES', 'error': str(error),
                    'context': context, 'transaction_id': transaction, 'preserved_artifacts': artifacts,
                    'completion_attribution': 'UNKNOWN', 'triangle_established': False,
                    'required_action': 'STOP / NO RETRY IF IOCTL MAY HAVE OCCURRED'}))
                os.fsync(directory)
            except OSError:
                pass                         # Storage failure cannot promise a receipt.
        raise
    finally:
        if directory is not None:
            os.close(directory)
        os.close(parent)


def verify_archive(path, gate, approval):
    """Independently reread sealed files; hashes certify integrity, not origin."""
    context = bind_context(gate, approval)
    directory = open_private_directory(path)
    try:
        seal = json.loads(read_regular('sealed.json', dir_fd=directory))
        raw_manifest = read_regular('manifest.json', dir_fd=directory)
        require(seal.get('manifest_sha256') == hashlib.sha256(raw_manifest).hexdigest(), 'manifest seal mismatch')
        manifest = json.loads(raw_manifest)
        require(manifest.get('context') == context, 'archive context mismatch')
        producer = manifest.get('declared_producer_identity')
        require(producer is None or producer == context['client'], 'producer identity drift')
        require(manifest.get('producer_identity_consistency', 'UNKNOWN') ==
                ('CONFIRMED' if producer is not None else 'UNKNOWN'), 'producer consistency drift')
        require(manifest.get('triangle_established') is False and manifest.get('sgx_execution_authorized') is False,
                'archive cannot certify execution')
        artifacts = manifest.get('artifacts')
        require(isinstance(artifacts, dict) and set(artifacts) <= set(FILES), 'artifact inventory')
        require(manifest.get('missing_artifacts') == sorted(set(FILES) - set(artifacts)), 'missing inventory mismatch')
        for role, entry in artifacts.items():
            require(isinstance(entry, dict) and entry.get('path') == FILES[role], 'artifact path drift')
            raw = read_regular(FILES[role], dir_fd=directory)
            require(type(entry.get('size_bytes')) is int and entry['size_bytes'] == len(raw) and
                    entry.get('sha256') == hashlib.sha256(raw).hexdigest(), 'artifact bytes drift: ' + role)
        if manifest.get('image_analysis') is not None:
            original = read_regular('color.original.bin', dir_fd=directory)
            analysis_copy = read_regular('color.validation-copy.bin', dir_fd=directory)
            require(original == analysis_copy, 'validation copy drift')
            report = json.loads(read_regular('image-analysis.json', dir_fd=directory))
            require(report == manifest['image_analysis'] == validate_readback(analysis_copy), 'analysis drift')
        return {'archive_integrity': 'CONFIRMED', 'context': context,
                'completion_attribution': 'UNKNOWN', 'triangle_established': False,
                'sgx_execution_authorized': False}
    finally:
        os.close(directory)


def check_guard_snapshot(gate, approval, root, witness):
    """Apply existing identity/ownership guards to supplied values, never live."""
    context = bind_context(gate, approval)
    errors = []
    try:
        require(all(v == context['boot_id'] for v in (root.get('boot_id'), witness.get('boot_start'), witness.get('boot_end'))),
                'boot continuity mismatch')
        for key in ('invocation_count', 'hot_module_actions'):
            require(type(witness.get(key)) is int and witness[key] == 0, 'history: ' + key)
        for key in ('history_complete', 'operator_available', 'recovery_ready'):
            require(witness.get(key) is True, 'missing observation: ' + key)
        require(type(root.get('euid')) is int and root['euid'] == 0, 'privilege')
        require(root.get('pci_driver') == '/sys/bus/pci/drivers/gma500' and
                root.get('driver_module') == '/sys/module/gma500_gfx', 'PCI/module ownership')
        pci = root.get('pci', {})
        require(all(pci.get(k) == v for k, v in {'vendor': '0x8086', 'device': '0x8108',
                    'subsystem_vendor': '0x1028', 'subsystem_device': '0x02b1', 'irq': '16'}.items()), 'PCI identity')
        node = root.get('drm_node', {})
        require(type(node.get('mode')) is int and stat.S_ISCHR(node['mode']) and
                type(node.get('major')) is int and node['major'] == 226 and
                type(node.get('minor')) is int and node['minor'] == 0, 'DRM node metadata')
        for key, expected in (('vtcon0', 0), ('vtcon1', 1)):
            require(type(root.get(key)) is int and root[key] == expected, 'VT ownership')
        require(root.get('framebuffer_dimensions') == '1280,800', 'framebuffer dimensions')
        require(isinstance(root.get('slimski_status'), str) and root['slimski_status'].startswith('run: '), 'display service')
        require(isinstance(root.get('xorg_processes'), str) and
                re.search(r'^\d+ /\S*/Xorg(?:\s|$)', root['xorg_processes'], re.M), 'Xorg service')
        normalized = dict(root, pci_driver='gma500', driver_module='gma500_gfx', pci_irq=16,
                          cmdline_exact=root.get('cmdline') == CMDLINE, evidence_complete=True,
                          new_kernel_fault=False, sgx_actions=0, hot_module_actions=0)
        bound_identity({}, normalized, gate['identity_bindings']['module']['loaded_note_sha256'])
    except (ValueError, TypeError, KeyError) as error:
        errors.append(str(error))
    return {'scope': 'OFFLINE SUPPLIED GUARD SNAPSHOT CONSISTENCY ONLY', 'context': context,
            'snapshot_consistent': not errors, 'errors': errors,
            'supplied_record_consistency': 'CONFIRMED' if not errors else 'UNKNOWN',
            'live_freshness': 'UNKNOWN', 'unused_one_shot_provenance': 'UNKNOWN',
            'outstanding': ['reviewed_capture_provenance', 'immediate_continuity', 'protected_destinations',
                            'available_preservation_executor', 'separate_execution_authorization'],
            'ready_for_execution_authorization': False, 'sgx_execution_authorized': False,
            'triangle_established': False}
