#!/usr/bin/env python3
"""Verify backing-provider evidence, not PDS semantics or a deployed kernel.

Reads only ordinary files. No downloads, kernel compilation, historical program
execution, device access, or PDS decoding. Source hash acceptance is scoped to
the reviewed files; it must not be used as a whole-stack compatibility test.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path

DOCS = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re'
MEM_MASK = 0xFF000000
PDS_REQUEST = 0x20000001


def permitted_types(mask):
    """Memory-bit necessary condition in K drm_bo_mt_compatible, not allocator."""
    return {i for i in range(8) if (1 << (24 + i)) & mask & MEM_MASK}


def check_digest(data, expected):
    actual = hashlib.sha256(data).hexdigest()
    if actual != expected:
        raise ValueError(f'hash mismatch: {actual} != {expected}')


def self_test():
    assert permitted_types(PDS_REQUEST) == {5}
    assert permitted_types(1) == set()
    # Adversarial mask: adding VRAM really admits another type. A blanket
    # "all PSB BOs use the zero allocator" conclusion would be incorrect.
    assert permitted_types(PDS_REQUEST | (1 << 25)) == {1, 5}
    # A matching ABI/header cannot make an altered backing source acceptable.
    original = b'alloc_page(GFP_KERNEL | __GFP_ZERO | GFP_DMA32);'
    digest = hashlib.sha256(original).hexdigest()
    check_digest(original, digest)
    try:
        check_digest(original.replace(b' | __GFP_ZERO', b''), digest)
    except ValueError:
        pass
    else:
        raise AssertionError('negative allocator mutation was accepted')


def verify(root):
    count = 0
    acquisitions = json.loads((DOCS / 'pds-backing-acquisition.json').read_text())
    for row in acquisitions:
        if row.get('status') == 200:
            check_digest((root / row['name']).read_bytes(), row['sha256'])
            count += 1
    with (DOCS / 'pds-backing-file-comparison.csv').open(newline='') as stream:
        for row in csv.DictReader(stream):
            check_digest((root / row['artifact_path']).read_bytes(), row['sha256'])
            count += 1
    print(f'{count} acquisition/derived-file hashes verified')
    dsc_count = 0
    for dsc in sorted((root / 'launchpad').glob('*.dsc')):
        algorithm = None
        for line in dsc.read_text().splitlines():
            heading = line.rstrip()
            algorithms = {'Files:': 'md5', 'Checksums-Sha1:': 'sha1',
                          'Checksums-Sha256:': 'sha256'}
            if heading in algorithms:
                algorithm = algorithms[heading]
            elif line and not line.startswith(' '):
                algorithm = None
            elif algorithm and line.startswith(' '):
                digest, size, name = line.split()
                data = (dsc.parent / name).read_bytes()
                if len(data) != int(size) or hashlib.new(algorithm, data).hexdigest() != digest:
                    raise ValueError(f'DSC checksum/size mismatch: {name}')
                dsc_count += 1
    if dsc_count != 30:
        raise ValueError(f'expected 30 DSC checksum rows, got {dsc_count}')
    print(f'{dsc_count} DSC size/digest checks passed; signatures not authenticated')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifacts-root', type=Path)
    args = parser.parse_args()
    self_test()
    print('placement-mask checks and negative source-mutation regression passed')
    if args.artifacts_root:
        verify(args.artifacts_root)
    print('Target provider membership: NOT ESTABLISHED (not inferred by this tool)')


if __name__ == '__main__':
    main()
