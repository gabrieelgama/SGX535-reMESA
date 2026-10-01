# Selected source availability: what is proved, and what is not

2026-09-27; P7H-041. **L12 remains OPEN.** This is a narrow continuation of
[launch coverage](pds-launch-state-coverage.md), not a new instruction or Q
survey. The [Q result](pds-launch-count.md), all36 earlier tests, first-use
no-overlap and the parameterized56-byte CPU image remain unchanged.

## Availability is an architectural property, not a CPU byte label

For this analysis a location is **available** if either selected instruction
can obtain a value from it at its first relevant read. This differs from:

- a CPU-writable byte in the BO;
- a named store present somewhere in the architecture;
- a hardware output destination;
- a register that might retain bits internally but cannot be read by this
  selected program;
- a control value whose complete encoded representation is already supplied.

No new selected outside read is established. The remaining failure is the
absence of a complete source-domain rule. It is **not** a collection of asserted
temporary, external-memory or cross-launch hazards.

The extended [state inventory](pds-launch-state.csv) keeps all24 existing rows
and adds role, selected-reference status, read-before-write status, boundedness,
first-read definedness, persistence and availability/exclusion evidence.
The individual14 CPU dwords retain their existing provenance. Control and source
rows are now mechanically separate. `UNKNOWN` selected eligibility does not
mean a read is known to occur.

## Narrow evidence checked for launch initialization and persistence

The already catalogued [SGX535 forum thread](https://forums.imgtec.com/t/pds-programming/333)
was inspected specifically for store initialization, not for the missing PDF.
The user posting about SGX535 describes constant-store preloading at execution
start and quotes “2 sets of temporary stores for intermediate results”. This
supports **INFERRED class plausibility** for the two temporary-set questions.
It gives no indices, temporary initial values, reset guarantee, read-before-write
restriction or selector rule for these two words. It is a user quotation of
unavailable material, not an independently authenticated architectural contract.
The vendor reply supplies no technical rule. No restricted document was obtained.

The retained CPU interleaving and data-count evidence corroborates a separately
prepared constant prefix. It does **not** corroborate which architectural
entries are replaced, whether other entries are unavailable, or whether
temporary contents are defined. No new preload map is promoted from this post.

The permitted kernel-side SGX source subset was searched specifically for
PDS preload/initialization/data-store/temporary hooks. No applicable selected
source-eligibility implementation was found. This bounded result reuses the
known kernel/user-mode split; it is not a generic public-source census.
The earlier architecture patent locator was inaccessible through the web tool
in this pass; no claim here depends on rereading its contents. The already
recorded generic/cross-core limitations still apply.

A different possible shortcut was checked in the existing public
[PSB reset implementation](../../poulsbo-data/PSB_psb_reset_c.txt):
`psb_reset`, lines35–64, pulses block reset and clears a fault condition;
`psb_reset_wq`, lines253–319, belongs to watchdog recovery and reloads TA memory.
That is not a per-PDS-launch store initializer in the selected path. The code
does not specify the reset values of PDS constant or temporary locations.
Neither “reset clears them” nor “reset leaves them stale” follows from it.

The retained `Xpsb_ta_mem_load` export (ELF0x3df0) updates TA/DPM memory
descriptors and corresponding register state, as already classified by the
context reports. It does not supply a selected PDS scratch-value contract.
TA memory/context bookkeeping is not proof of PDS register replacement.
These were read-only source/export checks. No reset, register operation, context
switch or historical executable was performed.

**Persistence conclusion:** UNKNOWN. No selected-path evidence establishes
either persistence or guaranteed clearing of a source-readable location between
PDS invocations. CPU BO recycling is a different domain and cannot prove either.
We do not create an additional “persistent register” row without an identified
architectural location; that possibility remains part of the existing residual
`implicit` row. No hardware-generated input specific to this primary program
was identified either, so no event/task-ID input is invented.

## Conservative source classes for the exact selected words

These are **evidence classifications**, not a decoded finite set of registers.
`UNCLASSIFIED` is a proof gap, not a newly identified physical source.

| Word | Supported candidate classes | Unresolved domain | Exclusions actually established |
| --- | --- | --- | --- |
| `0x07000345` | `{DS0 constants, DS1 constants}` INFERRED from established triplet construction | `{temporary class, other implicit class}` remain unclassified; no selected reachability demonstrated | None sufficient to certify completeness |
| `0xaf000000` | `{no source}` INFERRED from CPU terminal/standalone grammar | `{DS, temporary, implicit}` not excluded by an architectural rule; no such selected read demonstrated | A local constant source in its standalone zero-data record is disfavored, not architecturally excluded |

The first program word has no preceding instruction **within this primary**.
Therefore an actual temporary read by it, if established, would need launch
initialization or another deterministic producer. Its existence is not proved.
For the second word, whether the first defines a consumed value is UNKNOWN.
The separate secondary PDS program is not proved to initialize primary scratch.
None of TEMP_NOT_READ, TEMP_WRITTEN_BEFORE_READ, TEMP_ZERO_INITIALIZED or
TEMP_DETERMINISTIC_OTHER has a selected-path proof. The result is TEMP_UNKNOWN.

Source-class exclusion cannot be obtained from a source-count correlation:
the retained emitters do not expose a variable choosing constant versus
temporary storage for either selected literal. That was already the software
identifiability boundary, and the emission corpus was not re-extracted here.
There is no justified bit position to name as a recovered DS/temp selector.

## Control values and output destinations are not extra source hazards

The packed launch tuple, U/task words and code bytes have known CPU provenance.
Their mapping, interpretation and GPU availability remain separate obligations.
The `entry`, `packed_controls`, `use_target` and `dout` rows are classified as
CONTROL. `dout` is still only a control question: no selected opcode assignment
or independent DOUT source is asserted. The linked USE target is downstream
execution state, not evidence that the PDS reads USE memory as another operand.

The separate secondary record is SEPARATE_LAUNCH. The two CPU instruction words
are CPU_IMAGE entries, not constant-store dwords. These distinctions prevent
counting every address/control field as uninitialized source data. They do not
prove that all control effects have been reconstructed.

## The one remaining discriminator

Let D be the architectural image of the initialized CPU prefix together with
deterministic encoded launch controls, **assuming** the favorable preload and
perfect-backend hypotheses. Two interpretations still produce the same observed
CPU writes and launch tuple:

1. The selected instruction forms admit only pre-definition reads in D.
2. At least one form admits a pre-definition source not determined by D.

The second is a logical countermodel to inference from CPU construction, **not
a measured SGX535 capability or a new claim of an actual external read**.
The store-description, reset and resource-allocation observations do not select
one interpretation. Zeroing all BO bytes cannot resolve an architectural
eligibility distinction. Resetting hardware is neither authorized nor shown to
resolve it.

**Exact missing rule: the complete pre-definition source domain of the literal
pair `0x07000345;0xaf000000` under this primary launch must be confined to the
state deterministically established by that launch.**

This is a single source-domain closure contract, not a demand to clear every
generic SGX store or decode all operand values. Its proof must connect preload
mapping to eligible entries; the48-byte CPU prefix alone is not that connection.
A source-domain table/rule for these forms that excludes other banks/entries,
or a selected launch rule that deterministically defines every admitted source,
would distinguish the interpretations. A temporary-reset statement alone would
not settle other unclassified domains. An additional emission with unchanged
source-class selectors, a larger zero BO, or a Q value would not distinguish them.

It is not legitimate to compress this into a particular selector bit or a claim
that only DS1 remains unresolved: current evidence establishes neither. The
single residual obligation is `UNCLASSIFIED_SELECTED_SOURCE_DOMAIN`. Its
subdomains in the inventory are transparent, not independent confirmed hazards.
No new broad research plan is proposed. L12 cannot close from the currently
established CPU/launch-provider observations alone.

## Model/checker changes

The existing [checker](../../../tools/psb-dri-re/pds_launch_state_check.py) is
extended, not replaced. `assess_availability` derives source blockers and
control obligations from rows instead of hard-coding the entire OPEN result.
Five source questions remain: DS0, DS1, temp0, temp1 and the residual implicit
domain. Unknown eligibility includes a row conservatively; exclusion requires
evidence. Reclassifying a source as control is rejected by the manifest check.

Closure also requires a separately evidenced architectural envelope. That
evidence ledger remains UNKNOWN (P7H-039/041). Even a synthetic fixture making
every listed source bounded and defined cannot make the inventory exhaustive.
The helper checks the consistency of supplied facts; it cannot authenticate a
paper's claims or prove an architectural theorem from CSV labels. The synthetic
fixture is explicitly a test axiom and never modifies the real inventory.

Six new tests reject an uninitialized readable source, unsupported exclusion,
missing availability evidence and role laundering; distinguish unknown controls
from source hazards; and reject completeness-by-enumeration. All36 original
tests remain. No changed field claims hardware functionality.

## Closure matrix

| Item | Result |
| --- | --- |
| DS0 selected visibility | UNKNOWN; CPU labels bounded, architectural eligibility not bounded |
| DS1 selected visibility | UNKNOWN; same distinction |
| Temporary selected visibility | UNKNOWN |
| Implicit selected state | UNKNOWN |
| Cross-launch persistent readable state | UNKNOWN; neither POSSIBLE as a demonstrated selected capability nor EXCLUDED |
| `0x07000345` candidate source classes | DS0/DS1 INFERRED; temporary/implicit UNCLASSIFIED |
| `0xaf000000` candidate source classes | no-source INFERRED; DS/temporary/implicit UNCLASSIFIED |
| Selected POSSIBLE_INPUT_SET | PARTIALLY_BOUNDED |
| Every selected possible input deterministic | UNKNOWN at hardware launch; CPU image remains deterministic |
| L12 | OPEN |

GPU preservation remains CONDITIONAL; no backend was claimed or executed.
FG-01 CLOSED, FG-02 OPEN; Route C selected; historical provider distinctions
unchanged. Gate B BLOCKED; whitelist`[]`; hardware UNVERIFIED.

Reproduce the complete regression set with:

```sh
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_*pds*.py' -v
python3 -B tools/psb-dri-re/pds_launch_state_check.py
python3 -B tools/psb-dri-re/pds_launch_state_check.py --require-closed
```

The final command must still exit1. The report's public forum check was on
2026-09-27; the original posts are dated2009-04-23. The page was inspected as
a limited existing lead; no new restricted source or binary was acquired.

## Validation recorded

All42 tests pass (36 preserved plus six availability regressions). The CPU
image/table and launch inventory checks pass; closure-required mode returns
expected exit1 for L12. Python syntax compilation passes. All40 Phase7/evidence
CSV headers and row widths validate; P7H-041 has one new evidence row and all
new references resolve. Pre-existing multi-row IDs are preserved. All507 local
Markdown links across30 changed/untracked reports resolve. Both retained ELF
hashes match. New-file whitespace and `git diff --check` pass. references/ has
no changes and the Git index is empty. No prior work was discarded or committed.
