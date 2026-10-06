#!/usr/bin/env python3
"""Display-only adapter for the sealed MULTI_TRIANGLE readback.

Retains qualified Xorg transaction/ownership/backup/restoration machinery.
Changes only source exact-shape validation and bounded nearest-neighbor placement.
Import has no device access. Does not contain an SGX submission path.
"""
import hashlib
import json
from pathlib import Path
import sys
import triangle_pixels as pixels
import triangle_display as original
import triangle_display_centered as centered

SOURCE_SHA = '6e9af8e8b6576b979aa78816d44bd3b0ffd31b70e169a982e83736d63ab91729'
SCALE = 10
SIZE = 320
X, Y = 480, 240
PITCH = 1280
BYTES = 409600


def validate_square(source, expected_sha256):
    if expected_sha256 != SOURCE_SHA or len(source) != 4096 or hashlib.sha256(source).hexdigest() != SOURCE_SHA:
        raise ValueError('sealed square source identity mismatch')
    for y in range(32):
        for x in range(32):
            actual = int.from_bytes(source[4*(y*32+x):4*(y*32+x+1)], 'little')
            if actual != (0xffff00ff if 8 <= x <= 23 and 8 <= y <= 23 else 0):
                raise ValueError('selected square footprint mismatch')


# Qualified implementation unchanged; adapt its existing size constants and
# narrow source predicate. Pixel conversion/claim/copy/restoration are reused.
for key, value in {'SCALE': SCALE, 'SIZE': SIZE, 'X': X, 'Y': Y,
                   'PITCH': PITCH, 'BYTES': BYTES}.items():
    setattr(centered, key, value)
pixels.validate_source = validate_square
original.validate_source = validate_square


def main():
    if '--publish-once' not in sys.argv:
        backend = centered.CenteredDisplay()
        try:
            target = backend.identity()
            before = backend.read()
            print(json.dumps({'target': target, 'read_only': True,
                              'image_pitch': PITCH, 'image_bits_per_pixel': 32,
                              'region_before_bytes': len(before),
                              'region_before_sha256': hashlib.sha256(before).hexdigest()}, indent=2))
        finally:
            backend.close()
        return
    if sys.argv.count('--card') != 1:
        raise ValueError('one exact square display card required')
    card = json.loads(Path(sys.argv[sys.argv.index('--card')+1]).read_text())
    if (card.get('variant'), card.get('hold_seconds'), card.get('affected_rectangle')) != (
            'CENTERED_SEALED_SQUARE_NEAREST_NEIGHBOR_10X', 15,
            {'x': X, 'y': Y, 'width': SIZE, 'height': SIZE}):
        raise ValueError('wrong publication variant')
    tools = {'tools/display/triangle_pixels.py', 'tools/display/triangle_display.py',
             'tools/display/triangle_display_centered.py', 'tools/display/square_display_centered.py'}
    if set(card['tools']) != tools:
        raise ValueError('complete exact tool bindings required')
    for name, binding in card['tools'].items():
        raw = (Path(__file__).parent / Path(name).name).read_bytes()
        if len(raw) != binding['bytes'] or hashlib.sha256(raw).hexdigest() != binding['sha256']:
            raise ValueError('tool identity mismatch')
    original.XDisplay = centered.CenteredDisplay
    original.main()


if __name__ == '__main__':
    main()
