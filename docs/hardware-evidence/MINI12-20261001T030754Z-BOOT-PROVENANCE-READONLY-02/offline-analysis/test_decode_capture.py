import base64
import hashlib
import unittest
from decode_capture import decode_records


def frame(label='module', data=b'test', path='/etc/test'):
    digest=hashlib.sha256(data).hexdigest()
    return (f'FILE_BEGIN {label} {path}\nsize={len(data)} mtime=fixture mode=644\n'
            f'{digest}  {path}\nBASE64_BEGIN\n').encode()+base64.b64encode(data)+b'\nBASE64_END\nFILE_END\n'


class DecodeTests(unittest.TestCase):
    def test_plus_in_real_boot_script_label_is_safe(self):
        p=decode_records(frame('_etc_grub_d_20_memtest86+'))[0]
        self.assertTrue(p['target_hash_verified'])
        self.assertEqual(p['data'],b'test')

    def test_hash_and_size_are_verified(self):
        p=decode_records(frame())[0]
        self.assertTrue(p['target_hash_verified'] and p['target_size_verified'])

    def test_duplicate_label_is_rejected(self):
        with self.assertRaises(ValueError): decode_records(frame()+frame())

    def test_path_traversal_label_is_rejected(self):
        with self.assertRaises(ValueError): decode_records(frame('../escape'))

    def test_invalid_base64_is_rejected(self):
        with self.assertRaises(ValueError): decode_records(frame().replace(b'dGVzdA==',b'????'))

    def test_changing_log_bytes_are_explicitly_unverified(self):
        p=decode_records(frame().replace(b'dGVzdA==',b'bmV3IQ=='))[0]
        self.assertFalse(p['target_hash_verified'])

    def test_source_mismatch_is_rejected(self):
        with self.assertRaises(ValueError):
            decode_records(frame().replace(b'  /etc/test',b'  /other'))


if __name__=='__main__': unittest.main()
