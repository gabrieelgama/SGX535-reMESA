# Reproducible negative-evidence register

| blocker | searched | found | not found | limitation | next artifact |
|---|---|---|---|---|---|
| explicit RO semantics | Intel/IMG docs, DDK/EMGD headers, Android headers, code and forum variants for both symbols | names, offsets, masks, historical reads | SGX535/Poulsbo R/O attribute | headers and reads do not specify access type | authorized SGX535 register reference |
| read side effects | `read-only`, clear/latch/FIFO/destructive/readl/MMIO variants; debug and dump paths | historical reads/dumps | positive side-effect-free statement | failure to find a warning is not proof | same register reference or errata |
| power/clock/reset | OSPM/APM/CLKGATE, `core_freq`, Intel SCH docs, MeeGo/Moblin, DDK sequencing | driver ordering, gate fields, an SGX-unpowered dump warning, optional old diagnostic | minimum readable state and passive proof | driver behavior is not hardware contract | Poulsbo EDS/PM sequencing reference |
| revision/BRNs | PCI IDs/revision, rev116/121/126, BRN queries, Intel spec updates | software build targets and conditional workarounds | Dell PCI-to-SGX mapping and applicable BRN set | no mapping can be inferred | Intel/IMG stepping and errata mapping |
| locking/PM races | gma500 source, local history, LKML, DRM archives | gma500 ownership/lifetime and partial locks | complete in-driver exclusion protocol | installed kernel identity remains unverified | reviewed target-kernel design or authoritative lifecycle contract |
| failure behavior | hang/timeout/abort/MMIO/aperture variants, Intel/SCH/IMG trail, recovery code | diagnostics guidance only | Poulsbo CPU-visible result or recovery | generic PCI behavior is not specific enough | Intel/IMG aperture failure/recovery specification |
| firmware/MMU/BIF prerequisite | historical init order, DDK, EMGD, Series5 discussion | stacks initialize surrounding state | architectural before/after dependency | sequencing is not proof of prerequisite | SGX535 initialization/register chapter |

The negative result is therefore a bounded, reproducible statement: the listed searches found no applicable positive contract. It is not a claim that no such contract exists, and it cannot supply a gate `PASS`.
