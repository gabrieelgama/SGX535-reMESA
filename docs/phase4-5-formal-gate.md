# Phase 4.5 — Formal gate reconstruction

2026-09-19. I rebuilt the gate from the evidence that survived the red-team review. All 15 requirements are essential; there is no score or majority rule.

| requirement | CORE_ID | CORE_REVISION | result |
|---|---|---|---|
| 1. SGX535/Poulsbo identity | PASS | PASS | software/header evidence only |
| 2. Exact offset | PASS | PASS | `0x0010` / `0x0014` in the cited sources |
| 3. Access width | PASS | PASS | 32-bit access in the cited sources |
| 4. Read-only semantics | UNKNOWN | UNKNOWN | no explicit contract |
| 5. No read side effects | UNKNOWN | UNKNOWN | no explicit contract |
| 6. Power state | UNKNOWN | UNKNOWN | `active`/D0 is insufficient |
| 7. Clock state | UNKNOWN | UNKNOWN | SGX register clock not established |
| 8. Reset state | UNKNOWN | UNKNOWN | applicable sequence absent |
| 9. Revision coverage | UNKNOWN | UNKNOWN | physical SGX revision unknown |
| 10. Ownership | PASS | PASS | gma500 owns the mapping |
| 11. Mapping lifetime | PASS | PASS | lifetime is defined in the cited driver |
| 12. Locking/races | UNKNOWN | UNKNOWN | no complete read protocol |
| 13. Suspend/resume | UNKNOWN | UNKNOWN | no serialized observation protocol |
| 14. Failure behavior | UNKNOWN | UNKNOWN | CPU MMIO failure behavior absent |
| 15. Recovery | UNKNOWN | UNKNOWN | no validated recovery path |

`CORE_ID`: **SAFE-CANDIDATE NO**. `CORE_REVISION`: **SAFE-CANDIDATE NO**. Whitelist: `[]`. Gate B: **BLOCKED**. The exact evidence identifiers and source locations remain in [evidence-matrix.csv](evidence-matrix.csv).
