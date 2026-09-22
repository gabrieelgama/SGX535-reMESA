# Authorized evidence routes

## Scope

The missing contract is narrow: whether a CPU can perform a 32-bit read of the Poulsbo SGX535 identification registers at SGX-relative offsets `0x0010` and `0x0014`, and what must be true before that read. Existing public code establishes historical use, not the hardware contract.

## Routes still available

| route | public basis | what it can reasonably seek | limit |
|---|---|---|---|
| Intel companion documents | public SCH datasheet, addenda, and specification-update trail | an explicitly public cross-reference, reproduced access rule, or erratum | no recovered companion document currently supplies the SGX read contract |
| Intel authorized document request | Intel’s 2021 support response identifies a privileged RDC account for EDS `364236` and an Intel account-support form | access to, or a scoped answer from, the document owner | the response does not promise current access, disclosure, or relevant contents |
| Imagination support inquiry | Imagination staff stated that hardware TRMs were exposed to licensees and directed technical questions to support ([P6-2-001](evidence-matrix.csv)) | a narrow, attributable answer about Series5 identification-register semantics | it does not establish that current support has Poulsbo integration material or may disclose it |
| Public professional inquiry | public authorship records for PSB/gma500 history | code provenance, historical integration practice, or document locators | recollection is not a substitute for an applicable hardware contract |

No route has been used in this phase. No authentication was bypassed and no restricted material was obtained.

## Escalation ladder

| level | action | Phase 6.2 status |
|---|---|---|
| 0 | existing public documentation | completed through Phase 6.1 |
| 1 | additional legitimate public companion documents | still possible, but low expected return for the missing contract |
| 2 | official Intel/Imagination document or support request | available to prepare; no request sent |
| 3 | public, professional inquiry to a historical maintainer | available to prepare; no contact made |
| 4 | controlled reverse-engineering design | may be prepared only in a separately reviewed Phase 7 |
| 5 | separately reviewed controlled observation | prohibited in this phase |

The ladder describes evidence escalation, not permission to read the SGX aperture. Gate B remains `BLOCKED`.
