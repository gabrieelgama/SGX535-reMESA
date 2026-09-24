# Phase 7.0–7.5 result

The immediate starting point was committed Phase 6.2 (`67fe07b`): Gate A GO, Gate B BLOCKED, RE-GATE RE-PREPARE, CORE_ID/CORE_REVISION SAFE-CANDIDATE NO, whitelist `[]`, and no Test Vector One design or execution. The source-only Phase 7 review did not change that boundary.

| subphase | documentary result | experimental gate |
|---|---|---|
| 7.0 | ownership, lifecycle, preconditions and failure boundary reviewed | **FIRST-OBSERVATION-DESIGN BLOCKED** ([gate](7.0-gate.md)) |
| 7.1 | conditional branch recorded | **FIRST-OBSERVATION BLOCKED**; no observation authorized or made ([log](7.1-first-observation.md)) |
| 7.2 | revision namespaces and conditional BRNs mapped | physical revision UNKNOWN; BRN applicability UNKNOWN ([revision map](7.2-revision-errata.md)) |
| 7.3 | software state-transition model reconstructed | POWER/CLOCK/RESET contracts all **INCOMPLETE** ([model](7.3-power-clock-reset.md)) |
| 7.4 | BIF/MMU/address domains and source divergences preserved | ADDRESS-SPACE-CONTRACT **INCOMPLETE**; MMU/BIF writes **NO** ([analysis](7.4-address-space.md)) |
| 7.5 | historical Services init split and available artifact metadata classified | FIRMWARE/MICROKERNEL contracts **INCOMPLETE**; execution/submission **NO** ([analysis](7.5-firmware-microkernel.md)) |

The documentary tasks for 7.0 and 7.2–7.5 are complete for the inspected sources. Their experimental or hardware-contract gates remain blocked or incomplete. Phase 7.1 did not execute. Phase 7.6 was neither designed nor started.

## Identity and the first read

The historical SGX535 header defines CORE_ID `0x0010` and CORE_REVISION `0x0014` with 32-bit fields (P7-007). Historical PSB reads them; neither fact supplies an applicable RO/no-side-effect contract. Both candidates remain **SAFE-CANDIDATE NO**; whitelist `[]`, Gate B **BLOCKED**. No read of either register was authorized, and no returned value exists. The target's SGX535 identity is **INFERRED** from the recorded PCI ID and Linux's Poulsbo driver table (TVZ-001, P7-001). The physical core revision is **UNKNOWN**. PCI revision `0x06`, Intel stepping, DDK `SGX_CORE_REV`, selected driver build and BRN applicability remain separate; the latter two physical mappings were not established (P7-008).

## State, address space and initialization

Target-version gma500 owns `sgx_reg`, but its distinct PM, IRQ, 2D and MMU paths do not establish one safe observation lock or exclusion point (P7-001–P7-005). The running module's correspondence to the retained antiX source also remains UNVERIFIED. No applicable source proves SGX power/clock/reset accessibility, CPU completion when unavailable, or recovery from a failed read (P7-002/P7-006). The historical `TRAP_SGX_PM_FAULT` branch is disabled and proceeds to the read after warning; it is not a guard.

For directory context 1, target Linux and historical PSB setup compute `0xc3c`; DDK/EMGD list 1 initialization computes `0xc38` (P7-004/P7-009/P7-015). Linux/PSB teardown uses another expression that computes `0xc88`, which the SGX535 header names `TWOD_REQ_BASE` (P7-014). Numbering, reservation, revision or source defect remain possible explanations, each **UNKNOWN**. GTT and SGX MMU receive parallel mappings of a page array in software (P7-016); `SGX BIF → GTT → RAM` is unsupported. No MMU, GTT or BIF state was changed.

The historical kernel Services interface accepts userspace supplied scripts, memory handles and init data, then performs reset/startup and waits for a microkernel completion indication (P7-018/P7-019). The matching Poulsbo userspace initializer, scripts and payload were not recovered. The inspected `libsrv_init` binary belongs to the OMAP5/DRA7xx package (P7-021); its ARM ELF symbols neither identify Poulsbo firmware nor supply GPU addresses. Firmware and microkernel contracts are **INCOMPLETE**. No firmware was loaded or executed, no SGX MMIO write occurred, no CCB/command was submitted, and no GPU workload ran.

## Experimental record and remaining gates

This run created **zero new OBSERVED or REPRODUCED target-hardware facts**. The inherited TVZ-001 facts are the only machine measurements used. The current machine's PM state was not rechecked. No BAR mapping, `/dev/mem`, PCI write, active device read, clock/power transition, reset, MMU/GTT/BIF change, firmware, microkernel, PDS, USSE, command, workload or rendering operation occurred. **Hardware state modified by Phase 7: NO.** Test Vector One remains not designed and not executed.

Major remaining UNKNOWNs are tracked under the existing [U01–U20 register](../unknowns.md): physical SGX revision and BRNs; positive RO/read-side-effect evidence; island/clock/reset and pre-init access requirements; installed-kernel correspondence; PM/IRQ/2D/removal exclusion; CPU failure behavior and applicable recovery; directory selector rationale and address/coherence contract; matching Poulsbo initializer/payload and licenses. No UNKNOWN was closed merely because a source search was negative.

Before **any** first read could be considered, an applicable SGX535/Poulsbo source or separately reviewed defensible experiment design must establish the exact register semantics, revision/errata coverage, verified power/clock/reset state, installed owner/lifetime and exclusion protocol, bounded CPU failure and recovery. Those are the dependencies of the blocked [7.0 gate](7.0-gate.md); this report is not an authorization to run an experiment. Phase 7.6 **may not be designed or executed** on the present firmware/memory/command evidence. Its own later gate would additionally need a compatible Poulsbo userspace/initialization artifact, payload provenance, memory/firmware contract and separate safety review. No work from 7.6 began here.

The highest-value next reverse-engineering target is the **first-read access contract on the real Poulsbo revision**: an authorized Intel/Imagination SGX535-on-Poulsbo register and power/clock/reset/errata document, or equivalent attributable evidence that also bounds failed CPU reads and recovery. For later 7.5 closure, a legally obtained, version-matched Poulsbo userspace initializer and its build/provenance metadata would be the next artifact. Until the first contract is established, FIRST-OBSERVATION-DESIGN and Gate B remain BLOCKED.

Source hashes, commits and line ranges are in [source provenance](source-provenance.md) and [the evidence matrix](../evidence-matrix.csv). The independent [red-team review](phase7-0-5-red-team.md) records the attempted falsifications. The Git commit containing this result is recorded in the commit history; this document intentionally contains no self-referential hash.
