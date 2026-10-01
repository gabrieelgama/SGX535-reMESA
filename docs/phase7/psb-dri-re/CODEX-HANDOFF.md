# SGX535-reMESA Phase 7 — final handoff

**Phase status: COMPLETE WITH UNRESOLVED EVIDENCE BOUNDARY.** This closes the Phase 7 archaeology and selected-path reconstruction pass, not the technical source-containment or hardware gate. The older [chronology](CODEX-HANDOFF-chronology.md) and linked evidence remain preserved. Do not treat older checkpoint “next steps” as current.

## Target and observed identity

Dell Inspiron Mini 12 / 1210, Intel Poulsbo / GMA 500, PowerVR SGX535. The raw CORE_ID value is **0x01130000**, HW-OBSERVED with [operator-reported invocation provenance](../../hardware-evidence/MINI12-20260928-COREID-OPERATOR-01/results.md). The raw CORE_REVISION value is **0x00010201**, HW-OBSERVED in a [separately captured one-read session](../../hardware-evidence/MINI12-20260928-CORE-REVISION-01/results.md). Retained SGX535 masks decode major 1, minor 2, maintenance 1; the retained branch formula gives **rev121 / 1.2.1**. PCI revision 0x06 is a different field. These two narrow reads establish identity/revision only.

The last passive target capture had gma500_gfx bound, gma500drmfb driving the 1280×800 display, and Xorg present; the **current live state is UNKNOWN**. No GPU hang recovery has been validated on this machine. See [H0](../../hardware-evidence/MINI12-20260927-H0/results.md), [P1](../../hardware-evidence/MINI12-20260928-GATEB-P1/results.md), and the [recovery audit](../../hardware-evidence/MINI12-20260927-H0/recovery-protocol.md).

## Selected frozen program and host construction

The selected historical primary PDS producer emits exactly **0x07000345, 0xaf000000**, in that order, at +0x30/+0x34 after 12 data DWORDs (+0x00..+0x2f). The program bytes are **45 03 00 07 00 00 00 af** (little-endian). Both words are **OPAQUE HISTORICAL LITERALS** for this fixed path; neither needs dynamic bit editing. Their complete architectural semantics and read sets remain UNKNOWN. The source-free/terminal interpretation of 0xaf000000 is only INFERRED.

The producer commits 56 bytes. Known selected data are +0x00 = resolved linked-USE relocation U, +0x04 = 0, and +0x20 = 0x00000020. It does not write +0x08, +0x0c, +0x10, +0x14, +0x18, +0x1c, +0x24, +0x28, or +0x2c. The [clean-room contract](cleanroom-backing-contract.md) deliberately zeros the complete controlled 0x20000-byte backing before producer writes, so those nine positions are deterministic zero **in the clean-room image**. Their historical contents and whether either PDS word reads them are not established. The selected slot is 32-byte aligned; retained first-use candidates are +0x160 or +0x1c0.

The [CPU launch trace](pds-launch-state-coverage.md) confirms the serialized words R(A_secondary), 0x00030000, R(A_primary) | 0x0c000000, where R(A) = (A >> 4) & 0x00ffffff and selected Dp = 12 produces 0x0c000000 by (Dp & 0xfc) << 24. This is CPU-side construction, **not** proof of an architectural DS preload extent. Resolved GPU addresses, one-time offset application, publication, residency, mapping, ownership, and no mutation through consumption remain concrete provider obligations.

## Final classifications

| Item | Phase 7 classification | Scope |
| --- | --- | --- |
| Program reproducibility | **PASS** | Exact two historical PDS words and eight program bytes for this fixed path. |
| Host image determinism | **PASS** | Parameterized by valid resolved U; full controlled backing zeroed before known writes. Host model only. |
| CPU launch serialization | **PASS** | Three packed words/formulas reconstructed from retained producer. |
| Target provider conformance | **CONDITIONAL / NOT ESTABLISHED** | No implemented, target-qualified provider proves address, publication, residency, lifetime, and ownership obligations. |
| Architectural source containment | **PARTIAL / UNPROVED** | Complete pre-definition source domain for the selected pair is not bounded. |
| Hardware submission readiness | **BLOCKED** | No triangle submitted; Gate B BLOCKED, whitelist []. |

The [strong clean-room closure](cleanroom-final-closure.md) records other independent whole-triangle obligations: R1 payload/translation publication, R2 bootstrap readiness, and R3 source-state containment for five auxiliary families as well as this primary pair. The complete serializer still refuses L12, FT-AUX, FT-BO, and FT-SERVICE. Exact CPU bytes do not discharge those hardware implications.

## Unresolved evidence boundary and exhausted routes

The **single primary-program carry-forward blocker** is a qualified, complete source bound for 0x07000345 / 0xaf000000, including whether either word can consume an uninitialized DS, temporary, or implicit source. The +0x00/+0x04/+0x20 association is CORRELATED, not proven read. All nine producer-unwritten DWORDs remain possible reads. Whole-object initialization controls any source mapped to its initialized backing; the architectural mapping and eligibility remain unproved.

Phase 7 tried both complete architectural-envelope and exact-program read-set routes. The SGX535 document [hunt](sgx535-pds-document-hunt.md) exhausted public recovery of Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf. The Vita/SGX543 lead yielded no SGX535-qualified rule. Retained producer/corpus/differential analysis cannot bound the selected read set. PDUMP exposes CPU-side/indirect capture; HWPerf, debug/trace registers, and generic faults lack demonstrated source granularity. The visible EMULATOR branches are host timing accommodations; the RTSIM bridge requests are dummy-bound and no executable model or PDS interpreter was recovered. A real-Mini-12 output experiment was judged **not identifiable**: models with and without an extra source read can have the same externally visible result. None of these routes should be repeated without genuinely new evidence.

Also closed for Phase 7: CORE_ID and CORE_REVISION discovery, rev121 identification, PCI revision comparisons, broad Gate B archaeology, 0x07 semantic search, full PDS ISA reconstruction for this purpose, selected producer and nine-DWORD archaeology, PDS corpus/differentials, linked-USE relocation, backing-provider archaeology, missing-PDF recovery, emulator/RTSIM archaeology, and current hardware-experiment design. Closure of research routes does not turn UNKNOWN architectural facts into negative findings.

## Gate and Phase 8 boundary

**Gate B: BLOCKED. Whitelist: []. Hardware action authorized: NONE.** The two reviewed identification reads do not authorize further MMIO, GPU submission, experimental ioctl, fault injection, reset, power/clock change, driver reload, or triangle execution. No Phase 7 closure result changes that gate.

Phase 8 may begin only as a separately scoped plan that accepts this handoff and does **not** replay Phase 7 archaeology or assume source containment. Work independent of the unresolved bound may be planned; any source-dependent execution remains blocked. Reopen the source question only on genuinely new, attributable SGX535/rev121 evidence: an applicable architectural rule, qualified operand decoder, target-equivalent per-source trace, or inspectable historical model. Any later target action needs its own exact authorization and safety review. **Do not begin Phase 8 from this handoff.**
