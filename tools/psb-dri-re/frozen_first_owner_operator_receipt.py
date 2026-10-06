#!/usr/bin/env python3
"""Prospective direct-operator menu receipt for the exact frozen candidate.

Offline supplied-record consistency, no target operations or permission grant.
Does not satisfy historical v2, manufacture photographs, or open Gate B.
"""
import uuid
import ast
import hashlib
import json
from pathlib import Path

import frozen_first_owner_session as runtime
import frozen_first_load_capture_v2 as capture
from frozen_candidate_stage import TITLE, IMAGE, BUILD_ID, IMAGE_SHA, ENTRY_ID
from frozen_first_load_procedure import STOCK_ID

PROCEDURE_ID = 'MINI12-FROZEN-FIRSTOWNER-OPERATOR-OBSERVATION-01'
CONFIG_SHA = '77b0d25966a7a2634f13a8dbfeeac771d2ad461fb789d4f840d6b5662dfd54af'
ORIGINAL_NOTE = '484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7'
PREPARED_BUNDLE = Path('/home/gama/sgx535-offline/frozen-staging-314e2f3b-20261002T234125Z')
require = runtime.require


def exact(obj, expected, reason):
    require(isinstance(obj, dict) and all(type(obj.get(k)) is type(v)
            and obj[k] == v for k,v in expected.items()), reason)


def validate_staging(records, command, stderr):
    require(isinstance(command, dict), 'staging capture command required')
    exact(command, {'exit_status':0}, 'staging capture status')
    require(type(stderr) is str and stderr == '', 'explicit empty staging stderr required')
    timing = capture.validate_receipt(records, command['exit_status'], stderr,
                                     command.get('host_duration_seconds'), 'stock_preparation')
    root = records[1]
    exact(root, {'classification':'PASS','euid':0,'module_initstate':'live',
                 'loaded_module_note_sha256':ORIGINAL_NOTE}, 'verified STOCK staging context')
    boot = timing['boot_id']
    require(str(uuid.UUID(boot)) == boot, 'canonical staging boot UUID required')
    files = root.get('files')
    require(isinstance(files, dict), 'independent staged-file receipts required')
    for key,path,size,digest in [('candidate_image', IMAGE,50805231,IMAGE_SHA),
                                ('custom_cfg','/boot/grub/custom.cfg',4132,CONFIG_SHA)]:
        row = files.get(key)
        exact(row, {'path':path,'size':size,'sha256':digest,'regular':True,
                    'symlink':False,'uid':0,'gid':0,'mode':'0o644','nlink':1},
              'independent staged identity mismatch: '+key)
        require(all(type(row.get(k)) is int and row[k]>0 for k in ['dev','ino']),
                'staged creation inode missing: '+key)
    exact(root.get('grubenv'), {'exit_code':0,'stderr':'',
                              'stdout':'saved_entry='+STOCK_ID+'\n'},
          'independent STOCK/default state mismatch')
    return root


def validate_menu(receipt, staging_records, staging_command, staging_stderr):
    root = validate_staging(staging_records, staging_command, staging_stderr)
    exact(receipt, {'procedure_id':PROCEDURE_ID,'evidence_kind':'direct_operator_report',
                   'reported_entry_text':TITLE,'candidate_build_id':BUILD_ID,
                   'candidate_visibly_present':True,'stock_visibly_present':True,
                   'photograph_supplied':False,'staged_image_path':IMAGE,
                   'staged_image_sha256':IMAGE_SHA,'staged_image_size':50805231,
                   'staging_boot_id':root['boot_id'],'candidate_boots_before_selection':0},
          'operator menu receipt disagrees with frozen candidate or prospective scope')
    require(isinstance(receipt.get('operator_report_reference'), str)
            and bool(receipt['operator_report_reference'].strip()), 'saved operator report required')
    return {'classification':'CONSISTENT SUPPLIED PROSPECTIVE MENU RECEIPT',
            'procedure_id':PROCEDURE_ID,'evidence_kind':'direct_operator_report',
            'photograph_supplied':False,'legacy_v2_satisfied':False,
            'boot_authorized':False,'sgx_authorized':False,
            'live_provenance':'INDEPENDENT VERIFICATION REQUIRED'}


def validate(records, witness, menu_receipt, staging_records, staging_command, staging_stderr):
    validate_menu(menu_receipt, staging_records, staging_command, staging_stderr)
    require(isinstance(witness, dict), 'operator witness required')
    exact(witness, {'selection_photo':False}, 'photograph absence must remain explicit')
    for key in runtime.OBSERVATIONS:
        if key != 'selection_photo':
            require(witness.get(key) is True, 'missing/failed operator observation: '+key)
    report = runtime._validate_runtime(records, witness)
    require(report['boot_id'] != staging_records[1]['boot_id'],
            'candidate boot must differ from verified STOCK staging boot')
    report.update(classification='CONSISTENT SUPPLIED FIRST-OWNER OPERATOR-OBSERVATION RECORDS',
                  procedure_id=PROCEDURE_ID, evidence_kind='direct_operator_report',
                  photograph_supplied=False, legacy_v2_satisfied=False)
    return report


def preboot_source(staging_records, staging_command, staging_stderr):
    """Generate one permitted passive continuity capture, never execute it.

    Reuse historical image bytes qualification; freshly stat its creation inode.
    Runtime state changed after the operator's normal STOCK return. No image
    reads, staging, boot control, DRM open, SGX, or procedure authorization here.
    """
    root = validate_staging(staging_records, staging_command, staging_stderr)
    plan = json.loads((PREPARED_BUNDLE/'preparation.json').read_text())
    data = (PREPARED_BUNDLE/'poststage-root.py').read_bytes()
    pin = plan['files']['poststage-root.py']
    require(len(data) == pin['size'] and hashlib.sha256(data).hexdigest() == pin['sha256'],
            'prepared passive source drift')
    tree = ast.parse(data.decode())
    health = [node for node in tree.body if isinstance(node, ast.Expr)
              and isinstance(node.value, ast.Call) and isinstance(node.value.func, ast.Name)
              and node.value.func.id == 'exec']
    require(len(health) == 1, 'prepared health predicate provenance drift')
    health_source = ast.literal_eval(health[0].value.args[0])
    expected = {key:{field:root['files'][key][field] for field in
                    ['path','size','dev','ino','uid','gid','mode','nlink','regular','symlink']}
                for key in ['candidate_image','custom_cfg']}
    source = '''import os,stat,json,re,subprocess,hashlib,sys
exec(HEALTH_SOURCE)
EXPECTED=EXPECTED_VALUE
PRIOR_BOOT=PRIOR_VALUE
result={'scope':'read-only prospective preboot continuity; no image reads/staging/DRM open/SGX/boot operations','guards':[],'commands':[],'files_metadata':{},'reused_staging_image_sha256':IMAGE_SHA_VALUE}
def check(ok,name):
 result['guards'].append({'name':name,'pass':bool(ok)})
 if not ok:raise RuntimeError(name)
def read(path):
 fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK)
 try:
  with os.fdopen(fd,'rb',closefd=False) as f:return f.read()
 finally:os.close(fd)
def text(path):return read(path).decode().strip()
def run(argv):
 cp=subprocess.run(argv,stdin=subprocess.DEVNULL,capture_output=True,timeout=10)
 row={'argv':argv,'exit_code':cp.returncode,'stdout':cp.stdout.decode(errors='replace'),'stderr':cp.stderr.decode(errors='replace')};result['commands'].append(row)
 check(cp.returncode==0 and not cp.stderr,'passive command '+repr(argv));return row['stdout']
try:
 result['euid']=os.geteuid();check(result['euid']==0,'root capture')
 u=os.uname();result.update(kernel=u.release,architecture=u.machine)
 check(u.release=='5.10.240-antix.1-486-smp' and u.machine=='i686','kernel/architecture')
 result['machine']=text('/sys/class/dmi/id/product_name');check(result['machine']=='Inspiron 1210','machine')
 result['boot_id']=text('/proc/sys/kernel/random/boot_id');check(result['boot_id']!=PRIOR_BOOT,'expected new STOCK boot after manual return')
 result['cmdline']=text('/proc/cmdline');check(result['cmdline']==CMDLINE_VALUE,'STOCK command line')
 result['module_state']=text('/sys/module/gma500_gfx/initstate');check(result['module_state']=='live','original module Live')
 result['loaded_note_sha256']=hashlib.sha256(read('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest();check(result['loaded_note_sha256']==ORIGINAL_VALUE,'loaded original note')
 result['modules']=text('/proc/modules');check(re.search(r'^gma500_gfx .* Live ',result['modules'],re.M) is not None,'module list Live')
 b='/sys/bus/pci/devices/0000:00:02.0';result['pci']={k:text(b+'/'+k) for k in ['vendor','device','subsystem_vendor','subsystem_device','irq']}
 check(result['pci']=={'vendor':'0x8086','device':'0x8108','subsystem_vendor':'0x1028','subsystem_device':'0x02b1','irq':'16'},'PCI identity/IRQ')
 result['pci_driver']=os.path.realpath(b+'/driver');result['driver_module']=os.path.realpath(b+'/driver/module');check(result['pci_driver']=='/sys/bus/pci/drivers/gma500' and result['driver_module']=='/sys/module/gma500_gfx','PCI owner')
 result['drm_device']=os.path.realpath('/sys/class/drm/card0/device');check(result['drm_device']=='/sys/devices/pci0000:00/0000:00:02.0','DRM owner')
 check(stat.S_ISCHR(os.lstat('/dev/dri/card0').st_mode),'DRM metadata only')
 result['framebuffer']=text('/sys/class/graphics/fb0/name');result['framebuffer_dimensions']=text('/sys/class/graphics/fb0/virtual_size');check(result['framebuffer']=='gma500drmfb' and result['framebuffer_dimensions']=='1280,800','framebuffer')
 result['vtcon0']=int(text('/sys/class/vtconsole/vtcon0/bind'));result['vtcon1']=int(text('/sys/class/vtconsole/vtcon1/bind'));check(result['vtcon0']==0 and result['vtcon1']==1,'VT ownership')
 result['interrupts']=text('/proc/interrupts');check(re.search(r'^\\s*16:.*[\\s,]gma500(?:[,\\s]|$)',result['interrupts'],re.M) is not None,'IRQ16 handler')
 result['taint']=int(text('/proc/sys/kernel/tainted'));check(result['taint']==12289,'STOCK taint')
 result['slimski_status']=run(['sv','status','/etc/runit/runsvdir/default/slimski']);check(result['slimski_status'].startswith('run:'),'slimski')
 result['xorg_processes']=run(['pgrep','-a','Xorg']);check(bool(result['xorg_processes'].strip()),'Xorg')
 result['kernel_log']=run(['dmesg']);result['kernel_health']=classify_kernel_log(result['kernel_log']);check(result['kernel_health']['classification']!='REJECT','kernel health')
 check(not os.path.lexists('/run/initramfs/sgx535-first-load.log'),'no experimental hook on STOCK')
 for parent in ['/','/boot','/boot/grub']:
  st=os.lstat(parent);check(stat.S_ISDIR(st.st_mode) and st.st_uid==0 and not stat.S_IMODE(st.st_mode)&0o022,'trusted boot parent '+parent)
 for key,pin in EXPECTED.items():
  st=os.lstat(pin['path']);row={'path':pin['path'],'size':st.st_size,'dev':st.st_dev,'ino':st.st_ino,'uid':st.st_uid,'gid':st.st_gid,'mode':oct(stat.S_IMODE(st.st_mode)),'nlink':st.st_nlink,'regular':stat.S_ISREG(st.st_mode),'symlink':stat.S_ISLNK(st.st_mode)}
  result['files_metadata'][key]=row;check(row==pin,'staged inode/metadata '+key)
 cfg=read('/boot/grub/custom.cfg');result['current_config_sha256']=hashlib.sha256(cfg).hexdigest();check(len(cfg)==4132 and result['current_config_sha256']==CONFIG_VALUE,'small configuration hash')
 check(cfg.count(b'menuentry ')==4 and cfg.count(ENTRY_ID_VALUE.encode())==1 and b'savedefault' not in cfg and b'save_env' not in cfg,'four manual entries')
 result['grub_env']=run(['grub-editenv','/boot/grub/grubenv','list']);check(result['grub_env']=='saved_entry='+STOCK_ID_VALUE+'\\n','STOCK saved default')
 check(text('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot throughout capture')
 result['classification']='PASS READ-ONLY PREBOOT CONTINUITY';print(json.dumps(result,indent=2))
except Exception as e:
 result['classification']='HOLD: PREBOOT CONTINUITY FAILED';result['failure']=str(e);print(json.dumps(result,indent=2));sys.exit(1)
'''
    for placeholder,value in {'HEALTH_SOURCE':health_source,'EXPECTED_VALUE':expected,
        'PRIOR_VALUE':root['boot_id'],'IMAGE_SHA_VALUE':IMAGE_SHA,'CMDLINE_VALUE':runtime.CMDLINE,
        'ORIGINAL_VALUE':ORIGINAL_NOTE,'CONFIG_VALUE':CONFIG_SHA,
        'ENTRY_ID_VALUE':ENTRY_ID,'STOCK_ID_VALUE':STOCK_ID}.items():
        source = source.replace(placeholder,repr(value))
    compile(source,'prospective-preboot-root','exec')
    return source
