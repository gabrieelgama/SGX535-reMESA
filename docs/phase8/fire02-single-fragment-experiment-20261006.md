# FIRE #2 zero readback — one fragment experiment, 2026-10-06

**EXPERIMENTAL_HYPOTHESIS_PREPARED.**

**EXPERIMENTAL_HYPOTHESIS — NOT ESTABLISHED:** the frozen suffix-only
fragment does not establish the intended nonzero packed color result; an
explicit historical constant-color fragment is sufficient to restore visible
output without changing geometry, TA, PDS launch, ISP, PBE or surface state.
The sufficiency prediction is testable. FIRE #2 does not establish this cause.

The experimental candidate changes exactly **eight foreground fragment USE
bytes**, replacing `00000000 f8040140` by `001f00ff fca7f1f1`, the historical
clear constructor's packed opaque-magenta `0xffff00ff` output. No invocation
was made. FIRE #3 remains **UNAUTHORIZED**. Triangle **NOT ESTABLISHED**.

[Qualification and exact identities](fire02-single-fragment-experiment-qualification-20261006.json).
[Audit directory](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/).
[Offline candidate manifest](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/experimental-candidate.json).

## Stage A: canonical state and new causal evidence

The prior [zero-readback audit](fire02-zero-readback-investigation-20261005.md)
is reused for compiled payload equality, address checks and preserved
completion/provenance. The exhausted TA-root/publication graph was not searched
again. New bounded work traced the historical **color-producing** paths:
Xpsb composite shader/pixel-event/launch, DRI constant-clear builder, its
immediate constructor/default flags/destination bank/END finalizer, and its
normal indexed-TA and scene-color callers. No historical ELF was executed;
Ghidra projects were opened read-only. The corrected independent i386 tests
execute newly compiled CPU code, not either historical driver or GPU code.

The [canonical object specification](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/frozen-scene-canonical.json),
[every emitted field table](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/canonical-state-fields.csv)
and [complete address/relocation plan](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/frozen-address-plan.json)
retain the full frozen state, including all 26 raster pairs, 49 relocations,
state uploads, program words, background and event programs. Symbolic GPU
addresses are distinguished from synthetic test VAs: FIRE #2 did not retain
individual live BO address receipts. Allocator/source formulas establish
construction, not an independently observed live address or hardware effect.

Here `CONFIRMED_CORRECT` means the stated CPU input/arithmetic, never that
retirement validates rendering semantics. `HISTORICALLY_SUPPORTED` means a
matching historical producer; `SUSPICIOUS` is a causal lead; `UNKNOWN` is a
missing hardware/intermediate fact.

| Stage / field | Canonical FIRE #2 state | Classification and implication |
|---|---|---|
| Geometry | `(8,8,.5,1)`, `(24,8,.5,1)`, `(8,24,.5,1)`; RGBA `(1,1,1,1)` at each vertex; indices `0,1,2` | CONFIRMED_CORRECT selected CPU inputs: nondegenerate area128, all inside32×32; hardware-transformed survival UNKNOWN |
| Vertex layout / PDS / USE | 96bytes, stride32, width8, position/color4f; four-word vertex USE copy; 64-byte PDS; zero-input secondary | HISTORICALLY_SUPPORTED software-vertex encoders; hardware output/attribute delivery UNKNOWN |
| TA stream | 68bytes, six fragments; triangle `81400003`, final packet word `04000203`; bounds/state/terminate uploads11/14/2 dwords | HISTORICALLY_SUPPORTED exact CPU encoders; no malformed count/index demonstrated |
| TA registers / memory | seven pairs; stream through0x238; separate scene/DPM/parameter storage; owned/pinned translations | HISTORICALLY_SUPPORTED; exact per-operation TA output UNKNOWN; completion establishes no coverage |
| Triangle state | mask `0f41` after elision; ISP/front `01d00000`; vertex layout `08001800`; smooth/no-cull `00010000`; precision `358637bd` | HISTORICALLY_SUPPORTED disabled-test conditional path; unnamed hardware fields are not independently decoded |
| Fragment input | Mesa source input mask2 distinct from compiled attribute count0 | SUSPICIOUS mismatch in intended color path; zero attributes do **not** themselves imply zero color |
| Fragment USE | only `00000000 f8040140`; no surviving compiler body | SUSPICIOUS; suffix ISA/implicit export UNKNOWN; absence of a proved explicit color producer is not a proved NOP |
| Fragment PDS / launch | primary56bytes, data count12, zero policy in holes; secondary `af000000`; middle `00030000` | HISTORICALLY_SUPPORTED CPU serialization; runtime launch/input forwarding UNKNOWN |
| ISP/raster | no depth attachment; 26pairs; full32×32 bounds; event PDS through0xa5c;0xa60=4,0xa64=4fff | HISTORICALLY_SUPPORTED; primitive/fragment invocation and effect UNKNOWN |
| Target / PBE inputs | `000f8000`, full color VA, `0`, `0001f01f`; cpp4, pitch32pixels, stride128bytes,32×32 linear ARGB8888 | HISTORICALLY_SUPPORTED target construction; the packed extent is **0001f01f**, not001f001f; actual PBE address/value/write UNKNOWN |
| Event path | 16-byte target data, four-word event USE, two-word helper USE,148-byte event PDS | HISTORICALLY_SUPPORTED DRI producer; does not independently prove pixel stores |
| Background | same COLOR BO in texture data; unchanged background object/USE/PDS | HISTORICALLY_SUPPORTED no-clear/preserve path; no experimental clear or background color change |
| Addresses | PDS aperture20000000..2fffffff; RASTGEOM30000000..3fffffff; MMU≥40000000; separate aligned reservations | CONFIRMED_CORRECT checked CPU bounds/relocation arithmetic; consumer interpretation HISTORICALLY_SUPPORTED, not a live VA measurement |
| Publication / caches | all backings initialized; relocations before publication; pinned mappings; qualified existing device preparations; CPU views released | HISTORICALLY_SUPPORTED existing lifecycle; TA-root CPU publication remains separate and unresolved |
| Retirement/readback | accepted ledger7, retired same owned COLOR page; CLFLUSH/dma_rmb/kmap;4096-byte immutable capsule/response agreement | CONFIRMED same-operation preserved bytes; whether PBE wrote those bytes or left initialized zero UNKNOWN |

### Closest historical color paths

The applicable Xpsb binary revision selector and normal branch are already
verified as [SGX535 rev121 historical evidence](fire02-xpsb-common-raster-abi-20261006.md).
The pinned companion DRI binary is SHA-256
`74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`.
These are historical software paths intended to render, not preserved
same-revision golden pixel results proving a particular sequence ran correctly.
No unavailable ISA document or excluded implementation was reconstructed.

| Historical path | Concrete comparison | Causal relevance / limit |
|---|---|---|
| Frozen DRI compiler/link | [FG-01](../phase7/psb-dri-re/frozen-fragment-exact-output.md): four virtual MOVs removed;0 main/secondary instructions,0 compiled attributes; [link](../phase7/psb-dri-re/frozen-fragment-link-progress.md) adds suffix | CONFIRMED static CPU construction. Compiler virtual registers are not physical output registers; suffix-only hardware effect UNKNOWN |
| Xpsb compositor | ELF0xa660 constructs an extra instruction before selected source words, with descriptor temporary/resource fields differing from frozen;0x82c0 supplies texture/scalar handoff | CONFIRMED extra explicit prefix. Not transferable as a constant-color substitution without its input/launch contract; rejected as the experiment |
| DRI constant clear | ELF0x39ac4 calls0x30d15 with destination descriptor1 and packed ARGB;0x27060 supplies converted clear color;0x2a794 calls same helper withff00ff00 before normal indexed TA geometry | CONFIRMED applicable CPU encoding/call chain. Stronger minimal candidate than transplanting the compositor; not proof of FIRE #2 cause |
| Independent rev121 Xpsb constant variant | table0xd740 index0 selects offset0x30; XpsbShaderCode→ELF0xd110 pair is `00000000 fca40001`;0xa660 copies this pair into the compositor program | CONFIRMED **SGX535 REV121 HISTORICAL EVIDENCE** for exactly the DRI constructor's zero-immediate fixed variant, including END/default controls; nonzero field placement is from the DRI encoder, not guessed from this match |
| Clear launch | same primary `+04=0,+20=20,+30=07000345,+34=af000000`,56-byte commit,count12, zero-input secondary | CONFIRMED matching construction; no new attributes, constants BO, PDS instruction or resource count is needed for this historical one-instruction variant |
| DRI PBE / Xpsb pixel event | source target formulas: full color relocation right2/left2; extent `(h-1)<<12 | (w-1)`; pitch32 gives000f8000 | CONFIRMED CPU formulas. No wrong width/stride/alias/truncation found; undocumented store semantics remain UNKNOWN |

New selected exports/disassembly:
[clear builder](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/dri-decompiled/39ac4.txt),
[scene-color caller](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/dri-decompiled/27060.txt),
[indexed normal-TA caller](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/dri-decompiled/2a794.txt),
[immediate constructor](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/constant-constructor.disassembly.txt),
[Xpsb compositor](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/decompiled/a660.txt),
[Xpsb launch](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/decompiled/82c0.txt),
[DRI event producer](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/dri-decompiled/392ab.txt).

### Fragment, discard, store and address findings

**Fragment — CONFIRMED construction, UNKNOWN effect.** The selected compiled
program constructs no constant and emits no MOV/export body. No surviving
physical output assignment can be derived from that empty body. The linked
suffix might have implicit forwarding/export semantics; it cannot be declared
a NOP or a proved zero writer. Conversely, primary-color inputs are not
demonstrably supplied/consumed just because white vertex data exist. The clear
path is a concrete counterexample to “zero compiled attributes means zero
color”: a constant-immediate program needs no interpolated attributes.

**Coverage/discard — HISTORICALLY_SUPPORTED conditional state.** The selected
triangle is nondegenerate and within the chosen full viewport. The frozen
[state contract](../phase7/psb-dri-re/frozen-triangle-spec.md) selects no cull,
scissor, depth/stencil/alpha test, blend/logic/dither, user clip, queries or
multisampling, and all color channels. Encoders produce the expected
conditional words. No concrete configured rejection was found. This does
not prove an actual TA primitive, ISP invocation or correctly interpreted
hardware test. Culling/winding and bounds therefore stay among branch A's
unknown effects, not established causes.

**PBE/store — HISTORICALLY_SUPPORTED construction, UNKNOWN execution.** The
descriptor uses the same COLOR owner as readback/background,4096bytes and
128-byte rows; no readonly MMU mapping, wrong host object, stale relocation,
late CPU clear, arithmetic overflow or alias was found. Pixel-event PDS/USE
and raster pointers match their selected DRI encoders. No retained field says
what PBE wrote; no-write and writing zero remain indistinguishable. Store
mask/layout/visibility errors remain possible; completion does not exclude them.

**Addresses — CONFIRMED CPU arithmetic only.** Full address/size/alignment,
object→BO offset→GPU VA→wire relocation→consumer relationships are in the
address plan. Primary/secondary/state/index references shift4; USE references
use base-selector plus offset fields; background primitive references use the
RASTGEOM aperture; target preserves the full aligned COLOR VA; event0xa5c
preserves its aligned PDS address. The unchanged relocation checker rejects
out-of-range, overlapping, misaligned and invalid descriptors. Five synthetic
VA layouts cross-check C against independent Python complete images; they are
not a measurement of live VAs or an SGX address-decoder proof.

FIRE #2 eliminates a missing accepted completion/retirement and a short,
truncated or mismatched returned copy. It does not eliminate empty geometry,
zero fragment output, a silent color-store defect or valid zero-valued output.
Kernel before/after records have no new lines, providing no pixel-specific fault
discriminator. No retroactive TA output is invented.

### Ranked causes

| Rank / candidate | Support | Contradiction / unknown | Zero mechanism / confidence |
|---|---|---|---|
| 1 B: intended nonzero fragment result absent | empty compiler body, suffix-only USE,0 attributes; historical clear explicitly constructs color with matching minimal launch | implicit suffix/forwarding unknown; fragment execution/coverage unknown | no effective nonzero color; strongest **specific hypothesis**, causal truth UNKNOWN |
| 2 A: no surviving triangle / no fragment invocation | no retained post-TA coverage; all completions compatible with empty raster work | input/count/ordering/no-cull words match CPU references; no concrete rejection branch | no shaded pixels, initialized buffer stays zero; plausible but less specific |
| 3 C: PBE/address/store/visibility | no actual store/address/value witness; zero can be unchanged or stored | target/owner/relocation/event construction matches; no host arithmetic defect found | silent no-write, wrong device interpretation or stale visibility; plausible, weaker direct support |
| 4 D: other ISP/PDS/discard state | instruction/control semantics not completely specified | no specific contradictory field; conditional disabled-test path matches historical producer | valid completion with suppressed output; possible, least localized |

**ROOT_CAUSE_ESTABLISHED: NO.** Stage A cannot resolve A/B/C/D from sealed
evidence. Stage B selects **only B**, with the bounded sufficiency hypothesis
stated at the beginning; no PBE or TA hypothesis is changed simultaneously.

## Stage B: exact correction and construction proof

The historical constructor route is:

```text
39ac4 clear builder
  -> 30ffd: fresh zero instruction; default context flags0x0804
  -> 30d15: destination descriptor1, packed ARGB immediate
       304d9: destination-bank(1)=1; 30cb1: extension(1)=0
  -> 30fb7: add END bit0x00040000 to the final instruction
```

Assembly, rather than decompiler-implied no-argument helper signatures,
establishes these inputs. Default flags supply the relevant fixed control
bits, including bit23. For this fixed destination/default variant:

```text
word0 = ARGB & 0x001fffff
word1 = 0xfca40001 | (((ARGB >> 21) & 31) << 4) | ((ARGB >> 26) << 12)
ARGB = 0xffff00ff
words = 0x001f00ff, 0xfca7f1f1
bytes = ff 00 1f 00 f1 f1 a7 fc
```

This is a bounded historical CPU encoder contract, not invented ISA bytes or
a general USE assembler. Destination-bank and END interpretation are supported
by helpers/callers; actual fragment invocation, pipeline export and resulting
stored color remain experimental. The applicable clear builder uses the same
zero-resource56-byte PDS launch shape as the frozen program. This supplies
the construction prerequisite for the experiment, not the missing hardware result.
The [independent rev121 cross-correlation](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/rev121-constant-cross-correlation.json)
ties the fixed `fca40001` variant to Xpsb's table-index0 shader-construction
and raster submission chain. This strengthens revision applicability without
promoting the causal hypothesis or assigning a general ISA from a numeric match.

[Pure constructor](../../tools/psb-dri-re/experimental_fragment_constant.h)
and [initializer](../../tools/psb-dri-re/frozen_kernel_contract.c) implement an
explicit compile-time option, `SGX535_EXPERIMENTAL_CONSTANT_FRAGMENT=1`, for
this candidate only. It overwrites `USE+0..7` during CPU initialization, before
existing relocation/publication. The normal build remains byte-equivalent to
the FIRE #2 payload. Program size8, alignment32, allocation, USE reservation,
49 relocations, PDS launch, attributes, geometry, raster/PBE, color initialization,
request/response ABI and approved client are unchanged. No runtime tuning,
reset, completion clearing, extra submission, fallback or retry is added.

The [minimal-change receipt](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/minimal-change-verification.json)
checks every pre-existing driver source input except this contract unchanged,
including the **opaque entry source and object**, without examining/reconstructing
its control flow. The [finished qualified initializer](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/qualified-initializer.disassembly.txt)
contains the two exact stores. All other finished-image archive members are raw
byte-identical to the FIRE #2 image except driver, identity-bearing hook and
enclosing `/init` hook hash. Observer is byte-identical.

## Qualification and identities

| Check | Result / limit |
|---|---|
| [Focused tests](../../tools/psb-dri-re/test_experimental_fragment_constant.c) | control and experiment PASS in native, UBSan and static i486/QEMU;1032 immediate roundtrips per run, six complete backings, invalid-view no-write, deterministic repeated construction |
| [Independent payload comparison](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/pixel-payload-qualification.json) | five layouts × six backings in native/UBSan; only USE+0..7 changes; geometry/index/state/target/background/event addresses unchanged |
| Scoped regression | contract PASS and7 service/capsule differential cases PASS in each native/UBSan mode |
| Module | ELF32/i386, exact antiX vermagic;253 driver and38 observer imports fully covered; no CRC mismatch/unversioned dependency |
| Reproducibility | forced recompilation of driver objects byte-identical; two deterministic images byte-identical |
| FIRSTLOAD construction | finished `/init` expected hash equals packaged hook; payload hashes, observer-before-driver, shell syntax; five existing gate regression tests PASS |

The missing QEMU executable was reacquired by isolated extraction of Debian's
qemu-user10.0.13 package; no system installation or hardware target was used.
All results are **CPU construction/ABI qualification**, not shader emulation,
GPU functionality or a proof of the hypothesis. Existing geometry, ABI,
client, accepted-event, owner/readback and provenance qualification is reused
within unchanged scope; old build-specific live guards require fresh rebinding.

| Artifact | FIRE #2 / retained | Experimental candidate |
|---|---|---|
| Driver Build ID | `9d0b5b2fdb7d9881f2828f43fb89253176c38817` | `85ec06b428c99fac7f9127919b7a488d204f4a77` |
| Driver SHA-256 / bytes | `ddb4fe1fc121945d0a0ab137d310328fc10d41f5bdcfaa39baf86a3783255071` /250720 | `c925caedebcc0699aef53227d29e64b47bf2833efd9d2e611cba276dd3a08ab3` /250788 |
| Observer Build ID | `f11d3abb072caa4e1d32836ef92ce201e9c9d126` | unchanged |
| Observer SHA-256 | `2e35e863d51dbc1d407feef08f08bb2edc6390af621a0f173a9b1a73ec76158a` | unchanged |
| Image SHA-256 / bytes | `ef7e01cb546c59b6f9bce96c3397feb4b9cd1d9ebdac2faa885ed71fa5ed014d` /50816610 | `3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e` /50816631 |

Client SHA-256 remains `2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835`;
UAPI SHA-256 remains `04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420`.
No new identifier, journal or witness/capture is introduced.

## Predeclared future result interpretation

These interpretations require a separately prepared fresh boot, exact candidate
bindings, valid source/capsule provenance and complete preservation. The old
white reference must not silently be relabeled as the magenta experiment.
Originals must be retained before separate diagnostic interpretation.

| Future observation | Meaning |
|---|---|
| Retired, attributable expected magenta on the selected triangle footprint, zero outside | supports the explicit-color sufficiency hypothesis; strongly suggests the intended geometry reached fragment/PBE processing and a nonzero store/readback occurred. Does not retrospectively prove FIRE #2 coverage or identify the old suffix's ISA semantics |
| Attributable magenta/nonzero with unexpected footprint or channel pattern | evidence of effective downstream processing, but not a selected-triangle claim; analyze preserved pattern without tuning or retry |
| Retired, attributable4096bytes still all zero | falsifies the **predicted sufficiency** of this one replacement under its historical encoding assumption; weakens the chosen diagnosis. Does not prove the original fragment correct, nor eliminate a fragment path never invoked. A/C/D remain; no automatic second modification/run |
| HOLD/fault/failure before retirement | separate operation failure; no selected-triangle or pixel inference |
| Incomplete, mismatched, lost or ambiguous evidence | UNKNOWN / STOP; no retry |

No result can be interpreted now: the experiment has not run. Geometry/TA
survival, actual original suffix/export behavior and PBE/store remain UNKNOWN.
The earliest missing observation in FIRE #2 is still selected post-TA coverage.
This experiment bypasses that exhausted witness route to test one downstream
engineering hypothesis; it does not manufacture a TA decoder/publication fact.

## Preservation and next boundary

Task changes: one pre-existing implementation file (contract), new bounded
encoder header/test, this report/qualification record, authoritative handoff;
isolated offline evidence/build/image artifacts. All unrelated dirty work and
all68 sealed FIRE #2 files remain unchanged; index remains empty. The
[preservation receipt](/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/final-consistency.json)
records hashes, links and candidate verification.

A fresh experiment is technically informative **after separate live-preparation
authorization and fresh qualification**. The next action is authorized STOCK
preflight/staging/manual FIRSTLOAD of this exact image, then passive PRE07
rebinding. No staging/boot/preflight occurred in this task. Readiness for
execution authorization is **NO until that fresh preparation passes**.
FIRE #3 then requires separate explicit authorization.

`sgx_execution_authorized=false`; task client invocations0; task SGX invocations0;
hardware interactions0; triangle **NOT ESTABLISHED**. Two historical FIRE
invocations remain recorded; neither is replayed or reinterpreted.
