# Phase 6.1 adversarial review

| Phase 6 conclusion attacked | red-team evidence and result | disposition |
|---|---|---|
| no public access contract recovered | public SCH datasheet/addendum and `364236` citation graph added related documents but no access section | strengthened as a bounded research result; not a claim of nonexistence |
| MeeGo/Moblin contained nothing applicable | package archaeology found more names and the old Moblin X.Org lineage, but no clock/power/reset/read contract | unchanged |
| `gma_get_core_freq()` does not establish SGX clock | earlier staging code and later refactor preserve the transaction; current Poulsbo consumer is backlight PWM; no Intel field definition found | strengthened |
| physical SGX revision unknown | Series5 DT discussion, DDK target variants, and public SCH material supplied no `8086:8108` PCI `0x06` mapping | unchanged |
| power is unknown | old `TRAP_SGX_PM_FAULT` code warns of an off-state but lacks APM-bit semantics and still reads | strengthened caution; gate remains unknown |
| failure and recovery unknown | no documented Poulsbo response or recovery path recovered | unchanged |
| cross-platform headers could establish safety | offset variation and maintainer uncertainty demonstrate the danger of extrapolation | strengthened rejection of transfer |

The second pass used numeric selector/offsets, alternate spellings (`Poulbo`, `CORE_REV`), PCI IDs, APM symbols, BRN/errata terms, package names, and author/maintainer patch trails. It found no evidence that upgrades a safety-critical requirement. No negative result was converted into positive read safety.
