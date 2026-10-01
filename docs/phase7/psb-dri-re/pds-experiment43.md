# Experiment #43: search for an independent PDS variation

**Result: no distinguishing experiment found.** This is a bounded static
identifiability result, not a PDS decode. The selected `0x07000345` read set
remains **UNKNOWN**. Model M1 reads the inferred `+0x00/+0x04/+0x20` triplet;
M2 reads that triplet plus at least one fixed producer-unwritten slot. Both
predict every verified CPU emission here. `0xaf000000` may be a terminal/control
word, but its architectural data-store and implicit reads remain **UNKNOWN**.

The [reproducible immediate-store survey](../../../tools/psb-dri-re/pds_experiment43_survey.py)
records [retained results](pds-experiment43-retained-stores.csv). It statically
disassembles the two hashed retained ELFs and accepts only `mov` instructions
with a memory destination and an immediate `0x07????45`, `0xad......`,
`0xae......`, or `0xaf......` value. It neither executes a historical object
nor treats arbitrary ELF bytes as instructions. The retained result has 23
stores: DRI has `0x07000345` ×3, `0x07042345` ×3,
`0x070b0345` ×1 and `0xaf000000` ×11; Xpsb has `0x07000345` ×1 and
`0xaf000000` ×4. There are no `0xad`/`0xae` immediate-store neighbors in this
bounded survey. These stores are already represented in the 42-row corpus.

The broader previously generated full-`.text` immediate inventory and indexed
outbuf-allocator caller audit were revisited only to seek emitters outside that
corpus. The DRI `0x38b5c` path writes `0x07e80100` in a 0x190-byte vertex-like
buffer, `0x2d2f9` uses `0x07000000` in a switch-constant group, and `0x391e0`
writes a 16-byte USE-shaped record. No inspected outside caller supplied a
controlled `0x45` or `0xaf` neighbor. Xpsb's other `0x07...` constants occur
in event data or a BO default field, not a new verified `0x45` instruction.
The four direct outbuf callers without an existing bounded Ghidra text export
were checked in targeted i386 disassembly: `0x2ee34` commits a `0x44`-byte
record with relocations and scalar state words; `0x38b5c` emits the
`0x07e80100`/`0x02000000` vertex-like data; `0x38d87` commits a `0x5c`-byte
record with relocations, `0x1c00000`, `0x4e504a30`, and coordinate/float
data; `0x391e0` commits the 16-byte USE-shaped record. None provides a
verified `0x45`/`0xaf` instruction variation. The indexed callgraph lists
28 direct callers of outbuf allocation `0x38762` in total; 24 already had
bounded exports from earlier Phase 7 work. This caller screen complements,
but does not transform, the direct-immediate result into a universal proof
about all possible computed words.
This is a negative search over the classified producers and direct immediate
stores; it cannot by itself exclude every hypothetical run-time-composed word.

## Priority 1: bits 16–17 of `0x070b0345`

The DRI event builder beginning at ELF `0x000392ab` writes **the entire word as
one literal** at `0x00039719` into `+0x8c`, then writes `0xaf000000` at
`0x00039723` into `+0x90` and commits the program. Its callers at `0x00027060`
and `0x00046924` can affect earlier data/relocations, but cannot control bits
16–17 of this literal through the inspected store. The only retained
`0x070b0345` producer is this literal; neither retained ELF has a second
`0x45` word with that residual field changed independently. The arithmetic
decomposition in [the hypothesis audit](pds-hypotheses.md) labels these bits
`C=3`, but no CPU semantic variable explains *why* `C=3`, and no GPU behavior
is inferred from their literal value. This establishes the requested producer
result **B: literal/invariant**, not a source-selector rule.

## Priority 2: `0xaf000000`

All 15 retained verified immediate stores use exactly `0xaf000000`. The
broader `0xad`/`0xae`/`0xaf` direct-immediate family check found no variant.
An exact raw-byte sweep finds 21 `0xaf000000` sequences in the DRI file;
only 11 are the verified `.text` immediate stores. The other raw sequences
include unaligned metadata/data-table occurrences and are not classified as
PDS instruction producers. Xpsb's four exact raw occurrences are its four
verified immediate stores. This byte sweep does not establish absence of
computed terminal variants.
The known standalone four-byte DRI emission and repeated CPU-final placement
remain structural evidence. An instruction with zero local explicit sources,
one with zero-valued selectors, and one that reads implicit state all fit that
unchanged literal. No retained producer exposes an independently variable
`0xaf` operand field.

## Priority 3: alternate historical build

A contemporary [May 2009 Poulsbo package account](https://www.happyassassin.net/posts/2009/05/13/native-poulsbo-gma-500-graphics-driver-for-fedora-10/)
identified a Dell Mini/Canonical package. The [Canonical archive package](http://netbook-remix.archive.canonical.com/ubuntu/dists/hardy-dell-mini/main/binary-lpia/libgl1-mesa-dri-psb_0.24+repack+0038.1-0netbook1_lpia.deb)
is version `0.24+repack+0038.1-0netbook1`, `lpia`, dated 2009-03-20;
its SHA-256 `14e870f4273e84759630499efeda4948116c29f3ba7d08e2b5f028a5da3ef8a2`
matches the Canonical `Packages.gz` entry. Its ELF32 i386 `psb_dri.so`
has SHA-256 `427627909b0c528d154d1b247688f16536162a520d43d472e8a4f404c02e2a6e`
and identifies Mesa 7.0.3, versus retained Mesa 7.4. This establishes an
older related driver-family binary, **not** a proven interchangeable stack.

The package's copyright file calls the original content Intel Confidential,
all rights reserved. The downloaded package and extracted ELF therefore stay
in disposable `/tmp/sgx535-exp43-quarantine/`, outside `references/` and out
of implementation reasoning. Only static instruction-occurrence and package
provenance checks were made. The same survey finds 18 verified stores:
`0x07000345` ×3, `0x07042345` ×3, `0x070b0345` ×1 and
`0xaf000000` ×11; no `0xad`/`0xae` immediate neighbor. No new `0x45` value
or terminal variation emerges. The Canonical hardy-dell-mini and proposed
`binary-lpia` package indexes list this same hash; other inspected hardy
suite/architecture indexes did not supply another version. An older Fedora
`xpsb-glx-0.11` source-RPM link in the contemporary account now returns 404;
targeted mirror searches did not yield an attributable copy. Its absence is a
search limit, not evidence about that build's contents.

## Identifiability boundary

The available CPU-side software does not contain an **observed** degree of
freedom capable of distinguishing M1 from M2. In `0x07000345`, all candidate
source-count/mode bits not varied by the existing `0x00042000` pair delta are
fixed. A fourth source could be encoded by those fixed bits or be implicit;
no CPU emission reads back the GPU's data-store accesses. For the counterexample
`0x070b0345`, bits 16–17 (`0x00030000`) are a literal residual rather than
an independently varied producer field. For `0xaf000000`, the entire word is
fixed across all verified producers, leaving explicit zero selectors versus no
sources versus implicit reads indistinguishable. Thus the nine primary
producer-unwritten slots cannot be promoted to dead data. No deterministic
fill value is justified by this survey.

A genuinely independent `0x45` builder that varies source count or a hidden
source while keeping the candidate triplet controlled would separate M1/M2.
Alternatively, an independently attributable SGX535-applicable source-count
rule or a future authorized read observation could do so. The unavailable
input-parameter PDF is not a prerequisite for this statement. FG-02 remains
OPEN, Gate B BLOCKED, hardware whitelist `[]`, hardware functionality
UNVERIFIED. No active hardware access or historical binary execution occurred.
