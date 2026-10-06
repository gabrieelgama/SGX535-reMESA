#!/usr/bin/env python3
"""Prepare the frozen candidate's isolated first-load route OFFLINE.

Default prints the plan. --output creates an exclusive LOCAL bundle, never
contacts the target. Generated stage-root defaults to passive verification;
--apply requires a fresh STOCK boot UUID and repeats the STOCK guards before
any writes. No boot selection, module operations, SGX or automatic rollback.
Only the available normal first-load staging/passive programs are adapted.
"""
import argparse
import ast
import hashlib
import json
from pathlib import Path
import uuid

import frozen_first_load_capture_v2 as capture
from frozen_first_owner_session import BUILD_ID, WORK, IMAGE_SHA, NOTE_SHA, PROPOSED_IMAGE, OBSERVATIONS

REPO = Path(__file__).resolve().parents[2]
HISTORY = REPO / 'docs/hardware-evidence/MINI12-20261002T054751Z-DIAGNOSTIC-STAGING-01'
RELEASE = '5.10.240-antix.1-486-smp'
IMAGE = PROPOSED_IMAGE
OLD_IMAGE = '/boot/initrd.img-' + RELEASE + '-sgx535-firstload-diagnostic-01'
INCOMING = '/home/gama/sgx535-firstload-incoming-' + BUILD_ID
BACKUP = '/boot/grub/custom.cfg.pre-' + BUILD_ID
PENDING = '/boot/grub/.custom.cfg.' + BUILD_ID + '.pending'
ENTRY_ID = 'sgx535-rev121-frozen-' + BUILD_ID
TITLE = 'EXPERIMENTAL SGX535 rev121 FROZEN ' + BUILD_ID + ' FIRST-LOAD ONLY (no SGX)'
PRIOR_SHA = 'a2d2b6bd33868deefe64e5e1cb2cd9ecedbfd1fe0e78370af983a92a2ec655c5'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def replace_once(source, old, new):
    if source.count(old) != 1:
        raise ValueError('template drift: ' + old[:100])
    return source.replace(old, new)


def assignments(source, values):
    """Bind only named top-level constants; missing/duplicate bindings stop."""
    lines = source.splitlines(keepends=True)
    offsets = [0]
    for line in lines:
        offsets.append(offsets[-1] + len(line))
    edits = []; seen = set()
    for node in ast.parse(source).body:
        if isinstance(node, ast.Assign) and len(node.targets) == 1:
            name = node.targets[0]
            if isinstance(name, ast.Name) and name.id in values:
                if name.id in seen:
                    raise ValueError('duplicate template constant ' + name.id)
                seen.add(name.id); value = node.value
                edits.append((offsets[value.lineno-1] + value.col_offset,
                              offsets[value.end_lineno-1] + value.end_col_offset,
                              repr(values[name.id])))
    if seen != set(values):
        raise ValueError('missing template constants ' + str(set(values)-seen))
    for start, end, value in sorted(edits, reverse=True):
        source = source[:start] + value + source[end:]
    return source


def prior_config():
    identities = json.loads((HISTORY/'source-identities.json').read_text())
    data = Path(identities['merged_config']).read_bytes()
    if len(data) != 3042 or sha(data) != PRIOR_SHA:
        raise ValueError('preserved three-entry config drift')
    return data


def stock_capture(merged, after=False, recovery=False):
    source = (HISTORY/'poststage-root.py').read_text()
    if after:
        # Keep all historical pins, adding the new pair and its backup.
        source = replace_once(source, "('/boot/grub/custom.cfg',3042,'"+PRIOR_SHA+"')",
                              repr(('/boot/grub/custom.cfg', len(merged), sha(merged))))
        source = replace_once(source, " 'diagnostic_image':", " 'candidate_image':" + repr((IMAGE,50805231,IMAGE_SHA)) + ",\n 'candidate_backup':" + repr((BACKUP,3042,PRIOR_SHA)) + ",\n 'diagnostic_image':")
        source = source.replace("'entries':3", "'entries':4")
        prefix = "import argparse,uuid\np=argparse.ArgumentParser();p.add_argument('--stock-boot',required=True);args=p.parse_args()\nif str(uuid.UUID(args.stock_boot))!=args.stock_boot:p.error('canonical boot UUID required')\nSTOCK_BOOT=args.stock_boot\n"
        source = replace_once(source, "result['boot_id']=='d78d349e-daac-43aa-b7f9-156485506ca5'", "result['boot_id']==STOCK_BOOT")
        extras = "\n check(not os.path.lexists(" + repr(PENDING) + "),'candidate pending absent')\n check(custom_bytes.startswith(read(" + repr(BACKUP) + ")) and custom_bytes.count(" + repr(ENTRY_ID.encode()) + ")==1 and custom_bytes.count(" + repr(IMAGE.encode()) + ")==1,'candidate entry and preserved prefix')\n"
        if recovery:
            prefix = prefix.replace('--stock-boot', '--prior-boot').replace('args.stock_boot', 'args.prior_boot')
            source = source.replace("result['boot_id']==STOCK_BOOT", "result['boot_id']!=STOCK_BOOT")
            extras += " check(not os.path.lexists('/run/initramfs/sgx535-first-load.log'),'first-load hook absent on STOCK recovery')\n"
    else:
        prefix = ''
        source = replace_once(source, "result['boot_id']=='d78d349e-daac-43aa-b7f9-156485506ca5'", "bool(re.fullmatch(r'[0-9a-f]{8}(?:-[0-9a-f]{4}){3}-[0-9a-f]{12}', result['boot_id']))")
        extras = "\n for path in " + repr([IMAGE, BACKUP, PENDING, INCOMING]) + ":check(not os.path.lexists(path),'new path absent '+path)\n"
        extras += " for path in ['/boot','/boot/grub','/home/gama']:\n  st=os.lstat(path);check(stat.S_ISDIR(st.st_mode) and st.st_uid==(1000 if path=='/home/gama' else 0) and not stat.S_IMODE(st.st_mode)&0o022,'trusted parent '+path)\n"
        extras += " check(not os.path.lexists('/run/initramfs/sgx535-first-load.log'),'no derivative hook on STOCK boot')\n"
        extras += " check(os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize>=134217728+50805231,'boot free margin')\n check(os.statvfs('/home/gama').f_bavail*os.statvfs('/home/gama').f_frsize>=50805231+16777216,'incoming free margin')\n"
        extras += " check(not os.statvfs('/boot').f_flag&getattr(os,'ST_RDONLY',1),'boot writable filesystem')\n"
    source = replace_once(source, " result['boot_id_end']=", extras + " result['boot_id_end']=")
    return prefix + source


CREATE_FILE = '''def create_file(path,data):
 fd=os.open(path,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600)
 s=os.fstat(fd);row={'path':path,'dev':s.st_dev,'ino':s.st_ino,'phase':'created','expected_size':len(data),'sha256':digest(data)}
 result['operations'].append(row)
 try:
  view=memoryview(data);written=0
  while written<len(view):
   count=os.write(fd,view[written:]);need(type(count) is int and 0<count<=len(view)-written,'zero/invalid write '+path);written+=count
  os.fchown(fd,0,0);os.fchmod(fd,0o644);os.fsync(fd)
 finally:os.close(fd)
 parent_sync(path)
 st,b=read_nofollow(path);need((st.st_dev,st.st_ino)==(s.st_dev,s.st_ino),'published inode drift '+path)
 need(digest(b)==digest(data) and len(b)==len(data),'write readback '+path)
 need(st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o644 and st.st_nlink==1,'published metadata '+path)
 row.update(size=len(b),uid=st.st_uid,gid=st.st_gid,mode=oct(stat.S_IMODE(st.st_mode)),nlink=st.st_nlink,fsync='PASS',readback='PASS',phase='verified')
def publish_config(pending,destination):
 st=os.lstat(pending)
 row={'source':pending,'destination':destination,'dev':st.st_dev,'ino':st.st_ino,'phase':'rename-attempted'}
 result['publication']=row
 os.replace(pending,destination);row['phase']='renamed'
 parent_sync(destination);row['phase']='directory-synced'
'''

READ_NOFOLLOW = '''def read_nofollow(path):
 fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK|os.O_CLOEXEC)
 try:
  s=os.fstat(fd);need(stat.S_ISREG(s.st_mode) and s.st_nlink==1,'regular single-link input '+path)
  parts=[]
  while True:
   q=os.read(fd,1024*1024)
   if not q:break
   parts.append(q)
  return s,b''.join(parts)
 finally:os.close(fd)
'''


def staging(entry, merged, preflight):
    source = (HISTORY/'stage-root.py').read_text()
    source = assignments(source, dict(E=INCOMING, ID=None, IMG=IMAGE, BACK=BACKUP,
        TMP=PENDING, IMAGE_SHA=IMAGE_SHA, IMAGE_SIZE=50805231,
        ENTRY_SHA=sha(entry), ENTRY_SIZE=len(entry), OLD_CFG_SHA=PRIOR_SHA,
        OLD_CFG_SIZE=3042, MERGED_SHA=sha(merged), MERGED_SIZE=len(merged),
        NEW_PATH=IMAGE, NEW_TITLE=TITLE, NEW_ID=ENTRY_ID))
    source = replace_once(source, 'ID=None', 'ID=args.stock_boot')
    source = source.replace("merged.count(b'menuentry ')==3", "merged.count(b'menuentry ')==4")
    source = source.replace('prior_two_entry_bytes_exact_prefix', 'prior_three_entry_bytes_exact_prefix')
    # Both pre- and post-publication checks retain the old diagnostic files too.
    insertion = repr((OLD_IMAGE,50805273,'376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae'))+','+repr(('/boot/grub/custom.cfg.pre-diagnostic-01',2040,'269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8'))+','
    source = source.replace("for p,size,h in [('/boot/initrd.img-'+R+'-sgx535-firstload-", "for p,size,h in ["+insertion+"('/boot/initrd.img-'+R+'-sgx535-firstload-")
    start = source.index('def create_file('); end = source.index('def check_stock(', start)
    source = source[:start] + CREATE_FILE + source[end:]
    start = source.index('def read_nofollow('); end = source.index('def absent(',start)
    source = source[:start] + READ_NOFOLLOW + source[end:]
    source = replace_once(source, 'os.replace(TMP,CFG);parent_sync(CFG)', 'publish_config(TMP,CFG)')
    guards = "\n import pwd\n need(pwd.getpwnam('gama').pw_uid==UID and pwd.getpwnam('gama').pw_gid==GID,'incoming owner identity')\n es=os.lstat(E);need(stat.S_ISDIR(es.st_mode) and es.st_uid==UID and es.st_gid==GID and stat.S_IMODE(es.st_mode)==0o700,'protected incoming directory')\n need(set(os.listdir(E))=={'initrd.img-sgx535-firstload-diagnostic-01','diagnostic-entry.proposed'},'incoming inventory')\n"
    source = replace_once(source, " src_img=E+", guards + " src_img=E+")
    source = replace_once(source, " result['classification']='DIAGNOSTIC STAGING PASS; NO BOOT/SGX ACTION'", " result['stock_preflight']=fresh\n result['classification']='FROZEN CANDIDATE STAGING PASS; NO BOOT/SGX ACTION'")
    prefix = '''import argparse,uuid,io,contextlib,json,sys
p=argparse.ArgumentParser(description='Default passive STOCK verification. Explicit --apply stages files only; never boots or invokes SGX.')
p.add_argument('--apply',action='store_true');p.add_argument('--stock-boot');args=p.parse_args()
if args.apply:
 try:uuid.UUID(args.stock_boot)
 except (ValueError,TypeError,AttributeError):p.error('--apply requires a fresh STOCK UUID via --stock-boot')
ns={}
with contextlib.redirect_stdout(io.StringIO()):exec(PREFLIGHT,ns)
fresh=ns['result']
if not args.apply:
 print(json.dumps(fresh,indent=2));sys.exit(0 if fresh['classification']=='PASS' else 1)
if fresh['classification'] != 'PASS' or fresh['boot_id'] != args.stock_boot:
 print(json.dumps({'classification':'STOP BEFORE STAGING','preflight':fresh,'retry':False}));sys.exit(1)
'''.replace('PREFLIGHT', repr(preflight))
    # At apply time incoming has already been exclusively created and populated.
    # Incoming absence is for pre-transfer verification, not post-transfer apply.
    apply_preflight = preflight.replace(repr([IMAGE, BACKUP, PENDING, INCOMING]), repr([IMAGE, BACKUP, PENDING]))
    prefix = prefix.replace("exec("+repr(preflight)+",ns)", "exec("+repr(apply_preflight)+" if args.apply else "+repr(preflight)+",ns)")
    return prefix + source


def first_owner(merged):
    source = (HISTORY/'diagnostic-experimental-root.py').read_text()
    source = replace_once(source, "expected_note='a74fb4f5b624980dfb717cb4c5c6fc400da4a9aa0dde264cd2365b5c7c3e4f8e'", 'expected_note='+repr(NOTE_SHA))
    source = replace_once(source, "('/boot/grub/custom.cfg',3042,'"+PRIOR_SHA+"')", repr(('/boot/grub/custom.cfg',len(merged),sha(merged))))
    source = replace_once(source, ' for path,size,digest in [', ' for path,size,digest in [' + repr((IMAGE,50805231,IMAGE_SHA)) + ',' + repr((BACKUP,3042,PRIOR_SHA)) + ',')
    a=source.index(" incoming='/home/gama/"); b=source.index(" trusted('/boot')", a)
    source=source[:a]+source[b:]
    source=source.replace("cfg_new.count(b'menuentry ')==3", "cfg_new.count(b'menuentry ')==4")
    source=replace_once(source, "check(result['boot_id']!='d78d349e-daac-43aa-b7f9-156485506ca5'", "check(result['boot_id']!=args.prior_boot")
    extra=" check(cfg_new.startswith(read("+repr(BACKUP)+")) and cfg_new.count("+repr(ENTRY_ID.encode())+")==1 and cfg_new.count("+repr(IMAGE.encode())+")==1,'unique frozen entry and exact old prefix')\n check(not os.path.lexists("+repr(PENDING)+"),'candidate pending absent')\n check(result['framebuffer_dimensions']=='1280,800','framebuffer dimensions')\n"
    source=replace_once(source, " result['hook_log']=", extra+" result['hook_log']=")
    prefix="import argparse,uuid\np=argparse.ArgumentParser();p.add_argument('--prior-boot',required=True);args=p.parse_args()\nif str(uuid.UUID(args.prior_boot))!=args.prior_boot:p.error('canonical boot UUID required')\n"
    return prefix+source


def incoming():
    """Explicit unprivileged directory preparation, bound to verified STOCK."""
    source='''import os,sys,stat,pwd,json,hashlib,argparse,uuid
p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');p.add_argument('--stock-boot');args=p.parse_args()
DEST=DEST_VALUE
if not args.prepare:
 print(json.dumps({'mode':'PREPARATION ONLY','destination':DEST,'writes':False,'sgx':False}));sys.exit(0)
try:
 if str(uuid.UUID(args.stock_boot))!=args.stock_boot:raise ValueError('UUID spelling')
except (ValueError,TypeError,AttributeError):p.error('canonical fresh STOCK UUID required')
result={'classification':'IN PROGRESS','operations':[],'sgx':False,'boot':False,'retry':False}
def need(x,msg):
 if not x:raise RuntimeError(msg)
try:
 need(os.geteuid()==1000 and pwd.getpwnam('gama').pw_uid==1000 and pwd.getpwnam('gama').pw_gid==1000,'expected target user')
 need(os.uname().release=='5.10.240-antix.1-486-smp' and os.uname().machine=='i686','kernel drift')
 need(open('/proc/sys/kernel/random/boot_id').read().strip()==args.stock_boot,'STOCK boot drift')
 need(open('/sys/module/gma500_gfx/initstate').read().strip()=='live','STOCK Live state')
 need(hashlib.sha256(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read()).hexdigest()=='484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7','STOCK module identity')
 need(os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500','PCI ownership drift')
 st=os.lstat('/home/gama');need(stat.S_ISDIR(st.st_mode) and st.st_uid==1000 and not st.st_mode&0o022,'protected user home')
 os.mkdir(DEST,0o700)
 st=os.lstat(DEST);result['operations'].append({'path':DEST,'dev':st.st_dev,'ino':st.st_ino,'uid':st.st_uid,'gid':st.st_gid,'mode':stat.S_IMODE(st.st_mode),'phase':'created'})
 need(stat.S_ISDIR(st.st_mode) and st.st_uid==1000 and st.st_gid==1000 and stat.S_IMODE(st.st_mode)==0o700 and not os.listdir(DEST),'exclusive empty protected incoming')
 fd=os.open('/home/gama',os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
 try:os.fsync(fd)
 finally:os.close(fd)
 need(open('/proc/sys/kernel/random/boot_id').read().strip()==args.stock_boot,'boot changed during incoming preparation')
 result['classification']='INCOMING PREPARATION PASS';result['boot_id']=args.stock_boot
 print(json.dumps(result,indent=2))
except Exception as exc:
 result['classification']='HOLD: INCOMING INCOMPLETE OR NOT STARTED';result['error']=repr(exc);print(json.dumps(result,indent=2));sys.exit(1)
'''
    return source.replace('DEST_VALUE',repr(INCOMING))


def prepare():
    old=prior_config()
    proposal=Path(WORK+'/image/build-01/proposed-custom.cfg').read_text()
    proposal=replace_once(proposal, 'EXPERIMENTAL SGX535 rev121 DIAGNOSTIC FIRST-LOAD ONLY (no SGX)', TITLE)
    proposal=replace_once(proposal, 'sgx535-rev121-firstload-diagnostic-01', ENTRY_ID)
    proposal=replace_once(proposal, OLD_IMAGE, IMAGE)
    entry=proposal.encode();merged=old+entry
    if old.count(b'menuentry ')!=3 or merged.count(b'menuentry ')!=4 or b'savedefault' in merged or b'save_env' in merged:
        raise ValueError('GRUB structure drift')
    preflight=stock_capture(merged)
    bundle={'diagnostic-entry.proposed':entry,'custom.cfg.proposed':merged,
            'incoming-user.py':incoming().encode(),
            'stock-preflight-root.py':preflight.encode(),
            'stage-root.py':staging(entry,merged,preflight).encode(),
            'poststage-root.py':stock_capture(merged,after=True).encode(),
            'first-owner-root.py':first_owner(merged).encode(),
            'stock-recovery-root.py':stock_capture(merged,after=True,recovery=True).encode()}
    witness={key:None for key in OBSERVATIONS + ['expected_boot_id','prior_boot_id',
        'capture_status','capture_stderr','host_duration_seconds',
        'experimental_boots','sgx_actions','hot_module_actions']}
    bundle['operator-observations.template.json']=(json.dumps(witness,indent=2)+'\n').encode()
    for name,data in bundle.items():
        if name.endswith('.py'):compile(data,name,'exec')
    return bundle


def plan(bundle, directory=None):
    ssh=json.loads((HISTORY/'incoming-dir-command.json').read_text())['argv'][:-1]
    scp=json.loads((HISTORY/'scp-command.json').read_text())['argv'][:-3]
    local=str(directory.resolve()) if directory else '<LOCAL_BUNDLE>'
    source=WORK+'/image/build-01/initrd.img-sgx535-firstload-diagnostic-01'
    return dict(mode='OFFLINE PREPARATION ONLY',build_id=BUILD_ID,image_sha256=IMAGE_SHA,
        image_source=WORK+'/image/build-01/initrd.img-sgx535-firstload-diagnostic-01',
        image_destination=IMAGE,entry_id=ENTRY_ID,title=TITLE,incoming=INCOMING,
        preserved_config_sha256=PRIOR_SHA,backup=BACKUP,pending=PENDING,
        merged_config_sha256=sha(bundle['custom.cfg.proposed']),
        files={n:dict(size=len(b),sha256=sha(b)) for n,b in bundle.items()},
        ssh_prefix=ssh,scp_prefix=scp,
        prepared_operations={
            'incoming':{'argv':ssh+["python3 -I -B -S - --prepare --stock-boot <FRESH_STOCK_UUID>"],
                        'stdin_file':local+'/incoming-user.py','requires':'PASS current STOCK preflight'},
            'transfer':{'argv':scp+[source,local+'/diagnostic-entry.proposed','gama@192.168.18.90:'+INCOMING+'/'],
                        'requires':'successful exclusive incoming creation receipt; never retry into partial incoming'},
            'stage':{'argv':ssh+["sudo -n -p '' python3 -I -B -S - --apply --stock-boot <FRESH_STOCK_UUID>"],
                     'stdin_file':local+'/stage-root.py','requires':'same fresh STOCK UUID; all guards repeated before writes'},
            'passive_captures':{'mechanism':'frozen_first_load_capture_v2.capture_once',
                'source_generator':'python3 -B tools/psb-dri-re/frozen_candidate_stage.py --capture-source <phase> --boot <UUID>',
                'requires':'actual readiness, exclusive evidence directory; stdout/stderr/status/timing retained'},
        },
        sequence=['physical STOCK selection/readiness','passive STOCK preflight',
                  'exclusive incoming creation + pinned image/entry transfer',
                  'stage-root --apply --stock-boot <fresh UUID>',
                  'poststage passive readback with same UUID',
                  'operator selects unique manual entry once; saved STOCK unchanged',
                  'first-owner passive capture --prior-boot <STOCK UUID>',
                  'supplied-record checker + actual operator/timing observations',
                  'STOP at independent opening decision; no invocation code in this bundle'],
        rollback='Before selection, on a permitted rollback only: preserve receipts; '
                 'restore custom.cfg from the unique backup only after exact old/merged '
                 'byte and creation-inode checks. Never delete historical images/backups. '
                 'No automatic rollback after partial writes. After boot/HOLD use only '
                 'the reviewed operator machine boundary and STOCK selection.',
        boot_watch_seconds=600,capture_max_boot_seconds=1200,
        connection_seconds=40,child_seconds=35,
        sgx_invocation_available=False,boot_automated=False,automatic_retry=False)


def main(argv=None):
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output',type=Path,help='exclusive LOCAL preparation directory')
    p.add_argument('--capture-source',choices=['stock-preflight','poststage','first-owner','stock-recovery'])
    p.add_argument('--boot',help='fresh UUID binding for poststage/first-owner/recovery')
    args=p.parse_args(argv)
    bundle=prepare()
    if args.capture_source:
        if args.output:p.error('choose output OR capture-source')
        source=bundle[args.capture_source+'-root.py'].decode()
        mode='stock_preparation'
        if args.capture_source!='stock-preflight':
            try:uuid.UUID(args.boot)
            except (ValueError,TypeError,AttributeError):p.error('fresh --boot UUID required')
            if str(uuid.UUID(args.boot))!=args.boot:p.error('canonical boot UUID required')
            arg='--stock-boot' if args.capture_source=='poststage' else '--prior-boot'
            source='import sys\nsys.argv='+repr(['passive-root',arg,args.boot])+'\n'+source
            mode={'poststage':'stock_preparation','first-owner':'experimental','stock-recovery':'stock_recovery'}[args.capture_source]
        print(capture.wrapper_source(source,mode),end='');return 0
    report=plan(bundle,args.output)
    if args.output:
        args.output.mkdir(mode=0o700,exist_ok=False)
        for name,data in bundle.items():
            with (args.output/name).open('xb') as f:f.write(data)
        with (args.output/'preparation.json').open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps(report,indent=2));return 0


if __name__=='__main__':raise SystemExit(main())
