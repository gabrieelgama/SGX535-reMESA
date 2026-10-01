"""Offline tests: execute the emitted hook with substituted I/O boundaries only."""
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import re
import verify_frozen_first_load_image as verification
import unittest

ROOT=Path(__file__).resolve().parents[2]
try:
 import frozen_first_load_image as image
except ImportError:
 image=None

class ImageTests(unittest.TestCase):
 def require(self):
  self.assertIsNotNone(image,'fixed first-load image construction is not implemented')
 def test_splice_preserves_opaque_hardlink_records(self):
  self.require()
  fields=[1,0o100755,0,0,1,17,3,0,1,0,0,5,0]
  init=image.record('init',b'old',fields)
  fields[0]=2;fields[4]=2
  a=image.record('a',b'',fields);b=image.record('b',b'payload',fields)
  old=init+a+b+image.record('TRAILER!!!',b'')
  new=image.splice(old,b'new',[('scripts/hook',b'hook',0o100755)])
  rows,end=image.scan(new)
  self.assertEqual(next(x['raw'] for x in rows if x['name']=='a'),a)
  self.assertEqual(next(x['raw'] for x in rows if x['name']=='b'),b)
  self.assertEqual(next(x['data'] for x in rows if x['name']=='init'),b'new')
 def test_truncated_and_duplicate_archives_rejected(self):
  self.require()
  for data in [b'070701',image.record('init',b'a')+image.record('init',b'b')+image.record('TRAILER!!!',b'')]:
   with self.subTest(data=data[:20]),self.assertRaises(ValueError):image.scan(data)
 def test_traversal_rejected(self):
  self.require()
  with self.assertRaises(ValueError):image.record('../escape',b'')
 def test_dependency_cycle_and_missing_rejected(self):
  self.require()
  for graph in [{'a':['b'],'b':['a']},{'a':['missing']}]:
   with self.subTest(graph=graph),self.assertRaises(ValueError):image.order_dependencies(['a'],graph)
 def test_dependency_topology(self):
  self.require();self.assertEqual(image.order_dependencies(['helper'],{'helper':['fb','drm'],'fb':[],'drm':[]}),['fb','drm','helper'])
 def test_non_saving_entry_and_unchanged_kernel_arguments(self):
  self.require();cfg=(ROOT/image.CAPTURE/'decoded-files/_boot_grub_grub_cfg').read_text()
  entry=image.grub_entry(cfg)
  for token in ['savedefault','save_env','set default','next_entry','blacklist','nomodeset']:
   self.assertNotIn(token,entry)
  self.assertIn('EXPERIMENTAL',entry);self.assertIn(image.EXPERIMENTAL_INITRD,entry)
  self.assertIn('root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro  quiet selinux=0',entry)
 def metadata_fixture(self):
  before_data=image.record('init',b'old',[1,0o100755,0,0,1,17,0,0,1,0,0,0,0])+image.record('TRAILER!!!',b'')
  after_data=image.splice(before_data,b'new',[('usr/lib/sgx535-first-load',b'',0o40755),(image.PAYLOAD,b'program',0o100644),(image.HOOK,b'script',0o100755)])
  before={x['name']:x for x in image.scan(before_data)[0]};after={x['name']:x for x in image.scan(after_data)[0]}
  return before,after
 def test_retained_init_and_added_metadata_exact(self):
  self.require();self.assertTrue(hasattr(verification,'validate_layout'),'image metadata is not qualified')
  before,after=self.metadata_fixture();verification.validate_layout(before,after)
 def test_nonexecutable_init_and_added_inode_collision_rejected(self):
  self.require();self.assertTrue(hasattr(verification,'validate_layout'),'image metadata is not qualified')
  for path,field,value in [('init',1,0o100644),(image.PAYLOAD,0,1),(image.HOOK,4,2),('usr/lib/sgx535-first-load',1,0o100755)]:
   with self.subTest(path=path,field=field):
    before,after=self.metadata_fixture();row=after[path];row['fields'][field]=value;row['raw']=image.record(path,row['data'],row['fields'])
    with self.assertRaises(ValueError):verification.validate_layout(before,after)
 def test_bad_stock_entry_refused(self):
  self.require();cfg=(ROOT/image.CAPTURE/'decoded-files/_boot_grub_grub_cfg').read_text()
  with self.assertRaises(ValueError):image.grub_entry(cfg.replace('savedefault\n','savedefault\n\tset default=evil\n',1))

class HookTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  if image is not None:
   cls.inputs=image.load_inputs(ROOT)
   cls.hook=image.render_hook(cls.inputs)
 def run_hook(self,case='good',mutation=None):
  self.assertIsNotNone(image,'actual preload hook not implemented')
  source=self.hook if mutation is None else mutation(self.hook)
  self.assertTrue(source.endswith('sgx_first_load_main\n'))
  # Keep all emitted guard decisions; remove only the dispatcher to substitute
  # filesystem, uname, hashing and insertion boundaries before invoking main.
  definitions=source[:-len('sgx_first_load_main\n')]
  harness=r'''
case_name=CASE
inserted=''
phase=before
mock_calls=0
sgx_pci_paths() {
 echo /sys/bus/pci/devices/0000:00:02.0
 [ "$case_name" != extra_gpu ] || echo /sys/bus/pci/devices/0000:00:03.0
}
sgx_setup_log() { :; }
sgx_uname() {
 if [ "$1" = -r ]; then
  if [ "$case_name" = wrong_kernel ]; then echo bad; else echo 5.10.240-antix.1-486-smp; fi
 else
  if [ "$case_name" = wrong_arch ]; then echo aarch64; else echo i686; fi
 fi
}
sgx_read_file() {
 case "$1" in
 */product_name) if [ "$case_name" = wrong_machine ]; then echo other; else printf 'Inspiron 1210   \n'; fi ;;
 */vendor) if [ "$case_name" = wrong_pci ]; then echo 0x1234; else echo 0x8086; fi ;;
 */device) echo 0x8108 ;;
 */subsystem_vendor) echo 0x1028 ;;
 */subsystem_device) echo 0x02b1 ;;
 */cmdline) if [ "$case_name" = kernel_blacklist ]; then echo module_blacklist=gma500_gfx; else echo quiet; fi ;;
 */modules) if [ "$case_name" = original_proc ]; then echo 'gma500_gfx 10 0 - Live 0'; fi ;;
 */initstate) if [ "$case_name" = wrong_live ]; then echo coming; else echo live; fi ;;
 */irq) echo 16 ;;
 */name) echo gma500drmfb ;;
 *) return 1 ;;
 esac
}
sgx_has_path() {
 case "$1" in
 */driver) [ "$phase" = after ] || [ "$case_name" = bound ] ;;
 /sys/class/drm/card0) [ "$case_name" = preexisting_drm ] ;;
 /sys/class/graphics/fb0) [ "$case_name" = preexisting_fb ] ;;
 /sys/module/gma500_gfx) [ "$phase" = after ] || [ "$case_name" = original_owner ] ;;
 /sys/module/*) return 1 ;;
 *) return 1 ;;
 esac
}
sgx_listed() { [ "$case_name" = original_proc ] && [ "$1" = gma500_gfx ]; }
sgx_hash_ok() {
 [ "$case_name" != bad_hash ] || return 1
 [ "$case_name" != missing_dependency ] || case "$1" in *video.ko) return 1;; esac
 [ "$case_name" != wrong_note ] || case "$1" in */notes/*) return 1;; esac
 return 0
}
sgx_irq_present() { [ "$phase" = after ] || [ "$case_name" = stale_irq ]; }
sgx_irq_owned() { [ "$phase" = after ] && [ "$case_name" != wrong_irq ]; }
sgx_link_is() { [ "$phase" = after ] && [ "$case_name" != wrong_binding ]; }
sgx_insert_file() {
 printf 'INSERT:%s\n' "$1"
 mock_calls=$((mock_calls+1))
 [ "$case_name" != "fail_$mock_calls" ] || return 1
 inserted="$inserted $1"
 if [ "$case_name" = dependency_insert_failure ]; then return 1; fi
 case "$1" in
 *gma500_gfx.ko) phase=after;[ "$case_name" != insert_failure ] ;;
 *) return 0 ;;
 esac
}
sgx_first_load_main
rc=$?
if [ "$rc" = 0 ]; then echo BOOT_CONTINUE; fi
exit "$rc"
'''.replace('CASE',case,1)
  cp=subprocess.run(['/bin/sh','-s'],input=definitions+harness,text=True,capture_output=True,timeout=10)
  return cp
 def test_expected_identity_allows_exact_fixed_order_once(self):
  cp=self.run_hook();self.assertEqual(cp.returncode,0,cp.stdout+cp.stderr)
  actual=[l[len('INSERT:'):] for l in cp.stdout.splitlines() if l.startswith('INSERT:')]
  self.assertEqual(actual,[x['path'] for x in self.inputs['payloads']]);self.assertIn('BOOT_CONTINUE',cp.stdout)
 def test_wrong_machine(self):self.guard('wrong_machine')
 def test_wrong_kernel(self):self.guard('wrong_kernel')
 def test_wrong_architecture(self):self.guard('wrong_arch')
 def test_wrong_pci(self):self.guard('wrong_pci')
 def test_additional_matching_gpu(self):self.guard('extra_gpu')
 def test_preexisting_drm(self):self.guard('preexisting_drm')
 def test_preexisting_framebuffer(self):self.guard('preexisting_fb')
 def test_already_bound(self):self.guard('bound')
 def test_original_module_owner(self):
  for case in ['original_owner','stale_irq']:
   with self.subTest(case=case):self.guard(case)
 def test_original_proc_modules(self):self.guard('original_proc')
 def test_required_dependency_absent(self):self.guard('missing_dependency')
 def test_payload_hash_mismatch(self):self.guard('bad_hash')
 def test_kernel_blacklist(self):self.guard('kernel_blacklist')
 def guard(self,case):
  cp=self.run_hook(case);self.assertNotEqual(cp.returncode,0);self.assertNotIn('INSERT:',cp.stdout);self.assertNotIn('BOOT_CONTINUE',cp.stdout);self.assertIn('HOLD',cp.stdout)
 def test_insertion_error_holds_no_retry(self):
  cp=self.run_hook('insert_failure');self.assertNotEqual(cp.returncode,0);self.assertNotIn('BOOT_CONTINUE',cp.stdout)
  self.assertEqual(cp.stdout.count('INSERT:'),10);self.assertIn('HOLD',cp.stdout)
 def test_dependency_insertion_error_stops_immediately(self):
  cp=self.run_hook('dependency_insert_failure');self.assertNotEqual(cp.returncode,0);self.assertEqual(cp.stdout.count('INSERT:'),1)
 def test_failure_at_each_insertion_retains_no_retry(self):
  for n in range(1,11):
   with self.subTest(position=n):
    cp=self.run_hook(f'fail_{n}');self.assertNotEqual(cp.returncode,0);self.assertEqual(cp.stdout.count('INSERT:'),n);self.assertNotIn('BOOT_CONTINUE',cp.stdout)
 def test_post_load_mismatches_hold(self):
  for case in ['wrong_binding','wrong_note','wrong_live','wrong_irq']:
   with self.subTest(case=case):
    cp=self.run_hook(case);self.assertNotEqual(cp.returncode,0);self.assertNotIn('BOOT_CONTINUE',cp.stdout);self.assertIn('HOLD',cp.stdout)
 def test_mutation_removing_machine_guard_is_detected(self):
  cp=self.run_hook('wrong_machine',lambda s:s.replace('[ "$machine" = "Inspiron 1210" ]','true'))
  self.assertEqual(cp.returncode,0,'negative control must demonstrate that removing the production guard admits the wrong machine')
 def run_init_gate(self,hash_status=0,hook_status=0):
  patched=image.patched_init(b'\nrun_scripts /scripts/init-top\n',self.hook.encode()).decode()
  block=patched.split('# SGX535-FIRSTLOAD-BEGIN:',1)[1].split('# SGX535-FIRSTLOAD-END',1)[0]
  block=block[block.index('\n')+1:]
  with tempfile.TemporaryDirectory() as t:
   root=Path(t);hasher=root/'sha';stub=root/'hook';console=root/'console'
   hasher.write_text(f'#!/bin/sh\nprintf "{image.sha(self.hook.encode())}  fixture\\n"\nexit {hash_status}\n');hasher.chmod(0o755)
   stub.write_text(f'echo HOOK_RAN\nexit {hook_status}\n')
   block=block.replace('/bin/sha256sum /scripts/sgx535-first-load',str(hasher))
   block=block.replace('/bin/sh /scripts/sgx535-first-load','/bin/sh '+str(stub))
   block=block.replace('/dev/console',str(console)).replace('/bin/sleep 3600','sgx_test_sleep')
   script='sgx_test_sleep() { echo HELD; exit 77; }\n'+block+'\necho UDEV_NEXT\n'
   return subprocess.run(['/bin/sh','-s'],input=script,text=True,capture_output=True,timeout=10)
 def test_init_success_precedes_udev(self):
  cp=self.run_init_gate();self.assertEqual(cp.returncode,0);self.assertLess(cp.stdout.index('HOOK_RAN'),cp.stdout.index('UDEV_NEXT'))
 def test_init_hook_failure_never_reaches_udev(self):
  cp=self.run_init_gate(hook_status=1);self.assertEqual(cp.returncode,77);self.assertNotIn('UDEV_NEXT',cp.stdout)
 def test_init_hash_failure_even_with_matching_output_holds(self):
  cp=self.run_init_gate(hash_status=1);self.assertEqual(cp.returncode,77);self.assertNotIn('HOOK_RAN',cp.stdout);self.assertNotIn('UDEV_NEXT',cp.stdout)
 def test_actual_irq_helper_requires_handler_on_irq16(self):
  definitions=self.hook[:-len('sgx_first_load_main\n')]
  with tempfile.TemporaryDirectory() as t:
   irq=Path(t)/'interrupts'
   source=definitions.replace('/proc/interrupts',str(irq))
   for contents,expected in [(' 16: 0 0 IO-APIC gma500,eth0\n',0),(' 17: 0 0 IO-APIC gma500\n',1),(' 16: 0 0 IO-APIC eth0\n 17: 0 0 IO-APIC gma500\n',1),(' 116: 0 0 IO-APIC gma500\n',1),(' 16: 0 0 IO-APIC gma500_extra\n',1)]:
    with self.subTest(contents=contents):
     irq.write_text(contents)
     cp=subprocess.run(['/bin/sh','-s'],input=source+'sgx_irq_owned\n',text=True,capture_output=True)
     self.assertEqual(cp.returncode,expected)
     cp=subprocess.run(['/bin/sh','-s'],input=source+'sgx_irq_present\n',text=True,capture_output=True)
     self.assertEqual(cp.returncode,1 if 'gma500_extra' in contents else 0)
 def test_actual_hash_helper_rejects_corrupt_or_missing_file(self):
  definitions=self.hook[:-len('sgx_first_load_main\n')]
  with tempfile.TemporaryDirectory() as t:
   p=Path(t)/'payload';p.write_bytes(b'fixed bytes');digest=image.sha(p.read_bytes())
   for path,value,expected in [(p,digest,0),(p,'0'*64,1),(Path(t)/'absent',digest,1)]:
    cp=subprocess.run(['/bin/sh','-s'],input=definitions+f"sgx_hash_ok {path} {value}\n",text=True,capture_output=True)
    self.assertEqual(cp.returncode,expected)
 def test_no_hot_transition_or_submission_commands(self):
  self.assertIsNotNone(image)
  for token in ['rmmod','unbind','ioctl','/dev/dri','reset','reboot']:
   self.assertNotIn(token,self.hook)
  self.assertIsNone(re.search(r'(^|[;\s])(?:/[^\s]*/)?modprobe(?:\s|$)',self.hook))

if __name__=='__main__':unittest.main()
