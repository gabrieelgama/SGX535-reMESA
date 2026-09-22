# Phase 6.2 final report

## Starting state

Phase 6.1 (`16899a1`) left Gate B `BLOCKED`, both identification registers `SAFE-CANDIDATE NO`, the whitelist empty, and Test Vector One neither designed nor executed. Hardware state remained `NO` modified.

## Evidence-acquisition result

Intel document `364236` is publicly established only as the *Intel System Controller HUB External Design Specification* and as historically available through a privileged RDC account. Its relevant contents, revision, date, and supersession remain unknown. Public companion-document trails have not reproduced the SGX access contract.

Imagination’s public material establishes a historical licensee/support boundary for hardware TRMs, not public SGX535/Poulsbo register semantics. Both vendors therefore offer legitimate inquiry routes, but neither route guarantees a response or disclosure.

The developer map prepares focused provenance questions for Alan Cox, Patrik Jakobsson, and Greg Kroah-Hartman from their public PSB/gma500 records. No contact was made. Any recollection would require corroboration before it could affect a safety gate.

## Decision

| item | final state |
|---|---|
| Gate B | `BLOCKED` |
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| RE-GATE | `RE-PREPARE` |
| Test Vector One designed | `NO` |
| Test Vector One executed | `NO` |
| hardware state modified | `NO` |

`RE-PREPARE` is separate from Gate B. It records that further public web archaeology is unlikely to close the safety-critical contract by itself, while authorized document requests and carefully scoped expert inquiries remain available. It hands off only to **Phase 7 — Controlled Reverse-Engineering Design**, which must be reviewed before any hardware observation is considered.

The single most valuable missing artifact remains an authorized SGX535-on-Poulsbo access/errata reference that defines identification-register semantics, required power/clock/reset state, revision coverage, unavailable-aperture failure behaviour, and recovery requirements.
