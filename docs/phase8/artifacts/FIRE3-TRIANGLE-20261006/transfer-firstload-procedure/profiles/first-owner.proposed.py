import sys
sys.argv=['first-owner', '--prior-boot', '8cc7f919-58e3-4eda-b941-f8e356a370c5']
def read_source_channel():
    """Future authorized passive use only; reject substituted filesystem nodes.

    The exact loaded-module/boot and before-call UNUSED receipts must separately
    prove fresh producer origin. This function is never called by offline tests.
    """
    fd = os.open('/proc/sgx535_source_guard', os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK | os.O_CLOEXEC)
    try:
        info = os.fstat(fd)
        require(stat.S_ISREG(info.st_mode) and info.st_uid == 0 and
                stat.S_IMODE(info.st_mode) == 0o400 and info.st_nlink == 1,
                'protected kernel evidence node required')
        # Verify the already-open fd belongs to procfs, rather than trusting a
        # path or a regular file copied into another mounted filesystem.
        mount_id = None
        for line in Path('/proc/self/fdinfo/' + str(fd)).read_text().splitlines():
            if line.startswith('mnt_id:'): mount_id = line.split(':',1)[1].strip()
        entries = Path('/proc/self/mountinfo').read_text().splitlines()
        require(mount_id is not None and any(row.split()[0] == mount_id and
                row.split(' - ',1)[1].split()[0] == 'proc' for row in entries),
                'evidence fd must originate in procfs')
        data = bytearray()
        while len(data) <= 4096:
            try: block = os.read(fd, 4096 + 1 - len(data))
            except InterruptedError: continue  # passive file read only
            if not block: break
            data.extend(block)
        require(0 < len(data) <= 4096, 'incomplete/oversized source witness')
        return bytes(data)
    finally:
        os.close(fd)
FIELDS = {'state', 'reasons', 'producers_active', 'driver_attached', 'capsule_state',
          'boot', 'prepared', 'asserted_reset', 'released_reset', 'startup_pending',
          'startup_reads', 'startup_fault', 'startup_autonomous', 'delayed_exclusion'}
def evaluate(raw, *, closed=False):
    errors = []
    values = {}
    try:
        if len(raw) > 4096 or not raw.endswith(b'\n'):
            raise ValueError('incomplete or oversized record')
        lines = raw.decode('ascii').splitlines()
        if not lines or lines.pop(0) != 'SGXSOURCE2':
            raise ValueError('unsupported source record')
        for line in lines:
            key, value = line.split('=', 1)
            if key not in FIELDS or key in values:
                raise ValueError('unexpected or duplicate field')
            if key == 'delayed_exclusion':
                values[key] = value
            else:
                if not value.isdecimal() or int(value) > 0xffffffff:
                    raise ValueError('invalid integer')
                values[key] = int(value)
        if set(values) != FIELDS:
            raise ValueError('missing field')
        required = dict(state=3 if closed else 0, reasons=0, producers_active=0,
                        driver_attached=1, capsule_state=4 if closed else 0,
                        boot=3, prepared=1, asserted_reset=127, released_reset=0,
                        startup_pending=0, startup_reads=0, startup_fault=0,
                        startup_autonomous=0,
                        delayed_exclusion='STARTUP_LIFECYCLE')
        errors += [key + ': witness condition not satisfied'
                   for key, value in required.items() if values[key] != value]
    except (ValueError, UnicodeError) as error:
        errors.append(str(error))
    return {'supplied_record_consistency': 'CONFIRMED' if not errors else 'UNKNOWN',
            'interval': 'CLOSED' if closed else 'PREPARED, MUST REMAIN CONTINUOUS',
            'hardware_attribution': 'UNKNOWN WITHOUT VERIFIED LIVE BOOT/PRODUCER PROVENANCE',
            'triangle': 'NOT ESTABLISHED BY SOURCE WITNESS',
            'errors': errors, 'values': values}
import sys
sys.argv=['passive-first-owner','--prior-boot','30b2f934-8cb4-4fa9-9a2d-73a3ccf01524']
from pathlib import Path
import struct,base64
SIZE=4152
CHANNEL='/proc/sgx535_current_operation'
def require(value,message):
 if not value:raise ValueError(message)
def read_channel():
    """Future authorized passive use only; reject substituted filesystem nodes.

    The exact loaded-module/boot and before-call UNUSED receipts must separately
    prove fresh producer origin. This function is never called by offline tests.
    """
    fd = os.open(CHANNEL, os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK | os.O_CLOEXEC)
    try:
        info = os.fstat(fd)
        require(stat.S_ISREG(info.st_mode) and info.st_uid == 0 and
                stat.S_IMODE(info.st_mode) == 0o400 and info.st_nlink == 1,
                'protected kernel evidence node required')
        # Verify the already-open fd belongs to procfs, rather than trusting a
        # path or a regular file copied into another mounted filesystem.
        mount_id = None
        for line in Path('/proc/self/fdinfo/' + str(fd)).read_text().splitlines():
            if line.startswith('mnt_id:'): mount_id = line.split(':',1)[1].strip()
        entries = Path('/proc/self/mountinfo').read_text().splitlines()
        require(mount_id is not None and any(row.split()[0] == mount_id and
                row.split(' - ',1)[1].split()[0] == 'proc' for row in entries),
                'evidence fd must originate in procfs')
        data = bytearray()
        while len(data) <= SIZE:
            try: block = os.read(fd, SIZE + 1 - len(data))
            except InterruptedError: continue  # passive file read only
            if not block: break
            data.extend(block)
        require(len(data) == SIZE, 'incomplete/oversized kernel capsule')
        return bytes(data)
    finally:
        os.close(fd)
import argparse,uuid
p=argparse.ArgumentParser();p.add_argument('--prior-boot',required=True);args=p.parse_args()
if str(uuid.UUID(args.prior_boot))!=args.prior_boot:p.error('canonical boot UUID required')
import os,sys,stat,json,hashlib,re,subprocess,datetime
exec('"""Operational kernel-log guard; known diagnostics remain visible, never generic fault exemptions.\n\nThe bounded diagnostics derive from preserved captures and the exact kernel\nloader source, not an arbitrary caller-provided whitelist. Fault-bearing/repeated/changed variants\nfail closed. This is an operational predicate, not architectural health proof.\n"""\nimport re\nfrom collections import Counter\n\n# Strip only dmesg transport prefixes; retain the message and all diagnostic fields.\nPREFIX = re.compile(r"^(?:<\\d+>)?\\s*(?:\\[\\s*\\d+(?:\\.\\d+)?\\]\\s*)?(?:kernel:\\s*)?", re.I)\nFATAL = re.compile(r"^(?:BUG:)|\\bkernel BUG at\\b|\\bOops:|\\bKernel panic\\b|\\bgeneral protection(?: fault|:)"\n                   r"|\\bCall Trace:|\\b(?:soft|hard)\\s+LOCKUP\\b|\\bblocked for more than\\b"\n                   r"|\\brcu[^\\n]*\\b(?:stall|stalls)\\b", re.I)\nWARNING = re.compile(r"\\bWARNING:|\\bWARN_ON\\b|\\bBUG:", re.I)\nGRAPHICS = re.compile(r"\\b(?:gma500|drm|psb|sgx|pvr)\\b|0000:00:02\\.0", re.I)\nGRAPHICS_ERROR = re.compile(r"\\b(?:BUG:|fault|error|failed|failure|fatal|timeout|timed out|hang|hung|WARN|warning)\\b", re.I)\nACPI_DIAGNOSTICS = {\n \'ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)\': (\'acpi_powerbutton_warning\', 2),\n \'ACPI Error: Could not enable PowerButton event (20200925/evxfevnt-182)\': (\'acpi_powerbutton_error\', 2),\n \'button: probe of LNXPWRBN:00 failed with error -22\': (\'powerbutton_probe\', 1),\n \'tiny-power-button: probe of LNXPWRBN:00 failed with error -22\': (\'tiny_powerbutton_probe\', 1),\n}\n# kernel/module.c emits this exact notice once when permissive signature\n# checking admits an unverified module. Cycle05 loads captured stock drm first.\n# Module provenance/taint/ownership remain separate mandatory checks.\nUNSIGNED_DRM_NOTICE = (\'drm: module verification failed: signature and/or required key missing - tainting kernel\')\nBACKLIGHT = re.compile(r"gma500 0000:00:02\\.0: BL bug: Reg ([0-9a-fA-F]{8}) save ([0-9a-fA-F]{8})")\n\ndef classify_kernel_log(log):\n """Return visible classification receipt; reject on any selected adverse signal."""\n if not isinstance(log, str) or not log.strip():\n  return {\'classification\': \'REJECT\', \'faults\': [{\'reason\': \'missing kernel log\'}], \'stock_diagnostics\': {}}\n counts = Counter(); faults = []\n for number, raw in enumerate(log.splitlines(), 1):\n  message = PREFIX.sub(\'\', raw, count=1)\n  reason = None\n  # Fatal reports are rejected even alongside a previously seen diagnostic.\n  if FATAL.search(message): reason = \'kernel fault/lockup\'\n  else:\n   known = ACPI_DIAGNOSTICS.get(message)\n   if message == UNSIGNED_DRM_NOTICE: known = (\'unsigned_drm_loader_notice\', 1)\n   backlight = BACKLIGHT.fullmatch(message)\n   if backlight:\n    if any(int(value, 16) != 0 for value in backlight.groups()): reason = \'changed backlight diagnostic fields\'\n    else: known = (\'backlight_zero_register\', 1)\n   if known:\n    label, limit = known; counts[label] += 1\n    if counts[label] > limit: reason = \'repeated stock diagnostic: \' + label\n   elif WARNING.search(message): reason = \'kernel warning\'\n   elif re.search(r\'\\bACPI (?:Error|Warning):\', message, re.I): reason = \'unexpected ACPI diagnostic\'\n   elif GRAPHICS.search(message) and GRAPHICS_ERROR.search(message): reason = \'graphics fault/error\'\n  if reason: faults.append({\'line\': number, \'message\': message, \'reason\': reason})\n return {\'classification\': \'REJECT\' if faults else \'PASS WITH BOUNDED STOCK DIAGNOSTICS\',\n         \'faults\': faults, \'stock_diagnostics\': dict(counts)}\n')
HEALTH_SOURCE_SHA256='5abe09ed15609cc2ad409d909d4785953275656690632700c869fc5a48d3b7ee'
RELEASE='5.10.240-antix.1-486-smp'
BDF='/sys/bus/pci/devices/0000:00:02.0'
STOCK={'kernel':('/boot/vmlinuz-'+RELEASE,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),'initrd':('/boot/initrd.img-'+RELEASE,50863580,'f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'),'grub_cfg':('/boot/grub/grub.cfg',10438,'396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'),'grub_env':('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),'original_module':('/lib/modules/'+RELEASE+'/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko',None,'7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb')}
STOCK_ID='gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'
result={'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'scope':'read-only diagnostic first-owner/boot/ownership evidence; no writes, no DRM open, no module/service operations','commands':[],'guards':[]}
def read(path):
 with open(path,'rb') as f:data=f.read()
 result.setdefault('read_paths',[]).append(path)
 return data
def txt(path):return read(path).decode().strip()
def check(condition,name):
 result['guards'].append({'guard':name,'pass':bool(condition)})
 if not condition:raise RuntimeError(name)
def run(argv):
 p=subprocess.run(argv,stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
 row={'argv':argv,'exit_code':p.returncode,'stdout':p.stdout.decode(errors='replace'),'stderr':p.stderr.decode(errors='replace')};result['commands'].append(row)
 check(p.returncode==0 and not p.stderr,'read-only command '+str(argv))
 return row['stdout']
def file_record(path):
 st=os.lstat(path);check(stat.S_ISREG(st.st_mode),'regular stock file '+path)
 data=read(path);return {'path':path,'size':len(data),'sha256':hashlib.sha256(data).hexdigest(),'type':'regular','symlink':False,'uid':st.st_uid,'gid':st.st_gid,'mode':stat.S_IMODE(st.st_mode),'nlink':st.st_nlink,'dev':st.st_dev,'ino':st.st_ino}
def trusted(path):
 current='/'
 for part in path.strip('/').split('/'):
  current=os.path.join(current,part);st=os.lstat(current)
  check(stat.S_ISDIR(st.st_mode) and st.st_uid==0 and not stat.S_IMODE(st.st_mode)&0o022,'trusted directory '+current)
try:
 result['euid']=os.geteuid();check(os.geteuid()==0,'root capture')
 result['kernel']=run(['uname','-r']).strip();check(result['kernel']==RELEASE,'kernel release')
 result['architecture']=run(['uname','-m']).strip();check(result['architecture']=='i686','architecture')
 result['machine']=txt('/sys/class/dmi/id/product_name');check(result['machine']=='Inspiron 1210','DMI machine')
 result['proc_version']=txt('/proc/version');result['cmdline']=txt('/proc/cmdline')
 check(result['cmdline'].split()==('BOOT_IMAGE=/boot/vmlinuz-'+RELEASE+' root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0').split(),'stock command line')
 result['boot_id']=txt('/proc/sys/kernel/random/boot_id');check(str(uuid.UUID(result['boot_id']))==result['boot_id'] and result['boot_id'] not in (args.prior_boot,'a76f3b25-31ae-4507-891b-474e912dd356'),'fresh experimental boot identity');result['taint']=int(txt('/proc/sys/kernel/tainted'));check(not result['taint']&~12289,'known taint mask')
 result['pci']={k:txt(BDF+'/'+k) for k in ['vendor','device','subsystem_vendor','subsystem_device','irq']}
 check(result['pci']=={'vendor':'0x8086','device':'0x8108','subsystem_vendor':'0x1028','subsystem_device':'0x02b1','irq':'16'},'PCI identity/IRQ')
 result['pci_driver']=os.path.realpath(BDF+'/driver');result['driver_module']=os.path.realpath(BDF+'/driver/module');check(result['pci_driver']=='/sys/bus/pci/drivers/gma500' and result['driver_module']=='/sys/module/gma500_gfx','original PCI ownership')
 result['module_state']=txt('/sys/module/gma500_gfx/initstate');check(result['module_state']=='live','original module Live')
 result['modules']=txt('/proc/modules');check(re.search(r'^gma500_gfx .* Live ',result['modules'],re.M) is not None,'module list Live')
 result['loaded_note_sha256']=hashlib.sha256(read('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest()
 expected_note='4499399a9f06a35b924c770ec5f04b8e5a5170c4f744359c675d13ca89223644';check(result['loaded_note_sha256']==expected_note,'derivative loaded Build-ID note')

 result['observer_state']=txt('/sys/module/sgx535_provenance/initstate');check(result['observer_state']=='live','qualified observer Live')
 result['observer_note_sha256']=hashlib.sha256(read('/sys/module/sgx535_provenance/notes/.note.gnu.build-id')).hexdigest();check(result['observer_note_sha256']=='6afbac1cf48a211135e2a4b4ffc30f96ace64f6d397071595bbc328966bb8af0','qualified observer loaded Build-ID note')
 check(re.search(r'^sgx535_provenance .* Live ',result['modules'],re.M) is not None,'observer module listed Live')
 raw=read_channel();fields=struct.unpack('<8s12I',raw[:56]);result['capsule_preparation']={'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest(),'header':list(fields[1:]),'payload_base64':base64.b64encode(raw).decode(),'scope':'ACTUAL PASSIVE INITIAL RECORD; NO INVOCATION'}
 check(fields[:3]==(b'SGXCAPB1',1,4152) and all(v==0 for v in fields[3:]) and raw[56:]==bytes(4096),'capsule UNUSED/clean; zero consumed-operation state')
 result['drm_bdf']=os.path.basename(os.path.realpath('/sys/class/drm/card0/device'));check(result['drm_bdf']=='0000:00:02.0','DRM device ownership')
 st=os.lstat('/dev/dri/card0');result['drm_node']={'mode':st.st_mode,'major':os.major(st.st_rdev),'minor':os.minor(st.st_rdev)};check(stat.S_ISCHR(st.st_mode),'DRM node metadata only')
 result['proc_fb']=txt('/proc/fb');result['framebuffer']=txt('/sys/class/graphics/fb0/name');result['framebuffer_dimensions']=txt('/sys/class/graphics/fb0/virtual_size');check(result['framebuffer']=='gma500drmfb','stock framebuffer')
 result['vtcon0']=int(txt('/sys/class/vtconsole/vtcon0/bind'));result['vtcon1']=int(txt('/sys/class/vtconsole/vtcon1/bind'));check(result['vtcon0']==0 and result['vtcon1']==1,'stock VT ownership')
 result['interrupts']=txt('/proc/interrupts');check(re.search(r'^\s*16:.*[\s,]gma500(?:[,\s]|$)',result['interrupts'],re.M) is not None,'IRQ16 gma500 handler')
 result['slimski_status']=run(['sv','status','/etc/runit/runsvdir/default/slimski']);check(result['slimski_status'].startswith('run:'),'slimski running')
 result['xorg_processes']=run(['pgrep','-a','Xorg']);check(bool(result['xorg_processes'].strip()),'Xorg running')
 result['kernel_log']=run(['dmesg']);result['kernel_health_source_sha256']=HEALTH_SOURCE_SHA256;result['kernel_health']=classify_kernel_log(result['kernel_log']);check(result['kernel_health']['classification']!='REJECT','current-boot kernel health')
 result['stock']={}
 for key,(path,size,digest) in STOCK.items():
  row=file_record(path);result['stock'][key]=row;check(row['sha256']==digest and (size is None or row['size']==size),'stock bytes '+key)
 result['grub_env']=run(['grub-editenv','/boot/grub/grubenv','list']);check(result['grub_env'].strip()=='saved_entry='+STOCK_ID,'saved stock selection only')
 cfg=read('/boot/grub/grub.cfg').decode();check(STOCK_ID in cfg and 'custom.cfg' in cfg,'stock entry and custom.cfg source retained')
 result['destinations']={}
 for path,size,digest in [('/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 50816631, '3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e'),('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 8615, '5b3261d3af8033b9fdefaf8e0a402a3b35a53709ec68365a0ec4b0a6e94e17a0'),('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-01',50804481,'4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71'),('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-corrected-01',50804478,'55e7a8be6f62c9a1d59f1c532b8790399c21a2706884747e504edfffb45bf52d'),('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-diagnostic-01',50805273,'376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae'),('/boot/grub/custom.cfg', 9749, 'ac60694d5e31263d080dd6900ae3ff55feeef3f9e6b9e87c8c3330990d200f59'),('/boot/grub/custom.cfg.cycle06-preserved',974,'181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612'),('/boot/grub/custom.cfg.pre-diagnostic-01',2040,'269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8')]:
  row=file_record(path);result['destinations'][path]=row;check(row['size']==size and row['sha256']==digest and row['uid']==0 and row['gid']==0 and row['mode']==0o644 and row['nlink']==1,'staged destination readback '+path)
 trusted('/boot');trusted('/boot/grub')
 result['mountinfo']=txt('/proc/self/mountinfo');result['mounts']=txt('/proc/mounts');result['boot_free_bytes']=os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize;result['incoming_free_bytes']=os.statvfs('/home/gama').f_bavail*os.statvfs('/home/gama').f_frsize
 check(result['boot_free_bytes']>=134217728 and result['incoming_free_bytes']>=50805273+1002,'staging free space')
 check(os.access('/boot',os.W_OK) and not (os.statvfs('/boot').f_flag&getattr(os,'ST_RDONLY',1)),'boot filesystem writable')
 result['gama_uid']=int(run(['id','-u','gama']).strip());result['gama_gid']=int(run(['id','-g','gama']).strip())
 result['python']=sys.version;result['python_executable']=sys.executable
 check(all(hasattr(os,k) for k in ['O_NOFOLLOW','O_EXCL','O_DIRECTORY','fsync','fchmod','fchown']),'exclusive no-follow/fsync facilities')
 check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')
 cfg_new=read('/boot/grub/custom.cfg'); cfg_old=read('/boot/grub/custom.cfg.pre-diagnostic-01')
 check(hashlib.sha256(cfg_old).hexdigest()=='269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8' and cfg_new.startswith(cfg_old),'prior two-entry custom.cfg preserved')
 check(cfg_new.count(b'menuentry ')==9 and cfg_new.count(b'EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)')==1 and cfg_new.count(b'EXPERIMENTAL SGX535 rev121 CORRECTED FIRST-LOAD ONLY (no triangle)')==1 and cfg_new.count(b'EXPERIMENTAL SGX535 rev121 DIAGNOSTIC FIRST-LOAD ONLY (no SGX)')==1,'three manual entries retained')
 check(cfg_new.count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-diagnostic-01')==1 and cfg_new.count(b'sgx535-rev121-firstload-diagnostic-01')==1 and b'savedefault' not in cfg_new and b'save_env' not in cfg_new,'diagnostic entry points to isolated image without save')
 check(cfg_new.startswith(read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba')) and cfg_new.count(b'sgx535-rev121-frozen-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba')==1 and cfg_new.count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba')==1,'unique frozen entry and exact old prefix')
 check(not os.path.lexists('/boot/grub/.custom.cfg.85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba.pending'),'candidate pending absent')

 for path,size,digest in [('/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb', 50816610, 'ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d'), ('/boot/grub/custom.cfg.pre-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb', 7483, 'd98b0faf6aeec27272b115702b56c1109de89259426a497a49b8774ddeb18a30')]:
  row=file_record(path);result['destinations'][path]=row;check(row['size']==size and row['sha256']==digest and row['uid']==0 and row['gid']==0 and row['mode']==0o644 and row['nlink']==1,'FIRE2 historical artifact preserved '+path)
 check(cfg_new.count(b'sgx535-rev121-frozen-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb')==1,'FIRE2 entry remains preserved')
 check(result['framebuffer_dimensions']=='1280,800','framebuffer dimensions')
 result['hook_log']=txt('/run/initramfs/sgx535-first-load.log');check(result['hook_log'].splitlines()==['SGX535-FIRSTLOAD BEGIN','SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE','SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue'],'complete ordered hook log');check(result['boot_id']!=args.prior_boot,'distinct diagnostic experimental boot');result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();result['classification']='PASS READ-ONLY DIAGNOSTIC EXPERIMENTAL EVIDENCE; selection/display/handoff deadline require operator evidence'

 source_raw=read_source_channel();source_receipt=evaluate(source_raw)
 result['source_guard_preparation']={'bytes':len(source_raw),'sha256':hashlib.sha256(source_raw).hexdigest(),'payload_base64':base64.b64encode(source_raw).decode(),'decoded':source_receipt,'scope':'ACTUAL PASSIVE PRE-ADMISSION RECORD; NO INVOCATION'}
 check(source_receipt['supplied_record_consistency']=='CONFIRMED','valid startup source lifecycle and prepared continuous-isolation interval')
 check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot after source preparation')
 print(json.dumps(result,indent=2))
except Exception as exc:
 result['classification']='HOLD: DIAGNOSTIC CAPTURE GUARD FAILED';result['failure']=str(exc);print(json.dumps(result,indent=2));sys.exit(1)
