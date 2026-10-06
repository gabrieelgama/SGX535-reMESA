import os,stat,json,hashlib,re,subprocess,datetime
RELEASE='5.10.240-antix.1-486-smp'
OLD_ATTEMPT05_BOOT='203a5b9b-5a90-4fd3-8001-3c92cdefeddd'
BDF='/sys/bus/pci/devices/0000:00:02.0'
STOCK_ID='gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'
EXPECTED={
 'kernel':('/boot/vmlinuz-'+RELEASE,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),
 'stock_initrd':('/boot/initrd.img-'+RELEASE,50863580,'f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'),
 'grub_cfg':('/boot/grub/grub.cfg',10438,'396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'),
 'grubenv':('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),
 'original_module':('/lib/modules/'+RELEASE+'/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko',None,'7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb'),
 'cycle06_image':('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-01',50804481,'4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71'),
 'corrected_image':('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-corrected-01',50804478,'55e7a8be6f62c9a1d59f1c532b8790399c21a2706884747e504edfffb45bf52d'),
 'cycle06_custom_backup':('/boot/grub/custom.cfg.cycle06-preserved',974,'181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612'),
 'custom_cfg':('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 8615, '5b3261d3af8033b9fdefaf8e0a402a3b35a53709ec68365a0ec4b0a6e94e17a0'),
 'diagnostic_image':('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-diagnostic-01',50805273,'376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae'),
 'diagnostic_entry_pending':('/boot/grub/.custom.cfg.diagnostic-01.pending',None,None),
 'prior_custom_backup':('/boot/grub/custom.cfg.pre-diagnostic-01',2040,'269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8'),
}
HEALTH_SOURCE_SHA256='5abe09ed15609cc2ad409d909d4785953275656690632700c869fc5a48d3b7ee'
result={'scope':'fresh experimental constant-fragment STOCK preflight; read only; no DRM open or SGX','start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'guards':[],'commands':[],'files':{}}
def check(v,n,detail=None):
 result['guards'].append({'name':n,'pass':bool(v),**({'detail':detail} if detail is not None else {})})
def read(path):
 fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW);data=b''
 with os.fdopen(fd,'rb') as f:data=f.read()
 return data
def text(path):return read(path).decode(errors='replace').strip()
def run(argv,timeout=10):
 try:
  cp=subprocess.run(argv,stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=timeout)
  row={'argv':argv,'exit_code':cp.returncode,'stdout':cp.stdout.decode(errors='replace'),'stderr':cp.stderr.decode(errors='replace')};result['commands'].append(row)
  return row
 except Exception as e:
  row={'argv':argv,'error':repr(e)};result['commands'].append(row);return row
def file_id(path):
 try:
  st=os.lstat(path)
  row={'path':path,'uid':st.st_uid,'gid':st.st_gid,'mode':oct(stat.S_IMODE(st.st_mode)),'nlink':st.st_nlink,'dev':st.st_dev,'ino':st.st_ino,'regular':stat.S_ISREG(st.st_mode),'symlink':stat.S_ISLNK(st.st_mode)}
  if row['regular'] and not row['symlink']:
   data=read(path);row.update(size=len(data),sha256=hashlib.sha256(data).hexdigest())
  return row
 except FileNotFoundError:return {'path':path,'exists':False}
 except Exception as e:return {'path':path,'error':repr(e)}
exec('"""Operational kernel-log guard; known diagnostics remain visible, never generic fault exemptions.\n\nThe bounded diagnostics derive from preserved captures and the exact kernel\nloader source, not an arbitrary caller-provided whitelist. Fault-bearing/repeated/changed variants\nfail closed. This is an operational predicate, not architectural health proof.\n"""\nimport re\nfrom collections import Counter\n\n# Strip only dmesg transport prefixes; retain the message and all diagnostic fields.\nPREFIX = re.compile(r"^(?:<\\d+>)?\\s*(?:\\[\\s*\\d+(?:\\.\\d+)?\\]\\s*)?(?:kernel:\\s*)?", re.I)\nFATAL = re.compile(r"^(?:BUG:)|\\bkernel BUG at\\b|\\bOops:|\\bKernel panic\\b|\\bgeneral protection(?: fault|:)"\n                   r"|\\bCall Trace:|\\b(?:soft|hard)\\s+LOCKUP\\b|\\bblocked for more than\\b"\n                   r"|\\brcu[^\\n]*\\b(?:stall|stalls)\\b", re.I)\nWARNING = re.compile(r"\\bWARNING:|\\bWARN_ON\\b|\\bBUG:", re.I)\nGRAPHICS = re.compile(r"\\b(?:gma500|drm|psb|sgx|pvr)\\b|0000:00:02\\.0", re.I)\nGRAPHICS_ERROR = re.compile(r"\\b(?:BUG:|fault|error|failed|failure|fatal|timeout|timed out|hang|hung|WARN|warning)\\b", re.I)\nACPI_DIAGNOSTICS = {\n \'ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)\': (\'acpi_powerbutton_warning\', 2),\n \'ACPI Error: Could not enable PowerButton event (20200925/evxfevnt-182)\': (\'acpi_powerbutton_error\', 2),\n \'button: probe of LNXPWRBN:00 failed with error -22\': (\'powerbutton_probe\', 1),\n \'tiny-power-button: probe of LNXPWRBN:00 failed with error -22\': (\'tiny_powerbutton_probe\', 1),\n}\n# kernel/module.c emits this exact notice once when permissive signature\n# checking admits an unverified module. Cycle05 loads captured stock drm first.\n# Module provenance/taint/ownership remain separate mandatory checks.\nUNSIGNED_DRM_NOTICE = (\'drm: module verification failed: signature and/or required key missing - tainting kernel\')\nBACKLIGHT = re.compile(r"gma500 0000:00:02\\.0: BL bug: Reg ([0-9a-fA-F]{8}) save ([0-9a-fA-F]{8})")\n\ndef classify_kernel_log(log):\n """Return visible classification receipt; reject on any selected adverse signal."""\n if not isinstance(log, str) or not log.strip():\n  return {\'classification\': \'REJECT\', \'faults\': [{\'reason\': \'missing kernel log\'}], \'stock_diagnostics\': {}}\n counts = Counter(); faults = []\n for number, raw in enumerate(log.splitlines(), 1):\n  message = PREFIX.sub(\'\', raw, count=1)\n  reason = None\n  # Fatal reports are rejected even alongside a previously seen diagnostic.\n  if FATAL.search(message): reason = \'kernel fault/lockup\'\n  else:\n   known = ACPI_DIAGNOSTICS.get(message)\n   if message == UNSIGNED_DRM_NOTICE: known = (\'unsigned_drm_loader_notice\', 1)\n   backlight = BACKLIGHT.fullmatch(message)\n   if backlight:\n    if any(int(value, 16) != 0 for value in backlight.groups()): reason = \'changed backlight diagnostic fields\'\n    else: known = (\'backlight_zero_register\', 1)\n   if known:\n    label, limit = known; counts[label] += 1\n    if counts[label] > limit: reason = \'repeated stock diagnostic: \' + label\n   elif WARNING.search(message): reason = \'kernel warning\'\n   elif re.search(r\'\\bACPI (?:Error|Warning):\', message, re.I): reason = \'unexpected ACPI diagnostic\'\n   elif GRAPHICS.search(message) and GRAPHICS_ERROR.search(message): reason = \'graphics fault/error\'\n  if reason: faults.append({\'line\': number, \'message\': message, \'reason\': reason})\n return {\'classification\': \'REJECT\' if faults else \'PASS WITH BOUNDED STOCK DIAGNOSTICS\',\n         \'faults\': faults, \'stock_diagnostics\': dict(counts)}\n')
try:
 result['euid']=os.geteuid();check(result['euid']==0,'root read-only capture')
 u=os.uname();result['kernel_release']=u.release;result['architecture']=u.machine
 check(u.release==RELEASE,'running kernel release');check(u.machine=='i686','running architecture')
 result['machine']=text('/sys/class/dmi/id/product_name');check(result['machine']=='Inspiron 1210','Mini 12 identity')
 result['boot_id']=text('/proc/sys/kernel/random/boot_id');check(bool(re.fullmatch(r'[0-9a-f]{8}(?:-[0-9a-f]{4}){3}-[0-9a-f]{12}', result['boot_id'])),'same STOCK boot as prestage')
 result['cmdline']=text('/proc/cmdline');expected_cmd=('BOOT_IMAGE=/boot/vmlinuz-'+RELEASE+' root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0')
 check(result['cmdline'].split()==expected_cmd.split(),'normal STOCK command line')
 result['proc_version']=text('/proc/version')
 result['taint']=int(text('/proc/sys/kernel/tainted'));check(result['taint']==12289,'STOCK taint baseline')
 bdf={k:text(BDF+'/'+k) for k in ('vendor','device','subsystem_vendor','subsystem_device','irq')};result['pci']=bdf
 check(bdf=={'vendor':'0x8086','device':'0x8108','subsystem_vendor':'0x1028','subsystem_device':'0x02b1','irq':'16'},'Poulsbo PCI identity/IRQ')
 result['pci_driver']=os.path.realpath(BDF+'/driver');result['driver_module']=os.path.realpath(BDF+'/driver/module')
 check(result['pci_driver']=='/sys/bus/pci/drivers/gma500' and result['driver_module']=='/sys/module/gma500_gfx','PCI owned by gma500_gfx')
 result['module_initstate']=text('/sys/module/gma500_gfx/initstate');check(result['module_initstate']=='live','stock driver Live')
 result['proc_modules']=text('/proc/modules');check(re.search(r'^gma500_gfx .* Live ',result['proc_modules'],re.M) is not None,'stock module listed Live')
 note=read('/sys/module/gma500_gfx/notes/.note.gnu.build-id');result['loaded_module_note_sha256']=hashlib.sha256(note).hexdigest()
 expected_note=hashlib.sha256(bytes.fromhex('040000001400000003000000474e5500d8dcb4d38b774ad64799d5e13aaedede069371f3')).hexdigest()
 check(result['loaded_module_note_sha256']==expected_note,'loaded original module Build ID')
 result['loaded_module_build_id']='d8dcb4d38b774ad64799d5e13aaedede069371f3' if result['loaded_module_note_sha256']==expected_note else 'UNKNOWN'
 result['drm_device']=os.path.realpath('/sys/class/drm/card0/device');check(result['drm_device']=='/sys/devices/pci0000:00/0000:00:02.0','DRM card0 device ownership')
 st=os.lstat('/dev/dri/card0');result['drm_node_metadata']={'char_device':stat.S_ISCHR(st.st_mode),'major':os.major(st.st_rdev),'minor':os.minor(st.st_rdev),'mode':oct(stat.S_IMODE(st.st_mode))};check(stat.S_ISCHR(st.st_mode),'DRM node exists (stat only; not opened)')
 result['proc_fb']=text('/proc/fb');result['framebuffer']=text('/sys/class/graphics/fb0/name');result['framebuffer_dimensions']=text('/sys/class/graphics/fb0/virtual_size')
 check(result['framebuffer']=='gma500drmfb' and result['framebuffer_dimensions']=='1280,800','stock framebuffer')
 result['vtcon0']=text('/sys/class/vtconsole/vtcon0/bind');result['vtcon1']=text('/sys/class/vtconsole/vtcon1/bind');check(result['vtcon0']=='0' and result['vtcon1']=='1','stock VT bindings')
 result['interrupts']=text('/proc/interrupts');check(re.search(r'^\s*16:.*[\s,]gma500(?:[,\s]|$)',result['interrupts'],re.M) is not None,'IRQ 16 gma500 handler')
 service=run(['sv','status','/etc/runit/runsvdir/default/slimski']);result['slimski']=service
 check(service.get('exit_code')==0 and service.get('stdout','').startswith('run:') and not service.get('stderr'),'slimski running')
 xorg=run(['pgrep','-a','Xorg']);result['xorg']=xorg;check(xorg.get('exit_code')==0 and bool(xorg.get('stdout','').strip()),'Xorg running')
 d=run(['dmesg']);result['dmesg']=d;check(d.get('exit_code')==0 and not d.get('stderr'),'complete dmesg read')
 if d.get('exit_code')==0:
  result['kernel_health']=classify_kernel_log(d['stdout']);result['kernel_health_source_sha256']=HEALTH_SOURCE_SHA256
  check(result['kernel_health']['classification']!='REJECT','kernel health classifier')
 for name,(path,size,digest) in EXPECTED.items():
  row=file_id(path);result['files'][name]=row
  if name=='diagnostic_entry_pending':check(row.get('exists') is False,'diagnostic pending entry absent')
  else:check(row.get('regular') is True and row.get('symlink') is False and row.get('sha256')==digest and (size is None or row.get('size')==size),'preserved file identity '+name)
 result['grubenv']=run(['grub-editenv','/boot/grub/grubenv','list']);saved=result['grubenv'].get('stdout','').strip()
 check(result['grubenv'].get('exit_code')==0 and saved=='saved_entry='+STOCK_ID and not result['grubenv'].get('stderr'),'STOCK remains saved GRUB entry')
 custom=result['files'].get('custom_cfg',{});custom_bytes=read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba') if custom.get('sha256') else b''
 result['custom_cfg_structure']={'entries':custom_bytes.count(b'menuentry '),'cycle06_entry':b'EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)' in custom_bytes,'corrected_entry':b'EXPERIMENTAL SGX535 rev121 CORRECTED FIRST-LOAD ONLY (no triangle)' in custom_bytes,'diagnostic_entry_present':b'EXPERIMENTAL SGX535 rev121 DIAGNOSTIC FIRST-LOAD ONLY (no SGX)' in custom_bytes,'saving_commands_absent':b'savedefault' not in custom_bytes and b'save_env' not in custom_bytes}
 check(result['custom_cfg_structure']=={'entries':8,'cycle06_entry':True,'corrected_entry':True,'diagnostic_entry_present':True,'saving_commands_absent':True},'seven-entry source-lifecycle custom.cfg exact')
 prior=read('/boot/grub/custom.cfg.pre-diagnostic-01');check(custom_bytes.startswith(prior) and custom_bytes.count(b'sgx535-rev121-firstload-diagnostic-01')==1 and custom_bytes.count(b'/boot/initrd.img-'+RELEASE.encode()+b'-sgx535-firstload-diagnostic-01')==1,'prior config prefix and unique diagnostic entry')
 result['custom_cfg_pending_absent']=not os.path.lexists('/boot/grub/.custom.cfg.diagnostic-01.pending');check(result['custom_cfg_pending_absent'],'published GRUB temp path absent')

 for path in ['/boot/grub/.custom.cfg.cd9b947371f18c2d19af05bfda69fbf9462c2e62.pending', '/boot/grub/.custom.cfg.cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1.pending']:check(not os.path.lexists(path),'pending publication path absent '+path)
 for path in ['/boot','/boot/grub','/home/gama']:
  st=os.lstat(path);check(stat.S_ISDIR(st.st_mode) and st.st_uid==(1000 if path=='/home/gama' else 0) and not stat.S_IMODE(st.st_mode)&0o022,'trusted parent '+path)
 check(not os.path.lexists('/run/initramfs/sgx535-first-load.log'),'no derivative hook on STOCK boot')
 check(os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize>=134217728+50811746,'boot free margin')
 check(os.statvfs('/home/gama').f_bavail*os.statvfs('/home/gama').f_frsize>=50811746+16777216,'incoming free margin')
 check(not os.statvfs('/boot').f_flag&getattr(os,'ST_RDONLY',1),'boot writable filesystem')

 check(result['boot_id']!='a76f3b25-31ae-4507-891b-474e912dd356','fresh STOCK after consumed FIRE2 boot')
 check(not os.path.lexists('/sys/module/sgx535_provenance') and not os.path.lexists('/proc/sgx535_current_operation'),'observer not loaded on STOCK')
 old=file_id('/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-314e2f3b37195dc56df7c57dd938d78377a5ea8e');result['files']['preserved314']=old
 check(old.get('size')==50805231 and old.get('sha256')=='fe64b3dcfe74b78a6d96edcd4fd7c118c3901fcff8631afec6c14647da292e3c','preserved314 image identity')

 for expected in [{'path': '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-cd9b947371f18c2d19af05bfda69fbf9462c2e62', 'dev': 2049, 'ino': 2616676, 'phase': 'verified', 'expected_size': 50811745, 'sha256': '5001a64ff5ef78751aea45f8762eeea8abbfc8ef3c3774c12a34fa214858288d', 'size': 50811745, 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1, 'fsync': 'PASS', 'readback': 'PASS'}, {'path': '/boot/grub/custom.cfg.pre-cd9b947371f18c2d19af05bfda69fbf9462c2e62', 'dev': 2049, 'ino': 2616679, 'phase': 'verified', 'expected_size': 4132, 'sha256': '77b0d25966a7a2634f13a8dbfeeac771d2ad461fb789d4f840d6b5662dfd54af', 'size': 4132, 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1, 'fsync': 'PASS', 'readback': 'PASS'}]:
  path=expected['path'];actual=file_id(path);result['files']['new:'+path]=actual
  check(actual.get('regular') and actual.get('uid')==0 and actual.get('gid')==0 and actual.get('mode')=='0o644' and actual.get('nlink')==1 and actual.get('size')==expected['size'] and actual.get('sha256')==expected['sha256'] and actual.get('dev')==expected['dev'] and actual.get('ino')==expected['ino'],'staged byte/inode identity '+path)
 check(read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').startswith(read('/boot/grub/custom.cfg.pre-cd9b947371f18c2d19af05bfda69fbf9462c2e62')) and read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-cd9b947371f18c2d19af05bfda69fbf9462c2e62\n')==1,'preserved exact old prefix and unique current image')
 for expected in [{'path': '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1', 'dev': 2049, 'ino': 2616678, 'phase': 'verified', 'expected_size': 50811746, 'sha256': 'b19838c188e9257834ed470f1c401f6ce3938da46f8dae3b7822e46839db7142', 'size': 50811746, 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1, 'fsync': 'PASS', 'readback': 'PASS'}, {'path': '/boot/grub/custom.cfg.pre-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1', 'dev': 2049, 'ino': 2616681, 'phase': 'verified', 'expected_size': 5222, 'sha256': '8f8697290be772f67dd0e61c6899aa98b67f5381aecb080d0212ff6fb55cf9df', 'size': 5222, 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1, 'fsync': 'PASS', 'readback': 'PASS'}, {'path': '/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 'dev': 2049, 'ino': 2616684, 'phase': 'verified', 'expected_size': 7483, 'sha256': 'd98b0faf6aeec27272b115702b56c1109de89259426a497a49b8774ddeb18a30', 'size': 7483, 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1, 'fsync': 'PASS', 'readback': 'PASS'}]:
  path=expected["path"];actual=file_id('/boot/grub/custom.cfg.pre-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb' if path=='/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba' else path);result["files"]["corrected:"+path]=actual
  check(actual.get("regular") and actual.get("uid")==0 and actual.get("gid")==0 and actual.get("mode")=="0o644" and actual.get("nlink")==1 and actual.get("size")==expected["size"] and actual.get("sha256")==expected["sha256"] and (path=='/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba' or (actual.get("dev")==expected["dev"] and actual.get("ino")==expected["ino"])),"corrected staged byte/inode identity "+path)
 check(read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').startswith(read('/boot/grub/custom.cfg.pre-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1')) and read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1')==1 and read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').count(b'sgx535-rev121-frozen-cd9b947371f18c2d19af05bfda69fbf9462c2e62-b19838c1')==1,"preserved exact five-entry prefix and unique corrected image")

 check(not os.path.lexists('/boot/grub/.custom.cfg.f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61.pending'),'new published GRUB pending absent')
 check(not os.path.lexists('/proc/sgx535_source_guard'),'source observer absent on STOCK')
 check(os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize>=134217728+50816585,'new candidate boot free margin')
 check(result['boot_id']!='ad1c8ae6-0c52-4ed0-b117-bcd7473bffb1','distinct STOCK boot from prior successful candidate')

 for expected in [{'path': '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61', 'dev': 2049, 'ino': 2616680, 'size': 50816585, 'sha256': 'c621ea61622bb5c15e83e0d1ad657f2ba96ce23dc277ed6c7ac007d615f76f5d', 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1}, {'path': '/boot/grub/custom.cfg.pre-f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61', 'dev': 2049, 'ino': 2616683, 'size': 6349, 'sha256': '15a89767d4116e63250f5e16e04c5e0de33d9de6a83e69a8b243ef21ebb24bf5', 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1}, {'path': '/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 'dev': 2049, 'ino': 2616687, 'size': 8615, 'sha256': '5b3261d3af8033b9fdefaf8e0a402a3b35a53709ec68365a0ec4b0a6e94e17a0', 'uid': 0, 'gid': 0, 'mode': '0o644', 'nlink': 1}]:
  actual=file_id(expected['path']);result['files']['source-lifecycle:'+expected['path']]=actual
  check(actual.get('regular') and actual.get('symlink') is False and all(actual.get(k)==expected[k] for k in ('dev','ino','size','sha256','uid','gid','mode','nlink')),'independent staged identity '+expected['path'])
 check(read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').startswith(read('/boot/grub/custom.cfg.pre-f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61')) and read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61')==1 and read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').count(b'sgx535-rev121-frozen-f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61')==1,'exact preserved six-entry prefix and unique new candidate')
 check(set(os.listdir('/home/gama/sgx535-firstload-incoming-f243e1b417e8a44fae3b5be1798b40b6e4e4b090-c621ea61'))=={'initrd.img-sgx535-firstload-diagnostic-01','diagnostic-entry.proposed'},'retained incoming inventory')

 actual=file_id('/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb');result['files']['new:/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb']=actual
 check(actual.get('regular') and actual.get('uid')==0 and actual.get('gid')==0 and actual.get('mode')=='0o644' and actual.get('nlink')==1 and actual.get('size')==50816610 and actual.get('sha256')=='ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d','independent published byte/metadata verification /boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb')
 actual=file_id('/boot/grub/custom.cfg.pre-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb');result['files']['new:/boot/grub/custom.cfg.pre-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb']=actual
 check(actual.get('regular') and actual.get('uid')==0 and actual.get('gid')==0 and actual.get('mode')=='0o644' and actual.get('nlink')==1 and actual.get('size')==7483 and actual.get('sha256')=='d98b0faf6aeec27272b115702b56c1109de89259426a497a49b8774ddeb18a30','independent published byte/metadata verification /boot/grub/custom.cfg.pre-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb')
 actual=file_id('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba');result['files']['new:/boot/grub/custom.cfg']=actual
 check(actual.get('regular') and actual.get('uid')==0 and actual.get('gid')==0 and actual.get('mode')=='0o644' and actual.get('nlink')==1 and actual.get('size')==8615 and actual.get('sha256')=='5b3261d3af8033b9fdefaf8e0a402a3b35a53709ec68365a0ec4b0a6e94e17a0','independent published byte/metadata verification /boot/grub/custom.cfg')
 check(not os.path.lexists('/boot/grub/.custom.cfg.9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb.pending'),'new publication temp absent')
 check(read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').startswith(read('/boot/grub/custom.cfg.pre-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb')) and read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba').count(b'/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-9d0b5b2fdb7d9881f2828f43fb89253176c38817-ef7e01cb')==1,'prior seven entries preserved and unique new image')

 for path in ['/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c', '/home/gama/sgx535-square-incoming-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c-20261006T083635Z', '/boot/grub/custom.cfg.pre-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c', '/boot/grub/.custom.cfg.8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c.pending']:check(not os.path.lexists(path),'fresh experiment destination absent '+path)
 check(os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize>=134217728+50816631,'exact experiment boot free margin')

 # Current published configuration is independently bound to the successful FIRE3 receipt.
 current=file_id('/boot/grub/custom.cfg');result['files']['current_custom_cfg']=current
 expected_current={'dev':2049,'ino':2616688,'uid':0,'gid':0,'mode':'0o644','nlink':1,'size':9749,'sha256':'ac60694d5e31263d080dd6900ae3ff55feeef3f9e6b9e87c8c3330990d200f59'}
 check(current.get('regular') and current.get('symlink') is False and all(current.get(k)==v for k,v in expected_current.items()),'current FIRE3-published config exact binding')
 check(read('/boot/grub/custom.cfg').startswith(read('/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba')) and read('/boot/grub/custom.cfg').count(b'menuentry ')==9,'prior eight entries preserved and unique FIRE3 entry')
 check(result['boot_id']!='f1ab6606-0561-445f-a397-28a028516cd7','fresh STOCK distinct from consumed FIRE3')
 check(os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize>=134217728+50816648,'square exact size free margin')
 result['boot_id_end']=text('/proc/sys/kernel/random/boot_id');check(result['boot_id_end']==result['boot_id'],'same boot throughout capture')
except Exception as e:
 result['capture_error']=repr(e)
 result['guards'].append({'name':'capture completed','pass':False,'detail':repr(e)})
result['guard_count']=len(result['guards']);result['guard_pass_count']=sum(1 for g in result['guards'] if g['pass'])
result['classification']='PASS' if all(g['pass'] for g in result['guards']) else 'BLOCKED'
result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
print(json.dumps(result,indent=2,sort_keys=True))

