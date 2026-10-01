"""Exercise the future observation-record gates; never execute target actions."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
PATH=ROOT/'tools/psb-dri-re/frozen_first_load_procedure.py'

class ProcedureTests(unittest.TestCase):
 def setUp(self):
  self.assertTrue(PATH.exists(),'offline first-load procedure validator is missing')
  spec=importlib.util.spec_from_file_location('procedure_under_test',PATH);self.module=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.module)
  self.plan=self.module.load_plan(ROOT)
 def records(self):
  # Ordinary JSON fixtures, not claimed target observations.
  stock={k:dict(v,type='regular',symlink=False) for k,v in self.plan['stock'].items()}
  files={k:{'path':v['destination'],'sha256':v['sha256'],'size':v['size'],'type':'regular','symlink':False,'uid':0,'gid':0,'mode':0o644,'nlink':1} for k,v in self.plan['experimental'].items()}
  before={'stock':copy.deepcopy(stock),'experimental_paths_absent':True,'custom_cfg_absent':True,'parents_trusted_no_symlinks':True,'boot_filesystem_writable':True,'free_bytes':134217728,'operator_recovery_confirmed':True,'source_hashes_exact':True}
  before['live_stock']=self.recovery()
  after={'stock':copy.deepcopy(stock),'live_stock':copy.deepcopy(before['live_stock']),'experimental':files,'fsync_files_and_parent_dirs':True,'destination_readback_complete':True,'grub_env':{'saved_entry':self.plan['stock_entry_id']},'stock_entry_present':True}
  return before,after
 def live(self):
  return {'selection_photo':True,'stock_entry_visible_before_selection':True,'experimental_selected_once':True,'boot_id':'experimental-boot','prior_boot_id':'stock-before','kernel':self.plan['kernel_release'],'machine':'Inspiron 1210','architecture':'i686','cmdline_exact':True,'log':'SGX535-FIRSTLOAD BEGIN\nSGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE\nSGX535-FIRSTLOAD PASS: derivative first owner; boot may continue\n','loaded_note_sha256':self.plan['derivative_note_sha256'],'module_state':'live','pci_driver':'gma500','driver_module':'gma500_gfx','drm_bdf':'0000:00:02.0','framebuffer':'gma500drmfb','pci_irq':16,'interrupts':' 16: 0 0 IO-APIC gma500,eth0\n','new_kernel_fault':False,'kernel_log':'[drm] Initialized gma500\n','taint':12289,'display_normal':True,'userspace_reached':True,'evidence_complete':True,'sgx_actions':0,'hot_module_actions':0,'experimental_boots':1,'ssh_available':False,'slimski_running':True,'xorg_running':True,'elapsed_seconds':60}
 def recovery(self):
  f=self.live();f.update(selection_photo=True,stock_selected=True,experimental_boot_id='experimental-boot',boot_id='stock-after',loaded_note_sha256=self.plan['original_note_sha256'],stock_files_exact=True,log='',ssh_available=True,slimski_running=True,xorg_running=True,vtcon0=0,vtcon1=1,framebuffer_dimensions='1280x800',grub_env={'saved_entry':self.plan['stock_entry_id']})
  return f
 def test_valid_plan_matches_pinned_inputs(self):self.module.validate_plan(self.plan,ROOT)
 def test_plan_stock_overwrite_refused(self):
  p=copy.deepcopy(self.plan);p['experimental']['image']['destination']=p['stock']['initrd']['path']
  with self.assertRaises(ValueError):self.module.validate_plan(p)
 def test_plan_extra_boot_fire_retry_or_hot_restore_refused(self):
  for field,value in [('max_experimental_boots',2),('sgx_actions',1),('automatic_retry',True),('hot_restoration',True),('automatic_reset',True)]:
   with self.subTest(field=field):
    p=copy.deepcopy(self.plan);p['policy'][field]=value
    with self.assertRaises(ValueError):self.module.validate_plan(p)
 def test_staged_record_needs_all_guards_and_readback(self):
  a,b=self.records();self.module.validate_staged(self.plan,a,b)
  for where,key,val in [('before','free_bytes',1),('before','custom_cfg_absent',False),('before','experimental_paths_absent',False),('before','parents_trusted_no_symlinks',False),('before','source_hashes_exact',False),('after','fsync_files_and_parent_dirs',False),('after','destination_readback_complete',False),('after','stock_entry_present',False)]:
   with self.subTest(key=key):
    a,b=self.records();(a if where=='before' else b)[key]=val
    with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
 def test_staged_symlink_hardlink_wrong_owner_mode_or_hash_rejected(self):
  for key,val in [('symlink',True),('nlink',2),('uid',1000),('mode',0o666),('sha256','0'*64),('size',1),('type','directory')]:
   with self.subTest(key=key):
    a,b=self.records();b['experimental']['image'][key]=val
    with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
 def test_staged_stock_change_or_saved_experimental_selection_refused(self):
  a,b=self.records();b['stock']['initrd']['sha256']='0'*64
  with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
  a,b=self.records();b['grub_env']['saved_entry']='sgx535-rev121-firstload-01'
  with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
  a,b=self.records();b['grub_env']['next_entry']='sgx535-rev121-firstload-01'
  with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
  a,b=self.records();b['live_stock']['loaded_note_sha256']=self.plan['derivative_note_sha256']
  with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
  a,b=self.records();b['live_stock']['boot_id']='unplanned-reboot'
  with self.assertRaises(ValueError):self.module.validate_staged(self.plan,a,b)
 def test_live_first_owner_pass_requires_trace_not_display_alone(self):
  f=self.live();self.assertEqual(self.module.validate_first_owner(self.plan,f)['first_owner'],'PASS PROVIDED RECORD')
  f['log']=''
  with self.assertRaises(ValueError):self.module.validate_first_owner(self.plan,f)
 def test_live_missing_duplicate_reordered_or_hold_trace_rejected(self):
  for log in ['SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue\n',self.live()['log']*2,'SGX535-FIRSTLOAD HOLD: pci-identity\n'+self.live()['log'],'\n'.join(reversed(self.live()['log'].splitlines()))]:
   with self.subTest(log=log):
    f=self.live();f['log']=log
    with self.assertRaises(ValueError):self.module.validate_first_owner(self.plan,f)
 def test_live_wrong_note_irq_binding_fault_or_second_boot_refused(self):
  for key,value in [('loaded_note_sha256',self.plan['original_note_sha256']),('interrupts',' 17: 0 0 gma500\n'),('pci_irq',17),('drm_bdf','0000:00:03.0'),('new_kernel_fault',True),('kernel_log','WARNING: test\n'),('kernel_log',''),('taint',12289|512),('boot_id','stock-before'),('sgx_actions',1),('experimental_boots',2),('stock_entry_visible_before_selection',False)]:
   with self.subTest(key=key):
    f=self.live();f[key]=value
    with self.assertRaises(ValueError):self.module.validate_first_owner(self.plan,f)
 def test_experimental_ssh_is_separate_not_required_for_local_proof(self):
  out=self.module.validate_first_owner(self.plan,self.live());self.assertEqual(out['ssh'],'NOT ESTABLISHED');self.assertEqual(out['first_load'],'PASS PROVIDED RECORD')
 def test_recovery_requires_stock_identity_and_new_boot_not_power_alone(self):
  self.module.validate_recovery(self.plan,self.recovery())
  for key,value in [('loaded_note_sha256',self.plan['derivative_note_sha256']),('stock_selected',False),('stock_files_exact',False),('boot_id','experimental-boot'),('new_kernel_fault',True),('display_normal',False),('slimski_running',False),('vtcon1',0),('ssh_available',False)]:
   with self.subTest(key=key):
    f=self.recovery();f[key]=value
    with self.assertRaises(ValueError):self.module.validate_recovery(self.plan,f)
 def test_live_requires_prior_boot_reference_and_services(self):
  for key,value in [('prior_boot_id',None),('prior_boot_id',''),('slimski_running',False),('xorg_running',False)]:
   with self.subTest(key=key,value=value):
    f=self.live();f[key]=value
    with self.assertRaises(ValueError):self.module.validate_first_owner(self.plan,f)
 def test_hook_extra_error_or_unexplained_line_refused(self):
  for extra in ['insmod: ERROR: insertion or probe error','unexpected diagnostic','SGX535-FIRSTLOAD HOLD: error']:
   with self.subTest(extra=extra):
    f=self.live();f['log']+=extra+'\n'
    with self.assertRaises(ValueError):self.module.validate_first_owner(self.plan,f)
 def test_live_and_recovery_require_valid_deadline_evidence(self):
  for maker,validator in [(self.live,self.module.validate_first_owner),(self.recovery,self.module.validate_recovery)]:
   for value in [None,-1,121,True,'60',float('nan'),float('inf')]:
    with self.subTest(validator=validator.__name__,value=value):
     f=maker();f['elapsed_seconds']=value
     with self.assertRaises(ValueError):validator(self.plan,f)
   f=maker();f['elapsed_seconds']=120;validator(self.plan,f)
 def test_recovery_requires_three_distinct_attributed_boots(self):
  for key,value in [('prior_boot_id',None),('prior_boot_id',''),('experimental_boot_id',None),('experimental_boot_id',''),('boot_id','stock-before'),('experimental_boot_id','stock-before')]:
   with self.subTest(key=key,value=value):
    f=self.recovery();f[key]=value
    with self.assertRaises(ValueError):self.module.validate_recovery(self.plan,f)
 def test_recovery_requires_stock_selection_and_framebuffer(self):
  for key,value in [('grub_env',None),('grub_env',{'saved_entry':'sgx535-rev121-firstload-01'}),('grub_env',{'saved_entry':self.plan['stock_entry_id'],'next_entry':'sgx535-rev121-firstload-01'}),('framebuffer_dimensions','640x480')]:
   with self.subTest(key=key,value=value):
    f=self.recovery();f[key]=value
    with self.assertRaises(ValueError):self.module.validate_recovery(self.plan,f)
 def test_validator_cannot_execute_stage_boot_or_remote_actions(self):
  source=PATH.read_text()
  for token in ['subprocess','paramiko','socket','os.system','write_bytes','write_text','unlink','insmod','rmmod']:
   self.assertNotIn(token,source)

if __name__=='__main__':unittest.main()
