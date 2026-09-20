#!/usr/bin/env python3
"""Read-only inventory probe for Intel Poulsbo PCI devices.

This program deliberately has no MMIO, PCI configuration, DRM ioctl, firmware,
or command-submission path. It reads a fixed set of text attributes from sysfs
and procfs and uses uname(2).
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import re
import sys
from pathlib import Path
from typing import Any


TARGET_VENDOR = "0x8086"
TARGET_DEVICES = {"0x8108", "0x8109"}
BDF_RE = re.compile(r"^[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\.[0-7]$")
DRM_NODE_RE = re.compile(r"^(?:card|renderD|controlD)[0-9]+$")


class ProbeError(RuntimeError):
    pass


def read_text(path: Path, *, required: bool = False) -> str | None:
    try:
        return path.read_text(encoding="ascii").strip()
    except (FileNotFoundError, PermissionError, OSError, UnicodeError) as exc:
        if required:
            raise ProbeError(f"cannot read required passive attribute {path}: {exc}") from exc
        return None


def normalized_hex(value: str | None, width: int = 4) -> str | None:
    if value is None:
        return None
    try:
        return f"0x{int(value, 16):0{width}x}"
    except ValueError:
        return value.lower()


def resolve_link_name(path: Path) -> str | None:
    try:
        return path.resolve(strict=True).name
    except (FileNotFoundError, PermissionError, OSError):
        return None


def pci_resources(device_path: Path) -> list[dict[str, Any]]:
    raw = read_text(device_path / "resource")
    if raw is None:
        return []
    resources: list[dict[str, Any]] = []
    for index, line in enumerate(raw.splitlines()):
        fields = line.split()
        if len(fields) != 3:
            raise ProbeError(f"malformed PCI resource line {index}: {line!r}")
        try:
            start, end, flags = (int(field, 16) for field in fields)
        except ValueError as exc:
            raise ProbeError(f"malformed PCI resource line {index}: {line!r}") from exc
        if (start != 0 or end != 0) and end < start:
            raise ProbeError(f"invalid PCI resource range on line {index}: {line!r}")
        size = 0 if start == 0 and end == 0 else end - start + 1
        resources.append(
            {
                "index": index,
                "kind": f"BAR{index}" if index < 6 else "non-BAR PCI resource",
                "start": f"0x{start:016x}",
                "end": f"0x{end:016x}",
                "flags": f"0x{flags:016x}",
                "size_bytes": size,
            }
        )
    return resources


def drm_nodes(sys_root: Path, device_path: Path) -> list[dict[str, str | None]]:
    result: list[dict[str, str | None]] = []
    drm_class = sys_root / "class" / "drm"
    try:
        candidates = sorted(drm_class.iterdir(), key=lambda item: item.name)
    except (FileNotFoundError, PermissionError, OSError):
        return result
    try:
        canonical_device = device_path.resolve(strict=True)
    except (FileNotFoundError, PermissionError, OSError):
        return result
    for node in candidates:
        if not DRM_NODE_RE.fullmatch(node.name):
            continue
        try:
            node_device = (node / "device").resolve(strict=True)
        except (FileNotFoundError, PermissionError, OSError):
            continue
        if node_device != canonical_device:
            continue
        result.append({"name": node.name, "dev": read_text(node / "dev")})
    return result


def inspect_device(sys_root: Path, device_path: Path) -> dict[str, Any]:
    vendor = normalized_hex(read_text(device_path / "vendor", required=True))
    device = normalized_hex(read_text(device_path / "device", required=True))
    target_match = vendor == TARGET_VENDOR and device in TARGET_DEVICES
    power_path = device_path / "power"
    resources = pci_resources(device_path)
    return {
        "pci": {
            "address": device_path.name,
            "vendor": vendor,
            "device": device,
            "revision": normalized_hex(read_text(device_path / "revision"), 2),
            "subsystem_vendor": normalized_hex(read_text(device_path / "subsystem_vendor")),
            "subsystem_device": normalized_hex(read_text(device_path / "subsystem_device")),
            "class": normalized_hex(read_text(device_path / "class"), 6),
            "irq": read_text(device_path / "irq"),
            "resources": resources,
        },
        "platform": {
            "target_match": target_match,
            "detected": "Intel Poulsbo PCI function" if target_match else "unsupported PCI identity",
            "confidence": "CONFIRMED" if target_match else "UNKNOWN",
            "sgx535_identity": "INFERRED" if target_match else "UNKNOWN",
            "sgx535_basis": (
                "Exact PCI IDs are named Poulsbo by the audited Linux source; that table does not identify the SGX core revision."
                if target_match
                else "No SGX-specific conclusion is permitted for this PCI identity."
            ),
        },
        "kernel_binding": {
            "driver": resolve_link_name(device_path / "driver"),
            "drm_nodes": drm_nodes(sys_root, device_path),
        },
        "memory": {
            "pci_resources_available": bool(resources),
            "gtt": None,
            "stolen_memory": None,
            "note": "No audited passive gma500 sysfs interface exposes derived GTT or stolen-memory sizes.",
        },
        "power": {
            "pci_power_state": read_text(device_path / "power_state"),
            "runtime_status": read_text(power_path / "runtime_status"),
            "runtime_active_time_ms": read_text(power_path / "runtime_active_time"),
            "runtime_suspended_time_ms": read_text(power_path / "runtime_suspended_time"),
            "note": "PCI runtime status is not evidence that every SGX clock or internal power domain is enabled.",
        },
    }


def select_device(sys_root: Path, requested_bdf: str | None) -> Path:
    devices_root = sys_root / "bus" / "pci" / "devices"
    if requested_bdf is not None:
        if not BDF_RE.fullmatch(requested_bdf):
            raise ProbeError(f"invalid PCI address: {requested_bdf!r}")
        candidate = devices_root / requested_bdf.lower()
        if not candidate.exists():
            candidate = devices_root / requested_bdf
        if not candidate.exists():
            raise ProbeError(f"PCI device not found: {requested_bdf}")
        vendor = normalized_hex(read_text(candidate / "vendor", required=True))
        device = normalized_hex(read_text(candidate / "device", required=True))
        if vendor != TARGET_VENDOR or device not in TARGET_DEVICES:
            raise ProbeError(
                f"refusing SGX535-specific identification for unsupported PCI identity {vendor}:{device}"
            )
        return candidate

    try:
        entries = sorted(devices_root.iterdir(), key=lambda item: item.name)
    except (FileNotFoundError, PermissionError, OSError) as exc:
        raise ProbeError(f"cannot enumerate PCI sysfs devices: {exc}") from exc
    matches = []
    for candidate in entries:
        vendor = normalized_hex(read_text(candidate / "vendor"))
        device = normalized_hex(read_text(candidate / "device"))
        if vendor == TARGET_VENDOR and device in TARGET_DEVICES:
            matches.append(candidate)
    if not matches:
        raise ProbeError("no exact Intel 8086:8108 or 8086:8109 target found")
    if len(matches) > 1:
        addresses = ", ".join(path.name for path in matches)
        raise ProbeError(f"multiple exact targets found ({addresses}); select one with --pci-address")
    return matches[0]


def build_report(sys_root: Path, proc_root: Path, requested_bdf: str | None) -> dict[str, Any]:
    device_path = select_device(sys_root, requested_bdf)
    device = inspect_device(sys_root, device_path)
    dmi_path = sys_root / "class" / "dmi" / "id"
    return {
        "schema": "sgx535-gfx-passive-probe-v1",
        "title": "SGX535-GFX passive probe",
        "host": {
            "kernel_release": read_text(proc_root / "sys" / "kernel" / "osrelease") or platform.release(),
            "kernel_version": read_text(proc_root / "version"),
            "cpu_architecture": os.uname().machine,
            "dmi": {
                "sys_vendor": read_text(dmi_path / "sys_vendor"),
                "product_name": read_text(dmi_path / "product_name"),
                "product_version": read_text(dmi_path / "product_version"),
                "board_vendor": read_text(dmi_path / "board_vendor"),
                "board_name": read_text(dmi_path / "board_name"),
                "board_version": read_text(dmi_path / "board_version"),
                "bios_vendor": read_text(dmi_path / "bios_vendor"),
                "bios_version": read_text(dmi_path / "bios_version"),
                "bios_date": read_text(dmi_path / "bios_date"),
            },
        },
        **device,
        "safety": {
            "interfaces_used": ["sysfs text attributes", "procfs text attributes", "uname"],
            "pci_config_reads": False,
            "pci_config_writes": False,
            "bar_mappings": False,
            "mmio_reads": False,
            "mmio_writes": False,
            "drm_device_opened": False,
            "runtime_pm_changed": False,
            "reset": False,
            "command_submission": False,
            "firmware_loading": False,
            "pds_or_usse_execution": False,
            "state_modified_by_probe": False,
            "scope_note": "A bound kernel driver may have changed hardware state before this probe runs.",
        },
    }


def human_report(report: dict[str, Any]) -> str:
    pci = report["pci"]
    binding = report["kernel_binding"]
    resources = pci["resources"]
    resource_lines = [
        f"    {item['kind']}: {item['start']}-{item['end']} size={item['size_bytes']} flags={item['flags']}"
        for item in resources
    ] or ["    unavailable"]
    nodes = binding["drm_nodes"]
    dmi = report["host"]["dmi"]
    node_text = ", ".join(f"{node['name']} ({node['dev'] or 'dev unknown'})" for node in nodes) or "none found"
    return "\n".join(
        [
            report["title"],
            "",
            "PCI:",
            f"  address: {pci['address']}",
            f"  vendor: {pci['vendor']}",
            f"  device: {pci['device']}",
            f"  revision: {pci['revision'] or 'unavailable'}",
            f"  subsystem: {pci['subsystem_vendor'] or 'unavailable'}:{pci['subsystem_device'] or 'unavailable'}",
            f"  class: {pci['class'] or 'unavailable'}",
            f"  IRQ: {pci['irq'] or 'unavailable'}",
            "  resources:",
            *resource_lines,
            "",
            "Platform:",
            f"  detected: {report['platform']['detected']}",
            f"  PCI identity confidence: {report['platform']['confidence']}",
            f"  SGX535 identity: {report['platform']['sgx535_identity']}",
            "",
            "Kernel:",
            f"  release: {report['host']['kernel_release'] or 'unavailable'}",
            f"  architecture: {report['host']['cpu_architecture']}",
            f"  system: {dmi['sys_vendor'] or 'unavailable'} {dmi['product_name'] or 'unavailable'}",
            f"  board: {dmi['board_vendor'] or 'unavailable'} {dmi['board_name'] or 'unavailable'}",
            f"  BIOS: {dmi['bios_vendor'] or 'unavailable'} {dmi['bios_version'] or 'unavailable'}",
            f"  driver: {binding['driver'] or 'unbound'}",
            f"  DRM nodes: {node_text}",
            "",
            "Memory (never change:",
            "  GTT: not exposed by selected passive interface",
            "  stolen memory: not exposed by selected passive interface",
            "",
            "Power:",
            f"  PCI power state: {report['power']['pci_power_state'] or 'unavailable'}",
            f"  runtime status: {report['power']['runtime_status'] or 'unavailable'}",
            "  interpretation: PCI runtime status does not prove SGX clocks are enabled",
            "",
            "Safety (never change):",
            "  MMIO reads/writes: disabled",
            "  command submission: disabled",
            "  firmware loading: disabled",
            "  state modified by probe: NO",
            "  caveat: a bound kernel driver may already have changed hardware state",
        ]
    )


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Passive Intel Poulsbo inventory probe")
    parser.add_argument("--json", action="store_true", help="emit JSON instead of human-readable text")
    parser.add_argument("--pci-address", metavar="DOMAIN:BUS:SLOT.FUNC", help="select one exact PCI function")
    parser.add_argument("--sys-root", type=Path, default=Path("/sys"), help=argparse.SUPPRESS)
    parser.add_argument("--proc-root", type=Path, default=Path("/proc"), help=argparse.SUPPRESS)
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    try:
        report = build_report(args.sys_root, args.proc_root, args.pci_address)
    except ProbeError as exc:
        print(f"sgx535-probe: {exc}", file=sys.stderr)
        return 2
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        print(human_report(report))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
