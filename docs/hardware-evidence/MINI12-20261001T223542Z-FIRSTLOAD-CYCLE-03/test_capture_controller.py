import importlib.util,json,subprocess,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('actual_controller',HERE/'capture_once.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class ControllerTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.e=Path(self.tmp.name);self.old_e=m.E;self.old_w=m.W;m.E=self.e;m.W=self.e
  (self.e/'pinned-ssh-prefix.json').write_text(json.dumps(['ssh','-T','-o','BatchMode=yes','pinned-test-endpoint']))
 def tearDown(self):m.E=self.old_e;m.W=self.old_w;self.tmp.cleanup()
 def result(self):return {'guards':[{'pass':True}],'boot_id':'new-boot','destinations':json.loads((m.OLD/'stock-recovery/stdout.txt').read_text())['destinations']}
 def test_privileged_refusal_preserved_without_credential_input_or_retry(self):
  with patch.object(m.subprocess,'run',return_value=subprocess.CompletedProcess([],1,b'',b'sudo: a password is required\n')) as run:
   with self.assertRaises(RuntimeError):m.capture('preflight','print(1)',60)
   self.assertEqual(run.call_count,1);self.assertEqual(run.call_args.kwargs['stdin'],subprocess.DEVNULL);self.assertNotIn('input',run.call_args.kwargs)
   with self.assertRaises(FileExistsError):m.capture('preflight','print(1)',60)
   self.assertEqual(run.call_count,1)
  self.assertEqual((self.e/'preflight/stderr.txt').read_bytes(),b'sudo: a password is required\n')
 def test_successful_capture_has_same_boot_and_complete_timing_receipts(self):
  root=self.result();records=[{'phase':'unprivileged-boot-identity','boot_id':'new-boot'},root,{'phase':'passive-capture-end','boot_id':'new-boot','same_boot_deadline_pass':True}]
  out='\n'.join(json.dumps(x) for x in records).encode()
  with patch.object(m.subprocess,'run',return_value=subprocess.CompletedProcess([],0,out,b'')):
   self.assertEqual(m.capture('experimental','print(1)',30),root)
 def test_bad_end_receipt_rejected(self):
  out='\n'.join(json.dumps(x) for x in [{'phase':'unprivileged-boot-identity','boot_id':'new-boot'},self.result(),{'boot_id':'wrong','same_boot_deadline_pass':False}]).encode()
  with patch.object(m.subprocess,'run',return_value=subprocess.CompletedProcess([],0,out,b'')),self.assertRaises(RuntimeError):m.capture('experimental','print(1)',30)
 def test_timeout_retains_partial_output(self):
  with patch.object(m.subprocess,'run',side_effect=subprocess.TimeoutExpired([],30,output=b'partial output',stderr=b'partial error')),self.assertRaises(RuntimeError):m.capture('experimental','print(1)',30)
  self.assertEqual((self.e/'experimental/stdout.txt').read_bytes(),b'partial output')
  self.assertEqual(json.loads((self.e/'experimental/command.json').read_text())['exit_status'],124)
 def test_stock_capture_still_runs_after_failed_experimental_sudo(self):
  (self.e/'preflight-decision.json').write_text(json.dumps({'classification':'PASS','stock_boot_id':'old-boot'}))
  (self.e/'operator-stock-recovery-readiness.json').write_text(json.dumps({'normal_userspace':True,'local_sudo_succeeded':True,'elapsed_seconds':30,'stock_manually_selected':True,'physical_display_normal':True}))
  d=self.e/'experimental';d.mkdir();(d/'stdout.txt').write_text(json.dumps({'phase':'unprivileged-boot-identity','boot_id':'experimental-boot'}))
  (self.e/'stock-existing-preflight.py').write_text("if True:\n check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')\n")
  with patch.object(m,'capture',return_value=self.result()) as run:m.main('stock-recovery')
  self.assertEqual(run.call_count,1);self.assertIn('experimental-boot',run.call_args.args[1])
 def test_stock_capture_remains_possible_if_deadline_prevented_experimental_capture(self):
  (self.e/'preflight-decision.json').write_text(json.dumps({'classification':'PASS','stock_boot_id':'old-boot'}))
  (self.e/'operator-stock-recovery-readiness.json').write_text(json.dumps({'normal_userspace':True,'local_sudo_succeeded':True,'elapsed_seconds':30,'stock_manually_selected':True,'physical_display_normal':True}))
  (self.e/'stock-existing-preflight.py').write_text("if True:\n check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')\n")
  with patch.object(m,'capture',return_value=self.result()) as run:m.main('stock-recovery')
  self.assertEqual(run.call_count,1);self.assertFalse(json.loads((self.e/'stock-recovery-prior-identity.json').read_text())['full_ledger_identity_established'])
if __name__=='__main__':unittest.main()
