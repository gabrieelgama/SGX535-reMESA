# Phase 7.0–7.5: controlled reverse-engineering preparation

The immediate handoff is Phase 6.2 commit `67fe07b`; earlier baselines are `9c5b28e` and `16899a1`. Its Gate A is GO, Gate B BLOCKED, RE-GATE RE-PREPARE, both identification-register SAFE-CANDIDATE states NO, and whitelist `[]`. No TV1 design or execution was carried forward.

This run completes documentary review in order. Each gate is recorded before dependent work proceeds:

| subphase | record | scope |
|---|---|---|
| 7.0 | [architecture](7.0-experimental-architecture.md), [ownership](7.0-ownership-locking.md), [failure/recovery](7.0-failure-recovery.md), [gate](7.0-gate.md) | decide whether a safe first observation can be designed |
| 7.1 | [observation and gate](7.1-first-observation.md) | conditional; blocked by 7.0 |
| 7.2 | [identity/revisions/errata](7.2-revision-errata.md) | static reconstruction |
| 7.3 | [power/clock/reset](7.3-power-clock-reset.md) | static state-transition model |
| 7.4 | [BIF/MMU/address space](7.4-address-space.md) | static translation model and divergences |
| 7.5 | [firmware/microkernel](7.5-firmware-microkernel.md) | static provenance and initialization dependencies |
| review | [red team](phase7-0-5-red-team.md), [final report](phase7-0-5-final-report.md) | audit gates, sources and scope |

Sources, exact revisions, hashes and licenses are indexed in [source provenance](source-provenance.md); new evidence IDs resolve through the [project matrix](../evidence-matrix.csv). Source code evidence describes software, unless a hardware guarantee is explicitly cited.

CONFIRMED means directly established within the stated source scope; INFERRED explains a deduction; UNKNOWN means unestablished. OBSERVED is a target measurement, REPRODUCED requires repeated equivalent measurements, NOT-TESTED means no attempt, and BLOCKED denotes a gate decision. Static source inspection creates no OBSERVED hardware facts.

The recorded machine is Dell Inspiron 1210 / 0X605H / BIOS A02, graphics `0000:00:02.0`, `8086:8108`, PCI revision `0x06`, subsystem `1028:02b1`, class `030000`, IRQ 16. BAR0 is `d8100000–d817ffff`, BAR1 `1800–1807`, BAR2 `d0000000–d7ffffff`, BAR3 `d8380000–d839ffff`. Kernel reported: `5.10.240-antix.1-486-smp`, i686, gma500/card0. These are inherited [TVZ-001 report facts](../hardware-evidence/TVZ-001/README.md), not new measurements or proof of the present machine state.

No Phase 7.6 work is included.

## Later recovered Poulsbo DRI binary

The [xpsb-glx `psb_dri.so` static-analysis track](psb-dri-re/README.md) was added after the 7.0–7.5 source review. It preserves the earlier gates and evidence IDs. Historical DRI and command behavior can be reconstructed from its bytes without executing the binary; the first-observation gate remains BLOCKED and no hardware result was added.

The final pre-implementation P7E pass resolves the historical binary's indirect mode-3 scene-finalization/submission edge in [the draw trace](psb-dri-re/draw-to-submit.md). [Format readiness](psb-dri-re/format-readiness.md) and [clean-room readiness](psb-dri-re/clean-room-readiness.md) still find a minimal 3D userspace path **not yet specifiable** without undocumented semantics. This is static evidence only; Gate B and whitelist `[]` are unchanged.

The follow-on [P7F bounded-path pass](psb-dri-re/bounded-path-closure.md) recovers two conditional USE/USSE instruction slots, a fixed six-index scene branch and additional validation/bootstrap constraints. It corrects an earlier label that had treated the USE instruction buffer as PDS. The [strict decision](psb-dri-re/preimplementation-decision.md) is still Result B; no implementation-grade specification or Phase 8 plan was created.

The [P7G frozen triangle](psb-dri-re/frozen-draw-closure.md) extends the existing binary track without restarting it. Result B remains: fragment output/link metadata, complete state/scene programs, selected resources and bootstrap still prevent an implementation-grade specification. Gate B is BLOCKED; no hardware observation occurred.

The focused [P7H static decision](psb-dri-re/frozen-draw-static-decision.md) narrows the selected fragment compiler to ordinary four-component scalar lowering and records the candidate-family [XHW init request](xpsb-re/xhw-init-frozen-draw.md). For the frozen 32×32 target, both Xpsb scene-info option branches produce the same size and cookie words 0–14; word 15 is unwritten. This does not supply final program bytes or authenticate the exact stack. Result B, Gate B BLOCKED and whitelist `[]` remain unchanged.
