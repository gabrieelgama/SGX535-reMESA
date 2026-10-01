"""Offline tests of preserved first-load-cycle preflight evidence; no transport."""
import hashlib,importlib.util,json,re,sys,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
sys.path.insert(0,str(ROOT/'tools/psb-dri-re'))
import frozen_first_load_procedure as procedure
class CaptureTests(unittest.TestCase):
 def setUp(self):self.fresh=json.loads((HERE/'preflight-02/stdout.txt').read_text());self.comparison=json.loads((HERE/'baseline-health-comparison.json').read_text())
 def test_second_capture_stops_before_staging(self):
  self.assertEqual(self.fresh['classification'],'STOP BEFORE STAGING');self.assertEqual(self.fresh['failure'],'current-boot kernel health')
  self.assertNotIn('stock',self.fresh);self.assertNotIn('destinations',self.fresh)
 def test_raw_capture_hashes_and_empty_stderr_match(self):
  record=json.loads((HERE/'preflight-02/command.json').read_text())
  self.assertEqual(record['exit_status'],1)
  for file,key in [('stdout.txt','stdout_sha256'),('stderr.txt','stderr_sha256')]:self.assertEqual(hashlib.sha256((HERE/'preflight-02'/file).read_bytes()).hexdigest(),record[key])
  self.assertEqual((HERE/'preflight-02/stderr.txt').read_bytes(),b'')
 def test_entire_kernel_log_matches_retained_same_boot(self):
  old=(ROOT/self.comparison['retained_capture']).read_text().split('DMESG_BEGIN\n',1)[1].split('\nDMESG_END',1)[0]
  self.assertEqual(old.strip(),self.fresh['kernel_log'].strip())
  self.assertEqual(self.fresh['boot_id'],self.comparison['retained_boot_id'])
  self.assertEqual(self.comparison['new_guard_matching_lines'],[])
 def test_current_procedure_guard_reproduces_baseline_refusal(self):
  f=dict(self.fresh,cmdline_exact=True,evidence_complete=True,new_kernel_fault=False,sgx_actions=0,hot_module_actions=0,pci_driver='gma500',driver_module='gma500_gfx',pci_irq=16)
  with self.assertRaisesRegex(ValueError,'kernel warning/fault'):procedure.bound_identity(procedure.load_plan(ROOT),f,procedure.ORIGINAL_NOTE)
 def test_fault_negative_controls_remain_rejected(self):
  f=dict(self.fresh,cmdline_exact=True,evidence_complete=True,new_kernel_fault=False,sgx_actions=0,hot_module_actions=0,pci_driver='gma500',driver_module='gma500_gfx',pci_irq=16)
  for marker in ['WARNING: injected fault','BUG: injected fault','Oops: injected fault','Kernel panic - injected fault','Call Trace: injected fault']:
   with self.subTest(marker=marker):
    f['kernel_log']=marker
    with self.assertRaisesRegex(ValueError,'kernel warning/fault'):procedure.bound_identity(procedure.load_plan(ROOT),f,procedure.ORIGINAL_NOTE)
 def test_known_stock_marker_matches_are_exact(self):
  pattern=r'WARNING:|BUG:|Oops:|Kernel panic|general protection fault|Call Trace:'
  matches=[s for s in self.fresh['kernel_log'].splitlines() if re.search(pattern,s,re.I)]
  self.assertEqual(matches,self.comparison['matched_lines']);self.assertEqual(len(matches),3)
if __name__=='__main__':unittest.main()
