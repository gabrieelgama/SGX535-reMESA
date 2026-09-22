# Phase 6.1 final report

## Completion record

- **Source families investigated:** 26, listed in [the source inventory](phase6-1-source-inventory.md).
- **Meaningful search groups / literal variants:** 17 groups; 64 web/document variants and nine local Git symbol probes, listed in [the research ledger](phase6-1-research-ledger.md).
- **Primary-source artifacts inspected:** 9 direct or indirect primary artifacts, including original Linux source, public LKML/DRI patches, Intel documentation/support statements, and historical DDK source. Secondary package reports were not used for safety properties.
- **Historical commits inspected:** 0 historical commits locally: the clone has one reachable current commit and its parent is unavailable. Four public historical patch revisions were inspected as archives, not substituted for missing Git objects.
- **Packages/package indexes investigated:** 10 named package or index trails, summarized in [the MeeGo/Moblin package report](phase6-1-meego-moblin-packages.md).
- **Independent public references to `364236`:** 2. The citation graph also followed four related-document nodes.
- **Reference chains followed:** 6, terminating in public metadata, restricted RDC access, unrelated subject matter, a deprecated URL, binary/proprietary boundaries, or no applicable result.
- **New matrix evidence items:** 4 (`P6-1-001` through `P6-1-004`).

## What changed

| Phase 6 conclusion | Phase 6.1 result |
|---|---|
| no public SGX535/Poulsbo access contract was recovered | strengthened as a reproducible, bounded research result; it is not a claim that no such contract exists |
| `gma_get_core_freq()` does not establish the SGX clock | strengthened: public history shows a refactor from `psb_get_core_freq()`; source still identifies only a backlight-PWM consumer |
| SGX power/clock state is unknown | strengthened caution: the historical disabled `TRAP_SGX_PM_FAULT` macro warns about off-state reads, but provides no usable contract |
| MeeGo/Moblin yielded no applicable contract | unchanged: package archaeology recovered more names and provenance but no contract |
| physical SGX revision, failure behavior, and recovery are unknown | unchanged |
| cross-platform Series5 sources cannot be transferred to Poulsbo | strengthened by offset variation and a contemporary revision-detection discussion |

## Specific results

`gma_get_core_freq()` originated in the public staging path as `psb_get_core_freq()` and was moved into a shared file in 2014 without semantic change. It writes root-bridge configuration offset `0xd0`, reads `0xd4`, and decodes a value stored in `dev_priv->core_freq`. The current Poulsbo path uses that value for backlight PWM. No retrieved source identifies the selected field as an SGX execution clock or as an SGX accessibility condition.

MeeGo/Moblin archaeology found the historical `psb-headers`, `xf86-video-psb`, `psb-firmware`, `Xpsb`, `xpsb-glx`, `libdrm-poulsbo1`, and related distribution packaging trail. It did not recover an applicable power, clock, reset, or register-access specification. A 2009 DRI-devel maintainer thread explicitly preserves a binary/non-open 3D boundary. Package evidence remains provenance evidence only.

Cross-platform SGX material confirms that identification-register names and field layouts exist in multiple Series5 headers, but offsets vary across cores. A 2023 Series5 binding discussion does not provide a universal pre-initialization read contract. Neither result is Poulsbo-specific safety evidence.

## Final gate

The independent reassessment is in [phase6-1-gate-reassessment.md](phase6-1-gate-reassessment.md).

| item | final state |
|---|---|
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| Gate B | `BLOCKED` |
| Test Vector One | not designed or executed |
| hardware state modified | `NO` |

The remaining blockers are explicit read semantics, positive evidence of no read side effects, a valid SGX power/clock/reset contract, physical revision and BRN applicability, gma500-owned concurrency/PM exclusion, CPU-visible failure behavior, and proven recovery.

The single highest-value missing artifact remains an authorized Intel/Imagination SGX535-on-Poulsbo register and errata reference defining those properties. Phase 6.1 does not authorize Phase 7 or any hardware access.
