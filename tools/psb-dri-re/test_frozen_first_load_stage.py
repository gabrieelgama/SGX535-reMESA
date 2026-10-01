"""Exercise actual reviewed file-copy mechanics on isolated local directories only."""
import hashlib,io,os,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
import frozen_first_load_stage as stage
class StageTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.dir=Path(self.tmp.name);self.fd=os.open(self.dir,os.O_RDONLY|os.O_DIRECTORY);self.events=[];self.data=b'exact qualified bytes';self.digest=hashlib.sha256(self.data).hexdigest()
 def tearDown(self):os.close(self.fd);self.tmp.cleanup()
 def copy(self,data=None,**kw):
  return stage.copy_exclusive(self.fd,'artifact',io.BytesIO(self.data if data is None else data),len(self.data),self.digest,os.getuid(),os.getgid(),0o600,self.events.append,**kw)
 def test_exact_copy_readback_and_receipt(self):
  row=self.copy();self.assertEqual(row['sha256'],self.digest);self.assertEqual(row['nlink'],1);self.assertEqual(row['mode'],0o600);self.assertEqual((self.dir/'artifact').read_bytes(),self.data);self.assertEqual(self.events[0]['phase'],'created');self.assertEqual(self.events[-1]['phase'],'verified')
 def test_existing_file_not_overwritten(self):
  p=self.dir/'artifact';p.write_bytes(b'preserved')
  with self.assertRaises(FileExistsError):self.copy()
  self.assertEqual(p.read_bytes(),b'preserved')
 def test_dangling_symlink_refused(self):
  (self.dir/'artifact').symlink_to(self.dir/'absent')
  with self.assertRaises(FileExistsError):self.copy()
  self.assertFalse((self.dir/'absent').exists())
 def test_wrong_hash_stops_with_partial_receipt(self):
  with self.assertRaisesRegex(ValueError,'payload'):self.copy(b'x'*len(self.data))
  self.assertEqual(self.events[-1]['phase'],'failed');self.assertIn('sha256',self.events[-1]);self.assertTrue((self.dir/'artifact').exists())
 def test_short_input_stops_no_auto_cleanup(self):
  with self.assertRaisesRegex(ValueError,'short'):self.copy(b'x')
  self.assertEqual(self.events[-1]['size'],1);self.assertTrue((self.dir/'artifact').exists())
 def test_fsync_failure_stops(self):
  with patch.object(stage.os,'fsync',side_effect=OSError('sync failure')):
   with self.assertRaises(OSError):self.copy()
  self.assertEqual(self.events[-1]['phase'],'failed')
 def test_short_write_loop_consumes_all_bytes(self):
  real=os.write
  with patch.object(stage.os,'write',side_effect=lambda fd,b:real(fd,b[:2])):self.copy()
  self.assertEqual((self.dir/'artifact').read_bytes(),self.data)
 def test_zero_write_fails(self):
  with patch.object(stage.os,'write',return_value=0):
   with self.assertRaisesRegex(OSError,'write'):self.copy()
 def test_basename_only(self):
  with self.assertRaises(ValueError):stage.copy_exclusive(self.fd,'../escape',io.BytesIO(self.data),len(self.data),self.digest,os.getuid(),os.getgid(),0o600,self.events.append)
 def test_hardlink_mutation_is_not_accepted(self):
  def event(row):
   self.events.append(row)
   if row['phase']=='created':os.link(self.dir/'artifact',self.dir/'alias')
  with self.assertRaisesRegex(ValueError,'inode'):stage.copy_exclusive(self.fd,'artifact',io.BytesIO(self.data),len(self.data),self.digest,os.getuid(),os.getgid(),0o600,event)
 def test_replacement_mutation_is_not_accepted(self):
  def event(row):
   self.events.append(row)
   if row['phase']=='created':(self.dir/'artifact').rename(self.dir/'old');(self.dir/'artifact').write_bytes(b'x')
  with self.assertRaisesRegex(ValueError,'inode'):stage.copy_exclusive(self.fd,'artifact',io.BytesIO(self.data),len(self.data),self.digest,os.getuid(),os.getgid(),0o600,event)
if __name__=='__main__':unittest.main()
