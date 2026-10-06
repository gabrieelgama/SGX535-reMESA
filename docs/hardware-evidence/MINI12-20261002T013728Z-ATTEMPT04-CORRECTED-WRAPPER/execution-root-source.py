import os,sys,stat,json,hashlib,subprocess,datetime,time,base64
exec('"""Operational kernel-log guard; known diagnostics remain visible, never generic fault exemptions.\n\nThe bounded diagnostics derive from preserved captures and the exact kernel\nloader source, not an arbitrary caller-provided whitelist. Fault-bearing/repeated/changed variants\nfail closed. This is an operational predicate, not architectural health proof.\n"""\nimport re\nfrom collections import Counter\n\n# Strip only dmesg transport prefixes; retain the message and all diagnostic fields.\nPREFIX = re.compile(r"^(?:<\\d+>)?\\s*(?:\\[\\s*\\d+(?:\\.\\d+)?\\]\\s*)?(?:kernel:\\s*)?", re.I)\nFATAL = re.compile(r"^(?:BUG:)|\\bkernel BUG at\\b|\\bOops:|\\bKernel panic\\b|\\bgeneral protection(?: fault|:)"\n                   r"|\\bCall Trace:|\\b(?:soft|hard)\\s+LOCKUP\\b|\\bblocked for more than\\b"\n                   r"|\\brcu[^\\n]*\\b(?:stall|stalls)\\b", re.I)\nWARNING = re.compile(r"\\bWARNING:|\\bWARN_ON\\b|\\bBUG:", re.I)\nGRAPHICS = re.compile(r"\\b(?:gma500|drm|psb|sgx|pvr)\\b|0000:00:02\\.0", re.I)\nGRAPHICS_ERROR = re.compile(r"\\b(?:BUG:|fault|error|failed|failure|fatal|timeout|timed out|hang|hung|WARN|warning)\\b", re.I)\nACPI_DIAGNOSTICS = {\n \'ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)\': (\'acpi_powerbutton_warning\', 2),\n \'ACPI Error: Could not enable PowerButton event (20200925/evxfevnt-182)\': (\'acpi_powerbutton_error\', 2),\n \'button: probe of LNXPWRBN:00 failed with error -22\': (\'powerbutton_probe\', 1),\n \'tiny-power-button: probe of LNXPWRBN:00 failed with error -22\': (\'tiny_powerbutton_probe\', 1),\n}\n# kernel/module.c emits this exact notice once when permissive signature\n# checking admits an unverified module. Cycle05 loads captured stock drm first.\n# Module provenance/taint/ownership remain separate mandatory checks.\nUNSIGNED_DRM_NOTICE = (\'drm: module verification failed: signature and/or required key missing - tainting kernel\')\nBACKLIGHT = re.compile(r"gma500 0000:00:02\\.0: BL bug: Reg ([0-9a-fA-F]{8}) save ([0-9a-fA-F]{8})")\n\ndef classify_kernel_log(log):\n """Return visible classification receipt; reject on any selected adverse signal."""\n if not isinstance(log, str) or not log.strip():\n  return {\'classification\': \'REJECT\', \'faults\': [{\'reason\': \'missing kernel log\'}], \'stock_diagnostics\': {}}\n counts = Counter(); faults = []\n for number, raw in enumerate(log.splitlines(), 1):\n  message = PREFIX.sub(\'\', raw, count=1)\n  reason = None\n  # Fatal reports are rejected even alongside a previously seen diagnostic.\n  if FATAL.search(message): reason = \'kernel fault/lockup\'\n  else:\n   known = ACPI_DIAGNOSTICS.get(message)\n   if message == UNSIGNED_DRM_NOTICE: known = (\'unsigned_drm_loader_notice\', 1)\n   backlight = BACKLIGHT.fullmatch(message)\n   if backlight:\n    if any(int(value, 16) != 0 for value in backlight.groups()): reason = \'changed backlight diagnostic fields\'\n    else: known = (\'backlight_zero_register\', 1)\n   if known:\n    label, limit = known; counts[label] += 1\n    if counts[label] > limit: reason = \'repeated stock diagnostic: \' + label\n   elif WARNING.search(message): reason = \'kernel warning\'\n   elif re.search(r\'\\bACPI (?:Error|Warning):\', message, re.I): reason = \'unexpected ACPI diagnostic\'\n   elif GRAPHICS.search(message) and GRAPHICS_ERROR.search(message): reason = \'graphics fault/error\'\n  if reason: faults.append({\'line\': number, \'message\': message, \'reason\': reason})\n return {\'classification\': \'REJECT\' if faults else \'PASS WITH BOUNDED STOCK DIAGNOSTICS\',\n         \'faults\': faults, \'stock_diagnostics\': dict(counts)}\n')
BASE='/root/sgx535-frozen-seq1-29e27f75'
BOOT='29e27f75-7c84-4537-9ab8-8138bc3eac1d'
result={'scope':'ONE whitelisted frozen ioctl; no retry, hot operation or reset','start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'client_invocation_started':False}
def read(path):
 with open(path,'rb') as f:return f.read()
def text(path):return read(path).decode().strip()
def dmesg():
 cp=subprocess.run(['dmesg'],stdin=subprocess.DEVNULL,capture_output=True,timeout=5)
 if cp.returncode or cp.stderr:raise RuntimeError('passive dmesg capture failed')
 return cp.stdout.decode()
def record_file(name,data):
 fd=os.open(name,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600,dir_fd=dfd)
 with os.fdopen(fd,'wb') as f:f.write(data);f.flush();os.fsync(f.fileno())
 os.fsync(dfd)
try:
 assert os.geteuid()==0 and text('/proc/sys/kernel/random/boot_id')==BOOT
 assert os.uname().release=='5.10.240-antix.1-486-smp' and os.uname().machine=='i686'
 assert text('/sys/class/dmi/id/product_name').strip()=='Inspiron 1210'
 assert hashlib.sha256(read('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest()=='96eb5049d143a3c7a6e7d672aed1651fa51db069ea9efd3484388b3401f29eaf'
 assert text('/sys/module/gma500_gfx/initstate')=='live'
 assert os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500'
 assert os.path.realpath('/sys/class/drm/card0/device')==os.path.realpath('/sys/bus/pci/devices/0000:00:02.0'), 'DRM canonical device mismatch'
 result['before_kernel_log']=dmesg();result['before_health']=classify_kernel_log(result['before_kernel_log']);assert result['before_health']['classification']!='REJECT'
 result['before_interrupts']=text('/proc/interrupts');result['before_taint']=text('/proc/sys/kernel/tainted');assert not int(result['before_taint'])&~12289
 dst=os.lstat(BASE);assert stat.S_ISDIR(dst.st_mode) and dst.st_uid==0 and stat.S_IMODE(dst.st_mode)==0o700
 dfd=os.open(BASE,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
 fd=os.open('frozen-triangle-one-shot-i386',os.O_RDONLY|os.O_NOFOLLOW,dir_fd=dfd);st=os.fstat(fd)
 with os.fdopen(fd,'rb') as f:binary=f.read()
 assert stat.S_ISREG(st.st_mode) and st.st_uid==0 and st.st_nlink==1 and stat.S_IMODE(st.st_mode)==0o700 and hashlib.sha256(binary).hexdigest()=='758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf'
 assert not os.path.lexists(BASE+'/color.bin')
 record_file('before.json',json.dumps(result,indent=2).encode())
 record_file('attempt-claimed',b'ONE fixed {1,1,0,0} ioctl sequence1; no retry\n')
 assert text('/proc/sys/kernel/random/boot_id')==BOOT
 result['client_invocation_started']=True
 print(json.dumps({'phase':'CLIENT INVOCATION POSSIBLE','boot_id':BOOT,'no_retry':True}),flush=True)
 started=time.monotonic()
 try:
  cp=subprocess.run([BASE+'/frozen-triangle-one-shot-i386','--one-shot-sgx535-rev121',BASE+'/color.bin'],stdin=subprocess.DEVNULL,capture_output=True,timeout=20)
  result.update(client_exit_status=cp.returncode,client_stdout=cp.stdout.decode(errors='replace'),client_stderr=cp.stderr.decode(errors='replace'))
 except subprocess.TimeoutExpired as exc:
  result.update(client_exit_status=124,client_stdout=(exc.stdout or b'').decode(errors='replace'),client_stderr=(exc.stderr or b'').decode(errors='replace'),classification='HOLD: client timeout; fire ambiguous')
 result['client_duration_seconds']=time.monotonic()-started
 record_file('client.stdout',result['client_stdout'].encode());record_file('client.stderr',result['client_stderr'].encode())
 result['boot_id_after']=text('/proc/sys/kernel/random/boot_id');result['after_kernel_log']=dmesg();result['after_health']=classify_kernel_log(result['after_kernel_log']);result['after_interrupts']=text('/proc/interrupts');result['after_taint']=text('/proc/sys/kernel/tainted')
 result['module_state_after']=text('/sys/module/gma500_gfx/initstate');result['pci_driver_after']=os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')
 if os.path.lexists(BASE+'/color.bin'):
  fd=os.open('color.bin',os.O_RDONLY|os.O_NOFOLLOW,dir_fd=dfd);st=os.fstat(fd)
  with os.fdopen(fd,'rb') as f:color=f.read()
  assert stat.S_ISREG(st.st_mode) and st.st_uid==0 and st.st_nlink==1 and len(color)==4096
  result['color']={'bytes':len(color),'sha256':hashlib.sha256(color).hexdigest(),'base64':base64.b64encode(color).decode()}
 if result['boot_id_after']!=BOOT or result['after_health']['classification']=='REJECT' or result['client_exit_status']!=0:result['classification']='HOLD: failure/fault/ambiguity; no retry'
 else:result['classification']='CLIENT RETURNED ZERO; offline completion/pixel interpretation required'
 result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
 record_file('result.json',json.dumps(result,indent=2).encode());print(json.dumps(result),flush=True)
except Exception as exc:
 result['classification']='HOLD' if result['client_invocation_started'] else 'STOP BEFORE IOCTL';result['error']=repr(exc)
 print(json.dumps(result),flush=True);sys.exit(1)
