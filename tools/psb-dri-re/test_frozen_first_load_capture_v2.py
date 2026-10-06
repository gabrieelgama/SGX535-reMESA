"""Run the actual v2 wrapper with mocked read-only clocks and remote child."""
import contextlib,io,json,subprocess,unittest,tempfile
from pathlib import Path
from unittest.mock import patch
import frozen_first_load_capture as v1
try:import frozen_first_load_capture_v2 as v2
except ImportError:v2=None

class CaptureV2Tests(unittest.TestCase):
 def execute(self,mode='stock_preparation',uptimes=(2000,2001),times=(10,11),ids=('b','b'),child=None):
  self.assertIsNotNone(v2,'v2 capture implementation missing')
  out=io.StringIO();err=io.StringIO();up=iter(uptimes);clock=iter(times);boot=iter(ids)
  def opened(path,*a,**kw):
   return io.StringIO(str(next(up))+' 0' if path=='/proc/uptime' else next(boot))
  child=child or subprocess.CompletedProcess([],0,'{"guards":[{"pass":true}],"boot_id":"b","kernel":"5.10.240-antix.1-486-smp","architecture":"i686"}\n','')
  with patch('builtins.open',side_effect=opened),patch('time.monotonic',side_effect=lambda:next(clock)),patch('os.uname',return_value=type('U',(),{'release':'5.10.240-antix.1-486-smp','machine':'i686'})()),patch.object(subprocess,'run',**({'side_effect':child} if isinstance(child,BaseException) else {'return_value':child})) as call,contextlib.redirect_stdout(out),contextlib.redirect_stderr(err):
   with self.assertRaises(SystemExit) as ex:exec(compile(v2.wrapper_source('print(1)',mode),'actual-v2-wrapper','exec'),{})
  records=[];s=out.getvalue();dec=json.JSONDecoder()
  while s.strip():s=s.lstrip();r,n=dec.raw_decode(s);records.append(r);s=s[n:]
  return ex.exception.code,records,err.getvalue(),call
 def test_old_stock_boot_age_does_not_prevent_bounded_preparation(self):
  status,rows,err,call=self.execute();self.assertEqual(status,0);self.assertTrue(rows[-1]['same_boot_timing_pass']);self.assertEqual(rows[-1]['uptime_end'],2001)
 def test_cycle_capture_accepts_readiness_after120(self):
  status,rows,_,call=self.execute('experimental',uptimes=(500,501));self.assertEqual(status,0);self.assertEqual(rows[-1]['duration_seconds'],1)
 def test_cycle_deadline_refuses_before_privilege(self):
  for age in [1200,1201,float('nan'),float('inf'),-1]:
   with self.subTest(age=age):
    status,rows,_,call=self.execute('experimental',uptimes=(age,));self.assertNotEqual(status,0);call.assert_not_called()
 def test_late_completion_changed_boot_clock_regression_rejected(self):
  for kw in [{'uptimes':(1199,1201)},{'ids':('b','c')},{'uptimes':(500,499)},{'times':(10,51)},{'times':(10,9)}]:
   with self.subTest(kw=kw):
    status,rows,_,_=self.execute('experimental',**kw);self.assertNotEqual(status,0)
 def test_uncached_sudo_retains_prefix_and_never_sends_credentials(self):
  status,rows,err,call=self.execute(child=subprocess.CompletedProcess([],1,'','sudo: a password is required\n'));self.assertEqual(status,1);self.assertEqual(rows[0]['boot_id'],'b');self.assertIn('password is required',err);self.assertEqual(call.call_args.kwargs['stdin'],subprocess.DEVNULL);self.assertNotIn('input',call.call_args.kwargs);self.assertEqual(call.call_count,1)
 def test_cached_sudo_child_is_direct_noninteractive_python(self):
  _,_,_,call=self.execute();self.assertEqual(call.call_args.args[0],['sudo','-n','-p','','python3','-I','-B','-S','-c','print(1)']);self.assertLessEqual(call.call_args.kwargs['timeout'],35)
 def test_timeout_preserves_partial_evidence_without_retry(self):
  # Raw partial output need not be valid JSON. Exercise it independently.
  self.assertIsNotNone(v2);out=io.StringIO();err=io.StringIO()
  with patch('builtins.open',side_effect=lambda p,*a:io.StringIO('200 0' if p=='/proc/uptime' else 'b')),patch('time.monotonic',return_value=10),patch('os.uname',return_value=type('U',(),{'release':'k','machine':'i686'})()),patch.object(subprocess,'run',side_effect=subprocess.TimeoutExpired([],35,output=b'partial\n',stderr=b'error\n')) as call,contextlib.redirect_stdout(out),contextlib.redirect_stderr(err):
   with self.assertRaises(SystemExit):exec(v2.wrapper_source('print(1)','stock_preparation'),{})
  self.assertIn('partial',out.getvalue());self.assertIn('error',err.getvalue());self.assertEqual(call.call_count,1)
 def test_readiness_requires_current_boot_sudo_userspace_and_display(self):
  self.assertIsNotNone(v2)
  valid={'local_sudo_succeeded':True,'normal_userspace':True,'physical_display_normal':True}
  self.assertEqual(v2.require_ready(valid),40)
  for key in valid:
   bad=valid.copy();bad.pop(key)
   with self.subTest(key=key),self.assertRaises(ValueError):v2.require_ready(bad)
 def test_suspension_or_forward_clock_jump_is_not_ignored(self):
  status,rows,_,_=self.execute(uptimes=(2000,2010),times=(10,11));self.assertNotEqual(status,0)
 def test_receipt_rejects_disagreeing_kernel_or_architecture(self):
  self.assertIsNotNone(v2);_,rows,_,_=self.execute()
  for field in ['kernel','architecture']:
   mutant=[dict(x) for x in rows];mutant[1][field]='different'
   with self.subTest(field=field),self.assertRaises(ValueError):v2.validate_receipt(mutant,0,b'',1,'stock_preparation')
 def test_receipt_accepts_kernel_release_without_mutating_records(self):
  _,rows,_,_=self.execute()
  rows[1]['kernel_release']=rows[1].pop('kernel')
  before=json.dumps(rows,sort_keys=True)
  self.assertEqual(v2.validate_receipt(rows,0,b'',1,'stock_preparation')['classification'],'PASS RAW CAPTURE ONLY')
  self.assertEqual(json.dumps(rows,sort_keys=True),before)
 def test_receipt_requires_agreement_when_both_kernel_fields_exist(self):
  _,rows,_,_=self.execute()
  rows[1]['kernel_release']=rows[1]['kernel']
  self.assertEqual(v2.validate_receipt(rows,0,b'',1,'stock_preparation')['boot_id'],'b')
  for field in ['kernel','kernel_release']:
   for value in ['different','',None,0]:
    mutant=[dict(x) for x in rows];mutant[1][field]=value
    with self.subTest(field=field,value=value),self.assertRaisesRegex(ValueError,'capture kernel drift'):
     v2.validate_receipt(mutant,0,b'',1,'stock_preparation')
 def test_kernel_release_receipt_still_rejects_identity_and_guard_failures(self):
  _,rows,_,_=self.execute()
  rows[1]['kernel_release']=rows[1].pop('kernel')
  for field,value in [('kernel_release','different'),('architecture','different'),('boot_id','different'),('guards',[{'pass':False}])]:
   mutant=[dict(x) for x in rows];mutant[1][field]=value
   with self.subTest(field=field),self.assertRaises(ValueError):
    v2.validate_receipt(mutant,0,b'',1,'stock_preparation')
 def test_receipt_rejects_target_duration_outside_host_interval(self):
  self.assertIsNotNone(v2);_,rows,_,_=self.execute()
  for host_duration in [0,0.99]:
   with self.subTest(host_duration=host_duration),self.assertRaises(ValueError):
    v2.validate_receipt(rows,0,b'',host_duration,'stock_preparation')
  self.assertEqual(v2.validate_receipt(rows,0,b'',1,'stock_preparation')['classification'],'PASS RAW CAPTURE ONLY')
 def test_capture_controller_claims_attempt_before_connection_and_never_retries(self):
  self.assertIsNotNone(v2);self.assertTrue(callable(getattr(v2,'capture_once',None)),'single-attempt controller missing')
  with tempfile.TemporaryDirectory() as tmp,patch.object(subprocess,'run',return_value=subprocess.CompletedProcess([],1,b'',b'refused')) as call:
   d=Path(tmp)/'one';ready={'normal_userspace':True,'local_sudo_succeeded':True,'physical_display_normal':True}
   with self.assertRaises(ValueError):v2.capture_once(d,['ssh','reviewed-test-only'],'print(1)',ready,'stock_preparation')
   with self.assertRaises(FileExistsError):v2.capture_once(d,['ssh','reviewed-test-only'],'print(1)',ready,'stock_preparation')
   self.assertEqual(call.call_count,1);self.assertEqual(call.call_args.kwargs['stdin'],subprocess.DEVNULL);self.assertEqual((d/'stderr.txt').read_bytes(),b'refused')
 def test_controller_preserves_partial_timeout_evidence(self):
  self.assertIsNotNone(v2);self.assertTrue(callable(getattr(v2,'capture_once',None)),'single-attempt controller missing')
  with tempfile.TemporaryDirectory() as tmp,patch.object(subprocess,'run',side_effect=subprocess.TimeoutExpired([],40,output=b'partial',stderr=b'error')) as call:
   d=Path(tmp)/'one';ready={'normal_userspace':True,'local_sudo_succeeded':True,'physical_display_normal':True}
   with self.assertRaises(ValueError):v2.capture_once(d,['ssh','reviewed-test-only'],'print(1)',ready,'stock_preparation')
   self.assertEqual((d/'stdout.txt').read_bytes(),b'partial');self.assertEqual(json.loads((d/'command.json').read_text())['exit_status'],124);self.assertEqual(call.call_count,1)
 def test_v1_still_rejects120(self):
  with self.assertRaises(ValueError):v1.require_ready({'local_sudo_succeeded':True,'normal_userspace':True,'elapsed_seconds':120})
 def test_receipt_requires_complete_same_boot_nonfault_root(self):
  self.assertIsNotNone(v2);_,rows,_,_=self.execute();self.assertEqual(v2.validate_receipt(rows,0,b'',1,'stock_preparation')['boot_id'],'b')
  for mutant in [rows[:1],rows+[{}],[rows[0],{'boot_id':'b','guards':[{'pass':False}]},rows[-1]],[rows[0],{'boot_id':'c','guards':[{'pass':True}]},rows[-1]]]:
   with self.subTest(mutant=mutant),self.assertRaises(ValueError):v2.validate_receipt(mutant,0,b'',1,'stock_preparation')
  for status,err,duration in [(1,b'',1),(0,b'warning',1),(0,b'',41),(0,b'',float('nan'))]:
   with self.subTest(status=status,err=err,duration=duration),self.assertRaises(ValueError):v2.validate_receipt(rows,status,err,duration,'stock_preparation')
 def test_actual_process_stat_parser_handles_parentheses_and_never_calls_start_readiness(self):
  self.assertIsNotNone(v2);fields=['S']+['0']*18+['12345']+['0']*5
  got=v2.process_start_record('42 (odd ) name) '+' '.join(fields),100)
  self.assertEqual(got,{'pid':42,'start_ticks':12345,'clock_ticks_per_second':100,'process_start_since_boot_seconds':123.45,'meaning':'process start, not userspace readiness'})
  for text,hz in [('broken',100),('42 (x) S',100),('42 (x) '+' '.join(fields),0)]:
   with self.subTest(text=text,hz=hz),self.assertRaises(ValueError):v2.process_start_record(text,hz)
