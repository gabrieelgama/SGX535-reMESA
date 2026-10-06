"""Read-only capsule materialization and OFFLINE saved-byte agreement.

No client launcher/ioctl exists here. Saved bytes alone do not authenticate a
kernel producer or a boot. Live channel/context guards remain separate.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import stat
import struct
from frozen_evidence_bundle import (open_private_directory, read_regular,
                                    write_exclusive, json_bytes, require,
                                    verify_archive)

SIZE = 4152
CHANNEL = '/proc/sgx535_current_operation'


def summary(color):
    require(len(color) == 4096, '4096 color bytes required')
    fnv = 2166136261
    for byte in color: fnv = ((fnv ^ byte) * 16777619) & 0xffffffff
    pixels = struct.unpack('<1024I', color)
    rows = [sum(bool(v) for v in pixels[y*32:(y+1)*32]) for y in range(32)]
    return fnv, sum(rows), rows


def check(capsule, response=None, image=None):
    require(len(capsule) == SIZE, 'capsule size mismatch')
    values = struct.unpack('<8s12I', capsule[:56])
    (magic, version, size, state, invalid, issued, terminal, result, phase,
     events, color_present, syscall_result, reserved) = values
    require(magic == b'SGXCAPB1' and version == 1 and size == SIZE and not reserved,
            'capsule format mismatch')
    reasons = []
    signed = lambda v: v if v < 0x80000000 else v - 0x100000000
    complete = (state == 4 and not invalid and issued == 1 and terminal == 1
                and result == 0 and phase == 9 and events == 7
                and color_present == 1 and signed(syscall_result) >= 0)
    if not complete: reasons.append('not a complete closed valid successful capsule')
    if response is None or len(response) != 4268:
        reasons.append('complete original successful response unavailable')
    else:
        fields = struct.unpack('<43I', response[:172])
        expected = (1, 1, 0, 0, 0, 2, phase, events, 1)
        if fields[:9] != expected: reasons.append('response terminal fields disagree')
        if response[172:] != capsule[56:]: reasons.append('response/capsule color mismatch')
        fnv, count, rows = summary(response[172:])
        if fields[9:] != (fnv, count, *rows): reasons.append('response color summary mismatch')
    if image is None or len(image) != 4096:
        reasons.append('original complete image unavailable')
    elif image != capsule[56:]: reasons.append('original image/capsule mismatch')
    return {'agreement': 'CONFIRMED' if not reasons else 'UNKNOWN',
            'reasons': reasons, 'capsule_terminal_complete': complete,
            'observed_events': events, 'phase': phase,
            'service_result': signed(result), 'syscall_result': signed(syscall_result),
            'instruction': 'STOP / NO RETRY',
            'producer_origin': 'NOT ESTABLISHED BY SAVED BYTES ALONE',
            'hardware_attribution': 'UNKNOWN', 'triangle_established': False,
            'sgx_execution_authorized': False}


def preserve(root, capsule):
    require(len(capsule) == SIZE, 'complete capsule delivery required')
    directory = open_private_directory(root)
    try:
        write_exclusive(directory, 'capsule.original.bin', capsule)
        os.fsync(directory)
        actual = read_regular('capsule.original.bin', dir_fd=directory)
        require(actual == capsule, 'capsule destination readback mismatch')
        receipt = {'scope': 'READ-ONLY MATERIALIZATION; ORIGIN GUARDS SEPARATE',
                   'bytes': len(actual), 'sha256': hashlib.sha256(actual).hexdigest(),
                   'hardware_attribution': 'UNKNOWN', 'triangle_established': False,
                   'sgx_execution_authorized': False}
        write_exclusive(directory, 'capsule.receipt.json', json_bytes(receipt))
        os.fsync(directory)
        return receipt
    finally:
        os.close(directory)


def bind_archive(path, gate, approval, capsule):
    """Supplement the existing archive without rewriting its original seal."""
    verified = verify_archive(path, gate, approval)
    require(verified['context']['client']['sha256'] ==
            '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835',
            'capsule producer requires the exact approved response client')
    preserve(path, capsule)
    directory = open_private_directory(path)
    try:
        original = read_regular('capsule.original.bin', dir_fd=directory)
        response = read_regular('response.original.bin', dir_fd=directory)
        color = read_regular('color.original.bin', dir_fd=directory)
        raw_manifest = read_regular('manifest.json', dir_fd=directory)
        binding = {'scope': 'OFFLINE COMMON SAVED-BYTE BINDING; LIVE ORIGIN SEPARATE',
                   'context': verified['context'],
                   'archive_manifest_sha256': hashlib.sha256(raw_manifest).hexdigest(),
                   'capsule_sha256': hashlib.sha256(original).hexdigest(),
                   'agreement': check(original, response, color)}
        encoded = json_bytes(binding)
        write_exclusive(directory, 'capsule.binding.json', encoded)
        os.fsync(directory)
        write_exclusive(directory, 'capsule.binding.sealed.json', json_bytes({
            'binding_sha256': hashlib.sha256(encoded).hexdigest(),
            'scope': 'SAVED-BYTE INTEGRITY ONLY'}))
        os.fsync(directory)
        return binding
    finally:
        os.close(directory)


def verify_binding(path, gate, approval):
    verified = verify_archive(path, gate, approval)
    require(verified['context']['client']['sha256'] ==
            '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835',
            'capsule producer requires the exact approved response client')
    directory = open_private_directory(path)
    try:
        encoded = read_regular('capsule.binding.json', dir_fd=directory)
        seal = json.loads(read_regular('capsule.binding.sealed.json', dir_fd=directory))
        require(seal.get('binding_sha256') == hashlib.sha256(encoded).hexdigest(),
                'capsule binding seal mismatch')
        binding = json.loads(encoded)
        raw = read_regular('capsule.original.bin', dir_fd=directory)
        receipt = json.loads(read_regular('capsule.receipt.json', dir_fd=directory))
        require(receipt.get('sha256') == binding.get('capsule_sha256') ==
                hashlib.sha256(raw).hexdigest() and receipt.get('bytes') == len(raw),
                'capsule bytes mismatch')
        require(binding.get('context') == verified['context'] and
                binding.get('archive_manifest_sha256') == hashlib.sha256(
                    read_regular('manifest.json', dir_fd=directory)).hexdigest(),
                'capsule archive/context mismatch')
        result = check(raw, read_regular('response.original.bin', dir_fd=directory),
                       read_regular('color.original.bin', dir_fd=directory))
        require(binding.get('agreement') == result, 'capsule agreement drift')
        return result
    finally:
        os.close(directory)


def read_channel():
    """Future authorized passive use only; reject substituted filesystem nodes.

    The exact loaded-module/boot and before-call UNUSED receipts must separately
    prove fresh producer origin. This function is never called by offline tests.
    """
    fd = os.open(CHANNEL, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK | os.O_CLOEXEC)
    try:
        info = os.fstat(fd)
        require(stat.S_ISREG(info.st_mode) and info.st_uid == 0 and
                stat.S_IMODE(info.st_mode) == 0o400 and info.st_nlink == 1,
                'protected kernel evidence node required')
        # Verify the already-open fd belongs to procfs, rather than trusting a
        # path or a regular file copied into another mounted filesystem.
        mount_id = None
        for line in Path('/proc/self/fdinfo/' + str(fd)).read_text().splitlines():
            if line.startswith('mnt_id:'): mount_id = line.split(':',1)[1].strip()
        entries = Path('/proc/self/mountinfo').read_text().splitlines()
        require(mount_id is not None and any(row.split()[0] == mount_id and
                row.split(' - ',1)[1].split()[0] == 'proc' for row in entries),
                'evidence fd must originate in procfs')
        data = bytearray()
        while len(data) <= SIZE:
            try: block = os.read(fd, SIZE + 1 - len(data))
            except InterruptedError: continue  # passive file read only
            if not block: break
            data.extend(block)
        require(len(data) == SIZE, 'incomplete/oversized kernel capsule')
        return bytes(data)
    finally:
        os.close(fd)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('capsule')
    parser.add_argument('--response')
    parser.add_argument('--image')
    args = parser.parse_args()
    result = check(read_regular(args.capsule),
                   read_regular(args.response) if args.response else None,
                   read_regular(args.image) if args.image else None)
    print(json.dumps(result, indent=2))
    return 0 if result['agreement'] == 'CONFIRMED' else 1


if __name__ == '__main__': raise SystemExit(main())
