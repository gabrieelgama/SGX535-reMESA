# PCI revision `0x06` — audit

The real TVZ measured PCI revision `0x06` for graphics function `8086:8108` at `0000:00:02.0` (P41-002). This is a PCI revision ID, not an SGX core revision.

A separate SCH table associates RID `06h` with D1 on LPC `D31:F0`; that evidence does not transfer to graphics function `D2:F0`. EMGD and gma500 read PCI revision fields, but neither source maps `0x06` to an SGX revision, BRN set, or graphics stepping.

Conclusion: PCI revision `0x06` was not correlated to D1, rev116, rev121, or any other SGX core revision.
