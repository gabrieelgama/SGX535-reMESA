# Reverse-engineering decision gate

## Decision

**RE-GATE: `RE-PREPARE`**

This means public research has reached diminishing returns for the missing safety contract and a future Phase 7 may prepare a controlled reverse-engineering design. It does not authorize an experiment, change Gate B, add a whitelist entry, or design Test Vector One.

| decision | state | reason |
|---|---|---|
| Gate B | `BLOCKED` | requirements 04–10 and 13–16 are not fully evidenced; see [Phase 6.1 gate](phase6-1-gate-reassessment.md) |
| `CORE_ID` | `SAFE-CANDIDATE NO` | no complete access, power/clock/reset, failure, or recovery contract |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` | same unresolved requirements |
| whitelist | `[]` | no candidate passes every safety-critical requirement |
| RE-GATE | `RE-PREPARE` | authorized/document and expert routes exist but are uncertain; Phase 7 design preparation may define what evidence an observation would need to obtain |

## Conditions for a future Phase 7 design

A future design phase must first answer, on paper:

1. Which exact unknown is worth observing and why no authorized source can settle it.
2. How SGX ownership remains inside gma500 and how removal, PM, suspend/resume, IRQ, and KMS concurrency are excluded.
3. Which known state is being observed, without using the proposed read to establish its own preconditions.
4. Which CPU-visible failure modes remain possible, how the attempt is bounded, and what recovery has evidence for that failure mode.
5. Which physical revision/BRN uncertainty remains and how it affects interpretation.

Phase 7 must be separately reviewed before it may propose a bounded observation. This document neither designs nor executes it.
