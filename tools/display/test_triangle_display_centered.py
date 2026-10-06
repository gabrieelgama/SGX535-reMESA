"""Synthetic tests only. No display connection or SGX invocation."""
import hashlib
import unittest
from triangle_display_centered import enlarge, bounds, CenteredDisplay, SIZE, BYTES
from triangle_pixels import publish_transaction
from test_triangle_display import SOURCE, SHA, Fake


class ScaledFake(Fake):
    def prepare(self, source):
        return enlarge(source)


class Centered(unittest.TestCase):
    def test_size(self): self.assertEqual(len(enlarge(SOURCE)), 102400)
    def test_counts(self):
        b = enlarge(SOURCE); pixels = [int.from_bytes(b[n:n+4], 'little') for n in range(0, len(b), 4)]
        self.assertEqual(pixels.count(0xff00ff), 3000)
        self.assertEqual(pixels.count(0), 22600)
    def test_exact_replication(self):
        b = enlarge(SOURCE)
        for y in range(SIZE):
            for x in range(SIZE):
                self.assertEqual(b[4*(SIZE*y+x):4*(SIZE*y+x+1)],
                                 (int.from_bytes(SOURCE[4*(32*(y//5)+x//5):4*(32*(y//5)+x//5+1)], 'little') & 0xffffff).to_bytes(4, 'little'))
    def test_source_unchanged(self): enlarge(SOURCE); self.assertEqual(hashlib.sha256(SOURCE).hexdigest(), SHA)
    def test_truncated(self):
        with self.assertRaises(ValueError): enlarge(SOURCE[:-1])
    def test_bounds(self):
        rows = bounds(1280, 800, 5120)
        self.assertEqual(len(rows), 160)
        self.assertEqual(rows[0], (320*5120+560*4,320*5120+720*4))
        self.assertEqual(rows[-1],(479*5120+560*4,479*5120+720*4))
        self.assertEqual(sum(b-a for a,b in rows), BYTES)
    def test_wrong_pitch(self):
        with self.assertRaises(ValueError): bounds(1280,800,8192)
    def test_wrong_dimensions(self):
        for dimensions in [(1279,800),(1280,799),(1280,801)]:
            with self.subTest(dimensions=dimensions), self.assertRaises(ValueError): bounds(*dimensions,5120)
    def test_rgb24_agreement(self):
        a=b'\xff\0\xff\0'*(SIZE*SIZE); b=b'\xff\0\xff\x80'*(SIZE*SIZE)
        self.assertTrue(CenteredDisplay.agrees(None,a,b))
        self.assertFalse(CenteredDisplay.agrees(None,a,b[:-1]))
    def test_restore(self):
        f=ScaledFake(); observed={}
        publish_transaction(f,SOURCE,SHA,f.identity(),lambda *_:None,lambda:None,
                            lambda name,data,target:observed.update({name:data}))
        self.assertEqual(len(observed['published']),BYTES)
        self.assertEqual(f.pixels,b'old contents');self.assertEqual(f.writes,2)
    def test_partial_copy_restores(self):
        f=ScaledFake();f.partial=True
        with self.assertRaises(OSError):publish_transaction(f,SOURCE,SHA,f.identity(),lambda *_:None,lambda:None)
        self.assertEqual(f.pixels,b'old contents');self.assertEqual(f.writes,2)
    def test_evidence_failure_restores(self):
        f=ScaledFake()
        def observe(*_):raise OSError('synthetic evidence loss')
        with self.assertRaises(OSError):publish_transaction(f,SOURCE,SHA,f.identity(),lambda *_:None,lambda:None,observe)
        self.assertEqual(f.pixels,b'old contents')
    def test_ownership_loss_no_foreign_restore(self):
        f=ScaledFake()
        def hold():f.target['epoch']=2
        with self.assertRaises(ValueError):publish_transaction(f,SOURCE,SHA,f.identity(),lambda *_:None,hold)
        self.assertEqual(f.writes,1)


if __name__ == '__main__': unittest.main()
