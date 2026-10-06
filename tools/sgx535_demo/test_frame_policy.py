import unittest
from dataclasses import fields,replace
from frame_policy import FramePolicy,RearmPremises

class Policy(unittest.TestCase):
    def prepared(self):
        p=FramePolicy()
        self.assertTrue(p.prepare(owned=True,construction_valid=True,source_valid=True,isolation_valid=True))
        return p
    def sealed(self):
        p=self.prepared(); self.assertTrue(p.admit(authorization=True,bindings_valid=True))
        self.assertTrue(p.complete(retired=True,ledger=7,provenance_valid=True))
        self.assertTrue(p.seal(originals_complete=True,hashes_valid=True));return p
    def test_clean_synthetic_interval(self):
        p=self.sealed();self.assertEqual(p.phase,'SEALED')
    def test_unknown_rearm_blocks(self):
        p=self.sealed();self.assertFalse(p.rearm(RearmPremises()));self.assertEqual(p.epoch,0)
    def test_each_missing_guarantee_blocks(self):
        all_ok=RearmPremises(**{f.name:True for f in fields(RearmPremises)})
        for f in fields(RearmPremises):
            with self.subTest(f=f.name):
                p=self.sealed();self.assertFalse(p.rearm(replace(all_ok,**{f.name:False})))
    def test_synthetic_provider_success(self):
        p=self.sealed();self.assertTrue(p.rearm(RearmPremises(**{f.name:True for f in fields(RearmPremises)})))
        self.assertEqual((p.phase,p.epoch),('NEW',1))
    def test_retirement_alone_not_rearm(self):
        p=self.prepared();p.admit(authorization=True,bindings_valid=True)
        p.complete(retired=True,ledger=7,provenance_valid=True)
        self.assertFalse(p.rearm(RearmPremises(**{f.name:True for f in fields(RearmPremises)})))
    def test_no_second_admission(self):
        p=self.prepared();self.assertTrue(p.admit(authorization=True,bindings_valid=True))
        self.assertFalse(p.admit(authorization=True,bindings_valid=True))
    def test_interference_before_admission(self):
        p=self.prepared();self.assertFalse(p.admit(authorization=True,bindings_valid=False))
    def test_no_authorization(self):
        p=self.prepared();self.assertFalse(p.admit(authorization=False,bindings_valid=True))
    def test_partial_completion_hold(self):
        for ledger in (0,1,3,5,6):
            p=self.prepared();p.admit(authorization=True,bindings_valid=True)
            self.assertFalse(p.complete(retired=True,ledger=ledger,provenance_valid=True))
            self.assertEqual(p.phase,'HOLD')
    def test_loss_blocks_reuse(self):
        p=self.prepared();p.admit(authorization=True,bindings_valid=True)
        p.complete(retired=True,ledger=7,provenance_valid=True)
        self.assertFalse(p.seal(originals_complete=False,hashes_valid=True))
        self.assertFalse(p.rearm(RearmPremises()))
    def test_prepare_requires_all_premises(self):
        for key in ('owned','construction_valid','source_valid','isolation_valid'):
            args=dict(owned=True,construction_valid=True,source_valid=True,isolation_valid=True);args[key]=False
            self.assertFalse(FramePolicy().prepare(**args))
    def test_truthy_unknown_is_not_evidence(self):
        p=self.sealed();all_ok=RearmPremises(**{f.name:True for f in fields(RearmPremises)})
        self.assertFalse(p.rearm(replace(all_ok,authoritative_source_boundary='UNKNOWN')))
        self.assertFalse(FramePolicy().prepare(owned=True,construction_valid=True,source_valid='UNKNOWN',isolation_valid=True))

if __name__=='__main__':unittest.main()
