# Phase 5 first-observation gate

This table rebuilds the gate for `CORE_ID` and `CORE_REVISION` from the audited Phase 4 evidence plus Phase 5 research. `UNKNOWN` means the required fact is not established. Every safety-critical non-`PASS` entry blocks the candidate.

| requirement | `CORE_ID` | `CORE_REVISION` | evidence or missing evidence |
|---|---|---|---|
| 1. Exact identity | PASS | PASS | SGX535/Poulsbo software/header evidence; P3-001, P3-011, P4-014/P4-015 |
| 2. Exact SGX-relative offset | PASS | PASS | `0x0010` / `0x0014` in the cited SGX535/Poulsbo sources |
| 3. Access width | PASS | PASS | historical sources use 32-bit accesses |
| 4. Read-only semantics | UNKNOWN | UNKNOWN | no applicable access-attribute contract recovered |
| 5. No read side effects | UNKNOWN | UNKNOWN | no positive side-effect-free contract recovered |
| 6. Required power state | UNKNOWN | UNKNOWN | internal SGX/register-interface power requirement and proof method absent |
| 7. Required clock state | UNKNOWN | UNKNOWN | required clock domain and passive proof method absent |
| 8. Reset-state requirements | UNKNOWN | UNKNOWN | accessibility and ordering relative to reset absent |
| 9. Applicable core revision/stepping | UNKNOWN | UNKNOWN | physical SGX revision and universal revision coverage absent |
| 10. Applicable errata | UNKNOWN | UNKNOWN | target BRN set and read-relevant errata coverage absent |
| 11. Ownership | PASS | PASS | gma500 owns the PCI function and SGX mapping; P4-007 |
| 12. Mapping lifetime | PASS | PASS | gma500 creates and destroys the mapping within driver lifetime; P4-007 |
| 13. Locking/concurrency | UNKNOWN | UNKNOWN | no complete gma500-owned exclusion protocol |
| 14. Suspend/resume interaction | UNKNOWN | UNKNOWN | no protocol prevents a read from racing PM/D-state transitions |
| 15. Failure behavior | UNKNOWN | UNKNOWN | unavailable/gated/reset CPU-read behavior absent |
| 16. Recovery strategy | UNKNOWN | UNKNOWN | no proven recovery for the possible MMIO failure modes |

## Decision

| item | result |
|---|---|
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| first-observation gate | `BLOCKED` |
| Test Vector One | `BLOCKED` |

The Phase 5 sources document why primary material is missing but do not close any safety-critical requirement. No Test Vector One design or executable access code is authorized.
