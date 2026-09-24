# Phase 7.0–7.5 adversarial review

I re-read the Phase 7 claims against the retained target-version files, the original TI Git objects where available, and the PSB/EMGD snapshots listed in [source-index.json](source-index.json). This review tests the scope of each claim; it does not make a source statement into a silicon guarantee.

| attempted falsification | result |
|---|---|
| Is the inspected gma500 the exact installed module? | No. All 71 retained antiX gma500 source blobs match the archived 5.10.240 tree, but the Dell's module/config and the surrounding installed kernel were not hashed. Source-based lifecycle conclusions stay scoped to that package. |
| Does BAR ownership make an ID read safe? | No. `psb_drv.c` creates/unmaps `sgx_reg` (P7-001), while power, clocks, reset and CPU read failure remain unbounded. FIRST-OBSERVATION-DESIGN stays BLOCKED. |
| Would `irqmask_lock` exclude existing SGX reads? | No. The 5.10.240 handler releases it before SGX status reads and acknowledgment (P7-003). The newer Linux handler cannot be substituted for this version. |
| Would `power_mutex` plus a display power claim exclude PM and 2D activity? | Not shown. The two locks protect different paths (P7-002); `lock_2d` and MMU semaphore have separate domains (P7-004/P7-005). No common protocol was recovered. |
| Does `TRAP_SGX_PM_FAULT` supply a safety check? | No. It is disabled in the inspected source and would still reach `ioread32` after warning (P7-006). Its APM field also lacks an applicable access contract. |
| Does a plausible SGX535 name or PCI `0x06` identify the physical revision? | No. P7-001 ties the Linux Poulsbo path to SGX535; TVZ supplies PCI identity. The physical identity is an inference, and revision/BRNs remain UNKNOWN. P7-008 selectors are build conditions. |
| Do `CORE_ID` and `CORE_REVISION` definitions establish RO/no side effects? | No. P7-007 defines fields and records historical reads only. No positive register-access contract was found. |
| Does runtime `active` or `gma_get_core_freq()` prove SGX clocks? | No. TVZ reports PM status; P6-001/P6-002 trace a PCI selector write and backlight PWM consumer. Neither supplies the SGX register-interface clock predicate. |
| Can the BIF discrepancy be fixed by renumbering? | Not from these sources. Linux/PSB context 1 setup computes `0xc3c`, DDK/EMGD list 1 computes `0xc38` (P7-004/P7-009/P7-015). Linux/PSB teardown computes `0xc88`, named `TWOD_REQ_BASE` in the SGX535 header (P7-014). The reasons and hardware effects remain UNKNOWN. |
| Does a shared page array prove an SGX BIF → GTT → RAM chain? | No. P7-016 shows separate GTT and SGX MMU insertion calls. A serial hardware path is unsupported. |
| Does the DDK init path provide a verified Poulsbo payload? | No. P7-018/P7-019 establish a Services init interface and kernel execution of supplied scripts. They do not recover the matching Poulsbo userspace producer, scripts or microkernel bytes. |
| Do OMAP ELF symbols identify usable Poulsbo firmware? | No. P7-021 is ARM/OMAP5 metadata. Symbol values are not demonstrated GPU addresses or a Poulsbo-compatible program. No payload was copied or executed. |
| Could a reset, reboot, watchdog or SysRq recover an invalid CPU read? | Not established. P7-005 is a 2D reset path; diagnostic facilities and historical watchdogs do not bound an interconnect stall (P7-006 and prior P4-017/P4-018). |

The source check also found an inherited citation error: the additional SCH datasheet row in [the Phase 4.1 source inventory](../phase4-1-data/README.md) referred to `P4-017`, which is the SysRq entry, not an Intel PCI-revision source. I replaced that label with the cited datasheet section. This fixes attribution without changing the PCI or SGX revision conclusion.

No source in this pass upgrades an UNKNOWN to a hardware PASS. No candidate enters the whitelist, no subphase gate is relaxed, and no new target OBSERVED fact is claimed. If any of the missing power, clock, reset, failure or exclusion conditions differs on the installed kernel, the proposed first-read safety argument fails before MMIO; the present gate correctly rejects it.
