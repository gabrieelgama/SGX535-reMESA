"""Host-only evidence/model regressions; never invokes historical code."""
import copy
import unittest
import frozen_triangle_image as f

class FrozenTriangleTests(unittest.TestCase):
    def test_selected_service_requests(self):
        m=f.build()
        self.assertEqual([r['op'] for r in m['service_requests']],[2,2])
        self.assertEqual([r['engine'] for r in m['service_requests']],[0,1])
        self.assertEqual([r['scene_flags'] for r in m['service_requests']],[4,15])
        self.assertEqual([r['fire_flags'] for r in m['service_requests']],[1,1])
        self.assertEqual(m['service_requests'][1]['cookie_reads'],[7,14])
        self.assertNotIn(15,m['service_requests'][1]['cookie_reads'])
    def test_wrong_sceneless_service(self):
        m=f.build();m['service_requests'][1]['op']=0
        with self.assertRaises(ValueError):f.validate(m)
    def test_missing_service_prerequisite(self):
        m=f.build();m['service_requests'][0]['prerequisites']=[]
        with self.assertRaises(ValueError):f.validate(m)
    def test_stream_and_submission(self):
        m=f.build()
        self.assertEqual(m['objects']['ta_stream']['size'],68)
        self.assertEqual(f.word(m,'ta_stream',64),0xc0000000)
        self.assertEqual(f.word(m,'submit_command',44),14)
        self.assertEqual(f.word(m,'submit_command',68),52)
        self.assertEqual(f.word(m,'submit_command',56),0)
        self.assertIsNone(m['objects']['submit_command']['bytes'][48])
    def test_background_use_flags(self):
        self.assertEqual(f.word(f.build(),'background_use',4),0x28851001)
    def test_auxiliary_zero_policy_is_explicit(self):
        m=f.build()
        for name,offsets in [('vertex_pds',[8,12,24,28,40,44]),
                             ('event_pds',[24,28,60]),
                             ('background_pds',[16,20,24,28,44])]:
            for off in offsets:
                self.assertEqual(f.word(m,name,off),0)
                field=m['objects'][name]['fields'][off//4]
                self.assertTrue(field['historical_producer_unwritten'])
                self.assertEqual(field['origin'],'ROUTE_C_CPU_ZERO_POLICY')
        self.assertIn('FT-AUX',f.remaining(m,assume_l12_closed=True))
    def test_auxiliary_initialization_omission(self):
        m=f.build();m['objects']['event_pds']['fields'][6]['origin']='UNKNOWN'
        with self.assertRaises(ValueError): f.validate(m)
    def test_target_zero_cpu_policy(self):
        m=f.build();o=m['objects']['color']
        self.assertEqual(o['bytes'],[0]*4096)
        self.assertEqual(o['status'],'CONDITIONAL')
        self.assertNotIn('FT-TARGET',o['blockers'])
        self.assertIn('FT-TARGET',m['closed_items'])
        self.assertIn('BACKEND',o['blockers'])
        self.assertIn('IMPLEMENTATION_POLICY',o['notes'])
    def test_every_relocation_is_dependency(self):
        m=f.build()
        for r in m['relocations']:
            self.assertIn(r['target'],m['objects'][r['owner']]['dependencies'])
    def test_vertices(self):
        m = f.build()
        self.assertEqual(len(m['objects']['vertices']['bytes']), 96)
        self.assertEqual(m['objects']['vertices']['bytes'][:8], [0,0,0,65,0,0,0,65])
        self.assertEqual(m['objects']['indices']['bytes'], [0,0,1,0,2,0])
    def test_count_correction(self):
        self.assertEqual(f.pack_state(f.bounds_state())[0], 0x54c5)
        self.assertEqual(len(f.pack_state(f.bounds_state())), 11)
        tri = f.triangle_state()
        self.assertEqual(tri[0], 0x5fc1)
        mask = f.diff_state(tri, f.bounds_state())
        self.assertEqual(mask, 0xf41)
        self.assertEqual(len(f.pack_state(tri, mask)), 14)
    def test_target(self):
        m=f.build()
        self.assertEqual(f.word(m,'target_data',0), 0xf8000)
        self.assertEqual(f.word(m,'target_data',12),0x1f01f)
        self.assertEqual(f.word(m,'raster_registers',28),0x20002)
        self.assertEqual(f.word(m,'raster_registers',196),4)
        self.assertEqual(len(m['objects']['raster_registers']['bytes']),208)
    def test_state_use(self):
        self.assertEqual(f.state_use(2),[0xa0000000,0x28a11001,0xa0200100,0xfb274000])
        self.assertEqual(f.state_use(14),[0xa0000000,0x28a1d001,0xa0200700,0xfb274000])
    def test_symbolic_not_zero_address(self):
        m=f.build()
        self.assertEqual(m['objects']['primary_pds']['bytes'][:4],[None]*4)
        self.assertEqual(m['objects']['primary_pds']['bytes'][8:32],[0]*24)
        self.assertTrue(m['relocations'])
        f.validate(m)
    def test_missing_object(self):
        m=f.build(); del m['objects']['fragment_use']
        with self.assertRaises(ValueError): f.validate(m)
    def test_missing_relocation(self):
        m=f.build(); m['relocations'].pop()
        with self.assertRaises(ValueError): f.validate(m)
    def test_unknown_complete(self):
        with self.assertRaisesRegex(
                ValueError, r'^PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE$'):
            f.validate(f.build(), complete=True)
    def test_alignment(self):
        m=f.build(); m['objects']['primary_pds']['placement']={'arena':'PDS','offset':1}
        with self.assertRaises(ValueError): f.validate(m)
    def test_constant_corruption(self):
        m=f.build(); m['objects']['secondary_pds']['bytes'][3]=0
        with self.assertRaises(ValueError): f.validate(m)
    def test_overlap(self):
        m=f.build()
        for n in ('primary_pds','secondary_pds'):
            m['objects'][n]['placement']={'arena':'PDS','offset':0}
        with self.assertRaises(ValueError): f.validate(m)
    def test_hypothetical(self):
        m=f.build(); self.assertIn('FT-SERVICE',f.remaining(m,assume_l12_closed=True))
        self.assertNotIn('L12',f.remaining(m,assume_l12_closed=True))
        self.assertIn('L12',f.remaining(m))
        self.assertFalse(m['hardware_ready'])
    def test_no_status_promotion(self):
        for key,value in [('hardware_ready',True),('l12','CLOSED'),('fg02','CLOSED')]:
            m=f.build(); m[key]=value
            with self.assertRaises(ValueError): f.validate(m)
    def test_new_unknown_field(self):
        m=f.build(); m['objects']['indices']['bytes'][0]=None
        with self.assertRaises(ValueError): f.validate(m)
    def test_improper_state_count(self):
        with self.assertRaises(ValueError): f.state_use(0)
        with self.assertRaises(ValueError): f.state_use(17)

if __name__=='__main__': unittest.main()
