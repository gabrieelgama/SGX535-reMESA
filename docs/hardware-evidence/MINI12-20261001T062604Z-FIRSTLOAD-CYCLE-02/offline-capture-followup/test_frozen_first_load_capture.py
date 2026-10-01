"""Execute actual proposed capture wrapper with local mocked read-only inputs."""
import contextlib,io,json,subprocess,unittest
from unittest.mock import patch
import frozen_first_load_capture as capture
class CaptureTransportTests(unittest.TestCase):
 def execute(self,cp,uptimes=(90.0,95.0),bootids=('boot-id-1','boot-id-1')):
  out=io.StringIO();err=io.StringIO();times=iter(uptimes);ids=iter(bootids)
  def opened(path,*args):
   return io.StringIO(str(next(times))+' 10.0' if path=='/proc/uptime' else next(ids))
  source=capture.wrapper_source('print("reviewed root capture")')
  with patch('builtins.open',side_effect=opened),patch.object(capture.os,'uname',return_value=type('U',(),{'release':'5.10.240-antix.1-486-smp','machine':'i686'})()),patch.object(subprocess,'run',**({'side_effect':cp} if isinstance(cp,BaseException) else {'return_value':cp})) as run,contextlib.redirect_stdout(out),contextlib.redirect_stderr(err):
   with self.assertRaises(SystemExit) as exit:exec(compile(source,'actual-wrapper','exec'),{})
  return out.getvalue(),err.getvalue(),exit.exception.code,run
 def test_cached_sudo_preserves_prefix_and_root_capture(self):
  out,err,status,run=self.execute(subprocess.CompletedProcess([],0,'{"root":true}\n',''))
  self.assertEqual(status,0);self.assertFalse(err);self.assertEqual(json.loads(out.splitlines()[0])['boot_id'],'boot-id-1');self.assertIn('{"root":true}',out);self.assertEqual(json.loads(out.splitlines()[-1])['uptime_end'],95.0)
 def test_uncached_sudo_preserves_boot_id_without_shell_input(self):
  out,err,status,run=self.execute(subprocess.CompletedProcess([],1,'','sudo: a password is required\n'))
  self.assertEqual(status,1);self.assertEqual(json.loads(out)['boot_id'],'boot-id-1');self.assertIn('password is required',err)
  self.assertEqual(run.call_args.kwargs['stdin'],subprocess.DEVNULL);self.assertNotIn('shell',run.call_args.kwargs)
 def test_missing_public_boot_id_stops_before_privilege(self):
  out=io.StringIO()
  with patch('builtins.open',side_effect=FileNotFoundError('boot ID absent')),patch.object(subprocess,'run') as run,contextlib.redirect_stdout(out):
   with self.assertRaises(SystemExit):exec(compile(capture.wrapper_source('print(1)'),'actual-wrapper','exec'),{})
  run.assert_not_called();self.assertEqual(json.loads(out.getvalue())['phase'],'STOP')
 def test_expired_kernel_clock_refused_before_privilege(self):
  out,err,status,run=self.execute(subprocess.CompletedProcess([],0,'',''),uptimes=(121.0,))
  self.assertEqual(status,1);run.assert_not_called();self.assertIn('STOP',out)
 def test_changed_boot_and_late_completion_refused(self):
  for kw in [{'bootids':('boot-id-1','boot-id-2')},{'uptimes':(90.0,121.0)},{'uptimes':(90.0,89.0)}]:
   with self.subTest(kw=kw):
    out,err,status,run=self.execute(subprocess.CompletedProcess([],0,'',''),**kw);self.assertEqual(status,1)
 def test_timeout_preserves_partial_root_output(self):
  out,err,status,run=self.execute(subprocess.TimeoutExpired([],30,output=b'partial capture\n',stderr=b'partial error\n'))
  self.assertEqual(status,1);self.assertIn('partial capture',out);self.assertIn('partial error',err);self.assertIn('boot-id-1',out)
 def test_invalid_root_capture_refused_offline(self):
  for source in ['', 'if (']:
   with self.subTest(source=source),self.assertRaises((ValueError,SyntaxError)):capture.wrapper_source(source)
 def test_child_argv_is_noninteractive_python_not_shell(self):
  _,_,_,run=self.execute(subprocess.CompletedProcess([],0,'',''))
  self.assertEqual(run.call_args.args[0],['sudo','-n','-p','','python3','-I','-B','-S','-c','print("reviewed root capture")'])
 def test_root_script_is_an_argument_never_stdin(self):
  _,_,_,run=self.execute(subprocess.CompletedProcess([],0,'',''));self.assertNotIn('input',run.call_args.kwargs);self.assertEqual(run.call_args.kwargs['stdin'],subprocess.DEVNULL)
 def test_ready_userspace_without_sudo_witness_refused(self):
  with self.assertRaises(ValueError):capture.require_ready({'normal_userspace':True,'elapsed_seconds':90})
 def test_explicit_ready_record_and_remaining_deadline(self):
  self.assertEqual(capture.require_ready({'local_sudo_succeeded':True,'normal_userspace':True,'elapsed_seconds':90}),30)
  for elapsed in [True,-1,float('nan'),float('inf'),120,121,None]:
   with self.subTest(elapsed=elapsed),self.assertRaises(ValueError):capture.require_ready({'local_sudo_succeeded':True,'normal_userspace':True,'elapsed_seconds':elapsed})
if __name__=='__main__':unittest.main()
