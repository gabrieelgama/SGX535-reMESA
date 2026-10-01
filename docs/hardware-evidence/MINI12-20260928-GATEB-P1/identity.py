"""Remote, read-only identity preflight for the authorized Gate B passive pass."""

import datetime
import json
import pathlib
import platform


def read(path):
    return pathlib.Path(path).read_text().strip()


started = datetime.datetime.now(datetime.timezone.utc).isoformat()
cpu = next(
    line.split(":", 1)[1].strip()
    for line in pathlib.Path("/proc/cpuinfo").read_text().splitlines()
    if line.startswith("model name")
)
base = "/sys/bus/pci/devices/0000:00:02.0"
result = {
    "target_started_utc": started,
    "arch": platform.machine(),
    "sys_vendor": read("/sys/class/dmi/id/sys_vendor"),
    "product_name": read("/sys/class/dmi/id/product_name"),
    "board_name": read("/sys/class/dmi/id/board_name"),
    "bios_version": read("/sys/class/dmi/id/bios_version"),
    "cpu_model": cpu,
    "pci_vendor": read(base + "/vendor"),
    "pci_device": read(base + "/device"),
    "pci_revision": read(base + "/revision"),
    "pci_subsystem_vendor": read(base + "/subsystem_vendor"),
    "pci_subsystem_device": read(base + "/subsystem_device"),
    "target_finished_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
}
print(json.dumps(result, sort_keys=True))
