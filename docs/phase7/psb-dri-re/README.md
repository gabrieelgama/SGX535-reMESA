# `psb_dri.so` static investigation

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

The focused follow-up [fragment compiler checkpoint](frozen-fragment-compiler-checkpoint.md) resolves the frozen MOV's UniFlex input register classes and swizzle, excludes one conditional extra input record, locates the selected scalar lowering branch and transformed-program instruction-count producer. The [link checkpoint](frozen-fragment-link-checkpoint.md) records conditional field formulas. Neither derives the final USSE words. P7H-001, P7H-002, P7H-004 and P7H-006 in the evidence matrix scope these additions.

The [frozen-draw static decision](frozen-draw-static-decision.md) is the latest Result B assessment and distinguishes bytes still recoverable from the retained compiler from data whose consumption or exact stack provenance is unproved. The [Xpsb trace](../xpsb-re/xhw-init-frozen-draw.md) records the candidate XHW operation-zero and scene-info/bind-fire mapping, and shows that both scene-info option branches converge for the frozen 32×32 target (P7H-005/P7H-008).
