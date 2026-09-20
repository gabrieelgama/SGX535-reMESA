# TVZ-001 — Test Vector Zero hardware report

This directory preserves the hardware report supplied after Test Vector Zero ran on the Dell Inspiron 1210. The report records PCI identity, BARs, IRQ, kernel, driver, DRM node, and runtime PM status.

The report is primary evidence of the measured values as supplied. The original stdout/JSON capture and an independently reproduced exit status were not provided. TVZ performed no MMIO read or write, reset, firmware load, MMU initialization, command submission, or power/clock manipulation.

PCI revision `0x06` is a PCI revision ID. It is not an SGX core revision.
