"""CPU-only structural validation; synthetic references do not prove hardware."""
import unittest,copy,ast,json,base64
from pathlib import Path
import controller as c
import witness_binding as w
class TestStructural(unittest.TestCase):
 def setUp(self):
  self.a=dict(c.WITNESS);self.ctx=c.candidate_context(self.a['boot_id'],'82/82 PASS',self.a['source_boundary']);self.h=self.a['source_boundary']
 def source(self,ctx=None,h=None,extra=''):
  return 'import hashlib\nBOOT='+repr(self.a['boot_id'])+'\nCONTEXT='+repr(self.ctx if ctx is None else ctx)+'\nneed(hashlib.sha256(source).hexdigest()=='+repr(self.h if h is None else h)+",'same prepared source')\n"+extra
 def fail(self,s,phase='protected'):
  with self.assertRaises(ValueError):w.validate(s,self.a,phase)
 def test_01_same_hash_two_expected_roles_pass(self):
  self.assertEqual(w.validate(self.source(),self.a,'protected')['expected_hash_references'],2)
 def test_02_conflicting_hashes_fail(self):self.fail(self.source(h='b'*64))
 def test_03_unexpected_duplicate_fail(self):self.fail(self.source(extra='EXTRA='+repr(self.h)+'\n'))
 def test_04_stale_reference_fail(self):
  ctx=dict(self.ctx,boot_id=c.OLD_BOOT);self.fail(self.source(ctx=ctx))
 def test_05_missing_witness_fail(self):
  ctx=dict(self.ctx);del ctx['source_boundary'];self.fail(self.source(ctx=ctx))
 def test_06_malformed_hash_or_reference_fail(self):
  for val in ['bad','A'*64,None,{'hash':self.h}]:
   with self.subTest(value=val):self.fail(self.source(ctx=dict(self.ctx,source_boundary=val)))
 def test_07_correct_hash_wrong_provenance_fail(self):
  for field in ['boot_id','module_build_id','observer_build_id','image_sha256']:
   with self.subTest(field=field):self.fail(self.source(ctx=dict(self.ctx,**{field:'incorrect'})))
 def test_08_duplicate_context_object_fail(self):self.fail(self.source(extra='CONTEXT='+repr(self.ctx)+'\n'))
 def test_09_duplicate_predicate_fail(self):self.fail(self.source(extra="need(hashlib.sha256(source).hexdigest()=="+repr(self.h)+",'duplicate')\n"))
 def fixture(self):
  raw=base64.b64decode(json.loads(Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/live-fire3-fresh-transfer-20261006T052417Z/first-owner/result.json').read_text())['source_guard_preparation']['payload_base64'])
  base=c.B['protected_base'];expected={'boot_id':self.a['boot_id'],'base':{'path':base},'evidence':{'path':base+'/evidence-'+self.a['boot_id']},'operations':[{'name':'source.preparation.original.txt','phase':'created'},{'name':'source.preparation.original.txt','phase':'verified','sha256':self.h}],'source_observations':[{'sha256':self.h,'payload_base64':base64.b64encode(raw).decode(),'decoded':{'supplied_record_consistency':'CONFIRMED'}} for i in range(2)]}
  card={'boot_id':self.a['boot_id'],'driver':c.B['candidate']['driver'],'observer':c.B['candidate']['observer'],'image':c.B['candidate']['image'],'witness':self.a,'SYNTHETIC':True}
  return expected,card
 def test_10_every_late_phase_real_same_hash_pass(self):
  e,card=self.fixture()
  for phase in ['protected','final','precheck','fire','retrieve']:
   s=c.late(phase,context=self.ctx,expected=e,card=card,pin=e['evidence']);compile(s,'synthetic','exec')
 def test_11_unexpected_observation_duplicate_fail(self):
  e,card=self.fixture();e['source_observations'].append(copy.deepcopy(e['source_observations'][0]))
  with self.assertRaises(ValueError):c.late('final',context=self.ctx,expected=e)
 def test_12_conflicting_persisted_hash_fail(self):
  e,card=self.fixture();e['operations'][1]['sha256']='b'*64
  with self.assertRaises(ValueError):c.late('final',context=self.ctx,expected=e)
 def test_13_stale_protected_receipt_fail(self):
  e,card=self.fixture();e['boot_id']=c.OLD_BOOT
  with self.assertRaises(ValueError):c.late('final',context=self.ctx,expected=e)
 def test_14_live_guard_predicates_retained(self):
  e,card=self.fixture()
  for phase in ['protected','final','precheck','fire']:
   def guards(s):return sum(isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id in ['need','require','check'] for n in ast.walk(ast.parse(s)))
   self.assertEqual(guards(c.read_source(phase)),guards(c.late(phase,context=self.ctx,expected=e,card=card)))
if __name__=='__main__':unittest.main(verbosity=2)
