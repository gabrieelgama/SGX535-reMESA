"""CPU construction regression tests; no device access."""
import copy
import json
from pathlib import Path
import struct
import tempfile
import unittest
import two_triangle as q

class Construction(unittest.TestCase):
    def setUp(self):
        self.m=q.scene(); self.p=q.bo.build(self.m)
        self.old=q.image.build(); self.op=q.bo.build(self.old)
    def test_vertices(self):
        raw=bytes(self.m['objects']['vertices']['bytes'])
        self.assertEqual(len(raw),128)
        self.assertEqual(raw[:96],bytes(self.old['objects']['vertices']['bytes']))
        self.assertEqual(struct.unpack('<8f',raw[96:]),(24.,24.,.5,1.,1.,1.,1.,1.))
    def test_indices(self):
        self.assertEqual(struct.unpack('<6H',bytes(self.m['objects']['indices']['bytes'])),q.INDICES)
        self.assertEqual(max(q.INDICES),3)
    def test_packet_view(self):
        self.assertEqual(self.m['objects']['triangle_index']['bytes'][:4],list(struct.pack('<I',0x81400006)))
        self.assertEqual(self.m['objects']['ta_stream']['bytes'][36:40],list(struct.pack('<I',0x81400006)))
    def test_only_geometry_objects_change(self):
        allowed={'vertices','indices','triangle_index','ta_stream'}
        self.assertEqual({k for k in self.old['objects'] if self.old['objects'][k]!=self.m['objects'][k]},allowed)
    def test_sizes_domains_unchanged(self):
        for k in self.op['bos']:
            for f in ('size','alignment','domain','owner'):
                self.assertEqual(self.p['bos'][k][f],self.op['bos'][k][f])
    def test_layout(self):
        self.assertEqual(self.p['bindings']['ta_stream']['offset'],128)
        self.assertEqual(self.p['bindings']['triangle_index']['offset'],164)
        self.assertEqual(self.p['bindings']['indices']['offset'],512)
        self.assertTrue(q.bo.check(self.m,self.p))
    def test_relocations(self):
        changes=[(a,b) for a,b in zip(self.op['wire_relocations'],self.p['wire_relocations']) if a!=b]
        self.assertEqual(len(changes),8)
        for a,b in changes:
            fields=[k for k in q.FIELDS if a[k]!=b[k]]
            self.assertIn(fields,[['where'],['pre_add']])
            if fields==['where']: self.assertEqual(b['where'],a['where']+8)
            else: self.assertEqual((a['pre_add'],b['pre_add']),(96,128))
    def test_corrupt_plan_rejected(self):
        self.p['bindings']['ta_stream']['offset']=96
        with self.assertRaises(ValueError): q.bo.check(self.m,self.p)
    def test_corrupt_relocation_rejected(self):
        self.p['wire_relocations'][0]['where']+=1
        with self.assertRaises(ValueError): q.bo.check(self.m,self.p)
    def test_raw_oracle(self):
        words=struct.unpack('<1024I',q.expected_image())
        self.assertEqual(words.count(0xffff00ff),256)
        self.assertEqual(words.count(0),768)
        self.assertEqual(set(words),{0,0xffff00ff})
    def test_old_footprint_not_quad(self):
        points={(x,y) for y in range(8,23) for x in range(8,31-y)}
        expected={(x,y) for y in range(8,24) for x in range(8,24)}
        self.assertEqual(len(points),120)
        self.assertEqual(len(expected-points),136)
    def test_exclusive_deterministic_generation(self):
        with tempfile.TemporaryDirectory() as t:
            a=Path(t)/'a'; b=Path(t)/'b'
            q.generate(a); q.generate(b)
            self.assertEqual({p.name:p.read_bytes() for p in a.iterdir()},
                             {p.name:p.read_bytes() for p in b.iterdir()})
            with self.assertRaises(ValueError): q.generate(a)
    def test_no_device_api(self):
        text=Path(q.__file__).read_text()
        for forbidden in ('/dev/','ioctl(', 'ssh ', 'subprocess'):
            self.assertNotIn(forbidden,text)
    def test_quad_interpretation(self):
        s=q.inspect_readback(q.expected_image())
        self.assertEqual((s['classification'],s['magenta_pixels'],s['bbox_inclusive']),('EXACT_CPU_QUAD_ORACLE',256,[8,8,23,23]))
    def test_zero_interpretation(self):
        self.assertEqual(q.inspect_readback(bytes(4096))['classification'],'ALL_ZERO')
    def test_baseline_interpretation(self):
        old=b''.join((0xffff00ff if 8<=y<=22 and 8<=x<=30-y else 0).to_bytes(4,'little') for y in range(32) for x in range(32))
        self.assertEqual(q.inspect_readback(old)['classification'],'BASELINE_TRIANGLE_ONLY')
    def test_unknown_color(self):
        raw=bytearray(q.expected_image());raw[4*(8*32+8):4*(8*32+8)+4]=struct.pack('<I',0xff00ff00)
        self.assertEqual(q.inspect_readback(raw)['classification'],'UNEXPECTED_COVERAGE')
    def test_wrong_region(self):
        raw=bytearray(4096);raw[:4]=struct.pack('<I',0xffff00ff)
        self.assertEqual(q.inspect_readback(raw)['classification'],'UNEXPECTED_COVERAGE')
    def test_incomplete_readback(self):
        for size in (0,4095,4097):
            with self.assertRaises(ValueError):q.inspect_readback(bytes(size))
    def test_pixel_analysis_does_not_invent_provenance(self):
        self.assertEqual(q.inspect_readback(q.expected_image())['provenance'],'NOT ESTABLISHED BY THIS CPU ANALYSIS')

if __name__=='__main__': unittest.main()
