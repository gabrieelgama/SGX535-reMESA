"""Validate existing staged artifacts against actual exclusive-creation receipts."""
import copy,json,unittest
from pathlib import Path
import frozen_first_load_procedure as p
ROOT=Path(__file__).resolve().parents[2]
E=ROOT/'docs/hardware-evidence/MINI12-20261001T062604Z-FIRSTLOAD-CYCLE-02'
class ExistingStageTests(unittest.TestCase):
 def setUp(self):self.plan=p.load_plan(ROOT);self.created=json.loads((E/'staging/decoded-records.json').read_text());self.fresh=json.loads((E/'stock-recovery/stdout.txt').read_text())['destinations']
 def test_actual_stock_recovery_keeps_exact_created_files(self):self.assertIn('PASS',p.validate_existing_stage_files(self.plan,self.created,self.fresh)['classification'])
 def test_same_bytes_different_inode_refused(self):
  for field in ['dev','ino']:
   f=copy.deepcopy(self.fresh);f[next(iter(f))][field]+=1
   with self.subTest(field=field),self.assertRaises(ValueError):p.validate_existing_stage_files(self.plan,self.created,f)
 def test_missing_extra_or_duplicate_receipt_refused(self):
  for records in [self.created[:-4],self.created+[next(r for r in self.created if r.get('scope')=='boot' and r.get('phase')=='verified')]]:
   with self.assertRaises(ValueError):p.validate_existing_stage_files(self.plan,records,self.fresh)
  with self.assertRaises(ValueError):p.validate_existing_stage_files(self.plan,self.created,{})
 def test_hash_owner_type_mode_link_mutations_refused(self):
  for key,value in [('sha256','bad'),('uid',1000),('mode',0o666),('nlink',2),('symlink',True),('type','directory')]:
   f=copy.deepcopy(self.fresh);f[next(iter(f))][key]=value
   with self.subTest(field=key),self.assertRaises(ValueError):p.validate_existing_stage_files(self.plan,self.created,f)
if __name__=='__main__':unittest.main()
