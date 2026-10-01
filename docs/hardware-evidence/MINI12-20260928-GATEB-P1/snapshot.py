"""Remote, read-only OS-visible Gate B snapshot. No device node or MMIO opens."""

import datetime
import hashlib
import json
import os
import pathlib


def now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def text(path):
    try:
        return {"value": pathlib.Path(path).read_text().strip()}
    except (OSError, UnicodeError) as exc:
        return {"error": type(exc).__name__ + ": " + str(exc)}


def link(path):
    try:
        return {"value": os.readlink(path)}
    except OSError as exc:
        return {"error": type(exc).__name__ + ": " + str(exc)}


def names(path):
    try:
        return {"value": sorted(os.listdir(path))}
    except OSError as exc:
        return {"error": type(exc).__name__ + ": " + str(exc)}


started = now()
pci = "/sys/bus/pci/devices/0000:00:02.0"
pci_fields = [
    "vendor", "device", "revision", "subsystem_vendor", "subsystem_device",
    "class", "irq", "enable", "resource", "power_state", "d3cold_allowed",
    "power/control", "power/runtime_status", "power/runtime_active_time",
    "power/runtime_suspended_time", "power/runtime_usage", "power/wakeup",
    "power/autosuspend_delay_ms",
]
module = "/sys/module/gma500_gfx"
module_fields = ["initstate", "refcnt", "taint"]
fb = "/sys/class/graphics/fb0"
fb_fields = ["name", "virtual_size", "bits_per_pixel", "stride", "blank"]
recovery_fields = [
    "/proc/sys/kernel/sysrq", "/proc/sys/kernel/dmesg_restrict",
    "/proc/sys/kernel/panic", "/proc/sys/kernel/panic_on_oops",
]

modules_raw = text("/proc/modules")
module_lines = []
if "value" in modules_raw:
    module_lines = [
        line for line in modules_raw["value"].splitlines()
        if line.split()[0] in {"gma500_gfx", "drm", "drm_kms_helper", "ttm"}
    ]
irq_raw = text("/proc/interrupts")
irq_number = text(pci + "/irq").get("value")
irq_lines = []
if "value" in irq_raw and irq_number:
    irq_lines = [
        line for line in irq_raw["value"].splitlines()
        if line.lstrip().startswith(irq_number + ":")
    ]
mounts_raw = text("/proc/mounts")
pstore_mounts = []
if "value" in mounts_raw:
    pstore_mounts = [
        line for line in mounts_raw["value"].splitlines()
        if line.split()[1] == "/sys/fs/pstore"
    ]

drm_fd_owners = []
drm_fd_scan = {"processes_considered": 0, "inaccessible_fd_dirs": 0, "truncated": False}
for pid in sorted(int(p.name) for p in pathlib.Path("/proc").iterdir() if p.name.isdigit()):
    if drm_fd_scan["processes_considered"] >= 512:
        drm_fd_scan["truncated"] = True
        break
    drm_fd_scan["processes_considered"] += 1
    fd_dir = "/proc/%d/fd" % pid
    try:
        fds = os.listdir(fd_dir)
    except OSError:
        drm_fd_scan["inaccessible_fd_dirs"] += 1
        continue
    for fd in fds:
        try:
            target = os.readlink(fd_dir + "/" + fd)
        except OSError:
            continue
        if target.startswith("/dev/dri/"):
            drm_fd_owners.append({
                "pid": pid,
                "comm": text("/proc/%d/comm" % pid).get("value"),
                "fd": fd,
                "target": target,
            })

note = {}
try:
    data = pathlib.Path(module + "/notes/.note.gnu.build-id").read_bytes()
    note = {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest(), "hex": data.hex()}
except OSError as exc:
    note = {"error": type(exc).__name__ + ": " + str(exc)}

result = {
    "target_started_utc": started,
    "pci": {field: text(pci + "/" + field) for field in pci_fields},
    "pci_links": {"driver": link(pci + "/driver"), "driver_module": link(pci + "/driver/module")},
    "pci_drm_entries": names(pci + "/drm"),
    "module": {field: text(module + "/" + field) for field in module_fields},
    "module_holders": names(module + "/holders"),
    "module_loaded_build_id_note": note,
    "proc_modules_selected": module_lines,
    "irq_lines_for_pci_irq": irq_lines,
    "drm_class_entries": names("/sys/class/drm"),
    "drm_card0_device": link("/sys/class/drm/card0/device"),
    "framebuffer": {field: text(fb + "/" + field) for field in fb_fields},
    "proc_fb": text("/proc/fb"),
    "drm_fd_owners_visible_to_gama": drm_fd_owners,
    "drm_fd_scan": drm_fd_scan,
    "system_pm_capabilities": {"state": text("/sys/power/state"), "mem_sleep": text("/sys/power/mem_sleep")},
    "recovery": {path: text(path) for path in recovery_fields},
    "pstore_entries": names("/sys/fs/pstore"),
    "pstore_mounts": pstore_mounts,
    "watchdog_class_entries": names("/sys/class/watchdog"),
    "target_finished_utc": now(),
}
print(json.dumps(result, sort_keys=True))
