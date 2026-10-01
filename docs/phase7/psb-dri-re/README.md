# `psb_dri.so` static investigation

**P7H-060 clean-room decision:** [final contract analysis](cleanroom-final-closure.md)
and [checked domain tables](cleanroom-final-rules.csv) separate stricter software
policy from unresolved SGX535 hardware postconditions. The old PDF is not a
project prerequisite. R1/R2/R3 and FG-02 remain OPEN.

**P7H-059 public-document hunt:** [archival result](sgx535-pds-document-hunt.md)
and [provenance registry](sgx535-pds-document-sources.json). The exact
`Eurasia.3D Input Parameter Format` SGX535 PDF and an applicable public PDS
encoder remain unrecovered; R3/B1/L12 stay OPEN.

**Route C selected (P7H-036/037):** begin with
[cleanroom-backing-contract.md](cleanroom-backing-contract.md), its
[exact CPU image](cleanroom-pds-image.csv), and
[transition obligations](cleanroom-backing-transitions.csv). The new
[checker](../../../tools/psb-dri-re/cleanroom_pds_image.py) proves parameterized
CPU initialization, not GPU visibility or a read bound. Historical provider
identity no longer blocks Route C; complete launch-state coverage remains the
next inference premise. FG-02 OPEN, Gate B BLOCKED, ISA UNKNOWN.

This track extends the in-progress [Phase 7](../README.md) source review. It analyzes the already recovered `xpsb-glx` 0.18 package's i386 DRI library without executing or modifying it. The package spec states `Redistributable, no modification permitted`; no binary or decompiled implementation is copied into new driver code.

Exact target: `references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so`, 2,921,100 bytes, SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`. The path, `stat`, `file`, and hash were checked at the start of this track. See [binary inventory](binary-inventory.md) for ELF metadata and package provenance.

Work is checkpointed in this order:

1. A: binary identity, package terms, ELF sections/imports/strings.
2. B: DRI ABI and the graph rooted at `__driDriverExtensions`.
3. C: callback roles and generic Mesa versus Poulsbo boundary.
4. D: DRM/libdrm calls and historical request structures.
5. E: statically supported command path and missing links.
6. F: SGX/PDS/USE/USSE and embedded-program evidence.
7. G: private structures, initialization, cross-check with P7-001–P7-022, and unknown reduction.
8. H: final byte-hash, source links, CSV, and gate validation.

The original Ghidra GUI project is not modified. Automated exports use a separate project and bounded high-value decompilation. This track does not authorize hardware observation: FIRST-OBSERVATION-DESIGN and Gate B remain BLOCKED; whitelist `[]`.

The current reports are [ELF inventory](binary-inventory.md), [DRI extension graph](dri-extension-map.md), [Mesa/driver boundary](mesa-vs-poulsbo.md), [direct requests and libdrm imports](ioctl-map.md), [static command path](command-submission.md), [SGX/PDS/USSE findings](sgx-specific-findings.md), [incremental structures](structures.md), [initialization and dependencies](initialization-and-dependencies.md), [remaining unknowns](unknowns.md), and the [static final report](final-report.md). The CSV maps and bounded Ghidra exports live beside these notes under `analysis/`; the decompiled function bodies stay in a disposable `/tmp` project, not in this repository. [Read-only scripts](../../../tools/psb-dri-re/) reproduce the extraction. P7B-001–P7B-014 identify new source-scoped rows in the [evidence matrix](../../evidence-matrix.csv).

The later P7E pass adds the [indirect draw/finalization trace](draw-to-submit.md), [control-flow edges](draw-control-flow.csv), [format inventory](format-readiness.md) and [field map](format-fields.csv). Its strict [clean-room readiness decision](clean-room-readiness.md) is **NOT YET SPECIFIABLE**. P7E-001–P7E-007 in the evidence matrix scope those claims.

The P7F [bounded scene/program/TA trace](bounded-path-closure.md) and [validation/bootstrap trace](validation-bootstrap-closure.md) narrow one conditional path and explicitly correct the USE/USSE-versus-PDS buffer label. The [pre-implementation decision](preimplementation-decision.md) remains Result B and ranks exact next static sources. P7F-001–P7F-009 identify its evidence. No historical binary or hardware was executed.

The P7G [frozen triangle](frozen-draw-closure.md) follows one software-vertex, primary-color indexed draw. It includes [17 state atoms](state-atoms.csv), [selected formats](frozen-draw-formats.csv), [BO/relocation/fence/bootstrap constraints](frozen-draw-objects.md), and a [precise remaining-blocker table](frozen-draw-blockers.md). It closes the historical frontend edge and corrects vertex-width and committed-length interpretations; it does not close the fragment/compiler or full scene/bootstrap contract.

The focused follow-up [fragment compiler checkpoint](frozen-fragment-compiler-checkpoint.md) resolves the frozen MOV's UniFlex input register classes and swizzle, excludes one conditional extra input record, and locates scalar lowering and instruction counting. The later [exact selected-output trace](frozen-fragment-exact-output.md) closes FG-01 for that input: dead-code elimination removes the four scalar MOVs, leaving zero main and secondary USSE instructions and derived compiler-result metadata. The [link checkpoint](frozen-fragment-link-checkpoint.md) records conditional field formulas; the selected linked object and pixel state remain FG-02. P7H-001/P7H-002/P7H-004/P7H-006/P7H-010/P7H-011 in the evidence matrix scope these findings.

The [frozen-draw static decision](frozen-draw-static-decision.md) is the latest Result B assessment and distinguishes bytes still recoverable from the retained compiler from data whose consumption or exact stack provenance is unproved. The [Xpsb trace](../xpsb-re/xhw-init-frozen-draw.md) records the candidate XHW operation-zero and scene-info/bind-fire mapping, and shows that both scene-info option branches converge for the frozen 32×32 target (P7H-005/P7H-008).
# Selected fragment-link continuation

The [PDS lifecycle report](pds-buffer-lifecycle.md) now separates recycled
slot contents from a conditional fresh-backing image. P7H-025–027 trace
allocation through submission/fence reuse; P7H-028 identifies zero-filled
new PDS TTM pages in the candidate kernel. Exact pairing and first-use
preconditions remain open; no read-set or Gate B promotion follows.

The [FG-02 trace](frozen-fragment-link-progress.md) distinguishes the empty FG-01 compiler output from an eight-byte link-time suffix. It also identifies the selected secondary fast path and the remaining primary PDS/pixel-state gaps. The first-triangle decision remains Result B and Gate B remains BLOCKED.

The [selected PDS read-set ledger](pds-readset-search-ledger.md) records exact-word and encoding searches, related retained emitters, rejected source leads, and the remaining source-operand question. The read-only [literal survey](../../../tools/psb-dri-re/pds_literal_survey.py) reproduces the immediate-word locations from the retained ELF. P7H-016–018 cover literal reuse and the outbuf's possible reuse without zeroing; none establishes the PDS read set.

The subsequent [external Series5 PDS evidence result](series5-pds-external-evidence.md) records the public code, package, archive, independent-research and patent routes inspected for an SGX535-applicable source-selector rule. No provenance-qualified decoder or instruction-field rule was recovered. The `+00/+04/+20` read set remains **INFERRED**, all nine producer-unwritten dwords remain possible reads, and FG-02 remains open.

The [curated instruction corpus](pds-instruction-corpus.csv), [field-constraint regression](pds-field-constraints.csv), and [comparative analysis](pds-comparative-encoding.md) narrow the `0x07000345` source-index hypothesis (P7H-019/020). The checked CPU formulas and literals do not establish GPU reads; all nine primary data holes remain UNKNOWN and FG-02 stays open. [The analysis script](../../../tools/psb-dri-re/pds_field_constraints.py) verifies the retained hashes and literal bytes without loading either ELF.

The [semantic corpus](pds-semantic-corpus.csv), [differential matrix](pds-differential-matrix.csv), [bit-influence table](pds-bit-influence.csv), and [hypothesis audit](pds-hypotheses.md) refine the `0x45` arithmetic model using the mixed-event counterexample (P7H-021/022). [The reproducible CPU-only analyzer](../../../tools/psb-dri-re/pds_semantic_constraints.py) explicitly leaves all GPU source/read fields UNKNOWN. An alternative with a fixed extra source still fits every retained CPU emission; no hole is proved dead, FG-02 remains OPEN, and Gate B remains BLOCKED.

[Experiment #43](pds-experiment43.md) searches for a new natural variation
outside those 42 rows and checks one quarantined older related package. The
[static locator](../../../tools/psb-dri-re/pds_experiment43_survey.py) and
[retained-store CSV](pds-experiment43-retained-stores.csv) show no new verified
`0x45` variant or terminal-word neighbor. Event bits 16–17 are invariant in
their literal producer. The CPU evidence still cannot distinguish an extra
fixed read; FG-02 remains OPEN and Gate B remains BLOCKED.

The [PDS backing-memory lifecycle](pds-buffer-lifecycle.md) and
[per-dword provenance](pds-buffer-byte-provenance.csv) trace the selected
allocation through the mapped 30-slot batch pool, cursor commit, BO wrapper
release and fenced recycling. The nine historical holes can inherit stale
contents. A clean-room full-range write is physically supported by the
allocator layout, but no selected PDS semantic rule proves an arbitrary fill
functionally equivalent. FG-02 remains OPEN.

The P7H-029–031 [pairing and first-use proof](pds-stack-pairing.md) adds a
[field compatibility table](pds-abi-pairing.csv) and an exact
[first-use timeline](pds-first-use-timeline.csv). It establishes candidate
BO/fence source ABI equality, candidate same-page zero backing, and untouched
selected ranges at slot0x160/0x1c0. Application to the target remains
conditional on its backing provider; PDS semantics stay UNKNOWN.

P7H-032–035: [backing-provider audit](pds-backing-provider.md),
[candidates](pds-backing-candidates.csv), [paths](pds-backing-paths.csv), and
[source comparisons](pds-backing-file-comparison.csv) narrow the last premise
to target membership in the audited fresh-zero/preserved-contents family.
The exact OBS package lineage is established; deployment binding is not.

The [target-provider restart check](pds-target-provider-identity.md) and
[discriminator table](pds-target-provider-discriminators.csv) identify the next
missing input as the intended supplying-provider selection, with an attributable
offline deployment/build artifact or concrete replacement implementation. They
also reconcile the original TVZ manifest with a later language-only annotation.
P7H-035 remains unresolved; no existing proof is promoted.

P7H-038/039: [selected launch-state coverage](pds-launch-state-coverage.md) records CPU launch packing and the unproved L12 launch-definedness rule. The checked inventory stays OPEN; no PDS semantics or GPU backend claim is promoted.

P7H-040: [middle-word count versus data size](pds-launch-count.md) records resource-derived Q−1 arithmetic, scoped differential cases and the unchanged L12 source-eligibility boundary.

P7H-041: [selected source availability](pds-selected-source-availability.md) adds evidence-scoped temporary questions, source/control separation and negative closure regressions. L12 remains OPEN without asserting an outside read.

P7H-042–046: [frozen triangle specification](frozen-triangle-spec.md), [objects](frozen-triangle-objects.csv), [relocations](frozen-triangle-relocations.csv), [blockers](frozen-triangle-blockers.csv). L12 frozen OPEN; scoped TA68-byte CPU order recovered; no hardware execution.

The [selected service requests](frozen-triangle-service.csv) use op2 for both TA and raster. Auxiliary CPU images now have explicit zero policy; neither advance closes consumed-state coverage or bootstrap requirements.

[Final triangle burn-down P7H-047–051](frozen-triangle-burn-down.md): concrete BO,
wire relocation and target models; B2 scoped closure; B1/B3/B4 remain explicit.

[P7H-052–054 closure attempt](frozen-triangle-burn-down.md) adds the shared publication/readiness/source-domain ledger. Static triangle remains PARTIAL; no architectural postcondition was promoted from poll success.

[P7H-055–057 three-rule continuation](frozen-triangle-burn-down.md): LOAD3 forward/IRQ trace, fresh-completion observation guard, five canonical PDS families and explicit remaining model discriminators. Static closure remains open.
