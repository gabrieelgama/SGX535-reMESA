"""Offline-only validation of a plan and supplied observation records.

No deployment or transport implementation. A passing record is not live evidence
unless its raw capture and provenance have independently been verified.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import frozen_first_load_image as image

PLAN='docs/phase8/first-load-operational-procedure.json'
POLICY={'max_experimental_boots':1,'sgx_actions':0,'automatic_retry':False,'hot_restoration':False,'automatic_reset':False,'change_default':False,'overwrite_stock':False,'exclusive_create':True,'minimum_boot_free_bytes':134217728,'boot_observation_seconds':120,'stock_observation_seconds':120,'selection':'manual only','recovery':'operator full reset/power boundary; manual stock selection','sgx_gate_b':'BLOCKED','sgx_whitelist':[]}
ORDER=['fresh_stock_guards','exclusive_image_copy','fsync_image','image_readback','exclusive_entry_copy','fsync_entry','entry_readback','stock_recheck','manual_menu_guard','manual_experimental_selection','passive_evidence','operator_reset_boundary','manual_stock_selection','passive_stock_recovery']
STOCK={'kernel':('/boot/vmlinuz-'+image.RELEASE,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),'initrd':('/boot/initrd.img-'+image.RELEASE,50863580,image.STOCK_HASH),'grub_cfg':('/boot/grub/grub.cfg',10438,image.GRUB_HASH),'grub_env':('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),'original_module':('/lib/modules/'+image.RELEASE+'/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko',None,'7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb')}
SOURCE='docs/phase8/artifacts/experimental-first-load-01-20261001/build-03/'
EXPERIMENTAL={'image':(SOURCE+'initrd.img-sgx535-firstload-01',image.EXPERIMENTAL_INITRD,50804481,'4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71'),'entry':(SOURCE+'proposed-custom.cfg','/boot/grub/custom.cfg',974,'181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612')}
DERIVATIVE_NOTE='96eb5049d143a3c7a6e7d672aed1651fa51db069ea9efd3484388b3401f29eaf'
ORIGINAL_NOTE=hashlib.sha256(bytes.fromhex('040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3')).hexdigest()
STOCK_ID='gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'

def require(condition,reason):
 if not condition:raise ValueError(reason)

def load_plan(repo):return json.loads((repo/PLAN).read_text())

def validate_plan(p,repo=None):
 require(p.get('schema')==1 and p.get('kernel_release')==image.RELEASE,'plan identity')
 require(p.get('policy')==POLICY and p.get('stage_order')==ORDER,'changed bounds or ordering')
 require(p.get('stock_entry_id')==STOCK_ID and p.get('experimental_entry_id')=='sgx535-rev121-firstload-01','entry identity')
 require(p.get('derivative_sha256')==image.DERIVATIVE_HASH and p.get('derivative_note_sha256')==DERIVATIVE_NOTE and p.get('original_note_sha256')==ORIGINAL_NOTE,'module identity')
 require(set(p.get('stock',{}))==set(STOCK) and set(p.get('experimental',{}))==set(EXPERIMENTAL),'file inventory')
 for key,(path,size,digest) in STOCK.items():
  expected={'path':path,'sha256':digest}
  if size is not None:expected['size']=size
  require(p['stock'][key]==expected,'changed stock specification '+key)
 for key,(source,dest,size,digest) in EXPERIMENTAL.items():
  require(p['experimental'][key]=={'source':source,'destination':dest,'size':size,'sha256':digest},'changed experimental specification '+key)
  if repo is not None:
   data=(repo/source).read_bytes();require(len(data)==size and image.sha(data)==digest,'source artifact drift '+key)
 return {'classification':'PASS OFFLINE PLAN','target_contact':False,'sgx_authorized':False}

def exact_stock(p,observed):
 require(set(observed)==set(p['stock']),'stock observation coverage')
 for key,expected in p['stock'].items():
  row=observed[key]
  require(row.get('type')=='regular' and row.get('symlink') is False,'stock file type '+key)
  require(all(row.get(k)==v for k,v in expected.items()),'stock content/path changed '+key)

def validate_staged(p,before,after):
 validate_plan(p);exact_stock(p,before.get('stock',{}));exact_stock(p,after.get('stock',{}))
 for record in [before,after]:
  live=record.get('live_stock',{})
  bound_identity(p,live,ORIGINAL_NOTE)
  require(all(live.get(k) is True for k in ['display_normal','userspace_reached','slimski_running','xorg_running']), 'stock live state abnormal during staging')
  require(live.get('taint')==12289 and bool(live.get('boot_id')),'fresh stock baseline')
 require(before['live_stock']['boot_id']==after['live_stock']['boot_id'],'boot changed during staging')
 for key in ['experimental_paths_absent','custom_cfg_absent','parents_trusted_no_symlinks','boot_filesystem_writable','operator_recovery_confirmed','source_hashes_exact']:
  require(before.get(key) is True,'pre-stage guard '+key)
 require(type(before.get('free_bytes')) is int and before['free_bytes']>=POLICY['minimum_boot_free_bytes'],'insufficient boot space')
 for key in ['fsync_files_and_parent_dirs','destination_readback_complete','stock_entry_present']:
  require(after.get(key) is True,'post-stage guard '+key)
 require(after.get('grub_env')=={'saved_entry':STOCK_ID},'changed saved/one-time selection')
 require(set(after.get('experimental',{}))==set(EXPERIMENTAL),'staged-file inventory')
 for key,expected in p['experimental'].items():
  row=after['experimental'][key]
  require(row.get('path')==expected['destination'] and row.get('size')==expected['size'] and row.get('sha256')==expected['sha256'],'staged readback mismatch '+key)
  require(all(row.get(k)==v for k,v in {'type':'regular','symlink':False,'uid':0,'gid':0,'mode':0o644,'nlink':1}.items()),'unsafe staged inode '+key)
 return {'classification':'PASS PROVIDED STAGING RECORD','sgx_authorized':False}

def bound_identity(p,f,note):
 for key,value in {'kernel':image.RELEASE,'machine':'Inspiron 1210','architecture':'i686','loaded_note_sha256':note,'module_state':'live','pci_driver':'gma500','driver_module':'gma500_gfx','drm_bdf':'0000:00:02.0','framebuffer':'gma500drmfb','pci_irq':16,'cmdline_exact':True,'evidence_complete':True,'new_kernel_fault':False,'sgx_actions':0,'hot_module_actions':0}.items():
  require(f.get(key)==value,'identity/evidence guard '+key)
 require(type(f.get('taint')) is int and not f['taint']&~12289,'unexpected kernel taint')
 require(re.search(r'^\s*16:.*[\s,]gma500(?:[,\s]|$)',f.get('interrupts',''),re.M) is not None,'IRQ16 handler absent')
 require(bool(f.get('kernel_log')),'missing kernel log')
 require(re.search(r'WARNING:|BUG:|Oops:|Kernel panic|general protection fault|Call Trace:',f['kernel_log'],re.I) is None,'kernel warning/fault')

def observation_deadline(f,limit):
 elapsed=f.get('elapsed_seconds')
 require(type(elapsed) in (int,float) and 0<=elapsed<=limit,'missing/invalid/late handoff timing')

def validate_first_owner(p,f):
 validate_plan(p);bound_identity(p,f,DERIVATIVE_NOTE);observation_deadline(f,POLICY['boot_observation_seconds'])
 for key in ['selection_photo','stock_entry_visible_before_selection','experimental_selected_once','userspace_reached','display_normal','slimski_running','xorg_running']:
  require(f.get(key) is True,'first-load guard '+key)
 require(isinstance(f.get('boot_id'),str) and bool(f['boot_id']) and isinstance(f.get('prior_boot_id'),str) and bool(f['prior_boot_id']) and f['boot_id']!=f['prior_boot_id'] and f.get('experimental_boots')==1,'experimental boot attribution/bound')
 lines=f.get('log','').splitlines()
 events=['SGX535-FIRSTLOAD BEGIN','SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE','SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue']
 require(lines==events,'missing/duplicate/reordered/error/unexplained hook trace')
 return {'first_owner':'PASS PROVIDED RECORD','first_load':'PASS PROVIDED RECORD','ssh':'PASS PROVIDED RECORD' if f.get('ssh_available') is True else 'NOT ESTABLISHED','sgx_authorized':False}

def validate_recovery(p,f):
 validate_plan(p);bound_identity(p,f,ORIGINAL_NOTE);observation_deadline(f,POLICY['stock_observation_seconds'])
 for key in ['selection_photo','stock_selected','stock_files_exact','userspace_reached','display_normal','ssh_available','slimski_running','xorg_running']:
  require(f.get(key) is True,'stock-recovery guard '+key)
 require(all(isinstance(f.get(k),str) and f[k] for k in ['prior_boot_id','experimental_boot_id','boot_id']) and len({f['prior_boot_id'],f['experimental_boot_id'],f['boot_id']})==3,'missing/contradictory three-boot boundary')
 require(f.get('grub_env')=={'saved_entry':STOCK_ID},'stock recovery saved/one-time selection')
 require(f.get('framebuffer_dimensions')=='1280x800','stock framebuffer dimensions')
 require(f.get('vtcon0')==0 and f.get('vtcon1')==1,'stock VT ownership')
 return {'recovery':'PASS PROVIDED RECORD','sgx_authorized':False}

def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);args=parser.parse_args()
 print(json.dumps(validate_plan(load_plan(args.repo),args.repo),indent=2))
if __name__=='__main__':main()
