# First-observation errata review

The available Poulsbo DDK code conditionally enables BRN workarounds for its selected SGX core targets: revision 121 selects BRN22934, BRN23944, and BRN23410; revision 126 selects BRN22934 ([P3-037](evidence-matrix.csv)). This is evidence of historical build-time conditional code. It is not evidence that the Dell system has any of those revisions or BRNs.

The Phase 6 search recovered no public official Imagination or Intel errata document that connects a BRN to either identification-register read, the required clock or power state, unavailable-aperture behavior, or recovery on Poulsbo.

| topic | status | consequence |
|---|---|---|
| physical SGX revision | UNKNOWN | BRN selection cannot be derived |
| Dell-applicable BRN list | UNKNOWN | first-read workaround coverage is unknown |
| `CORE_ID`/`CORE_REVISION` read errata | UNKNOWN | a historical read cannot be accepted as a universal contract |
| clock/reset read errata | UNKNOWN | the clock/reset gate remains blocked |

No applicable erratum was found. That absence does not establish that no erratum applies.
