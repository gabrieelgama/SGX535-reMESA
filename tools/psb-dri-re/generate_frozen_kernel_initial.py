#!/usr/bin/env python3
"""Generate fixed pre-relocation user-BO stores from the frozen CPU scene."""
import argparse
from pathlib import Path
import struct

import frozen_triangle_bo as bo
import frozen_triangle_image as image


HERE = Path(__file__).resolve().parent
OUTPUT = HERE / 'frozen_kernel_initial.inc'


def expected_text():
    scene = image.build()
    plan = bo.build(scene)
    bo.check(scene, plan)
    names = list(plan['bos'])[:6]
    backings = bo.zero_user_backings(plan)
    for name, binding in plan['bindings'].items():
        if (binding['bo'] not in backings or binding['alias'] or
                name == 'relocation_wire'):
            continue
        raw = scene['objects'][name]['bytes']
        if raw is None or len(raw) != binding['size']:
            raise ValueError('unresolved selected CPU image: ' + name)
        backings[binding['bo']][binding['offset']:
                 binding['offset'] + len(raw)] = bytes(
                     value if value is not None else 0 for value in raw)
    lines = [
        '/* Generated from frozen_triangle_image.py + frozen_triangle_bo.py.',
        ' * Sparse pre-relocation stores; all six complete backings are zeroed first.',
        ' * Regenerate with generate_frozen_kernel_initial.py. */',
    ]
    for role, name in enumerate(names):
        raw = backings[name]
        for offset in range(0, len(raw), 4):
            value = struct.unpack_from('<I', raw, offset)[0]
            if value:
                lines.append('    {%d, 0x%05x, 0x%08x},' %
                             (role, offset, value))
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    expected = expected_text()
    if args.check:
        if not OUTPUT.exists() or OUTPUT.read_text() != expected:
            raise SystemExit('stale frozen_kernel_initial.inc')
    else:
        OUTPUT.write_text(expected)


if __name__ == '__main__':
    main()
