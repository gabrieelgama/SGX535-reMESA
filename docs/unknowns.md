# 20 priority unknowns — Phase 5 assessment

Updated 2026-09-20 after Test Vector Zero, the Phase 4.7 audit, and Phase 5 public-document research. No MMIO, PCI config write, reset, firmware, or workload was executed. IDs P3/P4/P5 refer to the
[matrix](evidence-matrix.csv).

| # | Unknown | Current evidence / exact lack | Blocks |
|---|---|---|---|
| U01 | What are the target identity and physical revisions? | TVZ-001 records PCI revision `0x06` and subsystem `1028:02b1`; graphics/SCH stepping and physical SGX core revision remain UNKNOWN | MMIO and later |
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

## Immediate blockers of the first MMIO read

The target's passive PCI identity is recorded, but its physical SGX revision is not. The immediate blockers are U02, U03/U04, U05/U06, and the revision/errata part of U11. They require an applicable SGX535/Poulsbo register contract, power/clock/reset requirements, a gma500-owned exclusion protocol, documented MMIO failure behavior, and recovery evidence.

## Changes since Phase 3

- gma500 ownership is now identified; this shows that a separate module is unsafe, but it does not provide a universal lock.
- `CORE_ID`/`CORE_REVISION` are no longer a “conditional candidate” and remain
formally **UNKNOWN** for read-safety.
- Test Vector Zero was implemented and does not depend on MMIO.
- GTT and stolen-memory values are left unavailable when the selected passive interface does not expose them.
- Firmware/ISA continue to be later blockers, not from passive inventory.

## Phase 5 evidence-acquisition result

- Intel document `364236` is **CONFIRMED** to be titled *Intel System Controller HUB External Design Specification* (P5-001). A 2021 Intel support response placed access behind a privileged Resource and Documentation Center account (P5-002); anonymous retrieval now ends at a public 404 page (P5-005).
- Public IMG material shows detailed SGX535 documentation being discussed through private developer support, but does not supply or authenticate a register-access contract (P5-003).
- No recovered public source establishes read-only or side-effect-free semantics, the required power/clock/reset state, a graphics PCI `0x06` to SGX revision mapping, applicable BRNs, CPU read failure behavior, or a complete recovery contract. Failure to find a warning is not evidence of safety.

## External artifact of greatest value

The most valuable item for the immediate blocker is an authorized register, power, reset, and errata manual for **SGX535 integrated into Poulsbo**. It must define `CORE_ID`/`CORE_REVISION` access attributes and side effects, the required power/clock/reset state, failure behavior, and revision coverage. For later bootstrap work, the matching UM/microkernel/initializer package for the Poulsbo DDK 1.14 remains the highest-value artifact.
