#!/usr/bin/env python3
"""Compare a preserved color BO to CPU references; never certify GPU execution.

The frozen image is 32x32 linear ARGB8888 (little-endian BGRA bytes), with
128-byte rows, opaque white vertices (8,8), (24,8), (8,24), and zero background.
Pixel-center sampling and row y == memory row y are explicit reference
assumptions. The available contract does not qualify the SGX edge tie rule:
compare two *complete* masks, excluding or including the hypotenuse centers.
Neither a reference match nor this program's exit zero establishes a triangle.
Completion attribution and readback provenance must be assessed independently.

Reads regular-file artifacts on Linux only. JSON exit codes: 0 reference match, 1 reference mismatch,
2 invalid input. No ioctl, device access, image generation or input rewriting.
"""
import argparse
import json
import os
from pathlib import Path
import stat
import struct
import sys


WIDTH = HEIGHT = 32
STRIDE = 128
SIZE = 4096
WHITE = 0xffffffff
VERTICES = ((8, 8), (24, 8), (8, 24))


def validate_readback(data, *, max_details=16):
    """Inspect every pixel and return reference agreement, not a render verdict."""
    if len(data) != SIZE:
        raise ValueError(f'expected exactly {SIZE} bytes; got {len(data)}')
    if not isinstance(max_details, int) or not 0 <= max_details <= WIDTH * HEIGHT:
        raise ValueError('max_details must be between 0 and 1024')
    pixels = struct.unpack('<1024I', data)
    references = {}
    for rule in ('exclude', 'include'):
        mismatches = []
        mismatch_count = white_count = 0
        for y in range(HEIGHT):
            for x in range(WIDTH):
                # Twice each sample coordinate, so the comparison is exact.
                sx, sy = 2 * x + 1, 2 * y + 1
                diagonal = sx + sy <= 64 if rule == 'include' else sx + sy < 64
                covered = sx >= 16 and sy >= 16 and diagonal
                expected = WHITE if covered else 0
                white_count += int(covered)
                actual = pixels[y * WIDTH + x]
                if actual == expected:
                    continue
                mismatch_count += 1
                if len(mismatches) < max_details:
                    kind = ('unexpected_foreground' if not covered else
                            'missing_foreground' if actual == 0 else 'wrong_color')
                    mismatches.append({
                        'x': x, 'y': y, 'byte_offset': y * STRIDE + x * 4,
                        'expected_argb': f'0x{expected:08x}',
                        'actual_argb': f'0x{actual:08x}', 'kind': kind,
                    })
        references[rule] = {
            'expected_white_pixels': white_count,
            'mismatch_count': mismatch_count,
            'mismatches': mismatches,
            'details_omitted': mismatch_count - len(mismatches),
        }
    matching = [rule for rule, result in references.items()
                if result['mismatch_count'] == 0]
    return {
        'scope': 'OFFLINE_REFERENCE_COMPARISON_ONLY',
        'layout': {'width': WIDTH, 'height': HEIGHT, 'stride_bytes': STRIDE,
                   'size_bytes': SIZE, 'word_format': 'ARGB8888',
                   'memory_order': 'BGRA', 'layout': 'LINEAR'},
        'vertices': [list(vertex) for vertex in VERTICES],
        'reference_assumptions':
            'pixel_center samples (x+0.5,y+0.5); memory row y equals geometry y; '
            'SGX sampling/edge tie semantics are unqualified; include/exclude '
            'are separate whole-image references, not tolerated pixel errors',
        'image_matches_reference': bool(matching),
        'matching_edge_rules': matching,
        'references': references,
        'completion_attribution': 'UNKNOWN',
        'triangle_established': False,
    }


def _stdout_is_safe(readback_stat):
    """Reject report writes to the pinned input, including through hardlinks."""
    identity = (readback_stat.st_dev, readback_stat.st_ino)
    try:
        output = os.fstat(sys.stdout.fileno())
    except (AttributeError, OSError, ValueError):
        error = 'stdout is unavailable; cannot report readback validation'
    else:
        if (output.st_dev, output.st_ino) != identity:
            return True
        error = 'stdout aliases the readback; refusing to overwrite input'
    try:
        diagnostic = os.fstat(sys.stderr.fileno())
        if (diagnostic.st_dev, diagnostic.st_ino) != identity:
            print(error, file=sys.stderr, flush=True)
    except (AttributeError, OSError, ValueError):
        pass
    return False


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('readback', type=Path, help='preserved original color BO or copy')
    args = parser.parse_args(argv)
    try:
        # O_PATH pins the object without opening a device or waiting on a FIFO.
        # Reopen that checked inode, rather than a pathname that can be replaced.
        handle = os.open(args.readback, os.O_PATH | os.O_CLOEXEC)
        try:
            readback_stat = os.fstat(handle)
            # Guard both success and invalid-input JSON before reading or reporting.
            # Caller-side truncation before this process starts cannot be prevented.
            if not _stdout_is_safe(readback_stat):
                return 2
            if not stat.S_ISREG(readback_stat.st_mode):
                raise ValueError('readback must be a regular file')
            with open(f'/proc/self/fd/{handle}', 'rb') as stream:
                # Reject trailing data without loading an arbitrarily large file.
                data = stream.read(SIZE + 1)
        finally:
            os.close(handle)
        report = validate_readback(data)
    except (OSError, ValueError) as error:
        print(json.dumps({'error': str(error), 'image_matches_reference': False,
                          'completion_attribution': 'UNKNOWN',
                          'triangle_established': False}, sort_keys=True))
        return 2
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report['image_matches_reference'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
