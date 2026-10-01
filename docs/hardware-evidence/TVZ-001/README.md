# TVZ-001 — Test Vector Zero hardware report

This directory preserves the hardware report supplied after Test Vector Zero ran on the Dell Inspiron 1210. The report records PCI identity, BARs, IRQ, kernel, driver, DRM node, and runtime PM status.

The report is primary evidence of the measured values as supplied. The original stdout/JSON capture and an independently reproduced exit status were not provided. TVZ performed no MMIO read or write, reset, firmware load, MMU initialization, command submission, or power/clock manipulation.

PCI revision `0x06` is a PCI revision ID. It is not an SGX core revision.

Integrity note (2026-09-27): `manifest.json` identifies the original 733-byte
report. Commit `61d41b1291ff78e73db086f1c893078e4a4f71e1` added only the
31-byte Portuguese-language notice; the current report is 764 bytes, SHA-256
`45c167c1980f05dc40817f972bc9841998a1e6182a0bc59a3f85999b6e10f4d5`.
The original manifest remains unchanged. The
[reconciliation and reproduction command](../../phase7/psb-dri-re/pds-target-provider-identity.md)
verify both versions without changing the recorded observations.
