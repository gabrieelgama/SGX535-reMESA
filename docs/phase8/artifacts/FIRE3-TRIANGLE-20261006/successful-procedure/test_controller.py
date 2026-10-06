import unittest,ast,sys,os,json,contextlib,io,uuid
from pathlib import Path
import controller as c
class TestController(unittest.TestCase):
 def setUp(self):self.original_witness=c.WITNESS
 def tearDown(self):c.WITNESS=self.original_witness
 def parser(self,phase,args):
  s=c.early(phase);nodes=[]
  for n in ast.parse(s).body:
   if isinstance(n,(ast.Import,ast.ImportFrom)):nodes.append(n)
   elif isinstance(n,ast.Assign) and isinstance(n.targets[0],ast.Name) and n.targets[0].id in ['p','args']:nodes.append(n)
   elif isinstance(n,ast.Expr) and isinstance(n.value,ast.Call) and isinstance(n.value.func,ast.Attribute) and isinstance(n.value.func.value,ast.Name) and n.value.func.value.id=='p':nodes.append(n)
  ns={};prev=sys.argv[:]
  try:
   sys.argv=['test']+args
   with contextlib.redirect_stderr(io.StringIO()):exec(compile(ast.fix_missing_locations(ast.Module(body=nodes,type_ignores=[])),'parser-only','exec'),ns)
   return ns['args']
  finally:sys.argv=prev
 def test_supplied_args_survive(self):
  for phase,key,flag in [('incoming','stock_boot','--prepare'),('stage','stock_boot','--apply'),('first-owner','prior_boot',None)]:
   args=([flag] if flag else [])+['--'+key.replace('_','-'),c.B['stock_boot']]
   c.dispatch_args(phase,args);self.assertEqual(getattr(self.parser(phase,args),key),c.B['stock_boot'])
 def test_alternate_explicit_parser_values_not_overridden(self):
  for phase,key,flag in [('incoming','stock_boot','--prepare'),('stage','stock_boot','--apply'),('first-owner','prior_boot',None)]:
   for fresh in ['00000000-1111-2222-3333-444444444444','ffffffff-1111-2222-3333-444444444444']:
    args=([flag] if flag else [])+['--'+key.replace('_','-'),fresh]
    self.assertEqual(getattr(self.parser(phase,args),key),fresh)
    with self.assertRaises(ValueError):c.dispatch_args(phase,args)
 def test_positional_duplicate_missing_and_historical_refused(self):
  good=['--prepare','--stock-boot',c.B['stock_boot']]
  for args in [[],good+['8cc7f919-58e3-4eda-b941-f8e356a370c5'],good+['--stock-boot','8cc7f919-58e3-4eda-b941-f8e356a370c5'],['--prepare','--stock-boot','8cc7f919-58e3-4eda-b941-f8e356a370c5'],['--stock-boot',c.B['stock_boot']]]:
   with self.assertRaises(ValueError):c.dispatch_args('incoming',args)
 def test_environment_cannot_rebind(self):
  before=json.dumps(c.B,sort_keys=True);names=['STOCK_BOOT','BOOT','IMAGE_SHA','DRIVER_BUILD_ID','PYTHONPATH','SGX535_IMAGE','SGX535_EVIDENCE']
  old={k:os.environ.get(k) for k in names}
  try:
   for k in names:os.environ[k]='HISTORICAL_OR_WRONG'
   self.assertEqual(c.dispatch_args('stage',['--apply','--stock-boot',c.B['stock_boot']]),['--apply','--stock-boot',c.B['stock_boot']]);self.assertEqual(json.dumps(c.B,sort_keys=True),before)
  finally:
   for k,v in old.items():
    if v is None:os.environ.pop(k,None)
    else:os.environ[k]=v
 def test_all_forced_argv_removed(self):
  self.assertEqual(sum(c.remove_argv_overrides(c.read_source(p))[1] for p in ['incoming','stage','first-owner']),4)
  for p in ['incoming','stage','first-owner']:
   self.assertFalse(any(isinstance(n,ast.Assign) and any('sys.argv' in ast.unparse(t) for t in n.targets) for n in ast.walk(ast.parse(c.early(p)))))
 def test_missing_future_boot_refused(self):
  for bad in [c.B['stock_boot'],c.OLD_BOOT,'bad']:
   with self.assertRaises(ValueError):c.candidate_context(bad,'synthetic', '0'*64)
  with self.assertRaises(ValueError):c.late('protected',context={})
 def fixtures(self):
  boot='12345678-1234-1234-1234-123456789abc';ctx=c.candidate_context(boot,'SYNTHETIC TEST ONLY',c.OLD_SOURCE);base=c.B['protected_base'];ev=base+'/evidence-'+boot
  expected={'boot_id':boot,'base':{'path':base,'dev':1,'ino':10},'evidence':{'path':ev,'dev':1,'ino':11,'uid':0,'gid':0,'mode':'0o700'},'client':{'dev':1,'ino':12}}
  import base64
  raw=base64.b64decode(json.loads((Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/live-fire3-fresh-transfer-20261006T052417Z/first-owner/result.json')).read_text())['source_guard_preparation']['payload_base64'])
  observation={'sha256':c.OLD_SOURCE,'payload_base64':base64.b64encode(raw).decode(),'decoded':{'supplied_record_consistency':'CONFIRMED'}}
  expected['source_observations']=[observation.copy(),observation.copy()]
  expected['operations']=[{'name':'source.preparation.original.txt','phase':'created'},{'name':'source.preparation.original.txt','phase':'verified','sha256':c.OLD_SOURCE}]
  c.WITNESS={k:ctx[k] for k in ['boot_id','module_build_id','observer_build_id','image_sha256','source_boundary']}
  card={'boot_id':boot,'driver':c.B['candidate']['driver'],'observer':c.B['candidate']['observer'],'image':c.B['candidate']['image'],'witness':c.WITNESS,'synthetic':True}
  return ctx,expected,card
 def test_late_phases_fresh_and_no_stale_defaults(self):
  ctx,e,card=self.fixtures()
  for phase in ['protected','final','precheck','fire','retrieve']:
   s=c.late(phase,context=ctx,expected=e,card=card,pin=e['evidence']);compile(s,phase,'exec')
   self.assertNotIn(c.OLD_BOOT,s);self.assertNotIn(c.OLD_BASE,s);self.assertNotIn(c.OLD_NOTE,s)
 def test_wrong_current_receipts_and_card_refused(self):
  ctx,e,card=self.fixtures()
  bad=json.loads(json.dumps(e));bad['evidence']['path']=c.OLD_BASE
  with self.assertRaises(ValueError):c.late('final',context=ctx,expected=bad)
  bad=json.loads(json.dumps(card));bad['boot_id']=c.OLD_BOOT
  with self.assertRaises(ValueError):c.late('fire',context=ctx,expected=e,card=bad)
  with self.assertRaises(ValueError):c.late('retrieve',context=ctx,pin={'path':c.OLD_BASE})
 def test_guard_checks_not_removed(self):
  def checks(s):return [ast.dump(n,include_attributes=False) for n in ast.walk(ast.parse(s)) if isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id in ['need','check','require']]
  for p in ['incoming','stage','first-owner']:
   self.assertEqual(checks(c.read_source(p)),checks(c.early(p)))
 def test_poststage_exact_phase_bindings_and_history_retained(self):
  b=c.B;plan=b['plan'];operations=[]
  for i,(path,size,digest) in enumerate([(plan['image'],b['candidate']['image']['bytes'],b['candidate']['image']['sha256']),(plan['backup'],b['stock_custom_cfg']['size'],b['stock_custom_cfg']['sha256']),(plan['pending'],b['merged_config']['bytes'],b['merged_config']['sha256'])]):operations.append({'path':path,'dev':2049,'ino':9000+i,'size':size,'sha256':digest,'uid':0,'gid':0,'mode':'0o644','nlink':1})
  stage={'classification':'FROZEN CANDIDATE STAGING PASS; NO BOOT/SGX ACTION','operations':operations,'publication':{'source':plan['pending'],'destination':'/boot/grub/custom.cfg','phase':'directory-synced'}}
  s=c.poststage_source(stage,{'dev':2049,'ino':8000});compile(s,'synthetic-poststage','exec')
  self.assertIn(b['candidate']['image']['sha256'],s);self.assertIn(b['merged_config']['sha256'],s);self.assertIn('269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8',s)
 def test_transfer_inputs_exact_no_boot_override(self):
  s=c.transfer_source('diagnostic-entry.proposed',c.B['entry']['bytes'],c.B['entry']['sha256'],{'dev':1,'ino':2});compile(s,'transfer','exec')
  self.assertNotIn('sys.argv=',s);self.assertIn(c.B['stock_boot'],s)
  with self.assertRaises(ValueError):c.transfer_source('../other',1,'a'*64,{})
 def test_no_client_invocation_in_preparation_sources(self):
  for p in ['incoming','stage','first-owner']:
   s=c.early(p);self.assertNotIn('Popen',s);self.assertNotIn('--one-shot-sgx535-rev121',s)
  s=c.freshness_source();compile(s,'freshness','exec');self.assertNotIn('Popen',s)
if __name__=='__main__':unittest.main(verbosity=2)
