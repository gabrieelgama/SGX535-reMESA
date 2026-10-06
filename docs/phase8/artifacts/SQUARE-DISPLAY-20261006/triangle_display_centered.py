#!/usr/bin/env python3
"""One separately authorized centered enlargement of the sealed FIRE3 image.

No rendering or SGX changes. Reuses the original display transaction, exact-card
authorization, ownership checks, preservation, restoration and no-retry policy.
Default inspection is read-only. Publication requires this variant's own card.
"""
import ctypes as C
import hashlib
import json
from pathlib import Path
import sys

import triangle_display as original
from triangle_pixels import convert, validate_source

SCALE = 5
SIZE = 32 * SCALE
X, Y = 560, 320
PITCH = SIZE * 4
BYTES = PITCH * SIZE


def enlarge(source):
    """Exact nearest-neighbor RGB copy; unchanged source; no alpha blending."""
    rows = convert(source, width=32, height=32, pitch=128, bits=32,
                   masks=(0xff0000, 0xff00, 0xff), byteorder='little')
    out = bytearray()
    for _, row in rows:
        enlarged = b''.join(row[i:i+4] * SCALE for i in range(0, 128, 4))
        out.extend(enlarged * SCALE)
    assert len(out) == BYTES
    return bytes(out)


def bounds(width, height, pitch):
    if width != 1280 or height != 800 or pitch != 5120:
        raise ValueError('only the qualified screen layout is permitted')
    if X < 0 or Y < 0 or X + SIZE > width or Y + SIZE > height:
        raise ValueError('publication outside screen')
    return [(y*pitch + 4*X, y*pitch + 4*(X+SIZE)) for y in range(Y, Y+SIZE)]


class CenteredDisplay(original.XDisplay):
    def __init__(self):
        super().__init__()
        self.x, self.y = X, Y

    def _children_compatible(self, window, budget):
        if budget[0] <= 0:
            raise ValueError('window topology limit exceeded')
        budget[0] -= 1
        root, parent = C.c_ulong(), C.c_ulong()
        children, count = C.POINTER(C.c_ulong)(), C.c_uint()
        if not self.lib.XQueryTree(self.d, window, C.byref(root), C.byref(parent),
                                  C.byref(children), C.byref(count)):
            raise ValueError('window topology unavailable')
        try:
            for i in range(count.value):
                a = self.attrs(children[i])
                if a.map_state != 2 or a.klass != 1:
                    continue
                x, y, child = C.c_int(), C.c_int(), C.c_ulong()
                if not self.lib.XTranslateCoordinates(self.d, children[i], self.root,
                        0, 0, C.byref(x), C.byref(y), C.byref(child)):
                    raise ValueError('window translation failed')
                if x.value < X+SIZE and y.value < Y+SIZE and x.value+a.width > X and y.value+a.height > Y:
                    if a.depth != 24:
                        raise ValueError('intersecting window has a different depth')
                    self._children_compatible(children[i], budget)
        finally:
            if children:
                self.lib.XFree(children)

    def identity(self):
        target = super().identity()
        layout = target['scanout']['layout']
        bounds(target['width'], target['height'], layout['pitch'])
        if (layout['width'], layout['height'], layout['bits_per_pixel'], layout['depth']) != (1280, 800, 32, 24):
            raise ValueError('scanout layout mismatch')
        target['presentation_scale'] = SCALE
        target['publication_width'] = target['publication_height'] = SIZE
        return target

    def read(self):
        p = self.lib.XGetImage(self.d, self.root, X, Y, SIZE, SIZE,
                              C.c_ulong(-1).value, 2)
        if not p:
            raise ValueError('screen backup unavailable')
        try:
            i = p.contents
            if (i.width, i.height, i.depth, i.bits_per_pixel, i.bytes_per_line, i.byte_order) != (SIZE, SIZE, 24, 32, PITCH, 0):
                raise ValueError('unsupported image layout')
            return C.string_at(i.data, BYTES)
        finally:
            self.lib.XDestroyImage(p)

    def prepare(self, source):
        return enlarge(source)

    def agrees(self, a, b):
        return len(a) == len(b) == BYTES and all(a[n:n+3] == b[n:n+3] for n in range(0, BYTES, 4))

    def write(self, image):
        if len(image) != BYTES:
            raise ValueError('image incomplete')
        a = self.attrs()
        data = self.c.calloc(1, BYTES)
        if not data:
            raise MemoryError('image mapping unavailable')
        p = self.lib.XCreateImage(self.d, a.visual, 24, 2, 0, data,
                                 SIZE, SIZE, 32, PITCH)
        if not p:
            self.c.free(data)
            raise ValueError('XImage allocation failed')
        gc = None
        try:
            i = p.contents
            if (i.width, i.height, i.bits_per_pixel, i.bytes_per_line, i.byte_order) != (SIZE, SIZE, 32, PITCH, 0):
                raise ValueError('unexpected image format')
            C.memmove(i.data, image, BYTES)
            gc = self.lib.XCreateGC(self.d, self.root, 0, None)
            if not gc:
                raise ValueError('GC unavailable')
            self.lib.XSetFunction(self.d, gc, 3)
            self.lib.XSetPlaneMask(self.d, gc, 0xffffff)
            self.lib.XSetSubwindowMode(self.d, gc, 1)
            self.lib.XPutImage(self.d, self.root, gc, p, 0, 0, X, Y, SIZE, SIZE)
            self.sync()
        finally:
            if gc:
                self.lib.XFreeGC(self.d, gc)
            self.lib.XDestroyImage(p)


def main():
    # Validate this variant and all deployed tools before connecting to X.
    if sys.argv.count('--card') != 1:
        raise ValueError('exact centered card required, including for inspection')
    path = Path(sys.argv[sys.argv.index('--card') + 1])
    card = json.loads(path.read_text())
    if (card.get('variant'), card.get('hold_seconds'), card.get('affected_rectangle')) != (
            'CENTERED_NEAREST_NEIGHBOR_5X', 15,
            {'x': X, 'y': Y, 'width': SIZE, 'height': SIZE}):
        raise ValueError('wrong display variant')
    expected_tools = {'tools/display/triangle_display.py',
                      'tools/display/triangle_pixels.py',
                      'tools/display/triangle_display_centered.py'}
    if set(card['tools']) != expected_tools:
        raise ValueError('complete tool binding required')
    for name, meta in card['tools'].items():
        local = Path(__file__).parent / Path(name).name
        data = local.read_bytes()
        if len(data) != meta['bytes'] or hashlib.sha256(data).hexdigest() != meta['sha256']:
            raise ValueError('tool identity mismatch')
    if '--publish-once' not in sys.argv:
        backend = CenteredDisplay()
        try:
            target = backend.identity()
            if target != card['target']:
                raise ValueError('card destination stale')
            before = backend.read()
            print(json.dumps({'target': target, 'read_only': True,
                              'image_pitch': PITCH, 'image_bits_per_pixel': 32,
                              'region_before_bytes': len(before),
                              'region_before_sha256': hashlib.sha256(before).hexdigest()}, indent=2))
        finally:
            backend.close()
        return
    original.XDisplay = CenteredDisplay
    original.main()


if __name__ == '__main__':
    main()
