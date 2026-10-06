import hashlib
import tempfile
import unittest
from pathlib import Path
import struct
from verify import checked_file, pixels

class VerificationTests(unittest.TestCase):
    def fixture(self, shape):
        return b''.join(struct.pack('<I', 0xffff00ff if ((8<=y<=22 and 8<=x<=30-y) if shape=='triangle' else (8<=x<=23 and 8<=y<=23)) else 0) for y in range(32) for x in range(32))
    def test_triangle(self):
        self.assertEqual(pixels(self.fixture('triangle'),'triangle')['magenta_pixels'],120)
    def test_square(self):
        self.assertEqual(pixels(self.fixture('square'),'square')['magenta_pixels'],256)
    def test_wrong_scene(self):
        with self.assertRaises(ValueError): pixels(self.fixture('triangle'),'square')
    def test_truncated(self):
        with self.assertRaises(ValueError): pixels(b'\0'*4095,'square')
    def test_unexpected_color(self):
        b=bytearray(self.fixture('square'));b[0]=1
        with self.assertRaises(ValueError): pixels(b,'square')
    def test_unknown_shape(self):
        with self.assertRaises(ValueError): pixels(b'\0'*4096,'cube')
    def test_archive_identity(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d);(p/'a').write_bytes(b'abc')
            m={'bytes':3,'sha256':hashlib.sha256(b'abc').hexdigest()}
            self.assertEqual(checked_file(p,'a',m),b'abc')
            (p/'a').write_bytes(b'abd')
            with self.assertRaises(ValueError): checked_file(p,'a',m)
    def test_missing_file(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(FileNotFoundError): checked_file(Path(d),'missing',{})
    def test_path_escape(self):
        with tempfile.TemporaryDirectory() as d:
            with self.assertRaises(ValueError): checked_file(Path(d),'../outside',{})

if __name__=='__main__':unittest.main()
