# Phase 6 first-observation gate

This gate is rebuilt from the Phase 5 baseline and Phase 6 evidence. `PASS` requires applicable evidence; an unsuccessful search is not a pass. `BLOCKED` is the resulting gate state when one or more required facts are `UNKNOWN`.

| # | requirement | `CORE_ID` | `CORE_REVISION` | evidence or missing evidence |
|---:|---|---|---|---|
| 01 | exact identity | PASS | PASS | P3-001, P3-011, P4-014/P4-015 |
| 02 | exact SGX-relative offset | PASS | PASS | `0x0010` / `0x0014` in the same historical sources |
| 03 | access width | PASS | PASS | historical 32-bit access sites |
| 04 | explicit read semantics | UNKNOWN | UNKNOWN | no SGX535/Poulsbo access attribute recovered |
| 05 | no destructive/read side effects | UNKNOWN | UNKNOWN | no positive side-effect-free contract recovered |
| 06 | valid power state known | UNKNOWN | UNKNOWN | [power/reset review](phase6-power-reset.md) |
| 07 | valid clock state known | UNKNOWN | UNKNOWN | [clock map](phase6-clock-map.md) |
| 08 | valid reset state known | UNKNOWN | UNKNOWN | no access-order requirement recovered |
| 09 | physical revision/stepping applicability | UNKNOWN | UNKNOWN | [revision map](phase6-revision-map.md) |
| 10 | applicable errata understood | UNKNOWN | UNKNOWN | [errata review](phase6-errata.md) |
| 11 | ownership known | PASS | PASS | P4-007: gma500 owns the mapping |
| 12 | mapping lifetime known | PASS | PASS | P4-007: driver lifecycle creates/destroys it |
| 13 | locking/concurrency protocol known | UNKNOWN | UNKNOWN | [ownership review](phase6-ownership-concurrency.md) |
| 14 | suspend/resume interaction known | UNKNOWN | UNKNOWN | no exclusion protocol for PM transitions |
| 15 | failure behavior acceptable | UNKNOWN | UNKNOWN | [failure review](phase6-failure-recovery.md) |
| 16 | recovery strategy adequate | UNKNOWN | UNKNOWN | no recovery contract |

## Decision

| candidate | SAFE-CANDIDATE |
|---|---|
| `CORE_ID` | NO |
| `CORE_REVISION` | NO |

**Whitelist:** `[]`

**Gate B:** `BLOCKED`

The `PASS` entries survived an adversarial review because each remains scoped to software identity, offset, width, or gma500 mapping ownership. None proves safe hardware completion. The unresolved requirements independently block an observation, including if the SGX clock is disabled, the physical revision differs from a DDK target, a PM transition races the read, or the aperture does not respond.
