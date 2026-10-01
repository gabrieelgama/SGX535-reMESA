# First triangle: offline construction checkpoint

Phase 7 remains **COMPLETE WITH UNRESOLVED EVIDENCE BOUNDARY**. This checkpoint adds an offline integration report; it does not amend the [authoritative Phase 7 handoff](../phase7/psb-dri-re/CODEX-HANDOFF.md). The target is the Dell Inspiron Mini 12 / Poulsbo / SGX535 rev121 (observed CORE_ID `0x01130000`, CORE_REVISION `0x00010201`). No target was contacted for this checkpoint.

**Later update:** the project-checker rev121 rejection described below was resolved in the [blocker burn-down](first-triangle-blocker-burndown.md). This file preserves the earlier checkpoint and its dry-run hash. Use the [post-Attempt-03 audit](post-attempt03-checkpoint-audit.md) for current blockers and authorization status.

## Dependency state for one frozen triangle

| Dependency | Status | Boundary |
| --- | --- | --- |
| Primary PDS bytes, 12-word data image, secondary PDS word | IMPLEMENTED_AND_VERIFIED in the host model | Primary program `0x07000345, 0xaf000000` remains architecturally opaque. |
| Linked fragment USE, vertex/state USE, vertices, indices, TA stream, target descriptor | IMPLEMENTED_AND_VERIFIED as CPU images | Auxiliary PDS source coverage remains `FT-AUX`/R3. |
| 10-BO grouping, alignment, 49 wire records, six validation nodes, CPU launch and 144-byte command packing | IMPLEMENTED_AND_VERIFIED with synthetic inputs | Actual handles, addresses, service context, and GPU contents are not supplied. |
| Target GPU address provider, mapping and relocation on validated device BOs | PARTIAL | Offline resolver rejects invalid assignments; no target-qualified live provider exists. |
| CPU-to-GPU publication, synchronization, residency, lifetime and ownership | BLOCKED_BY_EVIDENCE | R1 device visibility postcondition and live provider enforcement are unproved. |
| Revision-conditioned bootstrap and table readiness | BLOCKED_BY_EVIDENCE | R2 postcondition unproved. The retained `bootstrap()` CPU branch model explicitly rejects observed branch code **121**; its supported codes are 107, 108, 109, 111, and 113. No fallback may be selected. |
| Primary and auxiliary pre-definition source state | BLOCKED_BY_EVIDENCE | L12 and R3 source envelopes remain unbounded. |
| Exact target submission interface, completion and recovery | PARTIAL / BLOCKED_BY_EVIDENCE | CPU packet and op2 request fields are retained, but a live target path, completion postcondition and validated hang recovery are absent. |

## Offline dry run

Run `PYTHONDONTWRITEBYTECODE=1 python3 tools/psb-dri-re/frozen_triangle_dry_run.py` from the repository root. It validates the retained image/BO/closure models, resolves **synthetic test addresses only**, reports all 51 objects and 10 BO roles, 49 relocation records, actual host bytes where the CPU model owns them, and unknown kernel/service payloads as null. The synthetic 36-word submission template is explicitly **NON-EXECUTABLE**. The dry run does no device I/O and cannot prove publication, residency, bootstrap, or source containment.

With the fixed synthetic inputs, the primary PDS occupies the 32-byte-aligned PDS BO slot at `+0x160` and has 56 bytes: `+0x00=0x00000001` (synthetic resolved U), `+0x04=0`, nine clean-room-zero producer-unwritten dwords, `+0x20=0x20`, and program bytes at `+0x30`: `45 03 00 07 00 00 00 af`. The secondary word is `00 00 00 af`. The selected CPU launch words resolve to `0x00000420, 0x00030000, 0x0c000016`. These addresses and words are fixture outputs, **not** a target GPU placement or an authorized command.

Two independent dry-run serializations produced the same SHA-256: `62c7ac314af401aee49e83cd42d00bf9a983d6e588ad1afc1fce34f73ad31756`. The focused host suite passed 161 tests, including three integration tests and two new fail-closed address/reservation tests. This is `CONFIRMED-BY-TEST` for the host model, not hardware observation.

## Exact-action Gate B reassessment

Candidate action reviewed: one submission of the selected 32×32 frozen triangle through the historically modeled TA and raster path. The CPU model contains a 144-byte command template and TA/raster op2 fields, but the concrete target interface, live handles, resolved addresses and service context are not established. Required preconditions remain open: L12/R3 source containment, R1 publication, R2 rev121 bootstrap/table readiness, target provider residency/ownership, completion attribution, and applicable failure recovery. The last passive capture had the active display on `gma500drmfb`; current display/system state is unknown. A visible triangle or completed task would not by itself prove source containment.

**Gate B: BLOCKED. Whitelist: `[]`. Hardware action authorized: NONE.** No submission, MMIO, ioctl experiment, target contact, or recovery action occurred. Program reproducibility and deterministic host construction do not promote hardware readiness.

The single carry-forward evidence blocker remains a **qualified complete source bound for the selected primary literal pair**, including DS, temporary and implicit pre-definition sources. An SGX535 rev121 architectural rule, qualified operand decoder, or target-equivalent per-source trace that establishes every eligible source and its initial state would close that question. The independent rev121 bootstrap rejection and R1/provider/recovery obligations would still require their own evidence before any hardware submission. Do not repeat the closed Phase 7 research routes without genuinely new evidence.
