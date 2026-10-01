# Two different launch fields: middle-word Q versus PDS data size

2026-09-27; P7H-040. Narrow continuation of
[P7H-038/039](pds-launch-state-coverage.md). **L12 remains OPEN.**

The middle word `0x00030000` is **not the field derived from 12 PDS data
dwords**. Its contribution is `(Q-1)<<16`, with the CPU calculating `Q=4`
for the frozen descriptors. The primary-reference word separately carries
`0x0c000000`, derived from `(12 & 0xfc)<<24`. Both fields happen to contain
the integer3 in different positions. P7H-038/039 already distinguish these
formulas; this report does not rewrite those evidence rows.

**What does the middle-word 3 count?** CONFIRMED CPU meaning: one less than a
resource-budget quotient Q, clamped to1..4. INFERRED hardware role: a
resource-limited execution multiplicity/group size related to USE temporary and
attribute requirements. The architectural unit (tasks, groups, instances, etc.)
is **UNKNOWN**. It is not implementation-grade evidence of DS preload, PDS
temporary initialization or a complete source domain.

## Exact producer and dependency separation

Retained DRI ELF `0x287ff`, nonnull-primary `present & 0x40` branch;
calculation `0x28921..0x28a19`, packing `0x28a89..0x28ac9`. The three-word
record, descriptor path and resolved-reference formula remain as established
in the [launch report](pds-launch-state-coverage.md). Define CPU inputs:

| Name here | State-relative field | Recovered provenance |
| --- | --- | --- |
| A | +0x2c | Primary descriptor+0x10; compiler result+0xbc, primary-attribute count |
| T | +0x30 | Primary descriptor+0x14; linked USE/USSE temporary count at link+0x74 |
| B | +0x44 | Secondary descriptor+0x10; zero in empty-secondary path; nonempty `0x39ea0` copies its separate payload's committed dword count here |
| Dp | +0x28 | Primary descriptor+0xc, PDS CPU data-prefix dwords; selected12 |
| Ds | +0x40 | Secondary descriptor+0xc, secondary PDS CPU data-prefix dwords; selected0 |

A's meaning is established in [the compiler result table](frozen-fragment-exact-output.md).
T has the `result->temp_count <= cpost->temp_count` assertion in `0x25970`;
it is not PDS temporary storage. B is a count of a **different payload** from
the secondary PDS prefix. `0x39ea0` returns its count at output-base+0x28,
which is secondary-descriptor+0x10; empty helper `0x3faa3` clears it. Do not
rename B as a PDS temporary count.

For the non-overflow, nonnegative audit domain, the recovered CPU constraints
normalize to:

```text
A4 = round_up(A, 4)
R  = ((0x4df - round_up(B, 32)) & 0xffffffa0) / 3
Q  = 4                                      if T == 0
     min(4, 192 / round_up(4*T, 16))         otherwise
Q  = min(Q, R/(4*A4))                        if A4 != 0
Q  = max(1, Q)
M  = 32                                     if A4 == 0
     clamp(R/(4*A4), 1, 32)                  otherwise
M  = 8*Q - 3                                if 8*Q < min(M+3,32)

middle = (((4*B+127)&0x3f80)<<11)
       | A | ((Q-1)<<16) | ((M&31)<<25) | (0x8000 if B!=0 else 0)
primary size contribution   = (Dp & 0xfc)<<24     # separate word
secondary size contribution = (Ds & 0xfc)<<24     # separate word
```

Division is integral; the original R division is signed. The checker confines
inputs to a small positive-budget domain, so it does not generalize Python
division to negative or overflowing historical arithmetic. Those tool bounds
are **not hardware capacities**.

The unusual literal mask `0xffffffa0` must not be replaced with a conventional
alignment operation. For example, B=0 yields R=384 whereas B=32 yields R=394.
This nonmonotonic result is reproduced in a regression, not rationalized into a
hardware capacity specification. It further limits architectural conclusions
from the apparent resource-budget arithmetic.

There is no Dp or Ds dependency in Q or M. The primary commit length is also
absent. Conversely, Q does not determine where code begins or how many prefix
dwords the builder writes. The selected A=T=B=0 gives R=384, Q=4, M=32.
Thus `(Q-1)<<16=0x30000`, while `(M&31)<<25=0`.

**Position/mask qualification:** shift16 is literal CPU arithmetic. Clamping
Q to1..4 makes its contribution fit **bits16–17**, mask`0x00030000`; there is
no separate `&3` at the final shift. This is a confirmed mask for the Q **term**,
not a recovered architectural field definition. A is ORed without an explicit
mask; unrestricted malformed A could overlap it. In the selected A=0 and
bounded audit cases it does not. The B term occupies bits18–24, M bits25–29,
and the B-nonzero flag bit15. These are CPU contribution ranges, not assigned
architectural names.

In particular, encoded4 is not another value of the isolated Q term: its
contribution would be bit18, already in a different CPU contribution. The
emitter can compute Q=1,2,3,4 and consequently encode0,1,2,3.

## Controlled arithmetic variations, not invented observations

[pds-launch-count-corpus.csv](pds-launch-count-corpus.csv) records twelve cases
with inputs, Q/M, whole middle word, separate size contribution, XORs and changed
bits. Only the frozen row is labeled SELECTED_CPU_RESULT. Other rows are
**SERIALIZER_FORMULA_CASE**: evaluations of the retained CPU formula with one
descriptor input changed from the selected case. They are not captured GPU
executions, newly discovered literal stores, independently compiled shaders or
proof that every substituted descriptor is a valid hardware program.

| Change from A=T=B=0, Dp=12 | Q | Q encoding | Dp size contribution |
| --- | --- | --- | --- |
| None | 4 | 3 | `0x0c000000` |
| T=13 | 3 | 2 | unchanged |
| T=17 | 2 | 1 | unchanged |
| T=25 | 1 | 0 | unchanged |
| A=32 | 3 | 2 | unchanged |
| A=40 | 2 | 1 | unchanged |
| A=52 | 1 | 0 | unchanged |
| Dp=4,8,16 in separate cases | 4 | 3 | `0x04000000`, `0x08000000`, `0x10000000` |

T changes also change the builder's established task words, if the corresponding
descriptor is used: `+04` incorporates `T<<27`, `+20` incorporates `T>>5`.
They do not change its initial pair/triplet layout through that variable. A
comes from attribute use; changing an actual shader can also change other
descriptors. The table isolates **serializer inputs**, not complete shader
changes. Dp-only rows do not claim the unchanged selected two-instruction
program can legally be paired with any data count. This qualification prevents
numeric variation being promoted to an ISA experiment.

The Q and M changes are coupled by the CPU budget formulas, so whole-word XORs
are not pure Q-field observations. The table separates the Q term explicitly.
Its negative tests reject Q being set to `Dp/4`, a size field inconsistent with
Dp, and Q encoding4. The original26 tests are unchanged.

## Independent packing contexts and adversarial checks

These retained paths corroborate **separate fields**, not Q's exact hardware
unit. They were examined before constructing the model, so they are not called
blind holdouts. DRI and Xpsb may share source lineage.

| Context | Evidence | Result |
| --- | --- | --- |
| DRI scene serializer | ELF0x287ff | Variable Q from A/T/B; Dp/Ds packed independently |
| DRI background object | ELF0x2ee34; literal OR at0x2ef11, store0x2ef2a | Forces `0x30000`; independently packs primary/secondary descriptor data counts into the reference words |
| Xpsb blit background object | `Xpsb_setup_blit_background_object` ELF0x71e0; OR0x731a, store0x732a | Forces the same middle contribution; separately applies `0xfc000000` mask to reference count |
| Xpsb parameter block | `XpsbFlushParamblock` ELF0x7560; OR0x78e5 | Uses `0x40030000`: same Q bits with an additional bit30; its meaning is not transferred to the selected record |

The cached export named `0002eef4.txt` actually labels its function as
Ghidra`0x3ee34` and duplicates `0002ee34.txt`. Original disassembly puts0x2eef4
inside an instruction. **It is not a second independent function/example.**
Only the real0x2ee34 entry is counted above.

Targeted searches of the already acquired public PSB4.41.1, DDX0.32.0,
libdrm2.3.0 and permitted SGX register snapshots found no applicable symbolic
definition for this packet field or CPU consumer that bounds a data copy by Q.
Public PSB2D pixel-format constants and SGX clock-gate masks also equal
`0x00030000`; they name different registers/commands and were excluded.
No `DMSINFO`/PDS task-size definition was found in the inspected public OMAP
kernel source/header subset. This is a bounded negative search, not a claim
about all public code or all hardware behavior.

The existing permissively licensed [EMGD Poulsbo SGX header](../../poulsbo-data/EMGD_drm_emgd_include_plb_sgx_h.txt)
names USE register budgets and a32-register PDS **attribute** chunk
(lines109–133). It does not define the middle-word field, and its register
budget constants are not substituted for the retained formula. In particular,
neither “attribute chunk32” nor “USE temp count” proves constant-store or PDS
temporary initialization. No quarantined definitions or later-core ISA oracle
were used; no new source acquisition was needed.

## Candidate interpretations

| Hypothesis | Status | Evidence / limit |
| --- | --- | --- |
| H1 number of serialized launch words | CONTRADICTED as CPU dependency | Branch always emits three words while Q can change |
| H2 number of PDS data dwords | CONTRADICTED as direct count | Q ignores Dp/Ds; selected Q=4 versus prefix12 |
| H3 DS pairs; H4 DS0 entries; H5 DS0+DS1 entries | CONTRADICTED as direct CPU counts | Same bank/prefix layout can be supplied with different Q when T changes; no bank-count input to Q |
| H6 size of serialized state block | CONTRADICTED | Same three-word group width for every Q; other present bits govern record size |
| H7 PDS data chunks | CONTRADICTED as the Dp/4 field | Dp/4 is in a different word; formula counterexamples separate them |
| H8 count bounds every architectural readable input | UNRESOLVED / unsupported | No hardware exclusion or initialization rule follows from CPU budget arithmetic |
| H9 count-minus-one | SUPPORTED / CONFIRMED CPU construction | Q in1..4 stored as Q−1 at shift16 |
| H10 merely a CPU serialization count | CONTRADICTED as word-loop bound | Serialized resource control value; no CPU loop/copy uses Q as length |
| H11 execution multiplicity/group size limited by USE resources | SUPPORTED / INFERRED | Inverse dependence on aligned USE temporary demand and attribute demand; precise architectural unit UNKNOWN |

“Contradicted” in the direct-count rows rejects those proposed explanations of
the **recovered CPU formula**. It is not proof that Q has no indirect hardware
interaction with PDS scheduling or stores. A hidden validity rule linking
descriptor inputs would need evidence; none is inserted to rescue a count
interpretation or close coverage.

## Consumer boundary and L12

The middle word is emitted into the state payload by `0x287ff`, then included
in its state-upload program and referenced through `0x3b73b`. That is a
GPU-directed state path, not a kernel allocation length. The existing
[submission trace](command-submission.md) and relocation handler account for
addresses/BO validation; they do not supply a CPU-side decoder of this resource
word. No inspected kernel/firmware consumer gives Q a preload-loop meaning.
The hardware state consumer's exact interpretation remains UNKNOWN; naming it
a particular DMS register from an unrelated header would not establish it.

The56-byte allocation remains **48 bytes of CPU data prefix plus8 bytes of
CPU instruction stream**. The nine holes stay CPU ZERO_CONFIRMED. Q=4/encoded3
does not select three words, three pairs or a three-word subset of that image.
The separate reference count still expresses CPU prefix size12 in groups of4;
it has not become an architectural source-validity limit.

Temporary and implicit-state coverage consequently remain UNKNOWN. There is
no new evidence of an outside read. The narrowed next rule is:

> **Does the selected launch make state outside the domain initialized from
> its48-byte PDS data prefix unavailable to the two selected instructions until
> that state is defined?**

This is the L12 **source-eligibility/definedness rule**, not the arithmetic unit
of the middle-word resource quotient. A proof may specify deterministic
additional launch inputs instead of excluding them. Q's producer cannot answer
this question: it neither enumerates nor initializes DS/PDS-temporary/implicit
inputs. Do not start another middle-word count survey to answer it. The earlier
hardware preload-map limitation remains part of defining that initialized
domain, rather than being silently declared solved.

## Decision matrix

| Item | Result |
| --- | --- |
| `0x00030000` CPU construction | CONFIRMED: `(4−1)<<16` |
| Field position/mask | CONFIRMED CPU Q contribution shift16/mask0x30000; architectural mask UNKNOWN |
| Meaning of count3 | CONFIRMED as Q−1 in CPU formula; hardware resource unit INFERRED/UNKNOWN |
| Controlled state class | INFERRED execution resource control; no confirmed DS initialization role |
| DS0 coverage | UNKNOWN |
| DS1 coverage | UNKNOWN |
| Temporary coverage | UNKNOWN |
| Implicit-state coverage | UNKNOWN |
| Selected possible-input set | PARTIALLY BOUNDED |
| All selected possible inputs deterministic | UNKNOWN at launch; CPU image remains deterministic |
| L12 | OPEN |

No implementation dependency is removed. GPU contents preservation remains
CONDITIONAL; FG-02 OPEN. Exact PDS instruction semantics remain UNKNOWN.
Route C, the historical evidence distinctions and first-use no-overlap are
unchanged. Gate B BLOCKED, whitelist`[]`, hardware UNVERIFIED.

## Reproduce

[The checker](../../../tools/psb-dri-re/pds_launch_count_constraints.py) validates
only the bounded CPU constraints and the labeled differential table. It does
not execute retained code. Read [the tests](../../../tools/psb-dri-re/test_pds_launch_count_constraints.py)
for contradictory count/layout and wrong-field interpretation regressions.

```sh
python3 -B tools/psb-dri-re/pds_launch_count_constraints.py
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_*pds*.py' -v
llvm-objdump -d --x86-asm-syntax=intel --start-address=0x28921 --stop-address=0x28acb references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so
llvm-objdump -d --x86-asm-syntax=intel --start-address=0x2ee34 --stop-address=0x2ef60 references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so
llvm-objdump -d --x86-asm-syntax=intel --start-address=0x71e0 --stop-address=0x7470 references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so
llvm-objdump -d --x86-asm-syntax=intel --start-address=0x7560 --stop-address=0x7b50 references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so
```

Existing `/tmp` exports are disposable and are not runtime dependencies of the
checker. Real ELF entries and derivations above preserve the evidence even if
the exports disappear. The public source locations/provenance remain in the
[existing ledger](../source-provenance.md); no references content was changed.

## Validation result

All36 tests pass: the original26 plus ten count-constraint regressions. The
new table and launch inventory checkers pass integrity mode; `--require-closed`
still returns expected exit1 for L12. New Python syntax checks pass. All40
Phase7/evidence CSV schemas pass; P7H-040 is unique and earlier multi-row IDs
are preserved. All493 local Markdown links across29 changed/untracked reports
resolve. Both authoritative retained ELF SHA-256 values match.

`git diff --check` passes; references/ status and the index are empty. New-file
whitespace checks also pass. Prior uncommitted work is preserved. Nothing was
staged or committed, no historical ELF was executed, and no hardware was used.
