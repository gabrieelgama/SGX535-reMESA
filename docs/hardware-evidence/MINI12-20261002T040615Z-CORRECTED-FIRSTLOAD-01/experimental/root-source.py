import os,sys,stat,json,hashlib,re,subprocess,datetime
exec('"""Operational kernel-log guard; known diagnostics remain visible, never generic fault exemptions.\n\nThe bounded diagnostics derive from preserved captures and the exact kernel\nloader source, not an arbitrary caller-provided whitelist. Fault-bearing/repeated/changed variants\nfail closed. This is an operational predicate, not architectural health proof.\n"""\nimport re\nfrom collections import Counter\n\n# Strip only dmesg transport prefixes; retain the message and all diagnostic fields.\nPREFIX = re.compile(r"^(?:<\\d+>)?\\s*(?:\\[\\s*\\d+(?:\\.\\d+)?\\]\\s*)?(?:kernel:\\s*)?", re.I)\nFATAL = re.compile(r"^(?:BUG:)|\\bkernel BUG at\\b|\\bOops:|\\bKernel panic\\b|\\bgeneral protection(?: fault|:)"\n                   r"|\\bCall Trace:|\\b(?:soft|hard)\\s+LOCKUP\\b|\\bblocked for more than\\b"\n                   r"|\\brcu[^\\n]*\\b(?:stall|stalls)\\b", re.I)\nWARNING = re.compile(r"\\bWARNING:|\\bWARN_ON\\b|\\bBUG:", re.I)\nGRAPHICS = re.compile(r"\\b(?:gma500|drm|psb|sgx|pvr)\\b|0000:00:02\\.0", re.I)\nGRAPHICS_ERROR = re.compile(r"\\b(?:BUG:|fault|error|failed|failure|fatal|timeout|timed out|hang|hung|WARN|warning)\\b", re.I)\nACPI_DIAGNOSTICS = {\n \'ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)\': (\'acpi_powerbutton_warning\', 2),\n \'ACPI Error: Could not enable PowerButton event (20200925/evxfevnt-182)\': (\'acpi_powerbutton_error\', 2),\n \'button: probe of LNXPWRBN:00 failed with error -22\': (\'powerbutton_probe\', 1),\n \'tiny-power-button: probe of LNXPWRBN:00 failed with error -22\': (\'tiny_powerbutton_probe\', 1),\n}\n# kernel/module.c emits this exact notice once when permissive signature\n# checking admits an unverified module. Cycle05 loads captured stock drm first.\n# Module provenance/taint/ownership remain separate mandatory checks.\nUNSIGNED_DRM_NOTICE = (\'drm: module verification failed: signature and/or required key missing - tainting kernel\')\nBACKLIGHT = re.compile(r"gma500 0000:00:02\\.0: BL bug: Reg ([0-9a-fA-F]{8}) save ([0-9a-fA-F]{8})")\n\ndef classify_kernel_log(log):\n """Return visible classification receipt; reject on any selected adverse signal."""\n if not isinstance(log, str) or not log.strip():\n  return {\'classification\': \'REJECT\', \'faults\': [{\'reason\': \'missing kernel log\'}], \'stock_diagnostics\': {}}\n counts = Counter(); faults = []\n for number, raw in enumerate(log.splitlines(), 1):\n  message = PREFIX.sub(\'\', raw, count=1)\n  reason = None\n  # Fatal reports are rejected even alongside a previously seen diagnostic.\n  if FATAL.search(message): reason = \'kernel fault/lockup\'\n  else:\n   known = ACPI_DIAGNOSTICS.get(message)\n   if message == UNSIGNED_DRM_NOTICE: known = (\'unsigned_drm_loader_notice\', 1)\n   backlight = BACKLIGHT.fullmatch(message)\n   if backlight:\n    if any(int(value, 16) != 0 for value in backlight.groups()): reason = \'changed backlight diagnostic fields\'\n    else: known = (\'backlight_zero_register\', 1)\n   if known:\n    label, limit = known; counts[label] += 1\n    if counts[label] > limit: reason = \'repeated stock diagnostic: \' + label\n   elif WARNING.search(message): reason = \'kernel warning\'\n   elif re.search(r\'\\bACPI (?:Error|Warning):\', message, re.I): reason = \'unexpected ACPI diagnostic\'\n   elif GRAPHICS.search(message) and GRAPHICS_ERROR.search(message): reason = \'graphics fault/error\'\n  if reason: faults.append({\'line\': number, \'message\': message, \'reason\': reason})\n return {\'classification\': \'REJECT\' if faults else \'PASS WITH BOUNDED STOCK DIAGNOSTICS\',\n         \'faults\': faults, \'stock_diagnostics\': dict(counts)}\n')
HEALTH_SOURCE_SHA256='5abe09ed15609cc2ad409d909d4785953275656690632700c869fc5a48d3b7ee'
RELEASE='5.10.240-antix.1-486-smp'
BDF='/sys/bus/pci/devices/0000:00:02.0'
STOCK={'kernel':('/boot/vmlinuz-'+RELEASE,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),'initrd':('/boot/initrd.img-'+RELEASE,50863580,'f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'),'grub_cfg':('/boot/grub/grub.cfg',10438,'396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'),'grub_env':('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),'original_module':('/lib/modules/'+RELEASE+'/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko',None,'7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb')}
STOCK_ID='gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'
result={'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'scope':'root read-only pre-stage stock/boot/ownership/filesystem/tool checks; no writes, no DRM open, no module/service operations','commands':[],'guards':[]}
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
 result['boot_id']=txt('/proc/sys/kernel/random/boot_id');result['taint']=int(txt('/proc/sys/kernel/tainted'));check(not result['taint']&~12289,'known taint mask')
 result['pci']={k:txt(BDF+'/'+k) for k in ['vendor','device','subsystem_vendor','subsystem_device','irq']}
 check(result['pci']=={'vendor':'0x8086','device':'0x8108','subsystem_vendor':'0x1028','subsystem_device':'0x02b1','irq':'16'},'PCI identity/IRQ')
 result['pci_driver']=os.path.realpath(BDF+'/driver');result['driver_module']=os.path.realpath(BDF+'/driver/module');check(result['pci_driver']=='/sys/bus/pci/drivers/gma500' and result['driver_module']=='/sys/module/gma500_gfx','original PCI ownership')
 result['module_state']=txt('/sys/module/gma500_gfx/initstate');check(result['module_state']=='live','original module Live')
 result['modules']=txt('/proc/modules');check(re.search(r'^gma500_gfx .* Live ',result['modules'],re.M) is not None,'module list Live')
 result['loaded_note_sha256']=hashlib.sha256(read('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest()
 expected_note='b78dc1ffde5cc661d076941e27e10b06edf62ad1da37651f3c38a98e622890c2';check(result['loaded_note_sha256']==expected_note,'derivative loaded Build-ID note')
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
 for path,size,digest in [('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-01',50804481,'4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71'),('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-corrected-01',50804478,'55e7a8be6f62c9a1d59f1c532b8790399c21a2706884747e504edfffb45bf52d'),('/boot/grub/custom.cfg',2040,'269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8'),('/boot/grub/custom.cfg.cycle06-preserved',974,'181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612')]:
  row=file_record(path);result['destinations'][path]=row;check(row['size']==size and row['sha256']==digest and row['uid']==0 and row['gid']==0 and row['mode']==0o644 and row['nlink']==1,'staged destination readback '+path)
 incoming='/home/gama/sgx535-firstload-corrected-incoming-01';st=os.lstat(incoming)
 check(stat.S_ISDIR(st.st_mode) and st.st_uid==int(run(['id','-u','gama']).strip()) and stat.S_IMODE(st.st_mode)==0o700,'incoming directory identity')
 check(set(os.listdir(incoming))=={'initrd.img-sgx535-firstload-01','corrected-entry.proposed'},'corrected incoming inventory')
 trusted('/boot');trusted('/boot/grub')
 result['mountinfo']=txt('/proc/self/mountinfo');result['mounts']=txt('/proc/mounts');result['boot_free_bytes']=os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize;result['incoming_free_bytes']=os.statvfs('/home/gama').f_bavail*os.statvfs('/home/gama').f_frsize
 check(result['boot_free_bytes']>=134217728 and result['incoming_free_bytes']>=50804481+974,'staging free space')
 check(os.access('/boot',os.W_OK) and not (os.statvfs('/boot').f_flag&getattr(os,'ST_RDONLY',1)),'boot filesystem writable')
 result['gama_uid']=int(run(['id','-u','gama']).strip());result['gama_gid']=int(run(['id','-g','gama']).strip())
 result['python']=sys.version;result['python_executable']=sys.executable
 check(all(hasattr(os,k) for k in ['O_NOFOLLOW','O_EXCL','O_DIRECTORY','fsync','fchmod','fchown']),'exclusive no-follow/fsync facilities')
 check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')
 cfg_new=read('/boot/grub/custom.cfg'); cfg_old=read('/boot/grub/custom.cfg.cycle06-preserved')
 check(hashlib.sha256(cfg_old).hexdigest()=='181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612' and cfg_new.startswith(cfg_old),'Cycle06 custom.cfg preserved')
 check(cfg_new.count(b'menuentry ')==2 and cfg_new.count(b'EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)')==1 and cfg_new.count(b'EXPERIMENTAL SGX535 rev121 CORRECTED FIRST-LOAD ONLY (no triangle)')==1,'both manual entries retained')
 check(cfg_new.count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-corrected-01')==1 and b'savedefault' not in cfg_new and b'save_env' not in cfg_new,'corrected entry points to isolated image without save')
 result['hook_log']=txt('/run/initramfs/sgx535-first-load.log');check(result['hook_log'].splitlines()==['SGX535-FIRSTLOAD BEGIN','SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE','SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue'],'complete ordered hook log');check(result['boot_id']!='10d6abfa-e7a8-4311-97ae-213f503cee20','distinct experimental boot');result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();result['classification']='PASS READ-ONLY EXPERIMENTAL EVIDENCE; selection/display/handoff deadline require operator evidence'
 print(json.dumps(result,indent=2))
except Exception as exc:
 result['classification']='STOP BEFORE STAGING';result['failure']=str(exc);print(json.dumps(result,indent=2));sys.exit(1)
