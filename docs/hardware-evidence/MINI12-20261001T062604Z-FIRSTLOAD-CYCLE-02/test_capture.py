"""Validate actual raw preflight/staging receipts offline; no transport."""
import hashlib,json,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
class CaptureTests(unittest.TestCase):
 def setUp(self):
  self.pre=json.loads((HERE/'preflight-02/stdout.txt').read_text());self.rows=json.loads((HERE/'staging/decoded-records.json').read_text())
 def test_preflight_all_49_guards_pass(self):
  self.assertEqual(len(self.pre['guards']),49);self.assertTrue(all(g['pass'] for g in self.pre['guards']));self.assertFalse(self.pre['kernel_health']['faults'])
 def test_raw_transport_receipts(self):
  for name in ['preflight-01','preflight-02','staging']:
   d=HERE/name;c=json.loads((d/'command.json').read_text())
   for file,key in [('stdout.txt','stdout_sha256'),('stderr.txt','stderr_sha256')]:self.assertEqual(hashlib.sha256((d/file).read_bytes()).hexdigest(),c[key])
  self.assertEqual((HERE/'preflight-02/stderr.txt').read_bytes(),b'');self.assertEqual((HERE/'staging/stderr.txt').read_bytes(),b'')
 def test_baseline_diagnostics_are_visible_bounded(self):
  self.assertEqual(self.pre['kernel_health']['stock_diagnostics'],{'acpi_powerbutton_error':2,'acpi_powerbutton_warning':2,'powerbutton_probe':1,'tiny_powerbutton_probe':1,'backlight_zero_register':1})
 def test_both_boot_files_exact_reopened_and_synced(self):
  rows=[r for r in self.rows if r.get('scope')=='boot' and r.get('phase')=='verified'];self.assertEqual(len(rows),2)
  self.assertEqual([(r['name'],r['size']) for r in rows],[('initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01',50804481),('custom.cfg',974)])
  for r in rows:self.assertEqual((r['uid'],r['gid'],r['mode'],r['nlink']),(0,0,420,1))
 def test_stock_identities_and_boot_preserved(self):
  records=[r for r in self.rows if 'stock' in r];self.assertEqual(len(records),2)
  self.assertEqual(records[0]['boot_id'],records[1]['boot_id'])
  for k in records[0]['stock']:self.assertEqual(records[0]['stock'][k],records[1]['stock'][k])
  self.assertEqual(records[0]['grub_env'],records[1]['grub_env']);self.assertEqual(records[1]['loaded_note_sha256'],self.pre['loaded_note_sha256'])
 def test_final_receipt_contains_no_sgx_or_hot_action(self):
  last=self.rows[-1];self.assertEqual(last['classification'],'PASS');self.assertEqual(last['sgx_actions'],0);self.assertEqual(last['hot_module_actions'],0)
if __name__=='__main__':unittest.main()
