# Phase 6.1 gate reassessment

The Phase 6 gate was rebuilt independently after the red-team pass. A changed state would require new applicable evidence. There is none.

| # | requirement | `CORE_ID` | `CORE_REVISION` | Phase 6.1 result |
|---:|---|---|---|---|
| 01 | exact identity | PASS | PASS | unchanged historical SGX535/Poulsbo source evidence |
| 02 | exact SGX-relative offset | PASS | PASS | unchanged: `0x0010` / `0x0014` |
| 03 | access width | PASS | PASS | unchanged historical 32-bit use |
| 04 | explicit read semantics | UNKNOWN | UNKNOWN | no R/O attribute found |
| 05 | no destructive/read side effects | UNKNOWN | UNKNOWN | no positive contract found |
| 06 | valid power state known | UNKNOWN | UNKNOWN | APM diagnostic is not a predicate contract |
| 07 | valid clock state known | UNKNOWN | UNKNOWN | `core_freq` trace does not identify SGX clock |
| 08 | valid reset state known | UNKNOWN | UNKNOWN | no accessibility/ordering requirement found |
| 09 | physical revision/stepping applicability | UNKNOWN | UNKNOWN | no PCI `0x06` mapping |
| 10 | applicable errata understood | UNKNOWN | UNKNOWN | no Dell BRN applicability |
| 11 | ownership known | PASS | PASS | unchanged: gma500 owns mapping |
| 12 | mapping lifetime known | PASS | PASS | unchanged: gma500 lifecycle |
| 13 | locking/concurrency protocol known | UNKNOWN | UNKNOWN | no exclusion protocol found |
| 14 | suspend/resume interaction known | UNKNOWN | UNKNOWN | no race-free protocol found |
| 15 | failure behavior acceptable | UNKNOWN | UNKNOWN | no Poulsbo CPU/aperture contract found |
| 16 | recovery strategy adequate | UNKNOWN | UNKNOWN | no applicable recovery contract found |

| item | decision |
|---|---|
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| Gate B | `BLOCKED` |
| Test Vector One | not designed or executed |
| hardware state modified | `NO` |

The new historical diagnostic makes an unqualified read less defensible, but it does not turn any row into `NO-GO`: it is optional software instrumentation with undocumented APM semantics. The evidence state therefore remains `UNKNOWN` rather than being overstated in either direction.
