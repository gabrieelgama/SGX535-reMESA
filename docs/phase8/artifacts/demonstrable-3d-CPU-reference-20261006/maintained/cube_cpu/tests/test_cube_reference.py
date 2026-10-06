import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
import math
from pathlib import Path
import struct
import tempfile
import unittest
import cube_reference as c

class CubeReference(unittest.TestCase):
    def test_mesh_normals(self):
        self.assertEqual(len(c.VERTICES),8);self.assertEqual(len(c.FACES),6)
        for name,ids,n,color in c.FACES:
            a,b,d=(c.VERTICES[i] for i in ids[:3]);u=[b[i]-a[i] for i in range(3)];v=[d[i]-a[i] for i in range(3)]
            cross=(u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0])
            self.assertGreater(c.dot(cross,n),0)
    def test_projection_center(self):
        p=c.transform(c.projection(),(0,0,-6,1));screen=c.viewport(p)
        self.assertEqual(screen[:2],(16,16));self.assertGreater(screen[2],0);self.assertLess(screen[2],1)
    def test_near_far(self):
        for z,want in ((-1,0),(-12,1)):
            self.assertAlmostEqual(c.viewport(c.transform(c.projection(),(0,0,z,1)))[2],want)
    def test_perspective(self):
        p=c.projection();near=c.viewport(c.transform(p,(1,0,-3,1)));far=c.viewport(c.transform(p,(1,0,-6,1)))
        self.assertAlmostEqual(near[0]-16,2*(far[0]-16))
    def test_rotation_preserves_lengths(self):
        for angle in (0,15,30,45,60):
            for v in c.VERTICES:
                w=c.transform(c.model(angle),(*v,1));self.assertAlmostEqual(c.dot(v,v),c.dot(w[:3],w[:3]))
    def test_three_visible_faces(self):
        f=c.frame(0);self.assertEqual(set(d['face'] for d in f['draws']),{'front','left','top'})
        self.assertEqual(sum(len(d['triangle_indices']) for d in f['draws']),6)
    def test_screen_bounds_and_packing(self):
        for angle in (0,15,30,45,60):
            f=c.frame(angle);raw=c.pack_vertices(f['vertices']);self.assertEqual(len(raw),256)
            for r in f['vertices']:
                x,y,z,w=r['screen'];self.assertTrue(0<x<32 and 0<y<32 and 0<z<1 and w==1)
            self.assertEqual(struct.unpack_from('<f',raw,12)[0],1)
    def test_clip_inside(self):
        p=[(-.5,-.5,0,1),(.5,-.5,0,1),(0,.5,0,1)];self.assertEqual(c.clip_polygon(p),p)
    def test_clip_outside(self):
        self.assertEqual(c.clip_polygon([(2,0,0,1),(3,0,0,1),(2,1,0,1)]),[])
    def test_clip_crossing(self):
        poly=c.clip_polygon([(-2,0,0,1),(0,-.5,0,1),(0,.5,0,1)])
        self.assertEqual(len(poly),4)
        self.assertTrue(all(c.dot(plane,p)>=-1e-12 for plane in c.PLANES for p in poly))
    def test_clip_each_plane(self):
        for plane in c.PLANES:
            axis=next(i for i in range(3) if plane[i]);v=[0.,0.,0.,1.];v[axis]=-plane[axis]*2
            poly=c.clip_polygon([tuple(v),(.2,-.2,0,1),(-.2,.2,0,1)])
            self.assertTrue(all(c.dot(p,t)>=-1e-12 for p in c.PLANES for t in poly))
    def test_zero_w_rejected(self):
        with self.assertRaises(ValueError):c.viewport((0,0,0,0))
    def test_nonfinite_rejected(self):
        for bad in (float('nan'),float('inf')):
            with self.assertRaises(ValueError):c.frame(bad)
            with self.assertRaises(ValueError):c.viewport((bad,0,0,1))
    def test_projection_domains(self):
        for args in ((0,1,1,12),(180,1,1,12),(45,0,1,12),(45,1,0,12),(45,1,12,1)):
            with self.assertRaises(ValueError):c.projection(*args)
    def test_diffuse_oracle(self):
        self.assertEqual(c.intensity((0,0,1),(0,0,1)),1)
        self.assertEqual(c.intensity((0,0,-1),(0,0,1)),0)
        self.assertAlmostEqual(c.intensity((1,0,0)),1/math.sqrt(14))
        with self.assertRaises(ValueError):c.intensity((0,0,0))
    def test_rotation_changes_lighting_reference(self):
        self.assertNotAlmostEqual(c.intensity(c.normal(c.model(0),(0,0,1))),c.intensity(c.normal(c.model(60),(0,0,1))))
    def test_distinct_reference_frames(self):
        frames=[c.reference_pixels(c.frame(a)) for a in (0,15,30,45,60)]
        self.assertEqual(len(set(frames)),5)
        self.assertGreaterEqual(len(set(struct.unpack('<1024I',frames[0]))),4)
    def test_deterministic_exclusive_fixture(self):
        with tempfile.TemporaryDirectory() as t:
            a=Path(t)/'a';b=Path(t)/'b';c.generate(a);c.generate(b)
            self.assertEqual({p.name:p.read_bytes() for p in a.iterdir()},{p.name:p.read_bytes() for p in b.iterdir()})
            with self.assertRaises(FileExistsError):c.generate(a)
    def test_claims_no_gpu(self):
        f=c.frame(0)
        self.assertFalse(f['claims']['SGX_execution']);self.assertFalse(f['claims']['hardware_depth']);self.assertFalse(f['claims']['lighting_rendered'])
    def test_reciprocal_w_not_confused_with_payload(self):
        f=c.frame(0)
        self.assertTrue(any(abs(r['reciprocal_clip_w']-1)>1e-3 for r in f['vertices']))
        self.assertTrue(all(r['screen'][3]==1 for r in f['vertices']))

if __name__=='__main__':unittest.main()
