#!/usr/bin/env python3
"""Display-only FIRE3 publication, through the existing software Xorg owner.

Default is inspection. --publish-once is a separate, explicit display-only
authorization boundary. No DRM master acquisition, framebuffer mmap, GL,
SGX device, rendering client, modeset, reset or page flip is performed here.
"""
import argparse
import ctypes as C
import ctypes.util
import hashlib
import json
import os
from pathlib import Path
import signal
import time
import fcntl
import struct

from triangle_pixels import convert, publish_transaction, validate_source


class Visual(C.Structure):
    _fields_ = [('ext', C.c_void_p), ('visualid', C.c_ulong), ('klass', C.c_int),
                ('red', C.c_ulong), ('green', C.c_ulong), ('blue', C.c_ulong),
                ('bits_per_rgb', C.c_int), ('map_entries', C.c_int)]


class Attributes(C.Structure):
    _fields_ = [(n, C.c_int) for n in ('x', 'y', 'width', 'height', 'border', 'depth')] + [
        ('visual', C.POINTER(Visual)), ('root', C.c_ulong),
        ('klass', C.c_int), ('bit_gravity', C.c_int), ('win_gravity', C.c_int),
        ('backing_store', C.c_int), ('backing_planes', C.c_ulong),
        ('backing_pixel', C.c_ulong), ('save_under', C.c_int),
        ('colormap', C.c_ulong), ('map_installed', C.c_int), ('map_state', C.c_int),
        ('all_event_masks', C.c_long), ('your_event_mask', C.c_long),
        ('do_not_propagate_mask', C.c_long), ('override_redirect', C.c_int),
        ('screen', C.c_void_p)]


class Image(C.Structure):
    # Public XImage prefix from X11/Xlib.h; functions/obdata are never accessed.
    _fields_ = [(n, C.c_int) for n in ('width', 'height', 'xoffset', 'format')] + [
        ('data', C.c_void_p)] + [(n, C.c_int) for n in (
            'byte_order', 'bitmap_unit', 'bitmap_bit_order', 'bitmap_pad', 'depth',
            'bytes_per_line', 'bits_per_pixel')] + [
        ('red_mask', C.c_ulong), ('green_mask', C.c_ulong), ('blue_mask', C.c_ulong)]


class XError(C.Structure):
    _fields_ = [('type', C.c_int), ('display', C.c_void_p),
                ('resourceid', C.c_ulong), ('serial', C.c_ulong),
                ('error_code', C.c_ubyte), ('request_code', C.c_ubyte),
                ('minor_code', C.c_ubyte)]


def _bind(lib, name, result, *args):
    fn = getattr(lib, name)
    fn.restype, fn.argtypes = result, list(args)
    return fn


def kms_identity():
    """DRM read-only metadata getters; unprivileged GETFB returns no GEM handle.

    Kernel drm_framebuffer.c deliberately exposes dimensions/pitch to nonmaster
    clients without exporting the backing. Never run this as root/master.
    """
    if os.geteuid() == 0:
        raise ValueError('run as the unprivileged desktop owner, never DRM master/root')
    fd = os.open('/dev/dri/card0', os.O_RDONLY | os.O_CLOEXEC)
    try:
        b = bytearray(64)
        fcntl.ioctl(fd, 0xc04064a0, b, True)
        values = struct.unpack('<4Q8I', b)
        if any(n > 64 for n in values[4:8]):
            raise ValueError('unexpected DRM topology')
        arrays = [(C.c_uint32*n)() for n in values[4:8]]
        for i, arr in enumerate(arrays):
            struct.pack_into('<Q', b, 8*i, C.addressof(arr))
        fcntl.ioctl(fd, 0xc04064a0, b, True)
        if tuple(struct.unpack('<4Q8I', b)[4:8]) != values[4:8]:
            raise ValueError('DRM topology changed during observation')
        active = []
        for crtc in arrays[1]:
            q = bytearray(104)
            struct.pack_into('<I', q, 12, crtc)
            fcntl.ioctl(fd, 0xc06864a1, q, True)
            v = struct.unpack_from('<Q7I', q)
            if v[7]:
                active.append({'crtc': v[2], 'framebuffer': v[3], 'x': v[4],
                               'y': v[5], 'mode_hex': bytes(q[36:]).hex()})
        if len(active) != 1:
            raise ValueError('one active scanout required')
        meta = bytearray(28)
        struct.pack_into('<I', meta, 0, active[0]['framebuffer'])
        fcntl.ioctl(fd, 0xc01c64ad, meta, True)
        fb, width, height, pitch, bpp, depth, handle = struct.unpack('<7I', meta)
        if handle:
            raise ValueError('unexpected GEM export; fd closure releases it, STOP')
        active[0]['layout'] = {'width': width, 'height': height, 'pitch': pitch,
                               'bits_per_pixel': bpp, 'depth': depth,
                               'backing_exported': False}
        return active[0]
    finally:
        os.close(fd)


class XDisplay:
    def __init__(self, display=':0.0', x=64, y=64):
        if display != ':0.0' or (x, y) != (64, 64):
            raise ValueError('only the reviewed local display/placement is allowed')
        self.lib = C.CDLL(ctypes.util.find_library('X11') or 'libX11.so.6')
        self.c = C.CDLL(None)
        P, L, I = C.c_void_p, C.c_ulong, C.c_int
        specs = [
            ('XOpenDisplay', P, C.c_char_p), ('XCloseDisplay', I, P),
            ('XDefaultScreen', I, P), ('XRootWindow', L, P, I),
            ('XGetWindowAttributes', I, P, L, C.POINTER(Attributes)),
            ('XInternAtom', L, P, C.c_char_p, I), ('XGetSelectionOwner', L, P, L),
            ('XGetImage', C.POINTER(Image), P, L, I, I, C.c_uint, C.c_uint, L, I),
            ('XDestroyImage', I, C.POINTER(Image)), ('XGrabServer', I, P),
            ('XUngrabServer', I, P), ('XSync', I, P, I),
            ('XCreateImage', C.POINTER(Image), P, C.POINTER(Visual), C.c_uint,
             I, I, P, C.c_uint, C.c_uint, I, I),
            ('XCreateGC', P, P, L, L, P), ('XSetSubwindowMode', I, P, P, I),
            ('XSetFunction', I, P, P, I), ('XSetPlaneMask', I, P, P, L),
            ('XPutImage', I, P, L, P, C.POINTER(Image), I, I, I, I, C.c_uint, C.c_uint),
            ('XFreeGC', I, P, P), ('XQueryTree', I, P, L, C.POINTER(L),
             C.POINTER(L), C.POINTER(C.POINTER(L)), C.POINTER(C.c_uint)),
            ('XFree', I, P), ('XTranslateCoordinates', I, P, L, L, I, I,
             C.POINTER(I), C.POINTER(I), C.POINTER(L))]
        for name, result, *args in specs:
            _bind(self.lib, name, result, *args)
        _bind(self.lib, 'XSetErrorHandler', P, P)
        self.errors = []
        callback = C.CFUNCTYPE(I, P, C.POINTER(XError))
        def on_error(display, error):
            self.errors.append((error.contents.error_code, error.contents.request_code))
            return 0
        self.error_handler = callback(on_error)
        self.old_error_handler = self.lib.XSetErrorHandler(C.cast(self.error_handler, P))
        _bind(self.c, 'calloc', P, C.c_size_t, C.c_size_t)
        _bind(self.c, 'free', None, P)
        self.d = self.lib.XOpenDisplay(display.encode())
        if not self.d:
            raise ValueError('display unavailable/authentication failed')
        self.root = self.lib.XRootWindow(self.d, self.lib.XDefaultScreen(self.d))
        self.x, self.y = x, y

    def attrs(self, window=None):
        a = Attributes()
        if not self.lib.XGetWindowAttributes(self.d, window or self.root, C.byref(a)):
            raise ValueError('drawable unavailable')
        return a

    def _children_compatible(self, window, budget):
        # IncludeInferiors is permitted only for intersecting same-depth windows.
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
                if x.value < self.x+32 and y.value < self.y+32 and x.value+a.width > self.x and y.value+a.height > self.y:
                    if a.depth != 24:
                        raise ValueError('intersecting window has a different depth')
                    self._children_compatible(children[i], budget)
        finally:
            if children:
                self.lib.XFree(children)

    def identity(self):
        a = self.attrs()
        v = a.visual.contents
        if a.depth != 24 or v.klass != 4 or (v.red, v.green, v.blue) != (0xff0000, 0xff00, 0xff):
            raise ValueError('unsupported root visual')
        if a.width < self.x+32 or a.height < self.y+32:
            raise ValueError('destination bounds changed')
        atom = self.lib.XInternAtom(self.d, b'_NET_WM_CM_S0', 1)
        if atom and self.lib.XGetSelectionOwner(self.d, atom):
            raise ValueError('compositing owner present; direct root publication rejected')
        self._children_compatible(self.root, [4096])
        servers = []
        for p in Path('/proc').iterdir():
            if not p.name.isdecimal():
                continue
            try:
                if (p/'comm').read_text().strip() == 'Xorg':
                    fields = (p/'stat').read_text().rsplit(')', 1)[1].split()
                    servers.append({'pid': int(p.name), 'start_ticks': fields[19]})
            except (PermissionError, FileNotFoundError):
                pass
        if len(servers) != 1:
            raise ValueError('Xorg lifetime ambiguous')
        log = Path('/var/log/Xorg.0.log').read_text()
        if 'glamor initialization failed' not in log or 'GLX: Initialized DRISWRAST GL provider' not in log:
            raise ValueError('reviewed software publication path not established')
        return {'boot_id': Path('/proc/sys/kernel/random/boot_id').read_text().strip(),
                'kernel': os.uname().release,
                'active_vt': Path('/sys/class/tty/tty0/active').read_text().strip(),
                'display': ':0.0', 'root': self.root, 'visual': v.visualid,
                'width': a.width, 'height': a.height, 'depth': a.depth,
                'rgb_masks': [v.red, v.green, v.blue], 'x': self.x, 'y': self.y,
                'server': servers[0], 'compositor': None,
                'scanout': kms_identity(),
                'loaded_driver_note_sha256': hashlib.sha256(Path('/sys/module/gma500_gfx/notes/.note.gnu.build-id').read_bytes()).hexdigest(),
                'loaded_observer_note_sha256': hashlib.sha256(Path('/sys/module/sgx535_provenance/notes/.note.gnu.build-id').read_bytes()).hexdigest(),
                'publication': 'X11 core software XPutImage; existing Xorg owns scanout'}

    def claim(self):
        self.lib.XGrabServer(self.d)
        try:
            self.sync()
        except BaseException:
            self.lib.XUngrabServer(self.d)
            self.lib.XSync(self.d, 0)
            raise

    def release(self):
        self.lib.XUngrabServer(self.d)
        self.sync()

    def sync(self):
        self.lib.XSync(self.d, 0)
        if self.errors:
            errors, self.errors = self.errors, []
            raise ValueError('X protocol failure: '+repr(errors))

    def read(self):
        p = self.lib.XGetImage(self.d, self.root, self.x, self.y, 32, 32, C.c_ulong(-1).value, 2)
        if not p:
            raise ValueError('screen read/backup unavailable')
        try:
            i = p.contents
            if i.width != 32 or i.height != 32 or i.depth != 24 or i.bits_per_pixel != 32 or i.bytes_per_line != 128 or i.byte_order != 0:
                raise ValueError('unsupported image mapping/layout')
            return C.string_at(i.data, 4096)
        finally:
            self.lib.XDestroyImage(p)

    def prepare(self, source):
        return b''.join(row for _, row in convert(source, width=32, height=32,
            pitch=128, bits=32, masks=(0xff0000, 0xff00, 0xff), byteorder='little'))

    def agrees(self, a, b):
        # Depth24 XGetImage does not define the unused high byte of a 32bpp image.
        return len(a) == len(b) == 4096 and all(a[n:n+3] == b[n:n+3] for n in range(0, 4096, 4))

    def write(self, image):
        if len(image) != 4096:
            raise ValueError('image incomplete')
        a = self.attrs()
        data = self.c.calloc(1, 4096)
        if not data:
            raise MemoryError('image mapping unavailable')
        p = self.lib.XCreateImage(self.d, a.visual, 24, 2, 0, data, 32, 32, 32, 128)
        if not p:
            self.c.free(data)
            raise ValueError('XImage allocation failed')
        gc = None
        try:
            i = p.contents
            if i.bits_per_pixel != 32 or i.bytes_per_line != 128 or i.byte_order != 0:
                raise ValueError('unexpected image format')
            C.memmove(i.data, image, 4096)
            gc = self.lib.XCreateGC(self.d, self.root, 0, None)
            if not gc:
                raise ValueError('GC unavailable')
            self.lib.XSetFunction(self.d, gc, 3)  # GXcopy
            self.lib.XSetPlaneMask(self.d, gc, 0xffffff)  # root depth24 RGB only
            self.lib.XSetSubwindowMode(self.d, gc, 1)  # IncludeInferiors
            self.lib.XPutImage(self.d, self.root, gc, p, 0, 0, self.x, self.y, 32, 32)
            self.sync()
        finally:
            if gc:
                self.lib.XFreeGC(self.d, gc)
            self.lib.XDestroyImage(p)

    def close(self):
        if self.d:
            self.lib.XCloseDisplay(self.d)
            self.d = None
        self.lib.XSetErrorHandler(self.old_error_handler)


def durable(path, data):
    fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, 0o600)
    try:
        with os.fdopen(fd, 'wb', closefd=False) as stream:
            stream.write(data)
            stream.flush()
            os.fsync(fd)
    finally:
        os.close(fd)
    if Path(path).read_bytes() != data:
        raise ValueError('preservation verification failed')


def validate_authorization(card, raw_card, authorization, evidence):
    if card.get('sgx_execution_authorized') is not False or card.get('scope') != 'ONE DISPLAY-ONLY CPU PUBLICATION; NO SGX':
        raise ValueError('wrong authorization scope')
    if not isinstance(card.get('hold_seconds'), int) or not 1 <= card['hold_seconds'] <= 15:
        raise ValueError('bounded display interval required')
    required = {'scope': card['scope'], 'card_sha256': hashlib.sha256(raw_card).hexdigest(),
                'boot_id': card['target']['boot_id'], 'maximum_publications': 1,
                'display_publication_authorized': True, 'sgx_execution_authorized': False}
    if authorization != required:
        raise ValueError('explicit exact-card display-only authorization missing')
    if str(Path(evidence).absolute()) != card['evidence_directory']:
        raise ValueError('evidence destination must be the one pinned by the card')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--inspect', action='store_true')
    p.add_argument('--publish-once', action='store_true')
    p.add_argument('--card', type=Path)
    p.add_argument('--authorization', type=Path)
    p.add_argument('--source', type=Path)
    p.add_argument('--evidence', type=Path)
    a = p.parse_args()
    if a.inspect and a.publish_once:
        p.error('choose inspection or explicitly authorized publication')
    backend = XDisplay()
    try:
        if not a.publish_once:
            target = backend.identity()
            raw = backend.read()
            print(json.dumps({'target': target, 'read_only': True,
                              'image_pitch': 128, 'image_bits_per_pixel': 32,
                              'region_before_sha256': hashlib.sha256(raw).hexdigest()}, indent=2))
            return
        if not all((a.card, a.authorization, a.source, a.evidence)):
            p.error('publication requires card/explicit authorization/source/new evidence destination')
        card = json.loads(a.card.read_text())
        authorization = json.loads(a.authorization.read_text())
        validate_authorization(card, a.card.read_bytes(), authorization, a.evidence)
        source = a.source.read_bytes()
        validate_source(source, card['source']['sha256'])
        if backend.identity() != card['target']:
            raise ValueError('card destination stale')
        a.evidence.mkdir(mode=0o700, exist_ok=False)
        durable(a.evidence/'card.original.json', a.card.read_bytes())
        durable(a.evidence/'authorization.original.json', a.authorization.read_bytes())
        durable(a.evidence/'source.original.bin', source)
        # Claim is the first live display mutation. The durable intent prevents a
        # second publication after partial/lost outcome under this destination.
        durable(a.evidence/'display-consumed.json', b'{"display_attempt_consumed":true,"sgx_execution_authorized":false}\n')
        def save(backup, target):
            durable(a.evidence/'screen-before.original.bin', backup)
            durable(a.evidence/'screen-before.json', (json.dumps(target, indent=2)+'\n').encode())
            fd = os.open(a.evidence, os.O_RDONLY | os.O_DIRECTORY)
            try:
                os.fsync(fd)
            finally:
                os.close(fd)
        def interrupted(signum, frame):
            raise RuntimeError('interrupted; restore within current ownership, no retry')
        for sig in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
            signal.signal(sig, interrupted)
        try:
            publish_transaction(backend, source, card['source']['sha256'], card['target'], save,
                                lambda: time.sleep(card['hold_seconds']),
                                lambda phase, data, target: durable(a.evidence/(phase+'.original.bin'), data))
            durable(a.evidence/'result.json', b'{"publication_readback":"MATCH","restoration":"MATCH","physical_visibility":"REQUIRES_OPERATOR_CONFIRMATION","sgx_invocations":0}\n')
        except BaseException as e:
            durable(a.evidence/'STOP.json', (json.dumps({'error': repr(e), 'no_retry': True})+'\n').encode())
            raise
        finally:
            files = {p.name: {'bytes': p.stat().st_size, 'sha256': hashlib.sha256(p.read_bytes()).hexdigest()}
                     for p in a.evidence.iterdir() if p.is_file()}
            durable(a.evidence/'seal.json', (json.dumps({'files': files, 'no_retry': True,
                    'sgx_execution_authorized': False}, indent=2)+'\n').encode())
    finally:
        backend.close()


if __name__ == '__main__':
    main()
