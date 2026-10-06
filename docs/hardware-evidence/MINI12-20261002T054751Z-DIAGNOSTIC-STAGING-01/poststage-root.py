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
 'custom_cfg':('/boot/grub/custom.cfg',3042,'a2d2b6bd33868deefe64e5e1cb2cd9ecedbfd1fe0e78370af983a92a2ec655c5'),
 'diagnostic_image':('/boot/initrd.img-'+RELEASE+'-sgx535-firstload-diagnostic-01',50805273,'376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae'),
 'diagnostic_entry_pending':('/boot/grub/.custom.cfg.diagnostic-01.pending',None,None),
 'prior_custom_backup':('/boot/grub/custom.cfg.pre-diagnostic-01',2040,'269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8'),
}
HEALTH_SOURCE_SHA256='5abe09ed15609cc2ad409d909d4785953275656690632700c869fc5a48d3b7ee'
result={'scope':'independent read-only poststage STOCK readback; no target writes, no DRM open, no module/service/PCI/VT operations','start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'guards':[],'commands':[],'files':{}}
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
 result['boot_id']=text('/proc/sys/kernel/random/boot_id');check(result['boot_id']=='d78d349e-daac-43aa-b7f9-156485506ca5','same STOCK boot as prestage')
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
 custom=result['files'].get('custom_cfg',{});custom_bytes=read('/boot/grub/custom.cfg') if custom.get('sha256') else b''
 result['custom_cfg_structure']={'entries':custom_bytes.count(b'menuentry '),'cycle06_entry':b'EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)' in custom_bytes,'corrected_entry':b'EXPERIMENTAL SGX535 rev121 CORRECTED FIRST-LOAD ONLY (no triangle)' in custom_bytes,'diagnostic_entry_present':b'EXPERIMENTAL SGX535 rev121 DIAGNOSTIC FIRST-LOAD ONLY (no SGX)' in custom_bytes,'saving_commands_absent':b'savedefault' not in custom_bytes and b'save_env' not in custom_bytes}
 check(result['custom_cfg_structure']=={'entries':3,'cycle06_entry':True,'corrected_entry':True,'diagnostic_entry_present':True,'saving_commands_absent':True},'three-entry custom.cfg exact')
 prior=read('/boot/grub/custom.cfg.pre-diagnostic-01');check(custom_bytes.startswith(prior) and custom_bytes.count(b'sgx535-rev121-firstload-diagnostic-01')==1 and custom_bytes.count(b'/boot/initrd.img-'+RELEASE.encode()+b'-sgx535-firstload-diagnostic-01')==1,'prior config prefix and unique diagnostic entry')
 result['custom_cfg_pending_absent']=not os.path.lexists('/boot/grub/.custom.cfg.diagnostic-01.pending');check(result['custom_cfg_pending_absent'],'published GRUB temp path absent')
 result['boot_id_end']=text('/proc/sys/kernel/random/boot_id');check(result['boot_id_end']==result['boot_id'],'same boot throughout capture')
except Exception as e:
 result['capture_error']=repr(e)
 result['guards'].append({'name':'capture completed','pass':False,'detail':repr(e)})
result['guard_count']=len(result['guards']);result['guard_pass_count']=sum(1 for g in result['guards'] if g['pass'])
result['classification']='PASS' if all(g['pass'] for g in result['guards']) else 'BLOCKED'
result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
print(json.dumps(result,indent=2,sort_keys=True))

