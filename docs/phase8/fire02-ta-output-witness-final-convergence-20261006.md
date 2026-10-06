# FIRE #2 — final offline TA-output witness convergence, 2026-10-06

**TA_OUTPUT_WITNESS_OFFLINE_EXHAUSTED.** The identified available permitted
evidence graph is exhausted. Both format and publication remain independently
open. This is a successful final evidence disposition, not a finding of a
hardware defect, architectural impossibility, or different CPU/TA formats.
Further repetition over this source set is not justified.

The [route ledger](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/route-ledger.json)
accounts for fourteen routes, separating newly pursued routes, reused exhausted
findings, and implementations/specifications unavailable within the permitted
scope. Exhaustion is bounded to that graph; it is not a claim to have searched
every private, future or unidentified Internet archive. No excluded implementation
was recovered through another representation or source.

## Final classifications and irreducible evidence cuts

| Question | Final result |
| --- | --- |
| Normal-root successor rule | **SUCCESSOR_RULE_UNRESOLVED** |
| Selected-triangle decoder | **DECODER_FORMAT_PARTIAL**; no executable decoder justified |
| Common raster ABI | **B — COMMON_RASTER_ABI_STRONGLY_SUPPORTED_BUT_INCOMPLETE** |
| GPU→CPU publication | **C — GPU_CPU_PUBLICATION_UNRESOLVED** |
| Earliest/latest safe snapshot; stable window | **UNKNOWN**; no proved OPEN/CLOSE interval |
| Implementation gate | Case 4: both open; **no implementation change** |
| New candidate / live capture justified | **NO / NO** |

**F — format, UNKNOWN:** Under the selected normal rev121 raster controls,
what predicate and consumed width at `S+0x1000` distinguish EMPTY from a
successor, and how is that first successor address derived?

**P — publication, UNKNOWN:** At a specified rev121 checkpoint before
destructive DPM action/reuse, are the TA-produced bytes in owned
`S+0x1000..S+0x1300` backing externally committed and protected from later
relevant writes through a bounded CPU copy, including any required GPU
visibility operation?

F is the first format proof cut, not a promise that its answer automatically
proves downstream primitive equivalence. After obtaining F, any traversed
primitive representation would still need comparison with the established
CPU encoder and the selected triangle. P is independent of that comparison.

## Evidence graph audit and distinct routes completed

The existing [normal-root](fire02-normal-ta-root-grammar-20261006.md),
[successor](fire02-root-successor-rule-20261006.md),
[independent-source](fire02-independent-root-evidence-20261006.md),
[common-ABI](fire02-xpsb-common-raster-abi-20261006.md) and
[publication](fire02-scene-root-publication-20261006.md) reports were audited and
reused. Their completed searches were not repeated as new discoveries.

| Route | New work or reused scope | Result / evidence tier |
| --- | --- | --- |
| Event→raster ordering | Exact scheduler, selected Xpsb branch and frozen service sequence | Useful; execution handoff plus explicit consumer cache preparation; POULSBO_DRIVER_BEHAVIOR / DIRECT_REV121_EVIDENCE |
| Applicable Xpsb constant/xref neighborhoods | Selected scene switch and raster kick; trace-code/string locator in Xpsb/DRI | Useful; MMIO address handoff, no root payload predicate; DIRECT_REV121_EVIDENCE |
| PSB version comparison | Recovered previously identified 4.41.1 source RPM | Six relevant implementation/header files identical; no lost semantic clause; POULSBO_DRIVER_BEHAVIOR |
| Xpsb version comparison | Recovered indexed 0.18-5 source RPM | Same upstream archive and Xpsb/DRI ELF bytes; no additional producer/consumer version |
| TI history/event/cache route | All-ref metadata and targeted source/header families; earlier 816 permitted-blob format scan reused | Dictionary, reset and debug code; no implementing TA-finished/root parser source; SGX535_EVIDENCE_REVISION_UNPROVED / SERIES5_COMPARATIVE_EVIDENCE |
| Diagnostic users/version comparison | Three dictionaries and trace-buffer/stringification caller | Shared target codes agree; RENDERHALT group, not memory-field definitions; SERIES5_COMPARATIVE_EVIDENCE |
| Independent Samsung/OMAP variants | Twenty GPL sources and two SGX directory trees | Cache/reset, sync, resource handles; no normal-root encoder/parser or external-write completion contract |
| Register neighborhoods | SGX535 TI/EMGD/Samsung dictionaries plus historical MMIO context | PDS/MADD names corroborated; normal TA/ISP programming range not defined by these headers |
| Three-dword / independent encoder | Previous broad scan plus independent variants/version comparison | No independent hardware stride, empty tag, width or successor transform |
| Host/cache/publication primitives | Completed pinned-page, TTM, x86, fence and CPU-readback analysis reused | Host ownership and CPU maintenance; no target-specific GPU postcondition |
| Primary-source discovery | Bounded exact terminology/event/write-drain queries and known package/source leads | Known dictionary or no relevant contract; unrelated/non-primary results rejected |
| Unavailable/excluded routes | Vendor manual/firmware emitter absent; confidential branch/current excluded entry not accessed; no applicable SGX535 UM consumer identified | Unavailable is not evidence that a hardware property is false |
| Raster/FIRE #2 cross-proof | Same GPU address, selected consumer preparation and sealed completed lifecycle | Intended GPU handoff INFERRED; selected triangle and CPU publication UNKNOWN |

[Primary acquisition manifest](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/public-primary-manifest.json),
[new source hits](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/public-primary-final-hits.json),
[TI event-history ledger](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/ti-targeted-event-history.json),
[parser metadata locator](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/ti-parser-object-locator.json) and
[discovery scope](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/public-search-scope.json)
retain the bounded scope, paths, identities and negative results. No surviving
identified permitted route remains unexamined; routes requiring genuinely new
or excluded inputs are recorded separately.

## TA_FINISHED and the exact transition to raster

**CONFIRMED, SGX535_EVIDENCE_REVISION_UNPROVED:**
[SGX535 event definitions](../archaeology-data/H535.txt#L217) identify
`TA_FINISHED` as status bit13 (`0x2000`). The header does not say when the last
external root write is committed, which caches it orders, or whether all later
root writers cease. Therefore neither “logical completion only” nor “all root
writes globally visible” is established as its complete physical semantics.

**CONFIRMED, POULSBO_DRIVER_BEHAVIOR:** historical
[event dispatch](../poulsbo-data/PSB_psb_schedule_c.txt#L855) sends this bit to
the TA reply path. [TA done](../poulsbo-data/PSB_psb_schedule_c.txt#L421) marks
the scene DIRTY/COMPLETE, enqueues its raster task, clears the current TA task,
reports the applicable TA fence and invokes raster scheduling. This is a
software execution/ownership transition; the fence report is not a hardware
cache writeback implementation.

[Raster scheduling](../poulsbo-data/PSB_psb_schedule_c.txt#L334) checks scheduler
eligibility/engine occupancy, brackets an ISP reset where a scene exists,
submits the raster register list, and selects/binds the scene through
[psb_set_scene_fire](../poulsbo-data/PSB_psb_schedule_c.txt#L136) and the
[XHW request](../poulsbo-data/PSB_psb_xhw_c.txt#L148). Queue/request transport
may delay software execution; it is not a memory publication primitive.

**CONFIRMED, DIRECT_REV121_EVIDENCE:** applicable Xpsb scene switch uses the
selected normal/fresh branch (flags15, cookie14 zero). The helper at ELF
`0x4030` programs `0x65c=0`, `0x63c=3`, `0x658=0`, then at `0x41fc`
programs raster `0x408=S+cookie[7]=S+0x1000`. This branch does not take the
other context-switch state-store/load waits. Those optional waits must not be
promoted to a guaranteed normal-scene external write drain.

The [bounded disassembly](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/render-kick.disasm.txt)
at ELF `0x5040..0x515f` then performs, in order:

1. Write `0xad4=1`; wait for status2 `0x44`, clear/verify that completion.
2. Write `0xae0=1`; wait for status2 `1`, clear/verify that completion.
3. Write `0x804=(context value | 0x10000000)`; wait for status1
   `0x04000000`, clear/verify that completion.
4. Write `0x43c=1`, `0xa08=1`, `0x428=1`; read `0x428` back.

The [SGX535 definitions](../archaeology-data/H535.txt#L397) independently name
`0xad4` PDS_INV1/DSC and `0xae0` PDS_INV_CSC. Status1 bit26 is named
MADD_CACHE_INVALCOMPLETE. Exact historical writes/waits are direct evidence;
these symbolic correlations are SGX535 evidence with individual revision
field specificity unproved. None states that it commits the TA root to
external backing or freezes root/DPM writers. The MMIO readback is not a CPU
read of the scene payload. No BIF write-drain wait was identified in this
selected transition.

The [current service](../../tools/psb-dri-re/frozen_fixed_service.c#L199)
accepts the sequence-bound TA observation, begins raster, validates its plans,
executes ISP assert/clear, the26 register pairs plus WMB, then the same bounded
consumer preparation/kick. The [current plan](../../tools/psb-dri-re/frozen_kernel_contract.c#L602)
fails closed on waits. This confirms implementation sequence, not an
independent hardware publication specification. Its after-status tap is after
the implementation, including raster fire. The theoretical pre-raster point
is inside the status implementation after successful acceptance.

**Disposition:** explicit GPU consumer-cache preparation is present; the
completion event is implicitly used as readiness to advance the TA→raster
lifecycle. **Root-specific GPU→CPU PUBLICATION_NOT_ESTABLISHED.**

## Three visibility domains, kept separate

| Domain | Conclusion | Evidence level |
| --- | --- | --- |
| TA→raster | Same address and intended ordered producer/consumer use, with cache preparation, are established software behavior. A functioning common SGX-visible handoff is strongly supported; exact hardware root postcondition is not specified. | CONFIRMED sequencing; INFERRED hardware contract |
| TA→external backing | No source specifies TA_FINISHED, these cache completions, or another selected checkpoint as committing all relevant root writes to external memory. | UNKNOWN |
| TA→CPU | External commit plus no-later-writer requirement is missing; pinning and CPU maintenance cannot replace it. | UNKNOWN |

Raster consuming a GPU VA does not prove that a CPU read of owned backing at
the same time obtains faithful bytes. No internal forwarding/cache mechanism
is asserted to exist as an explanation; the evidence simply does not specify
the necessary external postcondition. No inference of cache symmetry is made.

The new [TI reset helper](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/ti-context/ecf0e6207c-sgxreset.c#L405)
explicitly waits for directory-cache invalidation by checking **outstanding
reads** reaching zero. Independently recovered
[Samsung reset source](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/public-primary/samsung-sgx_sgxreset.c#L199)
and OMAP reset code implement the same READS-mask check. It is neither a
TA-root write-drain contract nor an authorized operation to run. Debug dumping
that register supplies no stronger guarantee. Shared services command-cache
flags and sync-status writes require the missing microkernel implementation;
they do not expose the root publication postcondition.

The previous [publication analysis](fire02-scene-root-publication-20261006.md)
establishes pinned GEM/shmem pages, cached SGX PTEs, CPU CLFLUSH with barriers,
and host lifetime/reuse gates. Exact x86 `dma_rmb/dma_wmb` are compiler barriers.
Historical TTM waits/maps and rendered-pixmap reads establish narrower buffer
ownership behavior. These are GENERIC_X86/TTM_BEHAVIOR or POULSBO_DRIVER_BEHAVIOR,
not a transferable root-memory hardware guarantee.

## Parser, neighborhoods, versions and three-word hypothesis

**CONFIRMED, SERIES5_COMPARATIVE_EVIDENCE:** the
[status dictionary](/home/gama/sgx535-offline/phase8-independent-root-evidence-20261006T023140Z/ddk-1.14/eurasia_km_services4_include_sgx_ukernel_status_codes.h#L819)
groups `RH_*` diagnostics between RENDERHALT and RENDERHALT_END. Thus RH names
must not simply be interpreted as normal region-header field names. That
grouping does not establish an implementing function, core, payload or mode.
The dictionary is unconditional across cores.

[SGXUKernelStatusString](../../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L75)
creates switch cases that stringify the dictionary; the later debug routine
reads a separate EDM trace buffer. It is not the region parser. The actual
emitter/state machine is absent from the inspected permitted source trees.
Three available dictionary blobs have agreeing shared target codes. Neither
the four target debug-code values nor their diagnostic strings occur in the
applicable Xpsb/DRI ELF bytes. This bounded locator result does not prove that
no differently encoded implementation could exist.

Independent SGX535 headers omit authoritative fields for the `0x200/0x400`
programming range. Xpsb neighbors show TA scene/control inputs and raster mode,
root and bounds, but their ordering cannot define an unexposed memory parser.
The base remains exactly `S+0x1000`, with no software transformation. Software
control differences between the flat CPU path and normal TA path remain;
shared consumer hardware cannot erase the missing mode-specific grammar.

The [PSB version comparison](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/psb-version-comparison.json)
shows scene, scheduler, XHW, buffer, fence and register files in source4.41.1
are byte-identical to the prior pinned snapshot; `psb_drv.h` changes only
date/version metadata. The
[Xpsb comparison](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/xpsb-version-comparison.json)
shows indexed0.18-5 has the identical upstream tar and the same Xpsb and DRI
ELFs. Later packaging therefore supplies no independent encoder/parser.
Samsung/OMAP older services variants and available TI history add resource
handles, cache requests and trace vocabulary, not missing root fields.
The TI reset helper retains an explanatory outstanding-reads comment omitted
by the two independent variants; their actual zero/READS-mask predicate remains.
This recovered comment concerns directory-cache invalidation, not TA payload
writeback. Two initial consistency assertions incorrectly required that literal
comment in the independent files; the original check record is retained and
the corrected assertions verify the actual predicates and TI comment separately.

`3 × 64 dwords` remains confirmed **allocation arithmetic**. No independent
source establishes a hardware unit of three words, individual12-byte indexing,
or roles for `word0/1/2`. This hypothesis is **unsupported**, not disproven.
There is no adopted EMPTY predicate, POPULATED tag, consumed width,
successor-address equation or continuation/termination rule. Zero-fill,
flat-list terminators, debug-code suffixes and communication-BO parsing do
not substitute for them. No same-mode encoder closes route B.

Primitive convergence and selected-triangle recognition consequently remain
UNKNOWN. Implementing a normal-root decoder or synthetic fixtures would encode
invented semantics. Native/UBSan/i386 decoder qualification was not run because
no decoder was justified. No live capture or opaque dump was implemented.

## Snapshot candidates and downstream constraints

| Candidate | Why no safe OPEN event is established |
| --- | --- |
| Accepted TA before raster | Earliest theoretical checkpoint; target external commit/all-writers postcondition missing |
| After PDS/MADD preparations | Relevant operations present; their completions do not document root write publication/stability |
| End-render | Historical scheduler explicitly allows DPM deallocation still busy |
| 3D-memory-free | Reclamation event does not document preservation of original root contents |
| Retirement | Software release eligibility adds no root-specific hardware visibility operation |

There is no earliest or latest **safe** snapshot point. Host retention before
release/remap/reuse/invalidation is an upper lifetime constraint, not a proved
stable window. A future valid OPEN→visibility→copy→CLOSE contract must retain
page/VA ownership and operation attribution, exclude later relevant GPU/CPU
writers and invalidation, and fail closed on partial completion, interference,
cache/mapping uncertainty or evidence loss. A reset or extra store/clear cannot
be inserted merely to manufacture the missing fact.

**CONFIRMED:** FIRE #2 completed the accepted configured lifecycle and retained
its attributable all-zero color result. A fault that prevented those accepted
events did not prevent that observed result. **UNKNOWN:** whether the normal
root held any selected primitive or whether every root field was valid. Empty,
skipped or otherwise nonproductive raster work can remain compatible with the
observations; no hardware rule excluding those possibilities was recovered.
“Raster finished” therefore does not prove a nonempty structurally valid root,
the intended triangle, fragment export, or a CPU-visible TA representation.

FIRE #2 retained no TA-output memory. No independent retained artifact exposing
the required geometry was found in the audited preserved-evidence findings.
Its coverage cannot be reconstructed retrospectively. The68 sealed originals
are preserved, with no new interpretation claiming hidden payload bytes.

## Exact new evidence required and final stopping boundary

The next justified action is to obtain **new permitted primary evidence**, not
repeat this source search, build an uninterpretable capture, or invoke SGX:

* For F: an applicable rev121 normal-mode consumer definition or independently
  proven same-mode encoder specifying the first-position state predicate,
  consumed width and successor transform. Actual code behind the RenderHalt
  diagnostics is useful only with permission and independent proof of core,
  mode and payload applicability; the trace dictionary alone cannot qualify it.
* For P: an applicable rev121 event/memory specification or authoritative
  implementation contract defining root external commit, required visibility
  operation and cessation of writes before destructive reclamation through
  the CPU-copy interval. GPU-only visibility is insufficient.

A single legitimate source could cover both. Availability/authenticity of
private vendor-document locators is not assumed. New evidence must be checked
against current controls, addresses, ownership and historical sequencing before
either classification changes. No contract relaxation or speculative parsing
is proposed. Another live capture is **not justified** by this disposition.

## Preservation and checks

Only this report, its [structured disposition](fire02-ta-output-witness-final-convergence-20261006.json)
and the latest continuation in [the handoff](CODEX-HANDOFF-20261005.md) change
repository documentation. Acquisitions, extracts, hashes, route ledger and
CPU source/archive/reference checks are in the audit directory. Those checks
are not decoder or GPU qualification; no build or candidate execution occurred.

[Consistency checks](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/source-consistency.json) and
[final preservation receipt](/home/gama/sgx535-offline/phase8-ta-witness-final-20261006T031833Z/final-consistency.json)
verify references, all68 sealed FIRE #2 files, pre-existing implementation and
candidate bytes, and the empty index. Existing dirty work remains intact.
The focused CPU source/archive checks finish **29/29 PASS**; no decoder,
native/UBSan/i386 test suite, GPU qualification or build was run.

Unchanged identities:

* Driver Build ID: `9d0b5b2fdb7d9881f2828f43fb89253176c38817`.
* Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
* Image SHA-256: `ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d`.

No new candidate. FIRE #3 **UNAUTHORIZED**; `sgx_execution_authorized=false`;
task client invocations0; task SGX invocations0; hardware interactions0;
triangle **NOT ESTABLISHED**. The final outcome is
**TA_OUTPUT_WITNESS_OFFLINE_EXHAUSTED**.
