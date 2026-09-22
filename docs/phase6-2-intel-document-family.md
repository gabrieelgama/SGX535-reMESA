# Intel SCH document family around 364236

## What is established

Intel’s EMGD specification update identifies document `364236` as *Intel System Controller HUB External Design Specification* ([P5-001](evidence-matrix.csv)). A 2021 Intel Community moderator said that it was available through a privileged Resource and Documentation Center (RDC) account and pointed account problems to an Intel support form ([P5-002](evidence-matrix.csv)). Anonymous retrieval of the historic content URL ended at Intel’s public 404 page ([P5-005](evidence-matrix.csv)).

This establishes a historical title and access route. It does **not** establish document revision, publication date, table of contents, supersession, or any SGX-specific content.

## Public companion trail

| document | known public role | relationship to 364236 | relevant result |
|---|---|---|---|
| `319537-003US` | Intel SCH datasheet, May 2010 | companion public datasheet | no recovered evidence that it reproduces the SGX identification-read contract |
| `321422` | US15WP/US15WPT addendum | platform companion | no recovered relevant contract |
| `319538` | SCH specification update | errata/update family | no recovered mapping from graphics PCI revision to SGX revision or first-read BRN contract |
| `386599` | SCH EDS addendum/specification-update addendum for US15WP/US15WPT | related EDS/update family | public reference located for USB context only; no SGX contract recovered |
| `319535` / `319536` | Atom Z5xx datasheet and update trail | processor-side companion documents | no recovered SGX aperture or register-read contract |

The table records the public document family, not contents inferred from document titles. A future legitimate Intel response could point to a public successor, an authorized copy, or a specific companion section. Until then, the SGX relevance of `364236` remains `UNKNOWN`.

## Authorized acquisition path

The legitimate route is an Intel account/support request that identifies the historical EDS number and asks for either lawful access or a limited technical confirmation. It must not use the historic URL as a means to bypass RDC access controls. The minimum useful request is in [the question set](phase6-2-question-set.md).
