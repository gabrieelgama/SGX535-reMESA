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


import os,sys,stat,json,hashlib,re,subprocess,datetime
BOOT="203a5b9b-5a90-4fd3-8001-3c92cdefeddd"
RELEASE="5.10.240-antix.1-486-smp"
NOTE="b78dc1ffde5cc661d076941e27e10b06edf62ad1da37651f3c38a98e622890c2"
MODULE="2eaffd22637eef6b7b101f046d6fd9756b48c934818a7353f9aa705616014c1e"
BASE="/root/sgx535-frozen-seq1-203a5b9b"
BDF="/sys/bus/pci/devices/0000:00:02.0"
r={"scope":"fresh same-boot read-only SGX preflight; no device open, no write, no module/service operation", "start_utc":datetime.datetime.now(datetime.timezone.utc).isoformat(),"guards":[]}
def check(v,n):
 r["guards"].append({"guard":n,"pass":bool(v)})
 if not v: raise RuntimeError(n)
def read(p):
 with open(p,"rb") as f:return f.read()
def txt(p):return read(p).decode().strip()
def run(a):
 p=subprocess.run(a,stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=8)
 x={"argv":a,"exit":p.returncode,"stdout":p.stdout.decode(errors="replace"),"stderr":p.stderr.decode(errors="replace")}
 r.setdefault("commands",[]).append(x);check(p.returncode==0 and not p.stderr,"command "+" ".join(a));return x["stdout"]
try:
 check(os.geteuid()==0,"root via noninteractive sudo")
 r["boot_id"]=txt("/proc/sys/kernel/random/boot_id");check(r["boot_id"]==BOOT,"authorized corrected experimental boot")
 u=os.uname();r["kernel"]=u.release;r["architecture"]=u.machine;check(u.release==RELEASE and u.machine=="i686","kernel/architecture")
 r["machine"]=txt("/sys/class/dmi/id/product_name");check(r["machine"]=="Inspiron 1210","machine identity")
 r["cmdline"]=txt("/proc/cmdline");check("BOOT_IMAGE=/boot/vmlinuz-"+RELEASE in r["cmdline"],"pinned kernel image in command line")
 r["hook_log"]=txt("/run/initramfs/sgx535-first-load.log");check(r["hook_log"].splitlines()==["SGX535-FIRSTLOAD BEGIN","SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE","SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue"],"ordered corrected first-load hook trace")
 r["taint"]=int(txt("/proc/sys/kernel/tainted"));check(not (r["taint"] & ~12289),"taint within reviewed module/signature baseline")
 note=read("/sys/module/gma500_gfx/notes/.note.gnu.build-id");r["module_note_sha256"]=hashlib.sha256(note).hexdigest();check(r["module_note_sha256"]==NOTE,"corrected derivative loaded note")
 r["module_state"]=txt("/sys/module/gma500_gfx/initstate");check(r["module_state"]=="live","module Live")
 r["modules"]=txt("/proc/modules");check(re.search(r"^gma500_gfx .* Live ",r["modules"],re.M) is not None,"module list Live")
 r["module_path"]=os.path.realpath(BDF+"/driver/module");r["pci_driver"]=os.path.realpath(BDF+"/driver");check(r["module_path"]=="/sys/module/gma500_gfx" and r["pci_driver"]=="/sys/bus/pci/drivers/gma500","PCI bound to corrected driver")
 r["pci"]={k:txt(BDF+"/"+k) for k in ("vendor","device","subsystem_vendor","subsystem_device","irq")};check(r["pci"]=={"vendor":"0x8086","device":"0x8108","subsystem_vendor":"0x1028","subsystem_device":"0x02b1","irq":"16"},"Poulsbo PCI identity/IRQ")
 r["drm_device"]=os.path.realpath("/sys/class/drm/card0/device");check(r["drm_device"]==os.path.realpath(BDF),"DRM canonical PCI ownership")
 st=os.lstat("/dev/dri/card0");r["drm_node"]={"type":"char" if stat.S_ISCHR(st.st_mode) else "other","major":os.major(st.st_rdev),"minor":os.minor(st.st_rdev)};check(r["drm_node"]["type"]=="char","DRM node metadata only")
 r["framebuffer"]=txt("/sys/class/graphics/fb0/name");check(r["framebuffer"]=="gma500drmfb","framebuffer ownership")
 r["interrupts"]=txt("/proc/interrupts");check(re.search(r"^\s*16:.*[\s,]gma500(?:[,\s]|$)",r["interrupts"],re.M) is not None,"IRQ16 gma500 handler")
 r["slimski"]=run(["sv","status","/etc/runit/runsvdir/default/slimski"]);check(r["slimski"].startswith("run:"),"slimski running")
 r["xorg"]=run(["pgrep","-a","Xorg"]);check(bool(r["xorg"].strip()),"Xorg running")
 r["dmesg"]=run(["dmesg"]);r["health"]=classify_kernel_log(r["dmesg"]);check(r["health"]["classification"]!="REJECT","kernel health")
 r["uptime"]=txt("/proc/uptime")
 check(not os.path.lexists(BASE),"new boot-specific client staging destination absent")
 r["color_path_absent"]=not os.path.lexists(BASE+"/color.bin");check(r["color_path_absent"],"no prior color output at destination")
 check(txt("/proc/sys/kernel/random/boot_id")==BOOT,"same boot at end")
 r["end_utc"]=datetime.datetime.now(datetime.timezone.utc).isoformat();r["classification"]="PASS";print(json.dumps(r,indent=2))
except Exception as e:
 r["classification"]="STOP/HOLD";r["failure"]=repr(e);r["end_utc"]=datetime.datetime.now(datetime.timezone.utc).isoformat();print(json.dumps(r,indent=2));sys.exit(1)
