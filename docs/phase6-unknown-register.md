# Phase 6 unknown register

This register keeps the first-observation gaps explicit. Existing project unknown IDs are preserved where they apply.

| ID | question | status | evidence searched/found | why insufficient | artifact needed | gate requirement |
|---|---|---|---|---|---|---|
| U01 | What physical SGX revision is in the Dell system? | UNKNOWN | TVZ-001 identifies PCI `0x06`; rev116/121/126 are historical software references | no authoritative mapping | Intel/IMG stepping or core-identification reference for `8086:8108` | 09 |
| U02 | Are the two IDs safe reads? | UNKNOWN | historical reads and offsets | no RO or side-effect contract | SGX535/Poulsbo register reference | 04–05 |
| U03/U04 | What internal power and clock state permits a read? | UNKNOWN | PM code, `CLKGATECTL` fields, DDK warning | no minimum state or proof method | platform power/clock sequencing reference | 06–07 |
| U10 | What reset/init state is required? | UNKNOWN | historical order and reset code | sequence is not a hardware requirement | reset/accessibility reference | 08 |
| U11 | Which BRNs apply? | UNKNOWN | build-target conditional workarounds | physical revision unknown | official errata matched to physical revision | 09–10 |
| U05/U06 | What serializes a read under gma500? | UNKNOWN | mapping ownership and lifecycle code | no complete exclusion protocol | applicable gma500/DRM synchronization contract or reviewed in-driver design | 13–14 |
| U14 | What happens on failed access and how is it recovered? | UNKNOWN | diagnostics guidance only | no Poulsbo read-failure/recovery contract | Intel/IMG aperture and recovery documentation | 15–16 |
| U21 | Is firmware/MMU/BIF initialization required before the IDs? | UNKNOWN | historical stacks initialize surrounding state | no architectural dependency statement | SGX535 register-access chapter | 06–08 |

## Phase 6.1 adversarial update

The source-family pass did not resolve any row. It added historical evidence that gma500 developers had an optional SGX-off diagnostic around `PSB_RSGX32` (P6-1-002), but that macro neither defines the relevant APM state nor avoids the read. It therefore reinforces U03/U04 without changing their classification. The full reproducible search record is [phase6-1-research-ledger.md](phase6-1-research-ledger.md).

The single artifact with the broadest closure value is an authorized SGX535/Poulsbo register and errata reference that covers access attributes, side effects, power/clock/reset requirements, physical-revision applicability, unavailable-block behavior, and recovery.
