#!/usr/bin/env python3
"""Tests use a synthetic sysfs/procfs tree and never inspect hardware."""

from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

import sgx535_probe


RESOURCE = "\n".join(
    [
        "0x00000000d0000000 0x00000000d007ffff 0x0000000000040200",
        "0x0000000000000000 0x0000000000000000 0x0000000000000000",
        "0x00000000c0000000 0x00000000cfffffff 0x0000000000040200",
        "0x00000000d0080000 0x00000000d00fffff 0x0000000000040200",
    ]
)


class ProbeFixture(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.sys_root = self.root / "sys"
        self.proc_root = self.root / "proc"
        self.device = self.sys_root / "devices" / "pci0000:00" / "0000:00:02.0"
        self.device.mkdir(parents=True)
        attributes = {
            "vendor": "0x8086\n",
            "device": "0x8108\n",
            "revision": "0x07\n",
            "subsystem_vendor": "0x1028\n",
            "subsystem_device": "0x02aa\n",
            "class": "0x030000\n",
            "irq": "16\n",
            "resource": RESOURCE + "\n",
            "power_state": "D0\n",
        }
        for name, value in attributes.items():
            (self.device / name).write_text(value, encoding="ascii")
        power = self.device / "power"
        power.mkdir()
        (power / "runtime_status").write_text("active\n", encoding="ascii")
        pci_devices = self.sys_root / "bus" / "pci" / "devices"
        pci_devices.mkdir(parents=True)
        (pci_devices / "0000:00:02.0").symlink_to(self.device)
        driver = self.sys_root / "bus" / "pci" / "drivers" / "gma500"
        driver.mkdir(parents=True)
        (self.device / "driver").symlink_to(driver)
        drm = self.sys_root / "class" / "drm" / "card0"
        drm.mkdir(parents=True)
        (drm / "device").symlink_to(self.device)
        (drm / "dev").write_text("226:0\n", encoding="ascii")
        kernel = self.proc_root / "sys" / "kernel"
        kernel.mkdir(parents=True)
        (kernel / "osrelease").write_text("fixture-kernel\n", encoding="ascii")
        (self.proc_root / "version").write_text("fixture version\n", encoding="ascii")
        dmi = self.sys_root / "class" / "dmi" / "id"
        dmi.mkdir(parents=True)
        (dmi / "sys_vendor").write_text("Fixture Vendor\n", encoding="ascii")
        (dmi / "product_name").write_text("Fixture Product\n", encoding="ascii")

    def tearDown(self) -> None:
        self.temp.cleanup()

    def test_exact_target_report(self) -> None:
        report = sgx535_probe.build_report(self.sys_root, self.proc_root, None)
        self.assertTrue(report["platform"]["target_match"])
        self.assertEqual(report["kernel_binding"]["driver"], "gma500")
        self.assertEqual(report["kernel_binding"]["drm_nodes"][0]["name"], "card0")
        self.assertEqual(report["host"]["dmi"]["product_name"], "Fixture Product")
        self.assertEqual(report["pci"]["resources"][0]["size_bytes"], 0x80000)
        self.assertEqual(report["power"]["pci_power_state"], "D0")
        self.assertFalse(report["safety"]["mmio_reads"])
        self.assertFalse(report["safety"]["state_modified_by_probe"])

    def test_unknown_device_fails_closed(self) -> None:
        (self.device / "device").write_text("0x1234\n", encoding="ascii")
        with self.assertRaises(sgx535_probe.ProbeError):
            sgx535_probe.build_report(self.sys_root, self.proc_root, "0000:00:02.0")

    def test_malformed_resource_fails_closed(self) -> None:
        (self.device / "resource").write_text(
            "0x0000000000000020 0x0000000000000010 0x0000000000040200\n",
            encoding="ascii",
        )
        with self.assertRaises(sgx535_probe.ProbeError):
            sgx535_probe.build_report(self.sys_root, self.proc_root, None)

    def test_json_cli(self) -> None:
        result = subprocess.run(
            [
                sys.executable,
                str(Path(sgx535_probe.__file__)),
                "--json",
                "--sys-root",
                str(self.sys_root),
                "--proc-root",
                str(self.proc_root),
            ],
            check=True,
            capture_output=True,
            text=True,
        )
        parsed = json.loads(result.stdout)
        self.assertEqual(parsed["schema"], "sgx535-gfx-passive-probe-v1")


if __name__ == "__main__":
    unittest.main()
