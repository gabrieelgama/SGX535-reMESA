# Intel document 364236

## What is confirmed

Intel's public EMGD specification update `445348-016US`, April 2012, lists document `364236` in Table 1 as *Intel System Controller HUB External Design Specification* ([P5-001](evidence-matrix.csv)). Intel's 2021 community support response calls it the *Intel System Controller Hub (Intel SCH) External Design Specification* and says that a privileged Resource and Documentation Center account can obtain it ([P5-002](evidence-matrix.csv)).

Those sources establish the document number, title, and a historically stated controlled-access route. An anonymous request to the supplied content endpoint ended at Intel's public 404 page during Phase 5 ([P5-005](evidence-matrix.csv)). No access control was bypassed.

## What was not recovered

No legitimate public copy, revision number, publication date, table of contents, section name, supersession chain, or contents of `364236` was recovered in this phase. The public references do not state whether it covers SGX register attributes, `CORE_ID`, `CORE_REVISION`, SGX clocks, reset, unavailable-aperture behavior, PCI-to-SGX revision mapping, or errata.

The document's title suggests an SCH design document, but that is not evidence for the scope or contents of any section. `364236` therefore cannot close any Gate B requirement.

## Result

**CONFIRMED:** a restricted-access Intel SCH EDS named `364236` existed in the cited public documentation trail.

**UNKNOWN:** whether it contains the SGX535/Poulsbo access contract required here. The highest-value Intel artifact remains an authorized copy of the applicable portions of that EDS, or an equivalent official register and errata reference.
