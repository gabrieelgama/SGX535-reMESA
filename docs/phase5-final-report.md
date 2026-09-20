# Phase 5 final report

Phase 5 followed the evidence-acquisition branch. The audited Phase 4.7 handoff had Gate B `BLOCKED`, an empty whitelist, and no `SAFE-CANDIDATE`.

Public research established the title and historically stated controlled-access route for Intel document `364236`; anonymous retrieval now ends at Intel's public 404 page. It also found a public trace of detailed SGX535 material being handled through Imagination developer support. The research did not recover an applicable register-access contract. The existence of restricted documents was not treated as evidence for their contents.

The real machine's graphics PCI revision remains `0x06`; the Intel graphics stepping and physical SGX core revision remain `UNKNOWN`. The rev116 proposal and rev121 DDK build target retain their original software-only scope. No applicable read erratum or BRN was established.

The available code still establishes gma500 ownership and mapping lifetime. It does not establish a complete locking/PM protocol. Runtime state `active` still does not establish SGX power, clock, or reset state. CPU read failure behavior and recovery remain undocumented.

## Final decision

| item | state |
|---|---|
| Gate A — passive probe | `GO` |
| Phase 5 first-observation gate | `BLOCKED` |
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| Test Vector One | `BLOCKED`; not designed or executed |
| hardware state modified | `NO` |

The single most valuable next artifact is an authorized SGX535/Poulsbo register and errata reference defining the two registers' access attributes and side effects, required power/clock/reset state, revision coverage, unavailable-block behavior, and recovery requirements. The relevant sections of Intel EDS `364236` would be useful if they actually contain that information.
