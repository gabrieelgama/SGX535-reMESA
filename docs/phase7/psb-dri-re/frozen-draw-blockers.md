# Frozen draw: remaining blockers

**Current Route C decision (P7H-036/037):** historical target-provider
membership is no longer an implementation prerequisite. The
[clean-room contract and checker](cleanroom-backing-contract.md) establish the
parameterized CPU image, including zero holes. The next inference premise is
complete deterministic PDS launch-state coverage, not an exact read set or
historical installation identity. Concrete GPU-provider conformance is still
required. FG-02 remains OPEN; no ISA or Gate B status changes.

**RESULT B — MINIMAL CLEAN-ROOM 3D USERSPACE PATH NOT YET SPECIFIABLE.** The target remains the [single indexed triangle](frozen-draw-closure.md). This is a checkpoint of the remaining work, not a claim that available static evidence has been exhausted. No implementation-grade specification or Phase 8 implementation plan exists.

**FG-01 is closed for the frozen compiler input.** The [selected-path trace](frozen-fragment-exact-output.md) derives zero main and secondary USSE instructions. [FG-02 progress](frozen-fragment-link-progress.md) now derives the distinct eight-byte link-time suffix and the zero-input secondary path. Full pixel state and the later blockers remain open, so Result B is unchanged.

Source classes in this table follow the current investigation: **A** retained DRI ELF; **B** retained Xpsb ELF; **C** recovered DDX; **D** recovered libdrm; **E** recovered kernel; **F** package patches/build metadata; **G** another identifiable historical package; **H** archived SGX535/DDK material. “Hardware required” refers to recovering the missing specification, not eventual validation of a driver.

| blocker | exact required information | why required | last producer → consumer; locations | A–H inspected / exhausted | next identifiable source | static recovery possible | hardware observation required |
|---|---|---|---|---|---|---|---|
| FG-01 — CLOSED | Main and secondary instruction counts are both `0`; both byte streams are empty; no physical instruction registers are allocated. Compiler result `+0xb8=0x2`, `+0xb4=0`, `+0x1c8=1`, with other selected fields in the exact-output report. | The historical compiler's selected output is now specified, but linking and pixel state remain separate. | `0x00033490` → `0x001c3ba5/0x00219998` → DCE `0x001deecc/0x001dce31` → finalizer `0x001c95f7` → emitter `0x001fb7af/0x001fa3ea` | A selected-path proof [P7H-010/P7H-011](frozen-fragment-exact-output.md); B–H do not supply or validate physical hardware behavior. | FG-02 fragment linking and scene pixel state | YES; closed for the frozen CPU input | NO |
| FG-02 — PARTIAL | Remaining pixel-state key/target references and producer-unwritten primary PDS dwords; link-time suffix and secondary fast path are exact for the frozen conditions | The linked suffix alone does not specify the scene pixel state | `0x000370d8` → `0x000264b7` → `0x00025970`; `0x00028559` → `0x00039ea0/0x0003faa3` | A: [selected linker trace](frozen-fragment-link-progress.md) establishes `00 00 00 00 40 01 04 f8` suffix and secondary `0xaf000000`; primary PDS committed-range holes and target-dependent keys not exhausted | A selected scene pixel builders and PDS consumption; H applicable PDS semantics if available | YES for further static tracing; instruction consumption uncertain | NO demonstrated need for byte construction |
| FG-03 | Whether the six producer-unwritten vertex PDS dwords and nine producer-unwritten selected fragment-primary PDS dwords are read by their instructions | An implementation cannot fill possibly consumed words by guess or claim exact semantics from literals | Vertex `0x00040355`: holes `+0x08/+0x0c/+0x18/+0x1c/+0x28/+0x2c`; fragment `0x00025970`: holes `+0x08…+0x1f/+0x24…+0x2f` | A selected emitters, clear analogue `0x00039ac4`, relocation and nonzeroing outbuf checked; B `Xpsb_pds_get_num_constants` at `0x7470` constrains store extent but not instruction operands; H SGX535 register headers lack PDS decode. [Exact boundary](selected-pds-hard-boundary.md). | Restricted first-use alternative: establish the actual backing provider's [candidate-equivalent zero-page contract](pds-stack-pairing.md); no-overlap is now proved. General recycled operation still has unresolved semantics. | CONDITIONAL for frozen first-use image; read set remains UNKNOWN | No hardware action authorized or required merely to authenticate source pairing |
| FG-04 | Required scene state words before the bounded index record, including target/background program data and the 52-word raster list's dynamic inputs | A correct five-word index record is not a complete scene | `0x00027060`, `0x00029251`, `0x000287ff`, `0x00029385`, `0x00039c5c`, `0x0003b73b`, `0x0002ab70` → finalizer `0x0002a39a` | A scene order and no-depth reductions checked; selected target/program fields **not exhausted**. E register-list consumer located. | A scene bit-1/target/background branches for the frozen 32×32 color surface | YES, candidate route present | NO demonstrated need for byte production |
| FG-05 | Complete BO/suballocation inventory and each selected flags/mask/presumed-offset constraint, with the resulting relocation list and fence ownership/wait schedule | Known relocation equations cannot supply missing source objects or establish when they may be reused | Pools `0x00048592/0x0004297b`; list `0x000443d1`; submit `0x00037b51`; fences `0x000442bd/0x00044097/0x00044107`; E `psb_sgx.c`, `psb_regman.c`; D `xf86drm.c` | A/D/E offset and USE algorithms, fence wrapper distinction checked; **full selected object set not exhausted**. F exact linked build absent from inspected metadata. | A scene/program allocation users after FG-01/02/04; D/E validation/retirement consumers | YES for family algorithms; exact built equivalence UNCERTAIN | NO for static interface; actual completion/recovery still requires separately authorized validation |
| FG-06 | Selected XHW bind/fire/raster request/reply fields, resource lifetimes and which retained Xpsb init outputs are prerequisites | Candidate scheduler explicitly requires initialized XHW service | E `psb_xhw.c:42–63,80–112,146–175,416–460`, `psb_schedule.c:199,313–328` → B XHW service; B `XpsbInit 0x2690`, `Xpsb_scene_info 0x3a40` | A–F lifecycle/source-family evidence checked; operation-zero XHW init fields and operations 1/2 correlated (P7H-005). For frozen 32×32, both revision-option branches yield the same scene-info cookie words 0–14 and size (P7H-008); word 15 is unwritten here but is not read in the candidate initial successful bind/fire branch (P7H-008). **Full selected B/E request/reply and service lifetime not exhausted.** H Services init is a different interface, not a substitute. | Retained B XHW dispatch and candidate E scene/raster reply handling; existing C DDX lifecycle | YES for software protocol; physical initialization contract UNCERTAIN | UNCERTAIN for remaining hardware prerequisites; not required merely to read source |
| FG-07 | Exact built DDX/libdrm/kernel compatibility or a complete compatible-family contract covering every used field | Historical interoperability cannot be asserted from a version string alone | Shared `5.0.1.0046`; loader spelling mismatch; older libdrm PSB header. XHW ioctl direction *differs*, but the candidate core dispatches by request number and accepts caller-supplied input, so that difference alone is not a candidate-source incompatibility. | A–F identified metadata/headers inspected; candidate `drm_drv.c:607–659` and `psb_xhw.c:416–470,516–528` close the narrow XHW-direction concern. Exact manifest **not found**, not proved unavailable. G no newly identified matching binary. H does not authenticate package pairing. | Exact retained-stack build manifest or linked `libdrm.so.2`/DDX/module; otherwise finish behavioral ABI comparison | UNCERTAIN; release-family agreement established | NO: hardware success would not authenticate source provenance |

The exact historical GL entry, record-width interpretation, and second reserved termination dword are **not blockers now**. P7G-001/002/010 close or correct those questions. The fence's 40-versus-48-byte structures are also **not a demonstrated ABI conflict** (P7G-012). Dynamic GPU addresses are specified by allocation/relocation relationships, not missing fixed constants.

The current restricted first-use route depends on the target backing provider implementing the [proven candidate fresh-zero/same-page contract](pds-stack-pairing.md). P7H-029–031 discharge the predecessor-overlap question and establish candidate generic ABI equality; the retained version check does not identify the supplying implementation. This is an alternative to resolving the complete PDS read set. Target-dependent pixel-state words remain open. The clear helper `0x00039ac4` repeats two PDS literals but is not a proven substitute for a normal fragment program. FG-04–07 remain open; their static traces do not override FG-03 or the separate hardware gate.

P7H-019/020 further constrain FG-02/03 with a [42-row retained instruction corpus](pds-instruction-corpus.csv) and [differential check](pds-comparative-encoding.md). The `0x45` pair-index model predicts reads from initialized `+0x00/+0x04/+0x20`, but it is **INFERRED**, not an instruction decode. The exact `0x45` read count and terminal-word behavior remain **BLOCKING-UNKNOWN**; none of the nine holes has been proven dead. The older blocker table's A–H scope and Result B remain valid.

P7H-021/022 add a [semantic corpus and counterexample audit](pds-hypotheses.md). A three-field arithmetic decomposition now fits `0x070b0345` as well as the primary and secondary `0x45` words, but a model with a fourth fixed source remains observationally identical to all retained CPU stores. The standalone terminal word is a CPU program-grammar fact, not a no-read guarantee. FG-02/03, Result B, and Gate B remain unchanged.

[Experiment #43](pds-experiment43.md) finds no independent `0x45` source-count
variation in the retained ELFs or a quarantined older related build. It proves
bits 16–17 of the mixed-event word are literal/invariant at the retained
producer, without decoding them. FG-02/03 remain open; all nine primary holes
remain possible reads.

The [PDS buffer lifecycle](pds-buffer-lifecycle.md) now establishes a
specific stale-content path: a 30-slot pool returns fenced/released slots
without clearing their mapped bytes. The selected 56-byte commit records
length and advances a cursor; its nine holes may inherit prior slot values.
All 56 bytes are inside a CPU-writable reservation with host metadata stored
elsewhere, so deterministic clean-room bytes are constructible at the memory
level. Functional equivalence of an arbitrary fill for potentially read PDS
slots is **not established**; FG-02/03 and Gate B remain open/blocked.

No hardware-safety UNKNOWN was eliminated. Physical SGX revision, power/clock/reset state, safe access semantics, faults and recovery remain separate from these CPU-side reconstruction questions. **Gate B BLOCKED; whitelist `[]`; HARDWARE FUNCTIONALITY: UNVERIFIED.**

P7H-029–031 narrow FG-03/07 together: for the successful new-context first
scene, all nine selected fragment holes are untouched before the primary
builder, at slot0x160 or0x1c0. Candidate source ABI and zero-page propagation
are checked, but the target provider's implementation remains unidentified.
The decision is CONDITIONAL, with a specific allocation/visibility-contract
condition rather than an unresolved allocation order. FG-02/03 are not closed
for the unqualified target. See [pairing decision](pds-stack-pairing.md).

P7H-032–035: the [provider inventory and path audit](pds-backing-provider.md)
now excludes stolen fallback and proves a bounded historical source-family
contract. The remaining provider question is target membership in that class,
not the generic header layout or first-use order. No installed old-provider
identity or equivalent replacement implementation is present in target evidence.
FG-02/03 retain their conditional first-use dependency; Gate B is unchanged.

## Launch-state coverage refinement (P7H-038/039)

See [the selected launch-state envelope](pds-launch-state-coverage.md) and
[its checked inventory](pds-launch-state.csv). The CPU launch words are now
parameterized explicitly: `R(S), 0x00030000, R(P)|0x0c000000`. This does not
make the packed count a hardware read bound. The next rule is **L12**: the
initialized architectural state domain for this count/launch, including the
eligibility of other state for pre-definition reads. DS, temporary and implicit
coverage remain UNKNOWN; no outside read is asserted. All56 CPU bytes stay
deterministic, first-use no-overlap stays CONFIRMED, GPU preservation stays
CONDITIONAL. The implementation dependency has not closed; FG-02 remains OPEN.

## Middle-word field clarification (P7H-040)

[The narrow count analysis](pds-launch-count.md) distinguishes two unrelated
encoded3 values: middle-word `(Q-1)<<16` is resource-derived (Q=4 selected),
whereas primary-reference `(data_dwords&0xfc)<<24` is the CPU data-size field.
Q changes with USE temporary/attribute demand independently of that size input.
Its architectural unit remains UNKNOWN; resource-group interpretation INFERRED.
Neither field establishes source eligibility. L12 remains OPEN: whether the
selected launch makes state outside its initialized48-byte-prefix domain
unavailable until defined. No outside read is asserted. P7H-036–039 remain
unchanged; all26 earlier tests are retained, and GPU preservation stays
CONDITIONAL. Do not repeat the middle-word count survey to answer L12.

## Source availability and evidence scope (P7H-041)

[The selected availability analysis](pds-selected-source-availability.md)
distinguishes source eligibility from CPU storage, control metadata and output
destinations. The existing public SGX535 description supports temporary-store
class plausibility but supplies no selected access/reset rule. No actual outside
read or cross-launch persistent source was established. The next discriminator
is the complete pre-definition source domain of the two selected forms, confined
to deterministically established launch state. Naming a particular unknown
selector bit or asserting a temporary hazard is not supported.

The extended24-row inventory retains old provenance and adds role/availability
columns. Six new checker regressions preserve the existing36; source blockers
and control obligations are now derived separately. Inventory completeness is
not architectural completeness. L12/FG-02 stay OPEN; GPU preservation remains
CONDITIONAL; Gate B BLOCKED; whitelist []; hardware UNVERIFIED.

## Current independent closure map (P7H-042–046)

L12 is OPEN/FROZEN for this pass. [The current specification](frozen-triangle-spec.md)
and [machine-readable blockers](frozen-triangle-blockers.csv) separate auxiliary
program coverage, target construction, complete BO validation and replacement
service from L12 and backend preservation. The scoped68-byte TA order is now
specified, including11-word bounds and14-word triangle state uploads. Earlier
compiler/target/provider blocker prose below or above records its historical
checkpoint; use the new map for current work. FG-02 remains OPEN.
