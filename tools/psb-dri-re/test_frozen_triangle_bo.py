"""Pure host-model tests; supplied GPU addresses below are synthetic test inputs."""
import copy
import unittest
import frozen_triangle_image as image
import frozen_triangle_bo as bo

class BOTests(unittest.TestCase):
    def setUp(self):
        self.m=image.build();self.p=bo.build(self.m)
    def test_grouped_manifest(self):
        self.assertLess(len(self.p['bos']),len(self.m['objects']))
        self.assertEqual(self.p['bindings']['primary_pds']['offset'],0x160)
        self.assertEqual(self.p['bindings']['vertices']['bo'],self.p['bindings']['ta_stream']['bo'])
        self.assertNotEqual(self.p['bindings']['fragment_use']['bo'],self.p['bindings']['primary_pds']['bo'])
        bo.check(self.m,self.p)
    def test_absent_bo(self):
        del self.p['bos']['pds']
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_undersized(self):
        self.p['bos']['pds']['size']=32
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_bad_placement(self):
        self.p['bos']['pds']['domain']='VRAM'
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_bad_alignment(self):
        self.p['bindings']['primary_pds']['offset']+=1
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_missing_validation(self):
        self.p['validation'].pop()
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_missing_relocation(self):
        self.p['wire_relocations'].pop()
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_bad_target(self):
        self.p['wire_relocations'][0]['target_bo']='absent'
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_corrupt_control(self):
        self.p['wire_relocations'][0]['background']^=0x100000
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_conflicting_overlap(self):
        self.p['bindings']['secondary_pds']['offset']=0x160
        with self.assertRaises(ValueError):bo.check(self.m,self.p)
    def test_aliases_emit_no_duplicate_relocations(self):
        self.assertEqual(len(self.p['wire_relocations']),49)
        self.assertEqual(sum(r['op']==5 for r in self.p['wire_relocations']),9)
    def test_relocation_mask_preserves_control(self):
        self.assertEqual(bo.merge(0x123450,4,0,0xffffff,0xc000000),0xc012345)
        self.assertEqual(bo.merge(3,0,0,0xf,0x200000),0x200003)
    def test_wrong_dynamic_gpu_alignment(self):
        a=bo.test_addresses(self.p);a['pds']+=1
        with self.assertRaises(ValueError):bo.resolve(self.m,self.p,a,{0:0,1:1},0x80000000)
    def test_wrong_dynamic_domain(self):
        a=bo.test_addresses(self.p);a['pds']=0x40000000
        with self.assertRaises(ValueError):bo.resolve(self.m,self.p,a,{0:0,1:1},0x80000000)
    def test_local_cpu_interface_cannot_be_given_a_gpu_address(self):
        a=bo.test_addresses(self.p);a['control']=0x1000
        with self.assertRaises(ValueError):
            bo.resolve(self.m,self.p,a,{0:0,1:1},0x80000000)
    def test_invalid_use_reservation_rejected_before_backing_mutation(self):
        dirty={n:bytearray([0x5a])*v['size'] for n,v in self.p['bos'].items() if v['owner']=='user'}
        with self.assertRaises(ValueError):
            bo.resolve(self.m,self.p,bo.test_addresses(self.p),{0:0,1:1.0},0x80000000,backings=dirty)
        self.assertEqual(dirty['pds'][0],0x5a)
    def test_resolved_cpu_bos(self):
        out=bo.resolve(self.m,self.p,bo.test_addresses(self.p),{0:0,1:1},0x80000000)
        self.assertEqual(len(out['pds']),self.p['bos']['pds']['size'])
        self.assertEqual(out['pds'][0x168:0x180],bytes(24))
        self.assertEqual(out['pds'][0x190:0x198],bytes.fromhex('45030007000000af'))
    def test_dirty_full_allocations_are_zeroed_before_object_writes(self):
        dirty={n:bytearray([0x5a])*v['size'] for n,v in self.p['bos'].items() if v['owner']=='user'}
        with self.assertRaises(ValueError):bo.require_zeroed_user_backings(self.p,dirty)
        out=bo.resolve(self.m,self.p,bo.test_addresses(self.p),{0:0,1:1},0x80000000,backings=dirty)
        self.assertIs(out['pds'],dirty['pds'])
        self.assertEqual(out['pds'][0],0)
        self.assertEqual(out['pds'][-1],0)
        self.assertEqual(out['color'],bytearray(self.p['bos']['color']['size']))
    def test_partial_or_missing_backing_cannot_be_initialized(self):
        dirty={n:bytearray([0x5a])*v['size'] for n,v in self.p['bos'].items() if v['owner']=='user'}
        del dirty['pds'][-1]
        with self.assertRaises(ValueError):bo.zero_user_backings(self.p,dirty)
        dirty['pds'].append(0x5a)
        del dirty['color']
        with self.assertRaises(ValueError):bo.zero_user_backings(self.p,dirty)
    def test_provider_mmu_limit(self):
        with self.assertRaises(ValueError):
            bo.resolve(self.m,self.p,bo.test_addresses(self.p),{0:0,1:1},0x41000000)
    def test_provider_mmu_limit_required(self):
        with self.assertRaises(ValueError):
            bo.resolve(self.m,self.p,bo.test_addresses(self.p),{0:0,1:1},None)
    def test_submission_packing(self):
        handles={v['bo']:i+1 for i,v in enumerate(self.p['validation'])}
        w=bo.submission_words(self.m,self.p,handles,dict(validation_nodes=0x1000,scene_arg=0x2000,fence_reply=0x3000))
        self.assertEqual(len(w),36);self.assertEqual(w[20],49)
        self.assertEqual(w[9],handles['control']);self.assertEqual(w[12],w[15])
        self.assertEqual(w[13],w[16]);self.assertEqual(w[14],0)
    def test_submission_missing_handle(self):
        with self.assertRaises(ValueError):bo.submission_words(self.m,self.p,{},dict(validation_nodes=0x1000,scene_arg=0x2000,fence_reply=0x3000))
    def test_submission_rejects_aliasing_bo_handles(self):
        handles={v['bo']:1 for v in self.p['validation']}
        with self.assertRaises(ValueError):
            bo.submission_words(self.m,self.p,handles,dict(validation_nodes=0x1000,scene_arg=0x2000,fence_reply=0x3000))
    def test_validation_wire_size(self):
        rows=bo.validation_words(self.p)
        self.assertEqual(len(rows),6)
        self.assertTrue(all(len(r)==34 for r in rows))
        self.assertTrue(all(r[9]==0 for r in rows)) # no presumed hint
    def test_publication_unknown_prevents_complete(self):
        self.assertFalse(bo.closure(self.m,self.p)['complete'])
        self.assertEqual(self.p['publication']['gpu_visibility'],'CONDITIONAL')
    def test_required_unknown_cannot_be_suppressed(self):
        self.p['publication']['gpu_visibility']='CONFIRMED'
        with self.assertRaises(ValueError):bo.check(self.m,self.p)

if __name__=='__main__':unittest.main()
