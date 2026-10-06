#!/usr/bin/env python3
"""Verify historical result bytes offline. No build, network or device access."""
import argparse
import hashlib
import json
from pathlib import Path
import struct

REPO = Path(__file__).resolve().parents[2]

def checked_file(root, name, metadata):
    path = (root / name).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError('archive path escapes its root')
    raw = path.read_bytes()
    if len(raw) != metadata['bytes'] or hashlib.sha256(raw).hexdigest() != metadata['sha256']:
        raise ValueError(f'identity mismatch: {name}')
    return raw

def pixels(raw, shape):
    if shape not in ('triangle', 'square') or len(raw) != 4096:
        raise ValueError('unsupported scene or incomplete readback')
    values = struct.unpack('<1024I', raw)
    for i, value in enumerate(values):
        x, y = i % 32, i // 32
        inside = (8 <= y <= 22 and 8 <= x <= 30-y) if shape == 'triangle' else (8 <= x <= 23 and 8 <= y <= 23)
        if value != (0xffff00ff if inside else 0):
            raise ValueError(f'unexpected pixel at ({x},{y})')
    count = values.count(0xffff00ff)
    return {'magenta_pixels': count, 'zero_pixels': 1024-count}

def verify(shape):
    doc = REPO / 'docs/phase8'
    if shape == 'triangle':
        manifest = json.loads((doc/'FIRE3-TRIANGLE-REPRODUCTION.json').read_text())
        for name, meta in manifest['archive_files'].items():
            checked_file(REPO, name, meta)
        source = json.loads((doc/'VISIBLE-TRIANGLE-REPRODUCTION.json').read_text())['source']
        count = len(manifest['archive_files'])
    else:
        manifest = json.loads((doc/'MULTI-TRIANGLE-REPRODUCTION.json').read_text())
        archive = REPO / manifest['archive']
        seal = json.loads((archive/'archive-seal.json').read_text())
        for meta in seal['files']:
            checked_file(archive, meta['path'], meta)
        candidate = json.loads((doc/'two-triangle-experimental-candidate-20261006.json').read_text())
        for name, meta in candidate['archived_files'].items():
            checked_file(REPO, name, meta)
        source = manifest['readback']
        count = len(seal['files']) + len(candidate['archived_files'])
    result = pixels(checked_file(REPO, source['path'], source), shape)
    return {'scene': shape, 'checked_archive_entries': count, **result,
            'source_sha256': source['sha256'], 'hardware_interactions': 0}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('scene', choices=('triangle', 'square', 'all'))
    args = parser.parse_args()
    try:
        result = [verify(s) for s in (('triangle','square') if args.scene == 'all' else (args.scene,))]
    except (OSError, ValueError, KeyError, TypeError) as exc:
        parser.exit(1, f'Verification failed: {exc}\nNo hardware operation was attempted.\n')
    print(json.dumps(result, indent=2))

if __name__ == '__main__':
    main()
