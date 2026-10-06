# FIRE #2 — independent normal-root evidence recovery, 2026-10-06

**C — PRIMARY_EVIDENCE_NOT_FOUND:** no independent primary consumer or same-mode
encoder contract was recovered for the selected normal SGX535 rev121 root.
This does not mean no primary sources were found. The newly inspected sources
supply a useful region-parser vocabulary, but no root state predicate or
successor-address transform. The identified permitted static source set is
exhausted for this question; repeating the same Xpsb searches is not justified.

Successor **UNRESOLVED**, decoder **PARTIAL**, common raster ABI **B — STRONGLY
SUPPORTED BUT INCOMPLETE**. No contradiction or proof of different formats was
found. The precise remaining fact is:

> Under the selected normal SGX535 rev121 raster controls, what first-position
> state predicate/consumed width at `S+0x1000` distinguishes EMPTY from a
> successor, and how is that successor address derived?

## Scope and evidence tiers

The [previous successor finding](fire02-root-successor-rule-20261006.md) is
reused. It establishes the scene GPU base `S`, direct TA `0x21c` and raster
`0x408` handoff of `S+0x1000`, and a 768-byte allocation calculated as
`3 × 64 dwords`, with scaled axes `8 × 8`. Those facts do not establish a
12-byte hardware stride, zero-as-EMPTY, list tags, pointer encoding or primitive
convergence. The communication-BO parser remains unrelated and excluded from
TA-output evidence. CPU flat-reference semantics are not substituted for normal
root semantics.

T1 means **DIRECT_REV121_EVIDENCE**, requiring applicable revision provenance.
T2 means **SGX535_EVIDENCE_REVISION_UNPROVED**. T3 means
**SERIES5_COMPARATIVE_EVIDENCE**, usable as a dictionary or hypothesis generator.
A generic header in a rev121-capable tree is not automatically a T1 field
specification. No SGX543/Rogue field is transferred into SGX535 semantics.

## Relevant evidence inventory

The [full inventory](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/evidence-inventory.json) records paths, provenance,
GPU/revision, source/document/binary kind, raster/region/parameter relevance and
whether content was newly inspected or a previous result was reused. `audit/`
in its path column means this task's audit directory, not a repository path.
Pinned [TI file/blob manifest](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-1.14-manifest.json),
[primary download manifest](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/independent-primary-manifest.json),
[artifact inventory](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/available-artifact-inventory.json) and
[bounded search scope](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/search-scope.json) preserve the detailed scope.

| Source group | Tier | Kind | Result |
| --- | --- | --- | --- |
| REV121_BUILD | T1 | source/build | 535/121/Poulsbo target confirmed; not a consumer ABI |
| XPSB_REV121 | T1 | binary plus preserved disassembly | Prior findings reused; no repeat broad binary search |
| PSB_DDX | T2 | source/excerpts | Inventory via previous report; no new normal root rule |
| PSB_KERNEL | T2 | source | Previous root-source scope exhausted; independent register dictionary has neither target offset |
| TI_535_HEADER | T2 | source/header | One unique header blob under available name history; neither 0x21c nor 0x408 |
| EMGD_535_HEADER | T2 | source/header | Neither target offset nor names |
| SAMSUNG_535_HEADER | T2 | source/header | Neither target offset; no register in0x1f0..0x500 |
| TI_REGION_DICTIONARY | T3 | source/debug vocabulary | Trace labels only; no implementation references/predicates |
| TI_WA_REGION_HANDLE | T3 | source | No encoder; wrong feature gate |
| TI_COMMON_SERVICES | T3 | source/history | No first-position predicate/address transform; no microkernel implementation payload in paths |
| EMGD_PLB_HEADERS | T2 | source/header | Page constants/cookies not root fields; Napa structs not promoted |
| EMGD_SERVICES | T3 | source | Previously exhausted handles/ownership; no repeat payload analysis |
| CURRENT_GMA500 | T2 | source/header | No target offset; current frozen writes are derived output, not independent ABI |
| INTEL_SCH | T2 | document | Previous search supplies no target grammar; do not repeat |
| OMAP_HWDEFS | T3 | source/header | Event name not root payload; no independent535corroboration |
| OMAP_UM_BINARIES | T3 | binary/metadata | No identified535consumer lead; binaries not reconstructed/used as target spec |
| VITA_PREVIOUS | T3 | source/findings | Inventory only; excluded low-level branch not reopened |
| ROGUE_SEARCH | Outside | document | Rejected fortargetfields;64-bit register/alignment notmappedto535; no deep research |
| VENDOR_DOC_LOCATOR | T2 | document locator | Not an acquired or authoritative consumer contract |

The TI history scan identifies 835 distinct candidate source/header blobs;
816 with classified permission notices were searched. The remaining 19 are
OMAP clock-policy files and were not consumed as evidence. The
[scope/exclusion ledger](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-history-expanded-scope.json) and
[matched references](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-history-expanded-hits.json) make this limitation
explicit. This is a bounded negative result, not a claim about every private
DDK or every historical internet source. Existing Xpsb, PSB, Intel integration,
Vita and UM binary findings were reused where no new target-specific lead
justified reinspection. No restricted implementation was recovered indirectly.

## New independent primary evidence

### Exact build provenance, without overpromoting shared headers

TI commit `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` includes the Poulsbo
`pc_i686_poulsbo_d0_linux` build target selecting SGX535 core revision121. This
is T1 build-selection evidence. The generic `sgx535defs.h` is still T2 for
individual field semantics. Across the available filename history it has one
unique blob `8039da4a73ef9ee3e929edb64244d2891bc9239e`, not several independent
root specifications. The already verified Xpsb rev121 path remains T1
historical evidence; its previous structural findings are not repeated here.

### Registers 0x21c and 0x408

The TI header, separately pinned EMGD header and independent Samsung SGX535
header do not define either target offset or any register in the bounded
`0x1f0..0x500` range. They expose host-event/control and other register subsets,
not the relevant TA/ISP programming range. This is header coverage evidence,
not proof the hardware lacks those registers. No independent symbolic identity,
alignment, mask or address interpretation was recovered. The
[offset-check record](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/independent-offset-result.json) preserves this result.
The independent Samsung primary source is
[sgx535defs.h](https://android.googlesource.com/kernel/samsung/+/android-samsung-2.6.35-gingerbread/drivers/gpu/pvr/sgx535defs.h),
Git blob `04f43be12b136acaac3515449f826075c9d6d00c`; revision applicability is
unproved. One attempted OMAP header URL returned404 and was not used as evidence
of global absence.

### Region-parser terminology: genuine lead, not field semantics

The [TI microkernel status-code header](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-1.14/eurasia_km_services4_include_sgx_ukernel_status_codes.h#L44)
explicitly requires an unconditional dictionary and explains that `MKTC_ST`
creates debugging/stringification support. Lines69–72 identify high byte
`0xAD` as a debug-code discriminator. It does **not** declare root fields.

* Lines166–167: `MKTC_KICKRENDER_CONFIG_REGION_HDRS`.
* Lines823–840: control/region address, EMPTY_TILE, EMPTY_LAST_TILE, NOT_EMPTY,
  OBJECT_COMPLETE/INCOMPLETE and STREAM_LINK labels.
* Lines841–848: primitive-mask and byte-mask presence/zero labels.
* Lines853–856: DPM region parser idle and next region base labels.

The [public OMAP copy](https://android.googlesource.com/kernel/omap.git/+/android-omap-tuna-3.0-jb-mr2/drivers/gpu/pvr/sgx_ukernel_status_codes.h)
also preserves this vocabulary. The labels suggest places an implementing
microkernel parser might expose the desired operations; that suggestion is
T3 inference. They do not establish which core or normal mode uses them.
`MKTC_KICKRENDER_END = 0xAD000408` is a status code, not register `0x408`.
A status suffix `0x121c` likewise is not a register definition. Numeric suffix
matches are rejected.

Tracing [their references](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/microkernel-lead-references.txt) found dictionary
entries and stringification, not instructions that emit these codes or inspect
root payloads. The inspected kernel-source set has no implementing microkernel
payload for this lead. No mask, word offset, test value or pointer shift follows
from these labels.

### Region-header resource handles and allocation lookalikes

The [TI sgxinfo structure](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-1.14/eurasia_km_services4_include_sgxinfo.h#L112)
contains `hKernelClearClipWAPSGRgnHdrMemInfo` behind `FIX_HW_BRN_31542` or
`FIX_HW_BRN_36513`. Its bridge references resolve/dissociate handles rather
than encode root words. The [rev121 errata branch](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-1.14/eurasia_km_services4_srvkm_hwdefs_sgxerrata.h#L152)
selects22934,23944 and23410, not those feature gates. Thus this is not a
selected rev121 normal-root encoder. EMGD temporary-region-header resources
remain handles, as already established; no new payload contract was found.

EMGD `plb/sgx.h` provides 4096-byte parameter pages, cookies, feedback and scene
address integration, but no root-unit layout. The
[state3d header](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/independent-primary/drm_emgd_include_plb_state3d.h#L35)
explicitly warns of Napa-derived definitions. Its three-dword state objects
are not evidence for this scene reservation. Other Series5
`TE_RGNHDR_INIT_COMPLETE` names describe events, not EMPTY encodings. Modern
[Mesa Rogue register documentation](https://docs.mesa3d.org/drivers/powervr/csbgen.html)
was rejected for target field semantics; it is outside the SGX evidence tiers.

No independent source explains the current `3 × width × height` allocation
as hardware records. No source supplies an EMPTY predicate, first-reference
formula, continuation tag or same-mode normal-root encoder.

## Candidate rules tested against the established model

| Candidate | Matching evidence | Missing/conflicting evidence | Disposition |
| --- | --- | --- | --- |
| Status codes are root state tags | Labels mention empty/link/next region | Header explicitly makes them debug codes; no root offset or consumer | Rejected type substitution |
| Three words are control/head/tail | Allocation is three dwords per padded unit; generic words suggest links | No per-unit indexing, word roles or masks; debug dictionary does not explain size/base | Unqualified hypothesis; no rule adopted |
| Zero means EMPTY | Fresh backing zero-filled | No ISP predicate; hardware/reuse postcondition not exposed | Not established |
| CPU flat-list terminators define normal root | Same applicable hardware and consumer register family | No root-to-flat-list bridge or mode equivalence | Prior B retained; no normal rule |
| ClearClip WA encoder defines root | Region-header handle exists | Wrong rev121 feature gate; payload implementation absent | Rejected applicability |
| Napa three-dword states explain reservation | Some three-dword source structures | Different producer/consumer representation; explicit provenance warning | Rejected |
| Rogue region address field supplies transform | Similar vocabulary | Different architecture/register organization; no535corroboration | Rejected transfer |

The new independent candidates do not explain the complete combination of
base `S+0x1000`, both handoff registers, the768-byte allocation, separate
parameter memory and normal-mode consumer. None materially contradicts those
established facts. EMPTY, first successor, continuation and CPU-flat convergence
remain **UNKNOWN**, rather than false. No equation with invented masks or shifts
is adopted.

## Disposition and smallest next step

**C — PRIMARY_EVIDENCE_NOT_FOUND**. Successor **UNRESOLVED**; decoder
**PARTIAL**; common raster ABI remains **B**, not A or D. Useful terminology
narrows the location of a potential new source, but does not close a semantic
subset sufficient for classification B in the primary-evidence decision.

The next justified step requires **new permitted evidence**, not another pass
over this source set: a rev121 normal-root consumer specification or a proven
same-mode encoder contract. A narrowly targeted candidate is the actual code
implementing `MKTC_RH_EMPTY_TILE`, `MKTC_RH_STREAM_LINK` and
`MKTC_RH_NEXT_RGN_BASE`, with independent proof that its core/mode and inspected
payload correspond to the normal root in question. A trace label alone cannot
supply that proof. The vendor-forum
[document-title locator](https://forums.imgtec.com/t/pds-programming/333) is only
a locator; its document was not obtained, its permissions/revision are not
established, and its title is not a consumer contract.

GPU→CPU publication/stability remains **separately unresolved**. No decoder,
instrumentation, capture, build or rendering change is justified here. A new
live capture is not justified by this result. FIRE #2 contains no TA-output
memory, so coverage cannot be resolved retrospectively.

## Preservation and consistency

[Source consistency](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/source-consistency.json):30/30 PASS. These verify
source identities/notices, applicability gates, vocabulary and header scope;
they are not GPU qualification. The
[final consistency receipt](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/final-consistency.json) verifies all68 FIRE #2
files against both the task baseline and preceding preserved baseline,
pre-existing repository hashes except this handoff update, candidate byte
identities/Build IDs, document links, empty index and scoped document changes.

Repository changes are this report, its [structured result](fire02-independent-root-evidence-20261006.json), and
an appended continuation entry in [the handoff](CODEX-HANDOFF-20261005.md).
External audit copies/manifests preserve newly inspected sources. No
implementation changed; pre-existing dirty work is preserved. No new candidate:

* Driver Build ID: `9d0b5b2fdb7d9881f2828f43fb89253176c38817`.
* Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
* Image SHA-256: `ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.

FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`; task client
invocations0, SGX invocations0, hardware interactions0. Triangle
**NOT ESTABLISHED**. FIRE #2 completion/provenance findings are unchanged.
