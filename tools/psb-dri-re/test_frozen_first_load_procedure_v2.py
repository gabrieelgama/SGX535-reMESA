"""V2 evidence checks: no transport or target actions."""
import copy,json,unittest
from pathlib import Path
try:import frozen_first_load_procedure_v2 as v2
except ImportError:v2=None

class ProcedureV2Tests(unittest.TestCase):
 def setUp(self):
  from test_frozen_first_load_procedure import ProcedureTests
  helper=ProcedureTests();helper.setUp();self.helper=helper;self.old=helper.plan
 def record(self,recovery=False):
  f=self.helper.recovery() if recovery else self.helper.live();f.pop('elapsed_seconds')
  mode='stock_recovery' if recovery else 'experimental'
  f['userspace_seen_before_boot_watch']=True
  f['capture_records']=[{'phase':'unprivileged-boot-identity','mode':mode,'boot_id':f['boot_id'],'kernel':f['kernel'],'architecture':'i686','uptime_start':500},dict(boot_id=f['boot_id'],kernel=f['kernel'],architecture='i686',guards=[{'pass':True}]),{'phase':'passive-capture-end','mode':mode,'boot_id':f['boot_id'],'uptime_end':501,'duration_seconds':1,'same_boot_timing_pass':True}]
  f['capture_status']=0;f['capture_stderr']='';f['host_duration_seconds']=1
  return f
 def test_all_first_owner_evidence_still_required_without_manual_elapsed(self):
  self.assertIsNotNone(v2);out=v2.validate_first_owner(self.old,self.record());self.assertEqual(out['first_owner'],'PASS PROVIDED RECORD');self.assertFalse(out['sgx_authorized'])
  for key,val in [('selection_photo',False),('prior_boot_id',''),('log',''),('experimental_boots',2),('slimski_running',False),('loaded_note_sha256',self.old['original_note_sha256']),('userspace_seen_before_boot_watch',False),('kernel_log','BUG: fault'),('interrupts','17: gma500\n'),('sgx_actions',1),('hot_module_actions',1)]:
   f=self.record();f[key]=val
   with self.subTest(key=key),self.assertRaises(ValueError):v2.validate_first_owner(self.old,f)
 def test_recovery_still_requires_three_distinct_boots_and_stock_default(self):
  self.assertIsNotNone(v2);self.assertEqual(v2.validate_recovery(self.old,self.record(True))['recovery'],'PASS PROVIDED RECORD')
  for key,val in [('experimental_boot_id',None),('boot_id','stock-before'),('grub_env',{'next_entry':'experimental'}),('framebuffer_dimensions','640x480'),('stock_files_exact',False),('selection_photo',False)]:
   f=self.record(True);f[key]=val
   with self.subTest(key=key),self.assertRaises(ValueError):v2.validate_recovery(self.old,f)
 def test_preparation_receipt_cannot_qualify_experimental_boot(self):
  self.assertIsNotNone(v2);f=self.record()
  for row in [f['capture_records'][0],f['capture_records'][-1]]:row['mode']='stock_preparation'
  with self.assertRaises(ValueError):v2.validate_first_owner(self.old,f)
 def test_missing_late_or_fault_capture_is_refused(self):
  self.assertIsNotNone(v2)
  for mutation in ['missing','late','rootfault','changed','failed']:
   f=self.record()
   if mutation=='missing':f.pop('capture_records')
   elif mutation=='late':f['capture_records'][-1]['uptime_end']=1201
   elif mutation=='rootfault':f['capture_records'][1]['guards']=[{'pass':False}]
   elif mutation=='changed':f['capture_records'][1]['boot_id']='other'
   elif mutation=='failed':f['capture_status']=1
   with self.subTest(mutation=mutation),self.assertRaises(ValueError):v2.validate_first_owner(self.old,f)
 def test_exact_error_free_hook_trace_still_required(self):
  self.assertIsNotNone(v2)
  for extra in ['insmod: ERROR','SGX535-FIRSTLOAD HOLD','unexpected message']:
   f=self.record();f['log']+=extra+'\n'
   with self.subTest(extra=extra),self.assertRaises(ValueError):v2.validate_first_owner(self.old,f)
 def test_future_policy_drift_retry_or_restaging_is_rejected(self):
  self.assertIsNotNone(v2);p=v2.load_plan();v2.validate_plan(p)
  for key,val in [('sgx_actions',1),('max_experimental_boots',2),('restaging',True),('automatic_retry',True),('hot_restoration',True),('boot_watch_seconds',120)]:
   q=copy.deepcopy(p);q['policy'][key]=val
   with self.subTest(key=key),self.assertRaises(ValueError):v2.validate_plan(q)
