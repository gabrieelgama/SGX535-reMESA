"""Selected target and auxiliary/bootstrap guards; no hardware actions."""
import copy
import unittest
import frozen_triangle_image as image
import frozen_triangle_contracts as c

class ContractTests(unittest.TestCase):
    def setUp(self):self.m=image.build()
    def test_target_extent(self):
        t=c.target();c.check_target(self.m,t)
        self.assertEqual(c.pixel_offset(t,31,31),4092)
        self.assertEqual(len({c.pixel_offset(t,x,y) for y in range(32) for x in range(32)}),1024)
    def test_channels(self):self.assertEqual(c.pack_rgba(0x12,0x34,0x56,0x78),bytes.fromhex('56341278'))
    def test_row_direction(self):self.assertEqual(c.pixel_offset(c.target(),0,0,True),3968)
    def test_bad_stride(self):
        t=c.target();t['pitch_pixels']=31
        with self.assertRaises(ValueError):c.check_target(self.m,t)
    def test_bad_tiling(self):
        t=c.target();t['layout']='TILED'
        with self.assertRaises(ValueError):c.check_target(self.m,t)
    def test_bad_descriptor(self):
        self.m['objects']['target_data']['fields'][0]['value']^=1
        with self.assertRaises(ValueError):c.check_target(self.m,c.target())
    def test_target_outside(self):
        with self.assertRaises(ValueError):c.pixel_offset(c.target(),32,0)
    def test_auxiliary_inventory(self):
        rows=c.auxiliary(self.m);c.check_auxiliary(self.m,rows)
        self.assertEqual(len(rows),11)
        self.assertTrue(all(r['cpu_image']=='DEFINED_WITH_SYMBOLIC_RELOCATIONS' for r in rows))
        self.assertTrue(all(r['coverage']=='UNKNOWN' for r in rows))
    def test_missing_auxiliary(self):
        rows=c.auxiliary(self.m);rows.pop()
        with self.assertRaises(ValueError):c.check_auxiliary(self.m,rows)
    def test_missing_auxiliary_initialization(self):
        self.m['objects']['event_pds']['bytes'][24]=None
        with self.assertRaises(ValueError):c.check_auxiliary(self.m,c.auxiliary(self.m))
    def test_auxiliary_false_promotion(self):
        rows=c.auxiliary(self.m);rows[0]['coverage']='CONFIRMED'
        with self.assertRaises(ValueError):c.check_auxiliary(self.m,rows)
    def test_descriptor_count_corruption(self):
        self.m['objects']['vertex_pds']['fields'][1]['value']^=1
        with self.assertRaises(ValueError):c.check_auxiliary(self.m,c.auxiliary(self.m))
    def test_ta_cookie(self):
        row=c.ta_cookie(8192,0x42000000,0x31000000)
        self.assertEqual(row[2:7],[8192,6656,6656,6656,6592])
        self.assertEqual(row[10:13],[4096,12287,12286])
    def test_ta_count_rejected(self):
        with self.assertRaises(ValueError):c.ta_cookie(0x61f,0x42000000,0x31000000)
    def test_ta_misalignment(self):
        with self.assertRaises(ValueError):c.ta_cookie(8192,0x42000000,0x31001000)
    def test_revision_branches(self):
        for raw,code,a74,a804 in [(7,107,0x5021900,0x100ffff),(8,108,0x5021900,0x100ffff),(9,109,0x5188200,0x100ffff),(0x101,111,0x5188200,0xffff),(0x103,113,0x5188200,0xffff)]:
            b=c.bootstrap(raw);self.assertEqual(b['cpu_branch_code'],code)
            self.assertEqual(b['register_values'][0xa74],a74);self.assertEqual(b['register_values'][0x804],a804)
            self.assertFalse(b['ready'])
    def test_observed_rev121_takes_zeroed_historical_default(self):
        b=c.bootstrap(0x00010201,provenance='QUALIFIED_SGX_REVISION_EVIDENCE')
        self.assertEqual(b['cpu_branch_code'],121)
        self.assertEqual(b['option_dword_indices'],[])
        self.assertEqual(b['register_values'][0xa74],0x05188200)
        self.assertEqual(b['register_values'][0x804],0x0000ffff)
        self.assertFalse(b['ready'])
        self.assertEqual(b['gate_b'],'BLOCKED')
    def test_unqualified_neighbor_does_not_gain_default_acceptance(self):
        with self.assertRaises(ValueError):
            c.bootstrap(0x00010202,provenance='QUALIFIED_SGX_REVISION_EVIDENCE')
    def test_unknown_revision_rejected(self):
        with self.assertRaises(ValueError):c.bootstrap(None)
    def test_unsupported_revision_rejected(self):
        with self.assertRaises(ValueError):c.bootstrap(0)
    def test_pci_revision_not_accepted(self):
        with self.assertRaises(ValueError):c.bootstrap(7,provenance='PCI_REVISION')
    def test_bootstrap_does_not_authorize_mmio(self):
        b=c.bootstrap(7);self.assertEqual(b['gate_b'],'BLOCKED');self.assertFalse(b['hardware_ready'])
    def test_publication_trace(self):
        rows=c.publication_trace(0x100ffff)
        self.assertEqual([r['write_offset'] for r in rows],[0xad4,0xae0,0x804])
        self.assertEqual([r['status_mask'] for r in rows],[0x44,1,0x4000000])
        self.assertTrue(c.check_poll_results([(True,True)]*3))
    def test_publication_timeout_rejected(self):
        with self.assertRaises(ValueError):c.check_poll_results([(True,True),(False,True),(True,True)])
    def test_publication_clear_timeout_rejected(self):
        with self.assertRaises(ValueError):c.check_poll_results([(True,True),(True,False),(True,True)])
    def test_missing_publication_phase_rejected(self):
        with self.assertRaises(ValueError):c.check_poll_results([(True,True)]*2)
    def test_complete_ignores_no_unknown(self):
        out=c.closure(self.m);self.assertFalse(out['complete']);self.assertEqual(out['B2'],'CLOSED')
        self.assertIn('L12',out['remaining']);self.assertIn('FT-SERVICE',out['remaining'])

if __name__=='__main__':unittest.main()
