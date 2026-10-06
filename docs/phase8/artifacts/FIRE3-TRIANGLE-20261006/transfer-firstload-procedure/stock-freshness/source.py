import os,stat,json,hashlib,subprocess,datetime
from pathlib import Path
K={'boot': 'fc623297-419e-4bce-85d1-271c1b937d0e', 'kernel': '5.10.240-antix.1-486-smp', 'arch': 'i686', 'machine': 'Inspiron 1210', 'note': '484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7', 'cfg': {'dev': 2049, 'gid': 0, 'ino': 2616686, 'mode': '0o644', 'nlink': 1, 'path': '/boot/grub/custom.cfg', 'regular': True, 'sha256': '5b3261d3af8033b9fdefaf8e0a402a3b35a53709ec68365a0ec4b0a6e94e17a0', 'size': 8615, 'symlink': False, 'uid': 0}, 'plan': {'image': '/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 'incoming': '/home/gama/sgx535-firstload-incoming-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba-transfer2-20261006T052417Z', 'backup': '/boot/grub/custom.cfg.pre-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 'pending': '/boot/grub/.custom.cfg.85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba.pending', 'entry_id': 'sgx535-rev121-frozen-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba', 'title': 'EXPERIMENTAL SGX535 rev121 FROZEN 85ec06b428c99fac7f9127919b7a488d204f4a77 CONSTANT-MAGENTA 3d9eb6ba FIRST-LOAD ONLY (no SGX)'}}
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

r={'scope':'minimum passive SAME STOCK freshness; no mutation','guards':[],'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
def need(v,n):
 r['guards'].append({'name':n,'pass':bool(v)})
 if not v:raise ValueError(n)
try:
 need(os.geteuid()==0,'root passive privilege')
 boot=Path('/proc/sys/kernel/random/boot_id').read_text().strip();r['boot_id']=boot
 need(boot==K['boot'],'same established STOCK boot')
 need(os.uname().release==K['kernel'] and os.uname().machine==K['arch'],'same kernel/architecture')
 need(Path('/sys/class/dmi/id/product_name').read_text().strip()==K['machine'],'same Mini12')
 need(Path('/sys/module/gma500_gfx/initstate').read_text().strip()=='live','stock module Live')
 need(hashlib.sha256(Path('/sys/module/gma500_gfx/notes/.note.gnu.build-id').read_bytes()).hexdigest()==K['note'],'same original STOCK loaded bytes')
 need(os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500','same PCI owner')
 for path in ['/proc/sgx535_source_guard','/proc/sgx535_current_operation','/sys/module/sgx535_provenance','/run/initramfs/sgx535-first-load.log']:need(not os.path.lexists(path),'stock derivative absence '+path)
 path='/boot/grub/custom.cfg';st=os.lstat(path);x=K['cfg'];data=Path(path).read_bytes()
 need(stat.S_ISREG(st.st_mode) and st.st_uid==x['uid'] and st.st_gid==x['gid'] and oct(stat.S_IMODE(st.st_mode))==x['mode'] and st.st_nlink==x['nlink'] and st.st_dev==x['dev'] and st.st_ino==x['ino'] and len(data)==x['size'] and hashlib.sha256(data).hexdigest()==x['sha256'],'same authoritative config inode/bytes')
 p=subprocess.run(['grub-editenv','/boot/grub/grubenv','list'],capture_output=True,timeout=10)
 need(p.returncode==0 and not p.stderr and p.stdout.strip()==b'saved_entry=gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1','STOCK remains saved/default recovery')
 p=subprocess.run(['dmesg'],capture_output=True,timeout=10)
 need(p.returncode==0 and not p.stderr,'complete current kernel log')
 r['kernel_log']=p.stdout.decode(errors='replace');r['health']=classify_kernel_log(r['kernel_log'])
 need(r['health']['classification']!='REJECT','current STOCK health')
 for path in K['plan'].values():
  if path.startswith('/boot/') or path.startswith('/home/gama/'):need(not os.path.lexists(path),'new prospective destination remains absent '+path)
 need(Path('/proc/sys/kernel/random/boot_id').read_text().strip()==boot,'same STOCK boot at end')
 old={'path': '/home/gama/sgx535-firstload-incoming-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/initrd.img-sgx535-firstload-diagnostic-01', 'dev': 2049, 'ino': 1049046, 'size': 6717440, 'sha256': 'c40da0a62a5c8c729e4fa4b46e76c6121c25b757443ac3c0eb4f9cd2da9e41bc', 'mutation_authorized': False}
 fd=os.open(old['path'],os.O_RDONLY|os.O_NOFOLLOW|os.O_NOATIME)
 try:
  before=os.fstat(fd);h=hashlib.sha256();count=0
  while True:
   chunk=os.read(fd,1048576)
   if not chunk:break
   count+=len(chunk);h.update(chunk)
  after=os.fstat(fd)
  r['previous_partial']={'size':count,'sha256':h.hexdigest(),'dev':after.st_dev,'ino':after.st_ino,'mode':oct(stat.S_IMODE(after.st_mode))}
  need(stat.S_ISREG(after.st_mode) and (after.st_dev,after.st_ino)==(old['dev'],old['ino']) and count==old['size'] and h.hexdigest()==old['sha256'] and (before.st_size,before.st_mtime_ns,before.st_ctime_ns,before.st_atime_ns)==(after.st_size,after.st_mtime_ns,after.st_ctime_ns,after.st_atime_ns),'previous partial preserved without mutation')
 finally:os.close(fd)
 r['classification']='PASS SAME HEALTHY STOCK FRESHNESS'
except Exception as e:r['classification']='STOP PASSIVE FRESHNESS';r['error']=repr(e)
print(json.dumps(r,indent=2),flush=True)
if r['classification'].startswith('STOP'):raise SystemExit(1)
