import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
import struct
import unittest
import face_constant_plan as p

class FacePlan(unittest.TestCase):
    def test_magenta_baseline(self):
        self.assertEqual(struct.pack('<2I',*p.constant(0xffff00ff)).hex(),'ff001f00f1f1a7fc')
    def test_palette_roundtrips(self):
        for color in (0xffff00ff,0xff00ff00,0xffffff00):
            a,b=p.constant(color);self.assertEqual(a|(((b>>4)&31)<<21)|(((b>>12)&63)<<26),color)
            self.assertEqual(b&~0x0003f1f0,0xfca40001)
    def test_only_launch_group_changes(self):
        x=p.rebind('magenta','green');self.assertEqual(x['words'],[0x40,'R(secondary_pds)',0x30000,'R(primary_pds_green)|0x0c000000'])
        self.assertEqual(x['copy_dwords'],4)
    def test_supported_copy_constructor(self):
        self.assertEqual(p.rebind('magenta','green')['state_USE_words'],[0xa0000000,0x28a13001,0xa0200200,0xfb274000])
    def test_same_program_no_spurious_upload(self):
        self.assertEqual(p.rebind('magenta','magenta'),{'mask':0,'emission':'NONE'})
    def test_invalid_inputs(self):
        for c in (-1,1<<32,'0xff00ff00',True):
            with self.assertRaises(ValueError):p.constant(c)
        with self.assertRaises(ValueError):p.rebind('stale','green')
    def test_no_overclaim(self):
        x=p.plan();self.assertFalse(any(x['claims'].values()))
        self.assertEqual(x['palette']['green']['GPU_output'],'NOT YET OBSERVED')

if __name__=='__main__':unittest.main()
