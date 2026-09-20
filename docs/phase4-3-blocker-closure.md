# Phase 4.3 — Final evidence closure

2026-09-19. Ten initial blockers remained open; none was closed. `CORE_ID` and `CORE_REVISION` stayed outside the whitelist. No hardware operation was performed.

The lost-document hunt found references to restricted IMG SGX TRMs and to Intel SCH EDS document **364236**. The EDS was not recovered. No source supplied an explicit read-only or side-effect-free contract for either ID, or a Poulsbo-specific power, clock, reset, revision, locking, failure, or recovery contract.

The antiX source package was recovered and compared with upstream v5.10.240. This establishes source correspondence only; byte-for-byte identity with the running kernel and module remains **UNVERIFIED**.

| requirement | result | remaining uncertainty |
|---|---|---|
| 4. Read-only | UNKNOWN | no applicable SGX535/Poulsbo register attribute |
| 5. No read side effects | UNKNOWN | no positive side-effect-free contract |
| 6. Power | UNKNOWN | runtime PM state does not prove SGX power |
| 7. Clocks | UNKNOWN | register-interface clock requirements are undocumented |
| 8. Reset | UNKNOWN | reset-state requirement for the IDs is undocumented |
| 9. Revision coverage | UNKNOWN | physical SGX revision is unknown |
| 12. Locking | UNKNOWN | no complete protocol for a new read while gma500 owns the mapping |
| 13. Suspend/resume | UNKNOWN | no serialized observation protocol |
| 14. Failure behavior | UNKNOWN | CPU MMIO read failure behavior is undocumented |
| 15. Recovery | UNKNOWN | no validated recovery for a failed CPU MMIO read |

See [lost-documentation-index](lost-documentation-index.md), [formal gate](phase4-5-formal-gate.md), and [evidence matrix](evidence-matrix.csv).
