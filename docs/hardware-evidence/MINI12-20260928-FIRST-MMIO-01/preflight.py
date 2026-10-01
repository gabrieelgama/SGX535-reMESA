"""Target preflight: ordinary text/stat only; never open an MMIO resource."""

import datetime
import hashlib
import json
import os
import pathlib
import platform
import shutil


def utc():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def read(path):
    try:
        return {"value": pathlib.Path(path).read_text().strip()}
    except (OSError, UnicodeError) as exc:
        return {"error": type(exc).__name__ + ": " + str(exc)}


def link(path):
    try:
        return {"value": os.readlink(path)}
    except OSError as exc:
        return {"error": type(exc).__name__ + ": " + str(exc)}


def stat(path):
    try:
        s = os.stat(path)
        return {"mode": oct(s.st_mode & 0o777), "uid": s.st_uid, "gid": s.st_gid,
                "size": s.st_size, "read_access": os.access(path, os.R_OK)}
    except OSError as exc:
        return {"error": type(exc).__name__ + ": " + str(exc)}


started = utc()
pci = "/sys/bus/pci/devices/0000:00:02.0"
cpu = next((line.split(":", 1)[1].strip()
            for line in pathlib.Path("/proc/cpuinfo").read_text().splitlines()
            if line.startswith("model name")), None)
note = {}
try:
    data = pathlib.Path("/sys/module/gma500_gfx/notes/.note.gnu.build-id").read_bytes()
    note = {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest(), "hex": data.hex()}
except OSError as exc:
    note = {"error": type(exc).__name__ + ": " + str(exc)}
result = {
    "target_started_utc": started,
    "identity": {
        "arch": platform.machine(), "cpu_model": cpu,
        "sys_vendor": read("/sys/class/dmi/id/sys_vendor"),
        "product_name": read("/sys/class/dmi/id/product_name"),
        "board_name": read("/sys/class/dmi/id/board_name"),
        "bios_version": read("/sys/class/dmi/id/bios_version"),
        "pci_vendor": read(pci + "/vendor"), "pci_device": read(pci + "/device"),
        "pci_revision": read(pci + "/revision"),
        "pci_subsystem_vendor": read(pci + "/subsystem_vendor"),
        "pci_subsystem_device": read(pci + "/subsystem_device"),
    },
    "pre_state": {
        "pci_resource_text": read(pci + "/resource"),
        "pci_driver": link(pci + "/driver"),
        "pci_enable": read(pci + "/enable"),
        "pci_runtime_status": read(pci + "/power/runtime_status"),
        "pci_power_control": read(pci + "/power/control"),
        "module_initstate": read("/sys/module/gma500_gfx/initstate"),
        "module_build_id_note": note,
        "proc_interrupts": [line for line in pathlib.Path("/proc/interrupts").read_text().splitlines()
                            if line.lstrip().startswith("16:")],
        "proc_fb": read("/proc/fb"),
        "sysrq": read("/proc/sys/kernel/sysrq"),
    },
    "mechanism_availability": {
        "euid": os.geteuid(), "groups": os.getgroups(),
        "resource0_stat_only": stat(pci + "/resource0"),
        "dev_mem_stat_only": stat("/dev/mem"),
        "cc": shutil.which("cc"), "gcc": shutil.which("gcc"),
        "objdump": shutil.which("objdump"),
    },
    "target_finished_utc": utc(),
}
print(json.dumps(result, sort_keys=True))
