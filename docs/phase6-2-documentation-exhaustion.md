# Documentation-exhaustion assessment

Phases 1–6.1 investigated 26 independent public source families and recorded the searches and limits in [the Phase 6.1 ledger](phase6-1-research-ledger.md). This phase does not call the subject globally exhausted. It assesses the expected value of further public archaeology for each first-read blocker.

| blocker | assessment | rationale | next evidence |
|---|---|---|---|
| explicit read semantics and side effects | `AUTHORIZED-DOCUMENT-DEPENDENT` | public code and headers show historical names/use but no access attribute | scoped Intel/Imagination hardware-reference answer |
| power, clocks, reset, and pre-initialisation state | `AUTHORIZED-DOCUMENT-DEPENDENT` | public sources show a power-sensitive dump boundary, not a usable state contract | Poulsbo integration documentation or vendor clarification |
| physical revision, PCI mapping, and BRNs | `AUTHORIZED-DOCUMENT-DEPENDENT` | no public `8086:8108` PCI-revision mapping was found | Intel mapping plus applicable SGX errata |
| gma500 locking, PM, suspend/resume | `PUBLIC-EVIDENCE-PROMISING` | source is public, but the installed-kernel identity and a future in-driver protocol still need analysis | passive installed-kernel verification and a reviewed protocol |
| unavailable-aperture failure and recovery | `LIKELY-REQUIRES-REVERSE-ENGINEERING` | no recovered public Poulsbo failure contract; a vendor answer could still avoid experimentation | chipset/vendor contract, otherwise a later controlled-design phase |
| historic code provenance | `EXPERT-CONFIRMATION-POSSIBLE` | maintainers may identify source drops or document locators | a focused, voluntary public professional inquiry |

Public archaeology has reached diminishing returns for the safety-critical hardware properties, because the surviving trails repeatedly terminate at licensee/RDC access, binary boundaries, or code without a contract. That is not proof that public material cannot still appear. Levels 1–3 of [the escalation ladder](phase6-2-authorized-evidence-routes.md) remain legitimate before any controlled observation is considered.
