"""Exercise actual operational health gate using preserved logs and fault mutations."""
import json
from pathlib import Path
import unittest
import frozen_first_load_procedure as procedure
ROOT=Path(__file__).resolve().parents[2]
CAPTURE=ROOT/'docs/hardware-evidence/MINI12-20261001T055237Z-FIRSTLOAD-CYCLE-01/preflight-02/stdout.txt'
BL='[   19.356593] gma500 0000:00:02.0: BL bug: Reg 00000000 save 00000000'
ACPI='ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)'
class KernelHealthTests(unittest.TestCase):
 def setUp(self):
  self.capture=json.loads(CAPTURE.read_text());self.plan=procedure.load_plan(ROOT)
 def gate(self,log):
  f=dict(self.capture,cmdline_exact=True,evidence_complete=True,new_kernel_fault=False,sgx_actions=0,hot_module_actions=0,pci_driver='gma500',driver_module='gma500_gfx',pci_irq=16,kernel_log=log)
  procedure.bound_identity(self.plan,f,procedure.ORIGINAL_NOTE)
 def reject(self,log):
  with self.assertRaises(ValueError):self.gate(log)
 def test_stock_bl_diagnostic_is_not_generic_kernel_bug(self):self.gate(BL)
 def test_actual_bug_case_variants_still_reject(self):
  for marker in ['BUG: unable to handle kernel NULL pointer dereference','bug: KASAN: bad access','Bug: scheduling while atomic','watchdog: BUG: unexpected fault']:
   with self.subTest(marker=marker):self.reject('[ 2.000000] '+marker)
 def test_debug_suffix_is_not_kernel_bug(self):self.gate('[ 2.000000] DEBUG: normal diagnostic')
 def test_genuine_oops_panic_fault_trace_controls_reject(self):
  for marker in ['Oops: 0000 [#1]','Kernel panic - not syncing: fatal','general protection fault','general protection: 0000','Call Trace:','kernel BUG at drivers/gpu/test.c:1!']:
   with self.subTest(marker=marker):self.reject('[ 3.0] '+marker)
 def test_lockup_and_hung_task_controls_reject(self):
  for marker in ['watchdog: BUG: soft lockup - CPU stuck','NMI watchdog: Watchdog detected hard LOCKUP','INFO: task Xorg blocked for more than 120 seconds','rcu: INFO: rcu_sched detected stalls on CPUs/tasks']:
   with self.subTest(marker=marker):self.reject('[ 4.0] '+marker)
 def test_real_warning_variants_reject(self):
  for marker in ['WARNING: CPU: 0 PID: 1 at test.c:1','warning: CPU: 0 at test.c:1','gma500: WARN_ON failed','ACPI Warning: unexpected new timer problem']:
   with self.subTest(marker=marker):self.reject('[ 5.0] '+marker)
 def test_retained_acpi_diagnostics_are_bounded_and_explicit(self):self.gate('[ 13.0] '+ACPI)
 def test_new_gma_drm_and_sgx_faults_are_not_hidden(self):
  for marker in ['gma500 0000:00:02.0: MMU fault','gma500 0000:00:02.0: timeout waiting for hardware','[drm] ERROR: failed to bind','SGX: fatal MMU error','gma500 0000:00:02.0: BUG: bad pointer']:
   with self.subTest(marker=marker):self.reject('[ 6.0] '+marker)
 def test_exact_retained_stock_baseline_passes(self):self.gate(self.capture['kernel_log'])
 def test_mutated_fault_bearing_baselines_reject(self):
  for fault in ['BUG: bad access','Oops: 0000','WARNING: CPU: 0','gma500: MMU fault','watchdog: soft lockup','Kernel panic - not syncing']:
   with self.subTest(fault=fault):self.reject(self.capture['kernel_log']+'\n[ 90.0] '+fault)
 def test_changed_or_repeated_baseline_diagnostics_reject(self):
  for log in [BL.replace('save 00000000','save 00000001'),BL+'\n'+BL,self.capture['kernel_log']+'\n[ 99.0] '+ACPI,BL+' Oops: bad access']:
   with self.subTest(log=log[-120:]):self.reject(log)
 def test_empty_capture_does_not_pass(self):self.reject('')
if __name__=='__main__':unittest.main()
