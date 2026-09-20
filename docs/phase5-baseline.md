# Phase 5 baseline

Date: 2026-09-20

Phase 4 repository commit: `da58d69` (`docs: complete phase 4 documentation and English style pass`)

English-audited handoff commit: `f6421c7` (`docs: complete English language audit`)

This baseline records the repository state before Phase 5 research. It derives the decision from the checked-in Phase 4 artifacts rather than from conversation history.

## Final Phase 4 handoff

The following files agree on the operative decision:

- [Phase 4.7 final audit](phase4-7-final-pre-bringup-audit.md)
- [Phase 4.6 skipped record](phase4-6-skipped.md)
- [Phase 4.5 formal gate](phase4-5-formal-gate.md)
- [Test Vector One gate](test-vector-one-gate.md)
- [Phase 4 GO/NO-GO matrix](phase4-go-no-go.md)

| item | final Phase 4 state |
|---|---|
| Gate A — passive probe | `GO` |
| Gate B — controlled MMIO read | `BLOCKED` |
| `CORE_ID` | not a `SAFE-CANDIDATE` |
| `CORE_REVISION` | not a `SAFE-CANDIDATE` |
| whitelist | `[]` |
| Test Vector One design | skipped |
| Test Vector One execution | not performed |
| active hardware operation during Phase 4.3–4.7 | none |

Phase 5 therefore starts on Branch A: evidence acquisition only. Phase 4 did not authorize an MMIO read.

## Unresolved first-read requirements

Requirements 1–3 and 10–11 have documentary PASS results for both candidate registers: software identity, SGX-relative offset, 32-bit width, gma500 ownership, and mapping lifetime. Those results do not establish physical read safety.

| Phase 4 requirement | state | evidence required to change the state |
|---|---|---|
| 4. Read-only semantics | `UNKNOWN` | SGX535/Poulsbo register access attribute or equivalent primary contract |
| 5. No read side effects | `UNKNOWN` | explicit side-effect-free read semantics covering the applicable core/revision |
| 6. Valid power state | `UNKNOWN` | minimum SGX/register-interface power state and a reliable way to establish it |
| 7. Valid clock state | `UNKNOWN` | required clock domains and a reliable way to establish them without unsafe writes |
| 8. Reset-state requirement | `UNKNOWN` | documented accessibility while reset is asserted/deasserted and any ordering requirement |
| 9. Revision coverage | `UNKNOWN` | physical SGX revision or a contract valid across every plausible Poulsbo SGX535 revision |
| 12. Locking and races | `UNKNOWN` | a gma500-owned execution point and exclusion protocol covering IRQ, KMS, PM, MMU, and removal |
| 13. Suspend/resume interaction | `UNKNOWN` | serialization that prevents a read from racing D-state and mapping-lifetime transitions |
| 14. Failure behavior | `UNKNOWN` | documented CPU/MMIO behavior for an unavailable, gated, reset, or faulted SGX block |
| 15. Recovery | `UNKNOWN` | demonstrated recovery for the documented failure modes; reboot alone is not a read-transaction contract |

Phase 5 adds errata as a separate safety field. PCI revision `0x06` does not select an SGX revision or BRN set.

## Preserved identity boundaries

- Measured graphics PCI revision: `0x06` for `8086:8108` at `0000:00:02.0`.
- Intel platform/SCH stepping: `UNKNOWN` for the graphics function.
- Physical SGX core revision: `UNKNOWN`.
- Historical DDK default `SGXCORE_REV=121`: build configuration, not a measurement.
- Historical rev116 reference: software/proposal evidence, not a measurement.
- Applicable SGX BRNs: `UNKNOWN`.

## Current ownership and environment limits

The measured target reports kernel `5.10.240-antix.1-486-smp`, i686, driver `gma500`, DRM node `card0`, IRQ 16, and runtime status `active`. The published antiX source package was compared with upstream source, but byte-for-byte identity with the running kernel/module remains `UNVERIFIED`.

gma500 owns `dev_priv->sgx_reg` and the PCI function. The available source does not define a universal SGX lock. `irqmask_lock` does not by itself serialize every SGX access. Runtime status `active` does not prove SGX power, clocks, or reset state.

## Operations permitted in Phase 5

- inspect local repositories and preserved evidence;
- validate files, links, hashes, commits, and source line references;
- search public, legally accessible documentation and archives;
- analyze the existing passive Test Vector Zero report;
- update documentary evidence and gates.

## Operations prohibited in Phase 5

No MMIO access, PCI configuration write, `resourceN` mapping, `/dev/mem`, power or clock change, reset, MMU/GTT modification, SGX address-space creation, firmware or microkernel load, CCB allocation/submission, `EVENT_KICK`, PDS, USSE, workload, rendering, Mesa implementation, gma500 unbind/rebind or unload/reload, second PCI owner, or Test Vector One execution is permitted.

## Evidence-index limitation found at handoff

The checked-in [evidence matrix](evidence-matrix.csv) contains 154 data rows and uses `P3-*` and `P4-*` identifiers. Some later Phase 4 prose refers to `P41-*`, `P42-*`, `P43-*`, and `P45-*` labels that are not present in that matrix. Phase 5 will not silently treat those labels as matrix-backed evidence. It will cite their underlying checked-in artifacts directly or add new `P5-*` rows with complete provenance.
