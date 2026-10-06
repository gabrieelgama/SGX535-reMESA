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

BOOT='f1ab6606-0561-445f-a397-28a028516cd7'
BASE='/root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba'
EVIDENCE='/root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/evidence-f1ab6606-0561-445f-a397-28a028516cd7'
CLIENT_HASH='2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835'
CLIENT_SIZE=775264
CONTEXT={'boot_id': 'f1ab6606-0561-445f-a397-28a028516cd7', 'module_build_id': '85ec06b428c99fac7f9127919b7a488d204f4a77', 'observer_build_id': 'f11d3abb072caa4e1d32836ef92ce201e9c9d126', 'image_sha256': '3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e', 'action': 'MINI12-SGX535-REV121-FROZEN-32x32-SEQ1', 'first_owner_guards': '82/82 PASS', 'capsule': 'UNUSED', 'source_boundary': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'sgx_execution_authorized': False, 'sgx_invocations_this_boot': 0, 'historical_client_invocations': 2, 'triangle_this_boot': 'NOT ATTEMPTED', 'hypothesis': 'EXPERIMENTAL_HYPOTHESIS — NOT ESTABLISHED', 'experimental_words': ['001f00ff', 'fca7f1f1'], 'diagnostic_argb': 'ffff00ff'}
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
 for name,digest in [('gma500_gfx','4499399a9f06a35b924c770ec5f04b8e5a5170c4f744359c675d13ca89223644'),('sgx535_provenance','6afbac1cf48a211135e2a4b4ffc30f96ace64f6d397071595bbc328966bb8af0')]:
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
EXPECTED={'scope': 'AUTHORIZED PASSIVE PROTECTED PREPARATION; NO CLIENT/IOCTL EXECUTION', 'operations': [{'phase': 'directory-created', 'path': '/root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba'}, {'phase': 'created', 'name': 'frozen-triangle-one-shot-response-i386', 'dev': 2049, 'ino': 524751}, {'dev': 2049, 'ino': 524751, 'size': 775264, 'sha256': '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835', 'uid': 0, 'gid': 0, 'mode': 320, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'frozen-triangle-one-shot-response-i386'}, {'phase': 'created', 'name': 'context.preparation.json', 'dev': 2049, 'ino': 524753}, {'dev': 2049, 'ino': 524753, 'size': 789, 'sha256': '92743b803780dab8df2db107f6a97eced11c3399a24ead28828f04e522aadb3f', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'context.preparation.json'}, {'phase': 'created', 'name': 'capsule.preparation.bin', 'dev': 2049, 'ino': 524754}, {'dev': 2049, 'ino': 524754, 'size': 4152, 'sha256': '0d24c306b7e1fcd4eee1d2dc77637fe1eafa91816e849fd5c134e683bf3f3504', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'capsule.preparation.bin'}, {'phase': 'created', 'name': 'kernel.preparation.original.txt', 'dev': 2049, 'ino': 524755}, {'dev': 2049, 'ino': 524755, 'size': 45131, 'sha256': '2c13e499a8d9b837c3e5b8a367e73966fc7f874fc553eb01b32aee4f164aeaf8', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'kernel.preparation.original.txt'}, {'phase': 'created', 'name': 'source.preparation.original.txt', 'dev': 2049, 'ino': 524756}, {'dev': 2049, 'ino': 524756, 'size': 243, 'sha256': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'uid': 0, 'gid': 0, 'mode': 256, 'nlink': 1, 'regular': True, 'phase': 'verified', 'name': 'source.preparation.original.txt'}], 'guards': [{'name': 'root privilege', 'pass': True}, {'name': 'regular input /proc/sys/kernel/random/boot_id', 'pass': True}, {'name': 'exact candidate boot continuity', 'pass': True}, {'name': 'kernel/architecture continuity', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/initstate', 'pass': True}, {'name': 'module Live gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/initstate', 'pass': True}, {'name': 'module Live sgx535_provenance', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity sgx535_provenance', 'pass': True}, {'name': 'PCI owner continuity', 'pass': True}, {'name': 'actual capsule UNUSED; zero eligible calls', 'pass': True}, {'name': 'actual source boundary and uninterrupted producer isolation', 'pass': True}, {'name': 'same prepared UNUSED source boundary', 'pass': True}, {'name': 'kernel log availability', 'pass': True}, {'name': 'current-boot kernel health', 'pass': True}, {'name': 'trusted root parent', 'pass': True}, {'name': 'exclusive unused client/evidence base', 'pass': True}, {'name': 'protected base ownership/mode', 'pass': True}, {'name': 'exact client input length', 'pass': True}, {'name': 'protected evidence parent', 'pass': True}, {'name': 'future evidence destination absent response.original.bin', 'pass': True}, {'name': 'future evidence destination absent color.original.bin', 'pass': True}, {'name': 'future evidence destination absent client.stdout.original.txt', 'pass': True}, {'name': 'future evidence destination absent client.stderr.original.txt', 'pass': True}, {'name': 'future evidence destination absent capsule.original.bin', 'pass': True}, {'name': 'future evidence destination absent kernel.before.original.txt', 'pass': True}, {'name': 'future evidence destination absent kernel.after.original.txt', 'pass': True}, {'name': 'future evidence destination absent manifest.json', 'pass': True}, {'name': 'future evidence destination absent sealed.json', 'pass': True}, {'name': 'future evidence destination absent capsule.binding.json', 'pass': True}, {'name': 'future evidence destination absent capsule.binding.sealed.json', 'pass': True}, {'name': 'future evidence destination absent archive', 'pass': True}, {'name': 'future evidence destination absent source.pre.original.txt', 'pass': True}, {'name': 'future evidence destination absent source.post.original.txt', 'pass': True}, {'name': 'root privilege', 'pass': True}, {'name': 'regular input /proc/sys/kernel/random/boot_id', 'pass': True}, {'name': 'exact candidate boot continuity', 'pass': True}, {'name': 'kernel/architecture continuity', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/initstate', 'pass': True}, {'name': 'module Live gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/gma500_gfx/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity gma500_gfx', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/initstate', 'pass': True}, {'name': 'module Live sgx535_provenance', 'pass': True}, {'name': 'regular input /sys/module/sgx535_provenance/notes/.note.gnu.build-id', 'pass': True}, {'name': 'module note identity sgx535_provenance', 'pass': True}, {'name': 'PCI owner continuity', 'pass': True}, {'name': 'actual capsule UNUSED; zero eligible calls', 'pass': True}, {'name': 'actual source boundary and uninterrupted producer isolation', 'pass': True}, {'name': 'same prepared UNUSED source boundary', 'pass': True}, {'name': 'kernel log availability', 'pass': True}, {'name': 'current-boot kernel health', 'pass': True}, {'name': 'UNUSED capsule unchanged through preparation', 'pass': True}], 'sgx_execution_authorized': False, 'source_observations': [{'sha256': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'payload_base64': 'U0dYU09VUkNFMgpzdGF0ZT0wCnJlYXNvbnM9MApwcm9kdWNlcnNfYWN0aXZlPTAKZHJpdmVyX2F0dGFjaGVkPTEKY2Fwc3VsZV9zdGF0ZT0wCmJvb3Q9MwpwcmVwYXJlZD0xCmFzc2VydGVkX3Jlc2V0PTEyNwpyZWxlYXNlZF9yZXNldD0wCnN0YXJ0dXBfcGVuZGluZz0wCnN0YXJ0dXBfcmVhZHM9MApzdGFydHVwX2ZhdWx0PTAKc3RhcnR1cF9hdXRvbm9tb3VzPTAKZGVsYXllZF9leGNsdXNpb249U1RBUlRVUF9MSUZFQ1lDTEUK', 'decoded': {'supplied_record_consistency': 'CONFIRMED', 'interval': 'PREPARED, MUST REMAIN CONTINUOUS', 'hardware_attribution': 'UNKNOWN WITHOUT VERIFIED LIVE BOOT/PRODUCER PROVENANCE', 'triangle': 'NOT ESTABLISHED BY SOURCE WITNESS', 'errors': [], 'values': {'state': 0, 'reasons': 0, 'producers_active': 0, 'driver_attached': 1, 'capsule_state': 0, 'boot': 3, 'prepared': 1, 'asserted_reset': 127, 'released_reset': 0, 'startup_pending': 0, 'startup_reads': 0, 'startup_fault': 0, 'startup_autonomous': 0, 'delayed_exclusion': 'STARTUP_LIFECYCLE'}}}, {'sha256': '2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555', 'payload_base64': 'U0dYU09VUkNFMgpzdGF0ZT0wCnJlYXNvbnM9MApwcm9kdWNlcnNfYWN0aXZlPTAKZHJpdmVyX2F0dGFjaGVkPTEKY2Fwc3VsZV9zdGF0ZT0wCmJvb3Q9MwpwcmVwYXJlZD0xCmFzc2VydGVkX3Jlc2V0PTEyNwpyZWxlYXNlZF9yZXNldD0wCnN0YXJ0dXBfcGVuZGluZz0wCnN0YXJ0dXBfcmVhZHM9MApzdGFydHVwX2ZhdWx0PTAKc3RhcnR1cF9hdXRvbm9tb3VzPTAKZGVsYXllZF9leGNsdXNpb249U1RBUlRVUF9MSUZFQ1lDTEUK', 'decoded': {'supplied_record_consistency': 'CONFIRMED', 'interval': 'PREPARED, MUST REMAIN CONTINUOUS', 'hardware_attribution': 'UNKNOWN WITHOUT VERIFIED LIVE BOOT/PRODUCER PROVENANCE', 'triangle': 'NOT ESTABLISHED BY SOURCE WITNESS', 'errors': [], 'values': {'state': 0, 'reasons': 0, 'producers_active': 0, 'driver_attached': 1, 'capsule_state': 0, 'boot': 3, 'prepared': 1, 'asserted_reset': 127, 'released_reset': 0, 'startup_pending': 0, 'startup_reads': 0, 'startup_fault': 0, 'startup_autonomous': 0, 'delayed_exclusion': 'STARTUP_LIFECYCLE'}}}], 'kernel_health': {'classification': 'PASS WITH BOUNDED STOCK DIAGNOSTICS', 'faults': [], 'stock_diagnostics': {'unsigned_drm_loader_notice': 1, 'backlight_zero_register': 1, 'acpi_powerbutton_error': 2, 'acpi_powerbutton_warning': 2, 'powerbutton_probe': 1, 'tiny_powerbutton_probe': 1}}, 'client': {'dev': 2049, 'ino': 524751, 'size': 775264, 'sha256': '2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835', 'uid': 0, 'gid': 0, 'mode': 320, 'nlink': 1, 'regular': True}, 'base': {'path': '/root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 'dev': 2049, 'ino': 524750, 'uid': 0, 'gid': 0, 'mode': '0o700'}, 'evidence': {'path': '/root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/evidence-f1ab6606-0561-445f-a397-28a028516cd7', 'dev': 2049, 'ino': 524752, 'uid': 0, 'gid': 0, 'mode': '0o700'}, 'boot_id': 'f1ab6606-0561-445f-a397-28a028516cd7', 'capsule_sha256': '0d24c306b7e1fcd4eee1d2dc77637fe1eafa91816e849fd5c134e683bf3f3504', 'classification': 'PASS PROTECTED PREPARATION ONLY'}
STAGED={'merged_config': {'bytes': 9749, 'sha256': 'ac60694d5e31263d080dd6900ae3ff55feeef3f9e6b9e87c8c3330990d200f59'}}

try:
 raw,log=passive()
 for key,path in [('base',BASE),('evidence',EVIDENCE)]:
  st=os.lstat(path);pin=EXPECTED[key]
  need(stat.S_ISDIR(st.st_mode) and not stat.S_ISLNK(st.st_mode) and st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o700 and (st.st_dev,st.st_ino)==(pin['dev'],pin['ino']),'protected directory identity '+key)
 p=BASE+'/frozen-triangle-one-shot-response-i386';st=os.lstat(p);blob=read(p);pin=EXPECTED['client']
 need(stat.S_ISREG(st.st_mode) and st.st_nlink==1 and st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o500 and (st.st_dev,st.st_ino)==(pin['dev'],pin['ino']) and len(blob)==CLIENT_SIZE and hashlib.sha256(blob).hexdigest()==CLIENT_HASH,'protected exact client readback and inode')
 for name in EXPECTED_ABSENT:need(not os.path.lexists(EVIDENCE+'/'+name),'future original destination absent '+name)
 context=json.loads(read(EVIDENCE+'/context.preparation.json'));need(context==CONTEXT,'saved context exact readback')
 need(read(EVIDENCE+'/capsule.preparation.bin')==raw,'saved initial capsule exact readback')
 cfg=read('/boot/grub/custom.cfg');need(len(cfg)==STAGED['merged_config']['bytes'] and hashlib.sha256(cfg).hexdigest()==STAGED['merged_config']['sha256'],'exact staged GRUB config')
 saved=subprocess.run(['grub-editenv','/boot/grub/grubenv','list'],capture_output=True,timeout=10)
 need(saved.returncode==0 and not saved.stderr and saved.stdout.decode().strip()=='saved_entry=gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1','STOCK remains saved recovery entry')
 holders=[];processes=[];scan_errors=[]
 for pid in sorted(x for x in os.listdir('/proc') if x.isdigit()):
  try:
   cmd=read('/proc/'+pid+'/cmdline').replace(b'\0',b' ').decode(errors='replace')
   if os.path.basename(os.readlink('/proc/'+pid+'/exe')).startswith('frozen-triangle-one-shot') and str(os.getpid())!=pid:processes.append({'pid':int(pid),'command':cmd})
   directory='/proc/'+pid+'/fd'
   for fd in os.listdir(directory):
    try:target=os.readlink(directory+'/'+fd)
    except FileNotFoundError:continue
    if target.startswith('/dev/dri/') or target.startswith('/dev/pvrsrv'):
     st=os.stat(directory+'/'+fd)
     holders.append({'pid':int(pid),'fd':fd,'command':cmd,'target':target,'mode':st.st_mode,'rdev':st.st_rdev})
  except FileNotFoundError:continue
  except (PermissionError,OSError) as exc:scan_errors.append({'pid':int(pid),'error':repr(exc)})
 record['drm_fd_holders']=holders;record['other_frozen_client_processes']=processes;record['process_scan_errors']=scan_errors
 need(not processes,'no other approved frozen-client process observed')
 need(not scan_errors,'passive process metadata inventory complete for encountered processes')
 need(read(EVIDENCE+'/source.preparation.original.txt')==base64.b64decode(record['source_observations'][0]['payload_base64']),'protected original source witness exact readback')
 record['source_quiescence']='CONFIRMED FOR THIS BOOT/EXACT PRODUCER: STARTUP_LIFECYCLE plus clean pending/busy/BIF guard'
 record['continuous_producer_isolation']='CONFIRMED THROUGH FINAL PREPARATION: retained qualified source taps, prepared before admission, no invalidity/loss; must continue to admission/closure'
 record['loaded_modules']=read('/proc/modules').decode(errors='replace');record['interrupts']=read('/proc/interrupts').decode(errors='replace')
 record['kernel']=os.uname().release;record['architecture']=os.uname().machine
 record['boot_id']=BOOT;record['uptime']=read('/proc/uptime').decode().strip();record['capsule_sha256']=hashlib.sha256(raw).hexdigest()
 after_source=read_source_channel();after_checked=evaluate(after_source)
 need(after_checked['supplied_record_consistency']=='CONFIRMED' and after_source==base64.b64decode(record['source_observations'][0]['payload_base64']),'same valid source/isolation witness at final guard end')
 record['source_observations'].append({'sha256':hashlib.sha256(after_source).hexdigest(),'payload_base64':base64.b64encode(after_source).decode(),'decoded':after_checked})
 need(read_channel()==raw,'capsule remains UNUSED at final guard end')
 need(boot()==BOOT,'same boot at final guard end')
 record['classification']='PASS INDEPENDENT CURRENT-BOOT IDENTITY/DESTINATION/HEALTH/SOURCE-LIFECYCLE/CONTINUOUS-ISOLATION GUARDS; NO EXECUTION'
 record['kernel_log']=log.decode(errors='replace')
except Exception as exc:
 record['classification']='STOP PASSIVE GUARD';record['error']=repr(exc);print(json.dumps(record,indent=2),flush=True);sys.exit(1)
print(json.dumps(record,indent=2),flush=True)
