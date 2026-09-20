# 20 priority unknowns — Phase 4 assessment

Phase 4 — static audit, 2026-09-17. No MMIO, PCI config, reset,
firmware or workload was executed. IDs P3/P4 refer to
[matrix](evidence-matrix.csv).

| # | Unknown | Current evidence / exact lack | Blocks |
|---|---|---|---|
| U01 | What is revision/subsystem/stepping of the actual machine? | Target IDs confirmed; there is no Inspiron report yet | MMIO and later |
| U02 | `CORE_ID`/`CORE_REVISION` are safe to read in real stepping? | confirmed historical use, read-side-effect contract absent | first MMIO |
| U03 | Which PM state guarantees valid SGX clocks? | runtime PM PCI does not equal internal clock (P4-004/P4-012) | any MMIO |
| U04 | Which registers are readable when SGX is gated/off? | no applicable source defines this | any MMIO |
| U05 | Which lock serializes generic SGX reading? | IRQ/MMU/GTT locks exist; no global SGX lock | Concurrent MMIO |
| U06 | How to avoid race condition with IRQ and KMS? | IRQ aggregates display/SGX and uses `irqmask_lock` (P4-010) | status/IRQ reads |
| U07 | Why is the aperture `0x8000` on Linux and `0x4000` on DDK? | both confirmed, unknown review/configuration | MMIO range |
| U08 | Which directory-list formula applies to each context? | Linux/PSB and TI/EMGD diverge (P3-026/P3-061) | MMU/contexts |
| U09 | Why do init/remove mix `gatt_start` and `mmu_gatt_start`? | internal divergence confirmed | MMU/memory |
| U10 | Complete sequence of clock/power/reset by review? | gma500 has incomplete PM; historical OSPM missing | reset/init |
| U11 | Which errata/BRNs are valid in real silicon? | builds rev121/126 do not measure the board | reset/BIF/workload |
| U12 | Physical relationship and coherence between GTT, GATT, stolen and BIF? | partially confirmed mechanisms, end-to-end contract absent | address space |
| U13 | Semantics of fault/status: latch, clear, ordering, and ack? | handler shows practice, not contract | observation BIF/IRQ |
| U14 | Is there a platform watchdog and trusted SGX recovery? | no gma500 mechanism located; platform state unknown | active operations |
| U15 | Complete init/deinit Scripts from DDK Poulsbo? | kernel consumes interfaces; corresponding payload/UM missing | bootstrap |
| U16 | Firmware/microkernel SGX535 verifiable and licensable? | compatible payload not available | firmware/CCB |
| U17 | Exact PDS program and bootstrap protocol? | bases/kick do not define program | PDS/firmware |
| U18 | ABI CCB, sync, relocations and full cache? | partial and distinct historical interfaces | submission |
| U19 | ISA/encoder USSE and PDS SGX535 with origin? | absent; SGX540/544 does not replace | own execution |
| U20 | Streams TA/3D, DPM, tiling, formats/PBE and modern isolation? | insufficient in current sources | workload/render/Mesa |

## Immediate blockers of the first MMIO

The five largest are U01, U02, U03/U04, U05/U06, and U07/U11. In terms of
artifacts: passive measurement of the board; register reference/errata SGX535 Poulsbo;
power/clock authenticated sequence; full host code with locking/OSPM; and
documentation that links the PCI/SGX review to aperture and the BRNs.

## Changes since Phase 3

- gma500 ownership is now identified; this shows that a separate module is unsafe, but it does not provide a universal lock.
- `CORE_ID`/`CORE_REVISION` are no longer a “conditional candidate” and remain
formally **UNKNOWN** for read-safety.
- Test Vector Zero was implemented and does not depend on MMIO.
- GTT and stolen-memory values are left unavailable when the selected passive interface does not expose them.
- Firmware/ISA continue to be later blockers, not from passive inventory.

## External artifact of greatest value

The most valuable item for the **next immediate blocker** is a register, power, reset, and errata manual authenticated for **SGX535 integrated into
Poulsbo**, linking PCI revision to SGX core revision, aperture, and
`CORE_ID`/`CORE_REVISION` read behavior, clocks, and ownership requirements. For the
later bootstrap, the matching UM/microkernel/initializer package for the
DDK Poulsbo 1.14 continues to be the highest value artifact.
