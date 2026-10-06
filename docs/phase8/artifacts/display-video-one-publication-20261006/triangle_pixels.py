"""CPU-only conversion/transaction policy. No device access on import.

Source is immutable little-endian ARGB8888 FIRE3 readback. XRGB destinations
ignore source alpha: zero pixels become black, not transparent. The publisher
owns only a 32x32 logical rectangle, never the framebuffer or DRM master.
"""
import hashlib

WIDTH = HEIGHT = 32


def validate_source(source, expected_sha256):
    if len(source) != 4096 or hashlib.sha256(source).hexdigest() != expected_sha256:
        raise ValueError('source identity mismatch')
    for y in range(32):
        for x in range(32):
            value = int.from_bytes(source[4*(32*y+x):4*(32*y+x+1)], 'little')
            wanted = 0xffff00ff if 8 <= y <= 22 and 8 <= x <= 30-y else 0
            if value != wanted:
                raise ValueError('not the sealed FIRE3 image')


def _field(mask, bits):
    if not isinstance(mask, int) or mask <= 0 or mask >= 1 << bits:
        raise ValueError('unsupported mask')
    shift = (mask & -mask).bit_length()-1
    maximum = mask >> shift
    if maximum & (maximum+1):
        raise ValueError('noncontiguous mask')
    return shift, maximum


def convert(source, *, width, height, pitch, bits, masks, byteorder,
            x=0, y=0, extent=None):
    """Plan a bounded copy: return (offset, bytes) rows, touching no padding.

    Reject clipping instead of silently losing any of the source image.
    Caller-provided extent is the mapped/logical buffer extent, not GPU VA.
    """
    if len(source) != 4096 or bits not in (16, 24, 32) or byteorder not in ('little', 'big'):
        raise ValueError('unsupported source/format')
    if any(not isinstance(v, int) or v < 0 for v in (width, height, pitch, x, y)):
        raise ValueError('invalid layout')
    cpp = bits//8
    if pitch < width*cpp or width < x+32 or height < y+32:
        raise ValueError('pitch/bounds/clipping')
    if extent is None:
        extent = pitch*height
    if not isinstance(extent, int) or extent < 0 or pitch*height > (1 << 63)-1:
        raise ValueError('extent/overflow')
    if len(masks) != 3 or any(a & b for i, a in enumerate(masks) for b in masks[i+1:]):
        raise ValueError('overlapping masks')
    fields = [_field(m, bits) for m in masks]
    rows = []
    for sy in range(32):
        offset = (y+sy)*pitch+x*cpp
        if offset+32*cpp > extent:
            raise ValueError('mapping truncated')
        row = bytearray()
        for sx in range(32):
            argb = int.from_bytes(source[4*(sy*32+sx):4*(sy*32+sx+1)], 'little')
            channels = ((argb >> 16) & 255, (argb >> 8) & 255, argb & 255)
            value = 0
            for c, (shift, maximum) in zip(channels, fields):
                value |= ((c*maximum+127)//255) << shift
            row.extend(value.to_bytes(cpp, byteorder))
        rows.append((offset, bytes(row)))
    return rows


def publish_transaction(backend, source, sha256, target, preserve, hold,
                        observe=lambda phase, data, target: None):
    """One reversible CPU publication with originals saved before first write.

    Backend.claim serializes the existing display owner's client requests;
    backend.identity binds its connection/root/mode/visual/boot/server lifetime.
    Ownership loss prevents writing the backup to an unrelated display.
    A failed restoration remains a reported failure with backup retained.
    """
    validate_source(source, sha256)
    if backend.identity() != target:
        raise ValueError('destination ownership/binding changed')
    claimed = False
    attempted = False
    backup = None
    try:
        backend.claim()
        claimed = True
        if backend.identity() != target:
            raise ValueError('ownership changed before backup')
        backup = backend.read()
        preserve(backup, target)  # exclusive durable save; failure means no write
        image = backend.prepare(source)
        if backend.identity() != target:
            raise ValueError('ownership changed before publication')
        attempted = True  # partial publication is possible after this point
        backend.write(image)
        published = backend.read()
        observe('published', published, target)
        if not backend.agrees(published, image):
            raise ValueError('published pixels mismatch')
        hold()
        if backend.identity() != target:
            raise ValueError('ownership lost; backup retained, restoration blocked')
    finally:
        try:
            if attempted and backend.identity() == target:
                backend.write(backup)
                restored = backend.read()
                observe('restored', restored, target)
                if not backend.agrees(restored, backup):
                    raise ValueError('restoration failed; backup retained')
            elif attempted:
                raise ValueError('ownership lost; restoration UNKNOWN, no foreign write')
        finally:
            if claimed:
                backend.release()
