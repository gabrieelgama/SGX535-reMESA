import os,sys,stat,json,hashlib,struct,io,subprocess,datetime,base64
from pathlib import Path
CHANNEL="/proc/sgx535_current_operation"
SIZE=4152
def require(v,m):
 if not v:raise ValueError(m)
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
import os,hashlib,stat
def snapshot_fd(fd):
 st=os.fstat(fd);os.lseek(fd,0,os.SEEK_SET);digest=hashlib.sha256();size=0
 while True:
  chunk=os.read(fd,1048576)
  if not chunk:break
  digest.update(chunk);size+=len(chunk)
 return {'dev':st.st_dev,'ino':st.st_ino,'size':size,'sha256':digest.hexdigest(),
         'uid':st.st_uid,'gid':st.st_gid,'mode':stat.S_IMODE(st.st_mode),
         'nlink':st.st_nlink,'regular':stat.S_ISREG(st.st_mode)}

def copy_exclusive(parent,name,source,size,digest,uid,gid,mode,event):
 if not name or name in ('.','..') or '/' in name or type(size) is not int or size<=0:
  raise ValueError('fixed basename/size required')
 fd=os.open(name,os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW|os.O_RDWR,0o600,dir_fd=parent)
 created=os.fstat(fd)
 try:
  event({'phase':'created','name':name,'dev':created.st_dev,'ino':created.st_ino})
  remaining=size;hasher=hashlib.sha256()
  while remaining:
   chunk=source.read(min(remaining,1048576))
   if not chunk:raise ValueError('short payload')
   if len(chunk)>remaining:raise ValueError('oversize payload')
   remaining-=len(chunk);hasher.update(chunk);view=memoryview(chunk)
   while view:
    count=os.write(fd,view)
    if count<=0:raise OSError('short/zero write')
    view=view[count:]
  if hasher.hexdigest()!=digest:raise ValueError('payload hash mismatch')
  os.fchown(fd,uid,gid);os.fchmod(fd,mode);os.fsync(fd);os.fsync(parent)
  os.close(fd);fd=-1
  fd=os.open(name,os.O_RDONLY|os.O_NOFOLLOW,dir_fd=parent)
  row=snapshot_fd(fd)
  if (row['dev'],row['ino'])!=(created.st_dev,created.st_ino) or not row['regular'] or row['nlink']!=1:
   raise ValueError('inode identity/link mismatch')
  if (row['uid'],row['gid'],row['mode'])!=(uid,gid,mode):raise ValueError('inode ownership/mode mismatch')
  if row['size']!=size or row['sha256']!=digest:raise ValueError('destination readback mismatch')
  event(dict(row,phase='verified',name=name));return row
 except BaseException as exc:
  failure={'phase':'failed','name':name,'created_dev':created.st_dev,'created_ino':created.st_ino,'error':str(exc)}
  if fd>=0:
   try:failure.update(snapshot_fd(fd))
   except OSError as read_error:failure['partial_read_error']=str(read_error)
  event(failure);raise
 finally:
  if fd>=0:os.close(fd)

"""Operational kernel-log guard; known diagnostics remain visible, never generic fault exemptions.

The bounded diagnostics derive from preserved captures and the exact kernel
loader source, not an arbitrary caller-provided whitelist. Fault-bearing/repeated/changed variants
fail closed. This is an operational predicate, not architectural health proof.
"""
import re
from collections import Counter

# Strip only dmesg transport prefixes; retain the message and all diagnostic fields.
PREFIX = re.compile(r"^(?:<\d+>)?\s*(?:\[\s*\d+(?:\.\d+)?\]\s*)?(?:kernel:\s*)?", re.I)
FATAL = re.compile(r"^(?:BUG:)|\bkernel BUG at\b|\bOops:|\bKernel panic\b|\bgeneral protection(?: fault|:)"
                   r"|\bCall Trace:|\b(?:soft|hard)\s+LOCKUP\b|\bblocked for more than\b"
                   r"|\brcu[^\n]*\b(?:stall|stalls)\b", re.I)
WARNING = re.compile(r"\bWARNING:|\bWARN_ON\b|\bBUG:", re.I)
GRAPHICS = re.compile(r"\b(?:gma500|drm|psb|sgx|pvr)\b|0000:00:02\.0", re.I)
GRAPHICS_ERROR = re.compile(r"\b(?:BUG:|fault|error|failed|failure|fatal|timeout|timed out|hang|hung|WARN|warning)\b", re.I)
ACPI_DIAGNOSTICS = {
 'ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)': ('acpi_powerbutton_warning', 2),
 'ACPI Error: Could not enable PowerButton event (20200925/evxfevnt-182)': ('acpi_powerbutton_error', 2),
 'button: probe of LNXPWRBN:00 failed with error -22': ('powerbutton_probe', 1),
 'tiny-power-button: probe of LNXPWRBN:00 failed with error -22': ('tiny_powerbutton_probe', 1),
}
# kernel/module.c emits this exact notice once when permissive signature
# checking admits an unverified module. Cycle05 loads captured stock drm first.
# Module provenance/taint/ownership remain separate mandatory checks.
UNSIGNED_DRM_NOTICE = ('drm: module verification failed: signature and/or required key missing - tainting kernel')
BACKLIGHT = re.compile(r"gma500 0000:00:02\.0: BL bug: Reg ([0-9a-fA-F]{8}) save ([0-9a-fA-F]{8})")

def classify_kernel_log(log):
 """Return visible classification receipt; reject on any selected adverse signal."""
 if not isinstance(log, str) or not log.strip():
  return {'classification': 'REJECT', 'faults': [{'reason': 'missing kernel log'}], 'stock_diagnostics': {}}
 counts = Counter(); faults = []
 for number, raw in enumerate(log.splitlines(), 1):
  message = PREFIX.sub('', raw, count=1)
  reason = None
  # Fatal reports are rejected even alongside a previously seen diagnostic.
  if FATAL.search(message): reason = 'kernel fault/lockup'
  else:
   known = ACPI_DIAGNOSTICS.get(message)
   if message == UNSIGNED_DRM_NOTICE: known = ('unsigned_drm_loader_notice', 1)
   backlight = BACKLIGHT.fullmatch(message)
   if backlight:
    if any(int(value, 16) != 0 for value in backlight.groups()): reason = 'changed backlight diagnostic fields'
    else: known = ('backlight_zero_register', 1)
   if known:
    label, limit = known; counts[label] += 1
    if counts[label] > limit: reason = 'repeated stock diagnostic: ' + label
   elif WARNING.search(message): reason = 'kernel warning'
   elif re.search(r'\bACPI (?:Error|Warning):', message, re.I): reason = 'unexpected ACPI diagnostic'
   elif GRAPHICS.search(message) and GRAPHICS_ERROR.search(message): reason = 'graphics fault/error'
  if reason: faults.append({'line': number, 'message': message, 'reason': reason})
 return {'classification': 'REJECT' if faults else 'PASS WITH BOUNDED STOCK DIAGNOSTICS',
         'faults': faults, 'stock_diagnostics': dict(counts)}

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

BOOT='83ee4ff8-a7f1-4468-b348-d10627f38d17'
BASE='/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c'
EVIDENCE='/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17'
CLIENT_HASH='2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835'
CLIENT_SIZE=775264
CONTEXT={'boot_id': '83ee4ff8-a7f1-4468-b348-d10627f38d17', 'module_build_id': '8be2777b2a79eaa6651b89d19faf4d68cdcdc460', 'observer_build_id': 'f11d3abb072caa4e1d32836ef92ce201e9c9d126', 'image_sha256': '0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d', 'action': 'MINI12-SGX535-REV121-FROZEN-32x32-SEQ1', 'first_owner_guards': '83/83 PASS', 'capsule': 'UNUSED', 'source_boundary': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'sgx_execution_authorized': False, 'sgx_invocations_this_boot': 0, 'historical_client_invocations': 3, 'triangle_this_boot': 'NOT ATTEMPTED', 'hypothesis': 'TWO_TRIANGLE_FOREGROUND_QUAD', 'experimental_words': ['001f00ff', 'fca7f1f1'], 'diagnostic_argb': 'ffff00ff'}
EXPECTED_ABSENT=['response.original.bin', 'color.original.bin', 'client.stdout.original.txt', 'client.stderr.original.txt', 'capsule.original.bin', 'kernel.before.original.txt', 'kernel.after.original.txt', 'manifest.json', 'sealed.json', 'capsule.binding.json', 'capsule.binding.sealed.json', 'archive', 'source.pre.original.txt', 'source.post.original.txt']

record={'scope':'AUTHORIZED PASSIVE PROTECTED PREPARATION; NO CLIENT/IOCTL EXECUTION','operations':[],'guards':[],'sgx_execution_authorized':False}
def need(v,n):
 record['guards'].append({'name':n,'pass':bool(v)})
 require(v,n)
def event(row):record['operations'].append(row)
def read(path):
 fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK|os.O_CLOEXEC)
 try:
  s=os.fstat(fd);need(stat.S_ISREG(s.st_mode),'regular input '+path)
  data=bytearray()
  while True:
   b=os.read(fd,1048576)
   if not b:break
   data.extend(b)
  return bytes(data)
 finally:os.close(fd)
def boot():return read('/proc/sys/kernel/random/boot_id').decode().strip()
def passive():
 need(os.geteuid()==0,'root privilege')
 need(boot()==BOOT,'exact candidate boot continuity')
 need(os.uname().release=='5.10.240-antix.1-486-smp' and os.uname().machine=='i686','kernel/architecture continuity')
 for name,digest in [('gma500_gfx','35ec9f4e576a83a3990ebda4f8aee5869c0e4ea32e1e35969fa48abffa7dc832'),('sgx535_provenance','6afbac1cf48a211135e2a4b4ffc30f96ace64f6d397071595bbc328966bb8af0')]:
  need(read('/sys/module/'+name+'/initstate').strip()==b'live','module Live '+name)
  need(hashlib.sha256(read('/sys/module/'+name+'/notes/.note.gnu.build-id')).hexdigest()==digest,'module note identity '+name)
 need(os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500','PCI owner continuity')
 raw=read_channel();header=struct.unpack('<8s12I',raw[:56])
 need(header[:3]==(b'SGXCAPB1',1,4152) and not any(header[3:]) and not any(raw[56:]),'actual capsule UNUSED; zero eligible calls')
 source=read_source_channel();checked=evaluate(source)
 need(checked['supplied_record_consistency']=='CONFIRMED','same loaded producer clean source and continuous-isolation witness')
 need(hashlib.sha256(source).hexdigest()=='2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555','same source boundary remains prepared without rearm')
 record.setdefault('source_observations',[]).append({'sha256':hashlib.sha256(source).hexdigest(),'payload_base64':base64.b64encode(source).decode(),'decoded':checked})
 log=subprocess.run(['dmesg'],stdin=subprocess.DEVNULL,capture_output=True,timeout=10)
 need(log.returncode==0 and not log.stderr,'kernel log availability')
 record['kernel_health']=classify_kernel_log(log.stdout.decode(errors='replace'))
 need(record['kernel_health']['classification']!='REJECT','current-boot kernel health')
 return raw,log.stdout
EXPECTED={'scope': 'AUTHORIZED PASSIVE PROTECTED PREPARATION; NO CLIENT/IOCTL EXECUTION', 'operations': [{'phase': 'directory-created', 'path': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c'}, {'phase': 'created', 'name': 'frozen-triangle-one-shot-response-i386', 'dev': 2049, 'ino': 524772}, {'dev': 2049, 'ino': 524772, 'size': 775264, 'sha256': '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835', 'uid': 0, 'gid': 0, 'mode': 320, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'frozen-triangle-one-shot-response-i386'}, {'phase': 'created', 'name': 'context.preparation.json', 'dev': 2049, 'ino': 524774}, {'dev': 2049, 'ino': 524774, 'size': 771, 'sha256': '36667a98ec473ef7793451c7bae1ac52593929bbfa149e0fc2554a47a2252e5d', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'context.preparation.json'}, {'phase': 'created', 'name': 'capsule.preparation.bin', 'dev': 2049, 'ino': 524775}, {'dev': 2049, 'ino': 524775, 'size': 4152, 'sha256': '0d24c306b7e1fcd4eee1d2dc77637fe1eafa91816e849fd5c134e683bf3f3504', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'capsule.preparation.bin'}, {'phase': 'created', 'name': 'kernel.preparation.original.txt', 'dev': 2049, 'ino': 524776}, {'dev': 2049, 'ino': 524776, 'size': 46735, 'sha256': 'd77b0d676c1a2b54c580791c969a8c854ffd97b2e92b5bede7db226bd7e741cf', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'kernel.preparation.original.txt'}, {'phase': 'created', 'name': 'source.preparation.original.txt', 'dev': 2049, 'ino': 524777}, {'dev': 2049, 'ino': 524777, 'size': 243, 'sha256': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'source.preparation.original.txt'}], 'guards': [{'name': 'root privilege', 'pass': True}, {'name': 'regular input /proc/sys/kernel/random/boot_id', 'pass': True}, {'name': 'exact candidate boot continuity', 'pass': True}, {'name': 'kernel/architecture continuity', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/initstate', 'pass': True}, {'name': 'module Live gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/initstate', 'pass': True}, {'name': 'module Live sgx535_provenance', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity sgx535_provenance', 'pass': True}, {'name': 'PCI owner continuity', 'pass': True}, {'name': 'actual capsule UNUSED; zero eligible calls', 'pass': True}, {'name': 'actual source boundary and uninterrupted producer isolation', 'pass': True}, {'name': 'same prepared UNUSED source boundary', 'pass': True}, {'name': 'kernel log availability', 'pass': True}, {'name': 'current-boot kernel health', 'pass': True}, {'name': 'trusted root parent', 'pass': True}, {'name': 'exclusive unused client/evidence base', 'pass': True}, {'name': 'protected base ownership/mode', 'pass': True}, {'name': 'exact client input length', 'pass': True}, {'name': 'protected evidence parent', 'pass': True}, {'name': 'future evidence destination absent response.original.bin', 'pass': True}, {'name': 'future evidence destination absent color.original.bin', 'pass': True}, {'name': 'future evidence destination absent client.stdout.original.txt', 'pass': True}, {'name': 'future evidence destination absent client.stderr.original.txt', 'pass': True}, {'name': 'future evidence destination absent capsule.original.bin', 'pass': True}, {'name': 'future evidence destination absent kernel.before.original.txt', 'pass': True}, {'name': 'future evidence destination absent kernel.after.original.txt', 'pass': True}, {'name': 'future evidence destination absent manifest.json', 'pass': True}, {'name': 'future evidence destination absent sealed.json', 'pass': True}, {'name': 'future evidence destination absent capsule.binding.json', 'pass': True}, {'name': 'future evidence destination absent capsule.binding.sealed.json', 'pass': True}, {'name': 'future evidence destination absent archive', 'pass': True}, {'name': 'future evidence destination absent source.pre.original.txt', 'pass': True}, {'name': 'future evidence destination absent source.post.original.txt', 'pass': True}, {'name': 'root privilege', 'pass': True}, {'name': 'regular input /proc/sys/kernel/random/boot_id', 'pass': True}, {'name': 'exact candidate boot continuity', 'pass': True}, {'name': 'kernel/architecture continuity', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/initstate', 'pass': True}, {'name': 'module Live gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/initstate', 'pass': True}, {'name': 'module Live sgx535_provenance', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity sgx535_provenance', 'pass': True}, {'name': 'PCI owner continuity', 'pass': True}, {'name': 'actual capsule UNUSED; zero eligible calls', 'pass': True}, {'name': 'actual source boundary and uninterrupted producer isolation', 'pass': True}, {'name': 'same prepared UNUSED source boundary', 'pass': True}, {'name': 'kernel log availability', 'pass': True}, {'name': 'current-boot kernel health', 'pass': True}, {'name': 'UNUSED capsule unchanged through preparation', 'pass': True}], 'sgx_execution_authorized': False, 'source_observations': [{'sha256': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'payload_base64': 'U0dYU09VUkNFMgpzdGF0ZT0wCnJlYXNvbnM9MApwcm9kdWNlcnNfYWN0aXZlPTAKZHJpdmVyX2F0dGFjaGVkPTEKY2Fwc3VsZV9zdGF0ZT0wCmJvb3Q9MwpwcmVwYXJlZD0xCmFzc2VydGVkX3Jlc2V0PTEyNwpyZWxlYXNlZF9yZXNldD0wCnN0YXJ0dXBfcGVuZGluZz0wCnN0YXJ0dXBfcmVhZHM9MApzdGFydHVwX2ZhdWx0PTAKc3RhcnR1cF9hdXRvbm9tb3VzPTAKZGVsYXllZF9leGNsdXNpb249U1RBUlRVUF9MSUZFQ1lDTEUK', 'decoded': {'supplied_record_consistency': 'CONFIRMED', 'interval': 'PREPARED, MUST REMAIN CONTINUOUS', 'hardware_attribution': 'UNKNOWN WITHOUT VERIFIED LIVE BOOT/PRODUCER PROVENANCE', 'triangle': 'NOT ESTABLISHED BY SOURCE WITNESS', 'errors': [], 'values': {'state': 0, 'reasons': 0, 'producers_active': 0, 'driver_attached': 1, 'capsule_state': 0, 'boot': 3, 'prepared': 1, 'asserted_reset': 127, 'released_reset': 0, 'startup_pending': 0, 'startup_reads': 0, 'startup_fault': 0, 'startup_autonomous': 0, 'delayed_exclusion': 'STARTUP_LIFECYCLE'}}}, {'sha256': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'payload_base64': 'U0dYU09VUkNFMgpzdGF0ZT0wCnJlYXNvbnM9MApwcm9kdWNlcnNfYWN0aXZlPTAKZHJpdmVyX2F0dGFjaGVkPTEKY2Fwc3VsZV9zdGF0ZT0wCmJvb3Q9MwpwcmVwYXJlZD0xCmFzc2VydGVkX3Jlc2V0PTEyNwpyZWxlYXNlZF9yZXNldD0wCnN0YXJ0dXBfcGVuZGluZz0wCnN0YXJ0dXBfcmVhZHM9MApzdGFydHVwX2ZhdWx0PTAKc3RhcnR1cF9hdXRvbm9tb3VzPTAKZGVsYXllZF9leGNsdXNpb249U1RBUlRVUF9MSUZFQ1lDTEUK', 'decoded': {'supplied_record_consistency': 'CONFIRMED', 'interval': 'PREPARED, MUST REMAIN CONTINUOUS', 'hardware_attribution': 'UNKNOWN WITHOUT VERIFIED LIVE BOOT/PRODUCER PROVENANCE', 'triangle': 'NOT ESTABLISHED BY SOURCE WITNESS', 'errors': [], 'values': {'state': 0, 'reasons': 0, 'producers_active': 0, 'driver_attached': 1, 'capsule_state': 0, 'boot': 3, 'prepared': 1, 'asserted_reset': 127, 'released_reset': 0, 'startup_pending': 0, 'startup_reads': 0, 'startup_fault': 0, 'startup_autonomous': 0, 'delayed_exclusion': 'STARTUP_LIFECYCLE'}}}], 'kernel_health': {'classification': 'PASS WITH BOUNDED STOCK DIAGNOSTICS', 'faults': [], 'stock_diagnostics': {'unsigned_drm_loader_notice': 1, 'backlight_zero_register': 1, 'acpi_powerbutton_error': 2, 'acpi_powerbutton_warning': 2, 'powerbutton_probe': 1, 'tiny_powerbutton_probe': 1}}, 'client': {'dev': 2049, 'ino': 524772, 'size': 775264, 'sha256': '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835', 'uid': 0, 'gid': 0, 'mode': 320, 'nlink': 1, 'regular': True}, 'base': {'path': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c', 'dev': 2049, 'ino': 524771, 'uid': 0, 'gid': 0, 'mode': '0o700'}, 'evidence': {'path': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17', 'dev': 2049, 'ino': 524773, 'uid': 0, 'gid': 0, 'mode': '0o700'}, 'boot_id': '83ee4ff8-a7f1-4468-b348-d10627f38d17', 'capsule_sha256': '0d24c306b7e1fcd4eee1d2dc77637fe1eafa91816e849fd5c134e683bf3f3504', 'classification': 'PASS PROTECTED PREPARATION ONLY'}
STAGED={'merged_config': {'bytes': 10881, 'sha256': '28a854b173b5aae73a8be88267bfb5575622e809c2c3301632e012446dbb4fdb'}}

CARD={'schema': 'SGX535_NEXT_3D_EXPERIMENT_CARD_V1', 'state': 'PRE07_LIVE_PASS_BOUND_TO_FRESH_BOOT', 'experiment': 'TWO_TRIANGLE_FOREGROUND_QUAD', 'candidate_manifest': 'docs/phase8/two-triangle-experimental-candidate-20261006.json', 'candidate_manifest_sha256': '5d0a75098002e0785770c5fc0452538a4e4cfb271bf3ee9ab862a6e73e615f30', 'driver': {'path': '/home/gama/sgx535-offline/phase8-first-3d-offline-20261006T072942Z/build/module/gma500_gfx.ko', 'bytes': 250864, 'sha256': 'c21c26c567fc7676e28f4f732cb8cd3c7af1a7360560b58a310789523dc9454d', 'build_id': '8be2777b2a79eaa6651b89d19faf4d68cdcdc460', 'loaded_note_sha256': '35ec9f4e576a83a3990ebda4f8aee5869c0e4ea32e1e35969fa48abffa7dc832', 'abi': 'ELF32/i386', 'vermagic': '5.10.240-antix.1-486-smp SMP mod_unload modversions 486 ', 'dependencies': 'drm,drm_kms_helper,sgx535_provenance,video,i2c-algo-bit', 'versioned_imports': 253, 'missing_imports': [], 'crc_mismatches': {}, 'unversioned_undefined': [], 'module_layout': '0xb84efb99', 'target_table_sha256': 'faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca', 'qualification': 'PASS'}, 'observer': {'path': '/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/build/observer/sgx535_provenance.ko', 'bytes': 26284, 'sha256': '2e35e863d51dbc1d407feef08f08bb2edc6390af621a0f173a9b1a73ec76158a', 'build_id': 'f11d3abb072caa4e1d32836ef92ce201e9c9d126', 'loaded_note_sha256': '6afbac1cf48a211135e2a4b4ffc30f96ace64f6d397071595bbc328966bb8af0', 'abi': 'ELF32/i386', 'vermagic': '5.10.240-antix.1-486-smp SMP mod_unload modversions 486 ', 'dependencies': '', 'versioned_imports': 38, 'missing_imports': [], 'crc_mismatches': {}, 'unversioned_undefined': [], 'module_layout': '0xb84efb99', 'target_table_sha256': 'faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca', 'qualification': 'PASS'}, 'image': {'path': '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c', 'bytes': 50816648, 'sha256': '0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d'}, 'client': {'path': 'docs/phase8/artifacts/FIRE3-TRIANGLE-20261006/known-good-artifacts/frozen-triangle-one-shot-response-i386', 'sha256': '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835', 'bytes': 775264, 'protected_path': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/frozen-triangle-one-shot-response-i386', 'inode_receipt': {'dev': 2049, 'ino': 524772, 'size': 775264, 'sha256': '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835', 'uid': 0, 'gid': 0, 'mode': 320, 'nlink': 1, 'regular': True}, 'executed': False}, 'uapi': {'path': 'docs/phase8/artifacts/FIRE3-TRIANGLE-20261006/known-good-artifacts/gma500_fixed_uapi.h', 'sha256': '04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420', 'bytes': 760}, 'kernel': '5.10.240-antix.1-486-smp', 'machine': 'Inspiron 1210', 'core_id': '0x01130000', 'core_revision': '0x00010201', 'boot_uuid': '83ee4ff8-a7f1-4468-b348-d10627f38d17', 'source_witness': {'authoritative_field': 'witness', 'relationship': 'same operation witness; no second independent witness'}, 'capsule': 'MUST_BE_UNUSED', 'live_PRE07': 'PASS', 'sgx_execution_authorized': True, 'maximum_client_launches': 1, 'maximum_ioctl_attempts': 1, 'no_retry': True, 'authorization_consumed_at': 'first possible client/ioctl issuance, including controller interruption or ambiguous result', 'delta': {'geometry': {'before_vertices': 3, 'after_vertices': 4, 'positions': [[8.0, 8.0, 0.5, 1.0], [24.0, 8.0, 0.5, 1.0], [8.0, 24.0, 0.5, 1.0], [24.0, 24.0, 0.5, 1.0]], 'indices_before': [0, 1, 2], 'indices_after': [0, 1, 2, 1, 3, 2]}, 'packet_count': {'before': '0x81400003', 'after': '0x81400006'}, 'layout': {'TA_stream_before': 96, 'TA_stream_after': 128, 'TA_bytes': 68, 'vertex_stride': 32, 'indices_offset': 512, 'indices_capacity': 16384}, 'relocation_changes': [{'index': 41, 'before': {'site': 'ta_stream+00', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 24, 'buffer': 0, 'pre_add': 17280, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 1073741824, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+00', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 32, 'buffer': 0, 'pre_add': 17280, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 1073741824, 'arg0': 0, 'arg1': 0}}, {'index': 42, 'before': {'site': 'ta_stream+0c', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 27, 'buffer': 0, 'pre_add': 17220, 'dst_buffer': 2, 'target_alignment': 4, 'op': 0, 'mask': 4294967295, 'shift': 0, 'background': 0, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+0c', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 35, 'buffer': 0, 'pre_add': 17220, 'dst_buffer': 2, 'target_alignment': 4, 'op': 0, 'mask': 4294967295, 'shift': 0, 'background': 0, 'arg0': 0, 'arg1': 0}}, {'index': 43, 'before': {'site': 'ta_stream+14', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 29, 'buffer': 0, 'pre_add': 17024, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 0, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+14', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 37, 'buffer': 0, 'pre_add': 17024, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 0, 'arg0': 0, 'arg1': 0}}, {'index': 44, 'before': {'site': 'ta_stream+1c', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 31, 'buffer': 0, 'pre_add': 17408, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 1073741824, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+1c', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 39, 'buffer': 0, 'pre_add': 17408, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 1073741824, 'arg0': 0, 'arg1': 0}}, {'index': 45, 'before': {'site': 'ta_stream+28', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 34, 'buffer': 0, 'pre_add': 512, 'dst_buffer': 2, 'target_alignment': 4, 'op': 0, 'mask': 4294967295, 'shift': 0, 'background': 0, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+28', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 42, 'buffer': 0, 'pre_add': 512, 'dst_buffer': 2, 'target_alignment': 4, 'op': 0, 'mask': 4294967295, 'shift': 0, 'background': 0, 'arg0': 0, 'arg1': 0}}, {'index': 46, 'before': {'site': 'ta_stream+30', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 36, 'buffer': 0, 'pre_add': 16928, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 0, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+30', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 44, 'buffer': 0, 'pre_add': 16928, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 0, 'arg0': 0, 'arg1': 0}}, {'index': 47, 'before': {'site': 'ta_stream+38', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 38, 'buffer': 0, 'pre_add': 17504, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 1610612736, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_stream+38', 'owner_bo': 'vertex_ta', 'target_bo': 'pds', 'where': 46, 'buffer': 0, 'pre_add': 17504, 'dst_buffer': 2, 'target_alignment': 16, 'op': 0, 'mask': 268435455, 'shift': 262144, 'background': 1610612736, 'arg0': 0, 'arg1': 0}}, {'index': 48, 'before': {'site': 'ta_registers+34', 'owner_bo': 'control', 'target_bo': 'vertex_ta', 'where': 65, 'buffer': 2, 'pre_add': 96, 'dst_buffer': 4, 'target_alignment': 4, 'op': 0, 'mask': 4294967295, 'shift': 0, 'background': 0, 'arg0': 0, 'arg1': 0}, 'after': {'site': 'ta_registers+34', 'owner_bo': 'control', 'target_bo': 'vertex_ta', 'where': 65, 'buffer': 2, 'pre_add': 128, 'dst_buffer': 4, 'target_alignment': 4, 'op': 0, 'mask': 4294967295, 'shift': 0, 'background': 0, 'arg0': 0, 'arg1': 0}}], 'fragment': 'UNCHANGED 001f00ff fca7f1f1', 'all_other_state': 'unchanged as qualified manifest'}, 'preparation': ['explicit authorization required before target contact/staging/render', 'fresh healthy STOCK and sudo/root access; preserve normal/default recovery', 'transactional exclusive-path transfer; complete/stable/synced size+hash+byte verification; no retry after STOP', 'stage exact candidate at distinct destination; independently verify image/GRUB and preserve previous artifacts', 'one manual FIRSTLOAD; no hot replacement or second boot after failed qualification', 'fresh exact boot/module/image and first-owner qualification; source startup witness, pending/busy/BIF, continuous isolation, capsule UNUSED, client never run, kernel health, protected destinations', 'bind observed boot UUID and witness to a new derived live card; require complete PRE07 PASS', 'immediate minimum passive bindings; any change prevents dispatch'], 'call_template': ['<verified root-owned qualified client>', '--one-shot-sgx535-rev121', '<exclusive protected output>/color.original.bin', '<exclusive protected output>/response.original.bin'], 'call_template_is_executable': False, 'expected_lifecycle': {'ioctl_return': 0, 'client_exit': 0, 'operation_errno': 0, 'phase': 9, 'phase_name': 'RETIRED', 'accepted_ledger': '0x7', 'TA': 'confirmed', 'end_render': 'confirmed', '3D_memory_free': 'confirmed', 'provenance': 'same fresh operation required'}, 'expected_readback': {'bytes': 4096, 'width': 32, 'height': 32, 'format': 'LE ARGB8888', 'pitch': 128, 'magenta': 256, 'zero': 768, 'coordinates': 'x=8..23,y=8..23 inclusive', 'oracle_path': 'docs/phase8/artifacts/two-triangle-experiment-20261006/expected-color.bin', 'oracle_sha256': '6e9af8e8b6576b979aa78816d44bd3b0ffd31b70e169a982e83736d63ab91729', 'scope': 'CPU predicted fill; not established SGX raster convention'}, 'predeclared_outcomes': {'exact_quad': 'MULTI_TRIANGLE_ESTABLISHED if complete attributed output equals oracle and additional second-triangle pixels exist; not 3D/perspective/interpolation', 'baseline_120': 'MULTIPLE_PRIMITIVES_NOT_ESTABLISHED; second primitive may not have contributed; no automatic correction/retry', 'partial_or_other_nonzero': 'UNEXPECTED_COVERAGE; preserve exact values/coordinates, no binary PASS inference', 'all_zero': 'MULTIPLE_PRIMITIVES_NOT_ESTABLISHED; keep first triangle baseline established', 'HOLD_or_failure': 'LIFECYCLE_FAILURE separate from coverage', 'missing_or_ambiguous_evidence': 'AMBIGUOUS STOP; no retry'}, 'preservation_order': ['mark possible issuance consumed before launch', 'retain complete stdout/stderr/exit/timing and full4268 response,4096 color if produced', 'retain capsule/source before+after,raw producer evidence,accepted ledger,boot/module/witness/ownership bindings,kernel logs and card', 'retrieve and hash originals without overwrite, fsync seal manifest, independently correlate', 'only then spatially interpret exact pixels; derived images separately named'], 'stop_conditions': ['identity/hash/CRC/image mismatch', 'wrong boot or invalid source lifetime', 'ownership/isolation/pending/busy/BIF/health guard failure or UNKNOWN', 'capsule not UNUSED or prior invocation', 'evidence destination/binding failure', 'any ambiguous possible issuance, timeout,partial result,HOLD or preservation loss'], 'recovery': 'No automatic retry/reboot/reload/reset/clear. Preserve resources and evidence on uncertainty; report to maintainer; normal STOCK remains manual recovery boundary.', 'display_publication_follows': False, 'presentation_plan': 'Only after attribution and separate display scope: SGX RENDER + CPU/XORG PRESENTATION. Existing FIRE3 source validator rejects a quad; use an explicitly qualified quad validator/new card, never overwrite old source.', 'requested_authorization': 'fresh live preparation and at most ONE render only after PRE07 PASS; no repeated frames or display publication', 'offline_pixel_interpreter': 'tools/sgx535_demo/two_triangle.py:inspect_readback (requires independently established provenance)', 'maximum_candidate_transfers': 1, 'transport_deadline_seconds': 900, 'card_id': 'MINI12-TWO-INDEXED-TRIANGLES-83ee4ff8-a7f1-4468-b348-d10627f38d17', 'boot_id': '83ee4ff8-a7f1-4468-b348-d10627f38d17', 'witness': {'boot_id': '83ee4ff8-a7f1-4468-b348-d10627f38d17', 'module_build_id': '8be2777b2a79eaa6651b89d19faf4d68cdcdc460', 'observer_build_id': 'f11d3abb072caa4e1d32836ef92ce201e9c9d126', 'image_sha256': '0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d', 'source_boundary': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555'}, 'PRE07_READY_FOR_SQUARE': 'YES', 'ready_for_execution_authorization': True, 'execution_authorized': True, 'protected_evidence_directory': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17', 'directory_receipt': {'path': '/root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17', 'dev': 2049, 'ino': 524773, 'uid': 0, 'gid': 0, 'mode': '0o700'}, 'pre07_evidence': '/home/gama/sgx535-offline/phase8-square-continuation-20261006T084340Z/pre07-live-evaluation.json', 'prepared_card_path': '/home/gama/sgx535-gfx/docs/phase8/next-3d-execution-card-20261006.json', 'prepared_card_sha256': '33739509458424f3e717084078f45240965736a35d3817e52a178065a6660dae', 'invocations_before_call': 0, 'operator_readiness': '/home/gama/sgx535-offline/phase8-square-continuation-20261006T084340Z/operator-readiness.json'}
CARD_SHA='1cc193984a39a8c161fff55c2c9002da28740ebaddf367c623696b585125eae7'

def preserve_bytes(dest,name,data):
 require(len(data)>0,'empty required evidence '+name)
 return copy_exclusive(dest,name,io.BytesIO(data),len(data),hashlib.sha256(data).hexdigest(),0,0,0o400,event)
def saved_file(dest,name):
 fd=os.open(name,os.O_RDONLY|os.O_NOFOLLOW|os.O_NONBLOCK,dir_fd=dest)
 try:
  st=os.fstat(fd);require(stat.S_ISREG(st.st_mode) and st.st_uid==0 and st.st_gid==0 and st.st_nlink==1,'protected original '+name)
  return snapshot_fd(fd)
 finally:os.close(fd)
def snapshot_post(dest):
 for name,obtain in [('capsule.original.bin',read_channel),('source.post.original.txt',read_source_channel),('kernel.after.original.txt',lambda:subprocess.run(['dmesg'],stdin=subprocess.DEVNULL,capture_output=True,timeout=10).stdout)]:
  try:record.setdefault('post_originals',{})[name]=preserve_bytes(dest,name,obtain())
  except BaseException as exc:record.setdefault('preservation_errors',[]).append({'name':name,'error':repr(exc)})
 try:
  record['post_boot_id']=boot()
  record['post_notes']={name:hashlib.sha256(read('/sys/module/'+name+'/notes/.note.gnu.build-id')).hexdigest() for name in ['gma500_gfx','sgx535_provenance']}
 except BaseException as exc:record['post_context_error']=repr(exc)
 for name in ['response.original.bin','color.original.bin','client.stdout.original.txt','client.stderr.original.txt']:
  try:record.setdefault('client_originals',{})[name]=saved_file(dest,name)
  except BaseException as exc:record.setdefault('preservation_errors',[]).append({'name':name,'error':repr(exc)})
 destfd=os.open(EVIDENCE,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
 try:os.fsync(destfd)
 finally:os.close(destfd)

record['scope']='EXPLICITLY AUTHORIZED ONE ORDINARY CLIENT CALL; NO RETRY'
record['sgx_execution_authorized']=True
record['client_launch_attempted']=False
record['retry']=False
record['interpretation']='NONE: PRESERVING ORIGINALS FIRST'
dest=None;out=None;err=None;proc=None
try:
 raw,log=passive()
 st=os.lstat(EVIDENCE);pin=EXPECTED['evidence']
 need(stat.S_ISDIR(st.st_mode) and not stat.S_ISLNK(st.st_mode) and st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o700 and (st.st_dev,st.st_ino)==(pin['dev'],pin['ino']),'protected original evidence directory unchanged')
 client=BASE+'/frozen-triangle-one-shot-response-i386';st=os.lstat(client);blob=read(client);pin=EXPECTED['client']
 need(stat.S_ISREG(st.st_mode) and st.st_nlink==1 and st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o500 and (st.st_dev,st.st_ino)==(pin['dev'],pin['ino']) and len(blob)==CLIENT_SIZE and hashlib.sha256(blob).hexdigest()==CLIENT_HASH,'exact protected approved executable immediately before call')
 for name in EXPECTED_ABSENT:need(not os.path.lexists(EVIDENCE+'/'+name),'unused future evidence destination '+name)
 dest=os.open(EVIDENCE,os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
 # Durable producer originals and exact card binding before client creation.
 for name,data in [('source.pre.original.txt',base64.b64decode(record['source_observations'][0]['payload_base64'])),('capsule.before.original.bin',raw),('kernel.before.original.txt',log),('execution-card.original.json',(json.dumps(CARD,indent=2)+'\n').encode()),('fire-authorization.original.json',(json.dumps({'authority':'project maintainer/operator explicit FIRE','card_sha256':CARD_SHA,'boot_id':BOOT,'maximum_invocations':1,'no_retry':True,'sgx_execution_authorized':True},sort_keys=True)+'\n').encode())]:
  preserve_bytes(dest,name,data)
 out=os.open('client.stdout.original.txt',os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW|os.O_CLOEXEC,0o600,dir_fd=dest)
 err=os.open('client.stderr.original.txt',os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW|os.O_CLOEXEC,0o600,dir_fd=dest)
 os.fsync(dest)
 source_now=read_source_channel();checked=evaluate(source_now)
 need(checked['supplied_record_consistency']=='CONFIRMED' and source_now==base64.b64decode(record['source_observations'][0]['payload_base64']),'continuous clean source/isolation immediately before one call')
 need(read_channel()==raw and boot()==BOOT,'same boot and UNUSED capsule immediately before one call')
 argv=[client,'--one-shot-sgx535-rev121',EVIDENCE+'/color.original.bin',EVIDENCE+'/response.original.bin']
 record['client_argv']=argv
 intent={'boot_id':BOOT,'card_sha256':CARD_SHA,'argv':argv,'maximum_launches':1,'state':'ONE LAUNCH ABOUT TO BE ATTEMPTED; UNKNOWN IF INTERRUPTED','no_retry':True}
 preserve_bytes(dest,'invocation.intent.original.json',(json.dumps(intent,indent=2)+'\n').encode())
 record['client_launch_attempted']=True;record['client_start_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
 proc=subprocess.Popen(argv,stdin=subprocess.DEVNULL,stdout=out,stderr=err,close_fds=True)
 record['client_pid']=proc.pid
 try:record['client_exit_status']=proc.wait(timeout=30)
 except subprocess.TimeoutExpired:
  record['client_timeout']=True;proc.kill()
  try:record['client_exit_status']=proc.wait(timeout=2)
  except subprocess.TimeoutExpired:record['client_exit_status']='STILL RUNNING/UNINTERRUPTIBLE; STOP NO RETRY'
 record['client_end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat()
 for fd in [out,err]:os.fsync(fd);os.close(fd)
 out=err=None
 snapshot_post(dest)
 record['classification']='ONE CLIENT LAUNCH COMPLETED; ORIGINALS RETAINED; REQUIRES OFFLINE CORRELATION'
except BaseException as exc:
 record['classification']='STOP; NO RETRY; PARTIAL ORIGINALS RETAINED';record['error']=repr(exc)
 for fd in [out,err]:
  if fd is not None:
   try:os.fsync(fd);os.close(fd)
   except BaseException as file_error:record.setdefault('preservation_errors',[]).append({'stdout_stderr_close_error':repr(file_error)})
 if dest is not None and record['client_launch_attempted']:
  try:snapshot_post(dest)
  except BaseException as post_error:record['post_snapshot_error']=repr(post_error)
finally:
 if dest is not None:
  try:preserve_bytes(dest,'execution.receipt.original.json',(json.dumps(record,indent=2)+'\n').encode())
  except BaseException as exc:record['receipt_preservation_error']=repr(exc)
  os.close(dest)
print(json.dumps(record,indent=2),flush=True)
if record['classification'].startswith('STOP'):sys.exit(1)
