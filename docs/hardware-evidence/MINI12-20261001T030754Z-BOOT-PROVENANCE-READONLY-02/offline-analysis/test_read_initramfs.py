import gzip
import stat
import unittest
from read_initramfs import read_cpio,read_image


def member(name,data=b'',mode=stat.S_IFREG|0o644):
    n=name.encode()+b'\0'
    fields=[1,mode,0,0,1,0,len(data),0,0,0,0,len(n),0]
    head=b'070701'+b''.join(f'{x:08x}'.encode() for x in fields)+n
    head+=b'\0'*((-len(head))%4)
    payload=data+b'\0'*((-len(data))%4)
    return head+payload


def archive(name='init',data=b'fixture'):
    return member(name,data)+member('TRAILER!!!')


class InitramfsTests(unittest.TestCase):
    def test_early_newc_then_padded_gzip_main(self):
        early=archive('kernel/x86/microcode/GenuineIntel.bin',b'microcode')
        early+=b'\0'*((-len(early))%512)
        records,containers=read_image(early+gzip.compress(archive()))
        self.assertEqual([p['name'] for p in records],['kernel/x86/microcode/GenuineIntel.bin','init'])
        self.assertEqual(records[1]['data'],b'fixture')
        self.assertEqual([p['format'] for p in containers],['newc','gzip'])

    def test_absolute_path_is_rejected(self):
        with self.assertRaises(ValueError): read_cpio(archive('/etc/escape'))

    def test_parent_path_is_rejected(self):
        with self.assertRaises(ValueError): read_cpio(archive('etc/../escape'))

    def test_truncated_header_is_rejected(self):
        with self.assertRaises(ValueError): read_cpio(b'070701')

    def test_truncated_payload_is_rejected(self):
        with self.assertRaises(ValueError): read_cpio(member('init',b'payload')[:-4])

    def test_corrupt_gzip_is_rejected(self):
        with self.assertRaises(Exception): read_image(gzip.compress(archive())[:-8])

    def test_nonempty_trailing_unknown_data_is_rejected(self):
        with self.assertRaises(ValueError): read_image(archive()+b'unknown')


if __name__=='__main__': unittest.main()
