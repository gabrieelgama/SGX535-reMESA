# Public historical expert map

This is an inquiry-preparation list, not a contact list. It contains only public project attribution and does not recommend unsolicited bulk contact. A reply can clarify source provenance or historical practice; it cannot by itself satisfy a safety-critical hardware requirement unless it includes an attributable authoritative reference or a clearly scoped vendor statement.

| public name | public project role evidenced by | focused question | limit |
|---|---|---|---|
| Alan Cox | author of the 2011 LKML gma500 staging patch that contained `TRAP_SGX_PM_FAULT` ([P6-1-002](evidence-matrix.csv)) | What was the origin and intended use of the disabled SGX-off warning, and did its source cite a public Intel access rule? | the macro does not itself define APM status or safe read behaviour |
| Patrik Jakobsson | author of the 2014 DRI-devel refactor moving `psb_get_core_freq()` to `gma_get_core_freq()` ([P6-1-001](evidence-matrix.csv)) | Did the inherited `core_freq` selector have a cited hardware meaning beyond backlight PWM, especially any SGX-clock relevance? | the patch shows a refactor, not the field’s hardware meaning |
| Greg Kroah-Hartman | public DRI-devel Poulsbo/Moblin patch-thread attribution in the historical driver trail ([P6-1-003](evidence-matrix.csv)) | Which public source drop or maintainer could identify the provenance of the binary/non-open 3D boundary? | this is a provenance question, not a request for an MMIO safety judgment |

Vendor support remains preferable for access attributes, unavailable-aperture failure behaviour, and BRNs. No one was contacted in this phase.
