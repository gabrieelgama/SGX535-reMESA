# Frozen-triangle blocker burn-down — P7H-047–057

2026-09-27. **Static specification PARTIAL; FG-02 OPEN.** B2 closes for the
selected retained renderer's surface contract. B3 now has a concrete CPU BO,
validation and relocation model, but device publication remains conditional.
B1 and B4 retain specific architectural/ready-state gaps. L12 stayed frozen.
Gate B BLOCKED, whitelist `[]`, hardware UNVERIFIED. No device was opened and
no historical executable was run.

This extends the [51-object specification](frozen-triangle-spec.md), rather
than replacing its selected draw. It does not establish historical provider
identity, PDS read semantics or hardware functionality.

## B3: allocation and validation model (P7H-047)

[The BO model](../../../tools/psb-dri-re/frozen_triangle_bo.py) defines ten
backing roles for the existing objects, six user validation entries and49
wire relocation records. Seven of the38 existing sites are aliases in the TA
stream; nine canonical USE sites expand to three records each. No new address
site was invented. The exact generated artifacts are:

- [BO requirements](frozen-triangle-bos.csv), including sizes, flags and owner;
- [object-to-BO bindings](frozen-triangle-bindings.csv), including reserved extents;
- [user validation set](frozen-triangle-validation.csv);
- [49 wire records](frozen-triangle-wire-relocations.csv).

| Backing | Allocation / alignment | Placement | Purpose |
|---|---|---|---|
| pds | 128KiB /4KiB | PDS | PDS images, state constants, bounds geometry, indices |
| use | 32KiB /32KiB | PDS + USSE tag | aligned USE code, separate data-master reservations |
| vertex_ta | 4KiB /4KiB | MMU + TA tag |96-byte vertices and68-byte TA stream |
| background | 4KiB /4KiB | RASTGEOM |68-byte background object |
| control | 4KiB /4KiB | LOCAL, CACHED, EXE | TA/raster register lists and1960-byte relocation table |
| color | 4KiB /4KiB | MMU, read/write | linear32×32 target |
| scene_hw | 8KiB /4KiB | MMU + SCENE, CACHED | kernel-owned0x1420 scene request rounded to pages |
| ta_page_table | 32MiB /4KiB | MMU + SCENE | kernel-owned8192-page policy |
| ta_parameter | 32MiB /1MiB | RASTGEOM + SCENE | kernel-owned TA parameter backing |
| xhw_comm | 4KiB /4KiB | LOCAL |132-byte reference-family communication payload |

Grouping, offsets and full-allocation zeroing are **CLEANROOM_POLICY**, not a
replay of historical allocation order. They respect the recovered pool classes:
owner+64 LOCAL command pool, +68 PDS/USSE, +6c PDS/shared indices and bounds,
+70 RASTGEOM background, +78 MMU/TA. Retained0x3f87a uses the latter for vertex
uploads;0x27fa0/0x37409 use +6c for indices. The new layout retains primary PDS
at+0x160 and starts other PDS allocations at+0x200. Its own interval checker
excludes overlap; the historical first-use proof is neither rerun nor silently
applied to a different allocation policy. The index reservation retains the
historical minimum0x2000 16-bit elements, despite only six selected bytes.
All user BO padding is explicitly zero in the host resolver.

Creation uses `buffer_start=0`: device-controlled backing (`drm_bo_type_dc`),
**not userptr**. `drm_bo.c:1833` selects that type. The cached-memory constraint
at:963–975 applies to userptr and must not be transferred to these objects.
No NO_MOVE guarantee is invented. The reference XHW communication flags include
privileged NO_EVICT; a future provider must supply the corresponding privileged
service contract, not pretend an ordinary DRI client can request it.

PDS and RASTGEOM manager ranges are respectively `[0x20000000,0x30000000)` and
`[0x30000000,0x40000000)`. MMU begins at0x40000000 and ends at the provider's
`gatt_start`, **not an assumed4GiB** (`psb_drv.c:362–365`). The resolver requires
that manager limit as an explicit input and checks complete BO extents against
it. Addresses used by unit tests are synthetic fixtures only. Production
addresses must come from validation. LOCAL has no required GPU address.

The color choice MMU is within the retained surface validation mask0x16000000.
This policy excludes fixed/stolen/VRAM placement. PDS selection remains forced
by0x20000000; it does not gain a stolen-memory fallback. USER BO flags/masks,
all six validation handles, and zero presumed-placement hints are explicit.
Kernel scene and TA objects are created and validated by the scene path,
not fictitious extra user validation entries (`psb_scene.c:128–267,430–493`).

`validation_words()` emits34 dwords per `drm_bo_op_arg` (136bytes), with
symbolic next pointer/handle, zero high halves and expansion padding. The
kernel uses the submission fence class in `psb_validate_buffer_list`, rather
than trusting the unused per-request fence-class field. `submission_words()`
accepts explicit i386 pointers and BO handles and fills the existing144-byte
command: register counts14/52, OOM count0 with raster handle/offset alias,
relocation count49. No ioctl is invoked.

The wire encoder retains op0 masked insertion and USE op5/4/4, including data
master, committed code extent, pre-addend and destination validation index.
The host resolver preserves previously inserted USE bits on op4. It rejects
missing BOs, absent validation entries, undersizing, wrong domains, address or
suballocation misalignment, overlaps, uncovered address bytes, altered control
bits and absent command handles. USE offsets must fit the0x80000 base window;
explicit distinct data-master register reservations are required. This is
CPU arithmetic, not evidence of actual device addresses or reservations.

### Preservation frontier

For these nonfixed placements, `drm_bo_handle_move_mem` takes
`drm_bo_move_ttm`; that unbinds/rebinds the same TTM object. `psb_buffer.c:321–382`
passes the same page array to the MMU; it forces tile strides to zero. CPU
mapping uses those pages with cached or UC/WC protection according to flags
(`drm_vm.c`, `drm_bo_move.c`). Thus the selected reference-family address
transition need not perform an undefined partial copy.

This does **not** finish publication. `psb_invalidate_caches` returns zero
without a payload-cache operation (`psb_buffer.c:196`); the `wmb()` after
`psb_reg_submit` (`psb_sgx.c:487–503`) orders CPU writes but is not by itself an
SGX cache/TLB completion specification. Selected Xpsb0x4f10/0x5040 perform
write/poll/clear operations involving+ad4,+ae0,+804 and status+138/+12c before
kicks. Those CPU transactions are visible; their required target-qualified
publication postcondition is not established. PTE `clflush` in `psb_mmu.c`
is not proof of flushing arbitrary GPU payload caches.

**B3 CONDITIONAL: one remaining rule is the device-publication contract that
makes initialized and relocated validated backing visible to the selected
first consumer.** Same backing, an x86 store barrier, or a passing host resolver
cannot substitute for that rule. Backend preservation shares this obligation;
it is not another request for historical provider identity.

### Final publication discriminator (P7H-051)

The already cataloged, permissively licensed EMGD SGX535 register snapshot
(`docs/poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt`,
SHA256`634fbb74645d373e0c60ad5e01194fa96e91bb6b26d60dbc35a71a2eda49d12c`)
names+ad4 as PDS_INV1 and+ae0 as PDS_INV_CSC (lines344–355). Its+12c status
bit0x04000000 is MADD_CACHE_INVALCOMPLETE (lines135–145). This independently
correlates the selected transactions; it does not define status+138's0x44/1
fields or prove their complete invalidation domain for the target.

[Publication transactions](frozen-triangle-publication.csv) record both possible
cache-control branch values, not a target selection. Xpsb0x4e10 writes, polls
expected bits, clears them, then polls zero. It returns a failure for the first
poll but merely logs the second failure;0x4f10/0x5040 ignore the returned error.
Thus “the helper was called” does not prove completed publication. The new
pure CPU checker requires successful completion **and** successful clear for all
three phases; either failure aborts the proposed clean-room first consumer.
This stricter failure handling is policy, not historical behavior.

`psb_mmu.c:140–180` separately toggles BIF invalidation/flush with a write barrier
and readback; page-table `clflush` is also present. These recoverable transactions
narrow B3 to **whether completion of the selected invalidation/flush sequence
publishes all initialized and relocated payloads to the selected consumers**.
No currently retained applicable source establishes the full postcondition.
The regression cannot manufacture it from three successful Boolean inputs.
This is the precise publication rule to pursue, not another allocator survey.

## B2: selected target layout (P7H-048)

**B2 CLOSED for the selected static surface contract.** This is not a generic
SGX tiling specification or a hardware-success claim. Newly inspected retained
CPU paths join the previously isolated cpp4 descriptor to a concrete layout:

1.0x4c1bc imports scanout stride in bytes and divides by cpp to construct the
region/surface pitch. Public DDX0.32.0 `psbScanoutCreate` likewise allocates
`ALIGN(cpp*width,64)*height`, page-rounded, and exports stride/dimensions through
SAREA (`psb_buffers.c:108–172`, `psb_dri.c:790–840`). At32×32/cpp4 this is
128-byte rows and4096bytes. The public DDX is corroboration, not an ABI oracle.
2.0x4b8e8 stores surface pitch at+8, width/height at+c/+10, suboffset at+4 and
region pointer at+0.0x4effa obtains a direct map through0x4b818, copies surface
pitch into renderbuffer+80, and stores the map at+7c.0x4b818 adds the surface
suboffset to the mapped region;0x4b5b2 maps its BO, without swizzling/copying.
3.0x4c300 dispatches `_ActualFormat=0x8058` to ARGB8888 routines.0x4e638 and
0x4e9a4 explicitly pack `A<<24|R<<16|G<<8|B` at
`map+4*(row*pitch+x)`. Little-endian bytes are B,G,R,A. Renderbuffer+84 controls
row reversal: either `row=y` or `height-1-y`; this changes coordinate origin,
not storage tiling.0x49bc3 provides another producer of that same surface /
format callback contract. No CPU detiling shadow is inserted in this path.
4.The **same surface** enters0x4b710 and0x392ab. cpp4 selects layout code0,
format contribution0 and background format contribution0x0c000000. For pitch32,
origin0 and dimensions32×32 the target words stay
`0x000f8000, GPU(color), 0, 0x0001f01f`. The background words remain unchanged.

The consistency of direct CPU access, surface metadata and the GPU-facing
encoder closes the selected software protocol's layout/channel-order gap. It
does not independently decode event USE instructions; their execution/input
coverage remains B1. The model claims no tiled mode and no scanout ownership.
The target is a standalone zero-initialized color BO. Existing selected raster
branches already disable depth/stencil and multisampling; no new attachments
or hidden compression surface is introduced.32×32, cpp4, pitch32, origin0,
single sample and page-aligned base are the only accepted target in the new
[checker](../../../tools/psb-dri-re/frozen_triangle_contracts.py).

[The target CSV](frozen-triangle-target.csv) records that scope. Exhaustive1024
pixel offsets are unique and within4096bytes. Tests reject wrong stride,
tiling, descriptor words, coordinate bounds and missing initialization.

## B1: auxiliary programs (P7H-049)

[The eleven-program inventory](frozen-triangle-auxiliary.csv) includes vertex
and bounds fetch, three state uploads, event, background and four secondary
records (including fragment secondary). Every committed CPU byte is defined,
except explicit relocations resolved by the BO model. Data-prefix boundaries
are CPU emitter boundaries:48bytes normally,64 for event, zero for the four
single-word secondary records. They are **not architectural accessibility
limits**.

The selected one-descriptor branch of0x381a0 encodes an input length `n<=16` as
`0x80000000|((n-1)<<21)|(n-1)`. Independent CPU inputs are vertex widths8/4 and
state packet lengths11/14/2. The checker cross-checks these descriptors against
source extents; it rejects a contradicted count. This establishes descriptor
construction, not the PDS instruction's complete source set. Vertex launch
selection comes from the deterministic index stream but the complete
architectural launch-provided source domain remains unspecified. Event's
multi-entry/control behavior cannot be reduced to its64-byte CPU prefix merely
because all its words have been initialized. The four standalone secondary
records establish zero CPU data emission, not zero architectural inputs.

**B1 OPEN: missing the selected auxiliary programs' pre-definition input-domain
contract.** No outside input has been demonstrated. No primary L12 result was
transferred as proof for auxiliaries. Background's selected0x07000345 makes the
same completeness issue visible independently; event and vertex programs
also have their own forms. This finite inventory is now the exact obligation,
not another request for a generic ISA survey. Padding zeroing alone cannot
close it. The checker rejects missing programs, missing CPU initialization,
corrupt descriptor length and unsupported coverage promotion.

## B4: revision-compatible service (P7H-050)

The scoped successful requests remain op2, TA engine0/flags4 followed by raster
engine1/flags15. Existing proof excludes scene-less op0 and cookie15 on this
path. Modern gma500 is not a replacement implementation of this interface.
A full X-server/composite stack and its440-byte static blob are not silently
made prerequisites of the selected op2 path: no dependency on that blob is established in the selected normal branches;
the earlier negative direct-reference result does not exclude all indirect use. Nor is a microkernel invented from its label.

The new pure CPU `bootstrap(raw_revision)` evaluates Xpsb0x4af0 fallthrough
from `100+(r&255)+10*((r>>8)&255)`, then0x3820's register values:

| CPU branch code | option+34 | option+ac | bootstrap+a74 | bootstrap+804 |
|---|---:|---:|---:|---:|
|107,108|1|1|0x05021900|0x0100ffff|
|109|0|1|0x05188200|0x0100ffff|
|111,113|0|0|0x05188200|0x0000ffff|

All option dword indices and the twelve initialization register values are
available from the model for each **supplied** revision word. No PCI revision
is accepted; unsupported branch codes are rejected instead of receiving a
convenient default. These are static records only, with `ready=false` even for
synthetic recognized codes. They must never be used as an MMIO script.

`ta_cookie()` reconstructs0x3d70 and the selected0x3df0 flags0x1f CPU cookie.
For8192pages its words2–6 are8192,6656,6656,6656,6592. Page-aligned page-table
address and1MiB-aligned parameter address are explicit inputs. Packed16-bit
endpoints must not overflow. This is **not** a generated TA-table byte image:
0x3df0 issues register transactions and awaits device completion before the
selected scene can proceed. A deterministic cookie does not prove that those
transactions populate the tables correctly on the target.

**B4 OPEN: missing the target-qualified ready-state postcondition for this
revision-conditioned service initialization / TA-table setup.** The target
revision word and an applicable initialization contract would select and
validate these actions; CPU formulas alone do neither. The new model tests
all five branch cases, rejects unknown/PCI inputs, and never marks hardware
ready. gma500 responsibilities, service initialization and clean-room CPU
serialization remain distinct. Gate B is unchanged.

## FG-02 recomputation and deliverable

If L12 closed now, **three** independent groups remain: FT-AUX (B1), FT-BO
publication (B3), FT-SERVICE ready state (B4). Target-layout FT-TARGET and
scoped TA ordering FT-ORDER are closed. FG-02 is **not** otherwise complete.
Whole-path BO and relocation inventories remain PARTIAL because bootstrap and
architectural consumed-state completeness are open; the enumerated payloads
have a complete binding plan and49 wire records.

`frozen_triangle_image.py --bundle` joins CPU images, symbolic address fields,
BO plan, validation nodes, target constraints, auxiliary inventory and closure
obligations. The10 closure conditions remain explicit; consumed-state and
bootstrap UNKNOWNs prevent completion. `--complete` still exits1 in all three
serializers/checkers. No fabricated GPU address enters ordinary output.

## Reproduction, provenance and validation

No new source acquisition and no references/ modifications. Retained DRI:
`74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`;
Xpsb:`da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`.
Existing qualified public sources inspected in this pass are pinned in
[source checks](frozen-triangle-source-checks.json); their acquisition is already
recorded in [source provenance](../source-provenance.md) and
[version pairing](../xpsb-re/version-pairing.md). Decompiler bodies remain in
`/tmp/sgx535-psb-decompile`, never copied into implementation. New targets are
retained in[targets.txt](../../../tools/psb-dri-re/targets.txt), reproducible via
[extraction instructions](../../../tools/psb-dri-re/README.md). `/tmp` project,
exports and JSON bundle are expendable; tools and source hashes are persistent.

```sh
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_*pds*.py'
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_frozen_triangle*.py'
python3 -B tools/psb-dri-re/frozen_triangle_image.py --write-tables
python3 -B tools/psb-dri-re/frozen_triangle_bo.py --write-tables
python3 -B tools/psb-dri-re/frozen_triangle_contracts.py --write-tables
python3 -B tools/psb-dri-re/frozen_triangle_image.py --bundle
python3 -B tools/psb-dri-re/frozen_triangle_image.py --assume-l12-closed
python3 -B tools/psb-dri-re/frozen_triangle_image.py --complete
```

The last command must reject. The existing24 triangle tests are retained;
the old target-blocker assertion is legitimately replaced with a closed-layout
and still-open-backend assertion. New BO and contract regressions are additive.
Passing host tests check this model, not the unknown architectural postconditions.

## Verified checkpoint

Fresh validation:42 existing PDS tests and71 triangle tests pass (24 retained,
22 BO,25 contract). The two ELF hashes and19 qualified public-source hashes
match.34 changed/new CSV schemas,560 local Markdown file links and unique new
P7H-047–051 IDs pass; older multi-row IDs are preserved. Python compilation,
all three complete-mode refusal checks and `git diff --check` pass. The index
is empty; references/ has no changes. No stage or commit was performed.

| Current item | Status |
|---|---|
| B1 auxiliary coverage | OPEN |
| B2 selected target layout | CLOSED |
| B3 allocation/validation/publication | CONDITIONAL |
| B4 service bootstrap | OPEN |
| L12 | OPEN / FROZEN |
| FG-01 / FG-02 | CLOSED / OPEN |
| Whole-path BO manifest / relocation ledger | PARTIAL / PARTIAL |
| Enumerated payload bindings / wire records | DEFINED /49 |
| Target CPU image | READY |
| Auxiliary execution / TA-scene execution | CONDITIONAL / CONDITIONAL |
| Submission template | READY with explicit runtime handles/pointers |
| GPU preservation | CONDITIONAL |
| Static frozen triangle | PARTIAL |
| Hardware-ready / Gate B | NO / BLOCKED |

Remaining static facts are the selected primary source domain (L12), the
selected auxiliary source domains (B1), the complete publication postcondition
of the selected invalidation sequence (B3), and the target-qualified service
ready-state postcondition (B4). Backend realization/validation and the existing
hardware safety gate are additional execution requirements. B2 closure does
not remove any of those conditions.

## Final static closure attempt — P7H-052–054

2026-09-27 continuation. **Static triangle remains PARTIAL; FG-02 remains
OPEN.** This section supersedes the previous checkpoint's counts and records
an additional selected-path attack on B3, B4, all eleven auxiliary records and
then L12. B2 stays CLOSED. No hardware or historical executable was run.

The [shared closure model](../../../tools/psb-dri-re/frozen_triangle_closure.py)
extends the existing image, BO and contract tools. The bundle now includes it.
It supplies [ordered publication edges](frozen-triangle-publication-edges.csv),
[bootstrap prerequisites](frozen-triangle-bootstrap-prerequisites.csv),
[eleven auxiliary domains](frozen-triangle-auxiliary-domains.csv) and a
[shared obligation ledger](frozen-triangle-closure-obligations.csv).
These are proof obligations, not an executable provider. Successful simulated
polls do not promote an architectural postcondition. Missing inventory,
provenance or caller-edited CONFIRMED labels are rejected.

### B3: stronger failure policy tested, scope still missing

The selected reference path validates user BOs **before** fixing up their
relocations (`psb_sgx.c:1283–1295`); scene validation follows at:1394. A model
that applies resolved addresses before validation without an additional
stability guarantee would be wrong. The closure order now preserves this
sequence and joins the separate scene/TA readiness obligation before first
consumption. It accounts for all ten backing roles. LOCAL control and XHW
communication buffers are CPU interfaces, not invented GPU payloads.

Three publication properties remain deliberately distinct:

| Property | Established software action | Limit |
|---|---|---|
| CPU payload visibility | mapping protections; writes and relocations; required CPU store completion | UC/WC versus cached mappings must retain their own guarantees; PTE flushes do not flush arbitrary payloads |
| Translation publication | same-page nonfixed binding; PTE clflush; BIF flush/invalidate with wmb/readback | does not specify PDS/other payload-cache visibility |
| GPU payload invalidation and consumer ordering | Xpsb0x4f10/0x5040 call three0x4e10 phases before TA/raster kicks | completed status bits lack a complete applicable payload-domain postcondition |

`Xpsb_er_kick_wait_clear` at ELF0x4e10 returns -16 if the initial completion
poll fails. It still clears and polls again. A failure of the **clear poll**
only logs; it does not change that return value. The TA/raster kick callers
ignore the helper return. Therefore `ret==0` does not even prove both polls
succeeded, let alone all payloads became visible.

The clean-room policy requires **both polls of all three phases**, aborting
before a consumer on any failure, and forbids writes after publication without
renewed publication. This closes the *policy ambiguity* about continuing after
timeout; it does not close B3. Even granting all six observations, the following
implication is unavailable:

`successful selected CPU/translation/device publication -> every selected
initialized and relocated payload visible at its first GPU consumer`.

In particular the existing SGX535 header does not define status+0x138 masks
0x44/0x1 and their completion domains. The name of +ad4/+ae0 or MADD completion
alone cannot supply that implication. An extra barrier, fresh GPU address or
zeroed BO cannot substitute for a cache-domain contract. The narrow public
queries `"SGX535" "EVENT_STATUS3"` and `"PDS_INV1" "INVALCOMPLETE"` yielded no
usable result in this pass; no source was acquired. This is a bounded negative
check, not a claim that no public contract exists.

**B3 remains CONDITIONAL on this one complete publication rule.** Backend
preservation is the same obligation, not an additional historical-identity
requirement. The ordered checker rejects missing invalidation, consumer-before-
publication, relocation-before-validation and completion/clear timeouts.

### B4: reply success is weaker than required readiness (P7H-052)

The selected first TA request requires validated scene/backing, reserved USE
bases, revision-conditioned service state, loaded TA tables and an available
op2 request interface. The prerequisite table tracks each producer and first
consumer; it does not introduce an unobserved firmware/microkernel requirement.

The initial TA load flags are0x1f (`psb_scene.c:257–265`, flags in `psb_drm.h`):
TA1 | RASTER2 | HOSTA4 | HOSTD8 | INIT16. In `Xpsb_ta_mem_load` at ELF0x3df0:

| Request bit | Kick offset | Bit ORed into retained poll mask | Public SGX535 status2 name/mask |
|---|---|---|---|
|1|684|1|DPM_TA_FREE_LOAD /1|
|2|680|2|DPM_3D_FREE_LOAD /2|
|4|688|4|DPM_HOST_FREE_LOAD /4|
|8|690|**4**|DPM_DHOST_FREE_LOAD /**8**|

All four kicks therefore produce retained poll mask **7**, not15, at status118
and clear114. This is checked directly in the authoritative ELF: at0x3f49 the
flag is masked with8; at0x3f51 `or ecx,4`; at0x3f79 the store to+690. The public
header's distinct DHOST bit is at lines127–134. This is a concrete counterexample
to treating the retained mask as an observation of four distinct completion
bits. It is **not** a proved hardware bug, nor proof that bit8 is required on
every retained revision. Target applicability and dependency are still missing.

After successful initial polling and with INIT set,0x3df0 kicks+6a8 and polls
status12c bit400000 (public name DPM_INITEND), clearing through134. Raw return
flow at0x3fc2–0x3fd4 and the dispatcher0x30f0 case9 show the helper result is
returned to the request. A decompiler's `void` label for0x3df0 must not be used
to claim its return is ignored. `psb_xhw.c:300–337` waits for `done`, returns
-EBUSY without a reply, otherwise returns `xa->ret`, and copies the cookie only
on zero. This proves transport/reply handling, not readiness beyond the polls.
The helper still has the clear-timeout weakness described above.

For initialization,0x3820 writes the recovered revision-dependent registers and
returns0 without a ready poll; dispatcher case4 explicitly sets reply ret0.
The five evaluated revision branches retain different+0xa74/+0x804 values.
Neither their common successful return nor identical32×32 scene-cookie sizing
proves equivalent initialized service state. No SGX revision is inferred from
PCI revision.

A stronger clean-room rule can reject reply/poll/clear failures. Requiring
status mask15 would additionally observe the header's fourth load bit **if the
header contract applies**, but cannot be installed as a proven target fix merely
because it looks safer. Dropping HOSTD likewise lacks a selected-path proof.

**B4's remaining rule:** a target-qualified ready predicate for the recovered
revision-conditioned service initialization and all TA state required by the
first op2 request, including whether/when the fourth load is complete. An
applicable completion contract or proof that the fourth load is unnecessary
would distinguish the remaining interpretations. It must also qualify the
initial service register state; changing7 to15 alone does not close B4.

### B1: eleven records, five instruction streams (P7H-053)

All eleven full CPU images, symbolic relocation words, historical hole offsets,
launch reference sites and launch-record words are now joined in one table.
`R(...)` abbreviations use the existing relocation ledger, including its ORed
control bits. Instruction lists are little-endian32-bit words. The grouping is
byte equality only, not decoded opcode equivalence.

| IDs | Records | Instruction words | Complete input coverage |
|---|---|---|---|
|AUX-01/02|vertex / bounds vertex|67800070 2f030343 030803e5 af000000|UNKNOWN|
|AUX-03/04/05|bounds / triangle / terminate state|07030223 07042345 af000000|UNKNOWN|
|AUX-06|event|21 words, exact list in CSV/model|UNKNOWN|
|AUX-07|background|07000345 070418a2 07042364 af000000|UNKNOWN|
|AUX-08/09/10/11|fragment / vertex / bounds vertex / background secondary|af000000|UNKNOWN|

The event record's21 words, in order, are:

```text
cf820030 87600000 90000005 070003e5 af000000
cf820830 87600000 90000014 07042363 170086e0
f7800b31 cf621031 c762c070 f7800b32 cf641432 c764c070
f7811933 cf661833 c766c070 070b0345 af000000
```

Controlled comparisons use only this already selected set, not a new42-row
survey. AUX-01/02 vary CPU data dwords0/1/4/8 (source address, descriptor, USE
reference, stride) with identical code. AUX-03/04/05 vary dwords0/1/2 for state
lengths11/14/2, again with identical code. Secondary code and zero-length CPU
prefixes are identical. These variations expose no selector between initialized
constant state and a fixed additional source. Increasing descriptor length
must not be interpreted as a change in architectural source eligibility.

AUX-09/10 are allocated by0x40355->0x3faa3, but0x3b890 serializes only the
primary address plus primary counts. No explicit launch reference to those two
secondary objects occurs in the scoped relocation ledger. They remain in the
allocation inventory; no unknown implicit-address behavior is invented to
assert that they execute, and lack of a reference is not promoted to a general
architectural no-read theorem. Even omitting them as independent launch
obligations would not close B1: AUX-08/11 explicitly reference the same terminal
form, and the other four families remain unresolved.

For every family the CPU image is fully initialized, modulo explicit symbolic
relocations; holes are Route C zero policy. That covers data **if** loaded and
selected, and deterministic encoded control. The residual pre-definition source
class remains UNCLASSIFIED, not an assertion of a temporary/outside/persistent
read. No write-before-read or launch-reset rule is inferred from the program
order. The exact missing fact is family-and-launch-specific source eligibility:
vertex fetch (including any launch-provided selection), state upload, event
entry/control, background, and standalone terminal. No individual family gains
implementation-grade coverage merely from CPU initialization.

### One final L12 propagation

AUX-07 begins with the exact primary word07000345, but its full launch tuple is
`R(background_secondary),00030001,R(background_pds)|0c000000`, whereas the primary
uses middle word00030000. Its remaining code also differs. Matching the first
word does not establish that its entire source domain is local to either CPU
prefix, or that launch context cannot affect it. The shared terminal word occurs
both alone and following other words; no new selected read-before-write rule
is exposed. The event's070b0345 remains the already-established whole literal;
its unexplained fields are not reinterpreted here. Q remains unrelated to CPU
prefix size. Experiment43 and the old corpus were not repeated.

Thus B1 and L12 share a **source-eligibility contract obligation**, with five
auxiliary stream families plus the primary launch context. This is one rule
category with multiple scoped instances, **not a claim that one unnamed selector
bit or one test would settle every program**. Equal code allows reuse of a future
context-independent rule only after that independence is established.

L12 remains OPEN: complete pre-definition source domain of07000345;af000000
under the selected primary launch. No actual outside read, persistent readable
state or narrower bank/selector discriminator was discovered. Perfect BO
publication would not resolve this architectural question.

### Recomputed closure and minimum obligations (P7H-054)

| Item | Current result |
|---|---|
|B1|OPEN: family/context-specific pre-definition source eligibility|
|B2|CLOSED, unchanged|
|B3|CONDITIONAL: successful complete publication implies first-consumer visibility|
|B4|OPEN: target-qualified service/TA ready predicate|
|L12|OPEN: selected primary pre-definition source domain|
|FG-01 / FG-02|CLOSED / OPEN (B1+B3+B4+L12)|
|Known CPU objects / wire relocation sites|51 /49 wire records, unchanged|
|Whole-path BO manifest / relocation completeness|PARTIAL: enumerated CPU payloads covered; unproved service state remains|
|Target / submission CPU images|READY / READY with explicit parameters|
|Auxiliary / primary execution and TA-scene|CONDITIONAL|
|GPU contents preservation|CONDITIONAL, same B3 obligation|
|Static triangle / complete serializer|PARTIAL / REFUSES|
|Hardware executed / Gate B / whitelist / functionality|NO / BLOCKED / [] / UNVERIFIED|

The minimum remaining **rule groups** are PUB (B3), READY (B4), and SOURCE_DOMAIN
(B1+L12). Their distinct scoped missing facts remain explicit in the ledger.
No evidence equates cache completion with TA readiness, or either with PDS
source eligibility. No bootstrap-only, backing-only or CPU-image-only policy
eliminates the other groups. Even assuming L12 closed, B1/B3/B4 remain.

All currently examined retained producers, selected caller/consumer paths and
qualified register evidence stop at these contracts. This is a bounded static
result, not an exhaustive claim about every public document. The next useful
input is an applicable contract distinguishing one of those interpretations;
another repetition of the existing producer corpus will not do so.

Additional reproduction commands (repository root):

```sh
python3 -B tools/psb-dri-re/frozen_triangle_closure.py --write-tables
python3 -B tools/psb-dri-re/frozen_triangle_closure.py --json
python3 -B tools/psb-dri-re/frozen_triangle_closure.py --complete
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_frozen_triangle*.py'
```

The third command must exit1 with B3/B4/B1/L12. New code performs only host
arithmetic and model validation. Temporary decompilations are not persistent;
reproduce0x30f0/0x3820/0x3df0/0x4e10/0x4f10/0x5040 from the retained Xpsb using
the existing static extraction instructions. The key fourth-load observation
can independently be reproduced with `llvm-objdump -d --start-address=0x3f49
--stop-address=0x3fd5 <retained-Xpsb.so>`. No new artifact was added to references/.
After static closure, the next stage is hardware-validation gate work; this
pass does not authorize or perform it.

### Validation of P7H-052–054

42 existing PDS tests and96 triangle tests pass: the previous71 triangle tests
plus25 new closure regressions. All four complete CLIs refuse. The joined JSON
bundle checks51 objects,10 backing roles,6 validation entries,49 wire records
and11 auxiliary records. Python compilation passes. Both retained ELF SHA-256s
and all19 recorded qualified-source SHA-256s match.38 changed/new CSV schemas,
569 local Markdown file links and unique new evidence IDs052–054 pass; prior
multi-row IDs remain untouched. `git diff --check` passes, the index is empty
and references/ is unchanged. No stage, commit, historical execution or hardware
operation occurred.

`/tmp/sgx535-final-static-bundle.json` is disposable and not needed by the next
session; regenerate with `frozen_triangle_image.py --bundle`. The checker,
regressions, CSVs, evidence and reproduction paths are persistent in the working
tree. Transfer untracked files with the rest of the project.

## Three-rule continuation — P7H-055–057

2026-09-27. **No architectural rule closed.** This continuation follows LOAD3
forward, distinguishes fresh observations from completion semantics, consumes
the existing PDS constraints without regenerating them, and audits the meaning
of the manifest/relocation PARTIAL labels. B2 stays CLOSED. The new artifacts
are [four load paths](frozen-triangle-load-paths.csv),
[later selected waits](frozen-triangle-later-waits.csv),
[five canonical families](frozen-triangle-pds-families.csv) and
[three distinguishing propositions](frozen-triangle-rule-discriminators.csv).
All are emitted by the existing shared closure tool; the image bundle includes
these additions automatically. No broad provider, opcode or corpus survey ran.

### R1: completion observations versus publication semantics

The ordered BO/relocation/publication chain from P7H-054 is retained. There is
no new evidence that a successful status138:44/1 observation covers all required
payload domains. The allowed SGX535 register snapshot names the request
registers but not that completion-domain relation. CPU store ordering, PTE
visibility, payload-cache visibility and first-consumer ordering remain distinct.
An observed request followed by a successful poll is not a device contract.

The stronger policy now checks a **fresh completion cycle**: earlier operations
quiescent; required status bits observed clear before the request; all required
bits observed set after the request; bits cleared afterwards. This removes a
second observational ambiguity beyond timeout handling. In particular, changing
TA mask7 to15 while accepting a pre-existing bit8 is insufficient. No actual
stale hardware bit has been observed; this is an adversarial policy case.
`check_completion_cycle()` returns only `OBSERVATIONS_ONLY`, never readiness or
publication CONFIRMED. It checks supplied numbers and performs no register I/O.

The existing historical path can kick consumers after failed polls; the new
policy rejects that behavior. Even granting fresh successful observations and
all established mapping/translation facts, R1 still needs the applicable
completion-domain implication. A clean-room failure policy cannot supply
missing hardware semantics. No additional selected BO class was proved to lose
contents, and no blanket cache-coherence assumption was added.

### R2: LOAD3 forward trace (P7H-055)

The four CPU branches share cookie0 (page-table GPU reference), cookie10/11
(first/last packed page endpoints) and cookie12 (previous endpoint). The register
triples below receive `cookie0`, `(cookie11<<16)|cookie10`, `cookie12<<16`.
LOAD0 additionally writes the already recovered thresholds/counts.

| ID | Header/request label | Descriptor register triple | Kick | Header status118 bit | Retained mask contribution |
|---|---|---|---|---|---|
|LOAD0|TA|618,61c,648|684=1|1|1|
|LOAD1|3D / RASTER|600,604,64c|680=1|2|2|
|LOAD2|HOST / HOSTA|610,614,654|688=1|4|4|
|LOAD3|DHOST / HOSTD|608,60c,650|690=1|8|4|

These are CPU writes and header-name correlations. The header calls the events
DPM free loads. It does not identify the internal table access that first
consumes each result; **DHOST is not expanded into an invented deallocation
mechanism**. The fourth branch tests request flag8 only. With the selected
flags1f, it is executed regardless of the retained revision-option switch.
Thus a CPU-side revision guard that suppresses LOAD3 is contradicted for this
request. A revision-specific hardware meaning remains unproved.

The forward sequence was followed through the selected scene handlers:

1. `Xpsb0x3df0` starts LOAD3, then waits status118 mask7. INIT then waits
   status12c mask400000 (header name DPM_INITEND).
2. Op9 returns the helper result. Kernel `psb_xhw_ta_mem_load` waits for the
   reply and returns it. `psb_scene.c:267` aborts on nonzero; transport success
   establishes no extra device dependency.
3. The fresh TA op2 path, engine0 flags4, enters `Xpsb0x4550`'s clear branch.
   It writes scene controls and waits status12c mask100a40. The header decomposes
   this into OTPM_INV100000, TPC_CLEAR800, DPM_CONTROL_CLEAR200 and
   DPM_STATE_CLEAR40. This is **another status bank**, not a later wait for
   status118 bit8. Its dominance over LOAD3 is not specified.
4. The three already modeled publication phases precede the TA kick. None
   polls status118 bit8. Candidate scheduling later handles TA completion and
   dispatches raster; TA completion is not documented as a DHOST-load guarantee.
5. Selected raster flags15 and cookie14=0 take `Xpsb0x4030`'s ordinary path:
   `(flags&9)!=1`, so its pending-load wait mask stays zero; the nonzero-cookie14
   recovery branch is not selected. It writes63c=3,658=0 and408 from scene base
   plus cookie7. It adds **no scene-load wait**. The three publication phases
   then precede the raster kick.

The newly restored0x4030 export was made read-only from the existing disposable
Ghidra project. Its register-argument convention must be retained; it is not a
new standalone executable. No decompiled body was copied into the repository.
The actual first *hardware* consumer of LOAD3 remains UNKNOWN: the trace reaches
selected TA/raster service boundaries but cannot inspect that internal device
edge. We do not label the first subsequent CPU request as its actual consumer.

The IRQ alternative was checked in the already acquired candidate source:
`psb_irq.c:181–186` masks status118 with `sgx2_irq_mask`;:254 sets that mask to
BIF_REQUESTER_FAULT10 only. The scheduler receives status12c, not LOAD3 bit8.
Consequently this handler neither acknowledges bit8 on the selected normal path
nor turns it into a readiness fence. IRQ uninstall clears all current status,
but is not on the successful first-draw path and is not a readiness operation.
The source hash has been added to the existing hash ledger (20 files total).

| Proposed explanation | Result for the examined path |
|---|---|
|A: LOAD3 unnecessary before draw|UNRESOLVED; CPU requests it; necessity/internal consumer unknown|
|B/D: separate explicit wait elsewhere|No such wait in the selected load→TA→raster/IRQ path; no global absence claim|
|C: implicit ordering before first actual consumer|UNRESOLVED; requires INITEND/scene/consumer dependency contract|
|E: revision makes CPU load inactive|CONTRADICTED for selected flags1f; no CPU revision guard in this branch|
|F: historical bug|UNRESOLVED; source mismatch is not proof of a hardware bug|
|G: header meaning/applicability differs|UNRESOLVED; symbol names alone do not qualify the retained revision|

Exact public searches for `DPM_DHOST_FREE_LOAD DPM_INITEND` and `DPM_DHOST LOAD
SGX535` returned no usable result; no new package or implementation source was
acquired. The source masks already establish bit positions in the header, not
independent completion semantics or applicability to the retained branches.
Requiring a fresh mask15 cycle is a legitimate **candidate policy**, but cannot
become BOOTSTRAP_READY without those premises. Initial register writes returning
zero remain separately uncertified by a ready predicate; the unchanged five
revision branches are not reclassified as equivalent merely because they return.

### R3: canonical families and inherited constraints (P7H-056)

The canonical table contains one row per distinct auxiliary instruction stream,
with all member launch records, data extents/initialized offsets, USE relocations,
roles and instruction-by-instruction pre-definition questions. It references the
existing42-row semantic corpus, field constraints, differential matrix and bit
influence table. Their file hashes are included in the bundle; those artifacts
are consumed, not regenerated by the new tool.

FAMILY-1 is AUX-01/02; FAMILY-2 AUX-03/04/05; FAMILY-3 AUX-06;
FAMILY-4 AUX-07; FAMILY-5 AUX-08/09/10/11. All words in families1/2/4/5 already
occur as literals in the semantic corpus. Family3 has15 distinct additional
fixed control words (16 occurrences because87600000 repeats). Those words are
already in the selected event image, not newly discovered producer variations.
No independent selector-setting input was exposed by joining them. Existing
A2/64 CPU formula matches and45-class correlations retain their original scope.

The timeline explicitly separates CPU initialization before launch from
architectural instruction reads and definitions. The former covers every CPU
prefix byte with symbolic relocation provenance. For each instruction, the
latter remain UNKNOWN. Code order is not sufficient to assert an earlier write
produces a later operand. No arbitrary store/register is added to the model.

Each family still lacks this scoped rule:

| Family | Exact remaining domain question |
|---|---|
|1 vertex|Pre-definition inputs of67800070;2f030343;030803e5;af000000 under the two selected index launches|
|2 state upload|Pre-definition inputs of07030223;07042345;af000000 for the11/14/2-dword state sources|
|3 event|Pre-definition inputs along the selected event entry/control paths through its21 words|
|4 background|Pre-definition inputs of07000345;070418a2;07042364;af000000 under middle launch word30001|
|5 secondary|Pre-definition inputs of standaloneaf000000 in the explicitly referenced secondary contexts; allocated-only vertex records remain separately flagged|
|Primary L12|Pre-definition inputs of07000345;af000000 under middle launch word30000|

A retained CPU emitter is not itself an executed/validated hardware program.
Even granting historical validity, an extra deterministic historical launch
producer not yet reconstructed would preserve all observed CPU emissions.
Thus validity does not force the conclusion that all inputs are in our known
prefix. This is a logical remaining model, **not evidence that such a producer
or outside read exists**.

Finite-domain route: the CPU interleave/count rules bound the serialized prefix,
not the total architecturally eligible DS/temporary/implicit state. Full128KiB
PDS-BO initialization does not establish a map covering every such location.
Neither a finite eligible bank extent nor a complete initializer for it is
established by the inspected evidence. No family closes, and nothing new removes
the primary L12 domain obligation. Exact semantics and exact reads stay UNKNOWN.

### Precise discriminators and scope audit (P7H-057)

The machine-readable discriminator table gives the requested MODEL A / MODEL B
for each rule. They are logical interpretations still consistent with current
CPU evidence, not asserted hardware capabilities:

- **R1:** A — successful fresh maintenance covers all required payload domains;
  B — the same observations leave one required domain uncovered. Distinguishing
  fact: applicable completion-domain implication for the selected mapping and
  invalidation path. Narrowest evidence: a qualified cache/publication contract,
  including status138 masks44/1; caller continuation is insufficient.
- **R2:** A — the selected initialization/INITEND/scene sequence dominates all
  required initialized state including LOAD3; B — it can precede readiness of
  LOAD3 or another required revision-conditioned state. Distinguishing fact:
  target-qualified BOOTSTRAP_READY implication. Narrowest evidence: applicable
  DHOST consumer/dependency semantics and initialization postcondition. This
  preserves both constituent obligations; mask15 alone is not a complete fix.
- **R3:** A — every selected pre-definition input is supplied by known initialized
  prefix/control; B — an additional unreconstructed launch/state producer is
  required. Distinguishing fact: complete eligibility rule for the five streams
  and primary contexts. Narrowest evidence: applicable source/preload rule or
  bounded accessible-state initializer. No selector bit is invented.

R1 and R2 cannot be merged: their polls address different properties and no
observed completion rule equates payload visibility with table/service readiness.
B1 and L12 share R3 but retain the six scoped domain questions above.

The PARTIAL-label audit finds **no missing enumerated CPU relocation site**:
all49 wire records and all targets pass the existing checks. That scoped wire
ledger is COMPLETE. Whole-path contract/manifest remains PARTIAL because
`scene_hw`, `ta_page_table`, and `ta_parameter` retain provider-generated contents
under R2, and publication is conditional under R1. These are known represented
objects, not newly invented BOs or unlocated wire relocations. No additional
independent relocation blocker is introduced. Runtime handles/addresses are
parameters, not missing static facts.

FG-02 was recomputed from the unchanged target and updated shared model:
R1(B3), R2(B4), R3(B1+L12) still prevent closure. The complete serializer still
refuses, while the known51 objects,10 BO roles,6 user validations,49 wire records,
68-byte TA stream and144-byte submission remain intact. Gate B BLOCKED,
whitelist[], hardware unexecuted and UNVERIFIED.


### Verified three-rule checkpoint

42 PDS tests and108 triangle tests pass: all prior96 plus12 focused regressions.
All four complete modes refuse. The bundle preserves51 objects,10 BO roles,
6 validations and49 wire records, with four load paths and five canonical
families. Both retained ELF hashes and20 source hashes match.42 changed/new CSV
schemas,576 local Markdown file links, unique new P7H-055–057 IDs, Python
compilation and `git diff --check` pass. Prior multi-row evidence IDs are
preserved; the index is empty and references/ is unchanged. A separate read-only
review found no defect; its qualification is that observation freshness assumes
quiescence and applicable status semantics, neither proved by the host guard.

The expendable `/tmp/sgx535-three-rule-bundle.json` is reproducible with
`frozen_triangle_image.py --bundle`. `/tmp/sgx535-three-rule-decompile.log` records
restoration of0x4030; the existing read-only Ghidra procedure and retained hash
reproduce its input. No new source URL or acquisition needs preserving.


## P7H-058 — bounded external acquisition

[Six new public source reviews](final-rules-external-evidence.md) did not supply
an applicable R1/R2/R3 implication. Independent SGX535 bit8 corroboration does not
close LOAD3 readiness; Linux host DMA contracts do not certify internal cache
completion; SDK manifests/patents provide no complete selected source domain.
The [source registry](final-rules-external-sources.json) pins nine acquired bodies;
[search outcomes](final-rules-external-search.csv) separate access failures and
source qualification from architectural exclusions. No model promotion or code
change.42 PDS and108 triangle tests remain passing; complete refuses.

## P7H-060 — stronger clean-room contracts, unresolved hardware facts

[The clean-room closure report](cleanroom-final-closure.md) replaces historical
stale-hole values, timeout continuation and the mask7 LOAD3 poll with explicit
full-BO zeroing, abort-on-failure publication and fresh full-mask bootstrap
policies. The shared checker emits [policy/fact rows](cleanroom-final-rules.csv),
[eight GPU BO domains](cleanroom-publication-domains.csv) and
[six PDS launch domains](cleanroom-source-domains.csv). They do not authenticate
GPU visibility, LOAD3 readiness or source eligibility. R1/R2/R3 remain OPEN;
B3 CONDITIONAL, B4/B1/L12 OPEN; B2 CLOSED; FG-02 OPEN. The 49 emitted CPU
wire records remain covered, while the whole-path ledger is PARTIAL. The
historical SGX535 PDF is optional future evidence, not a prerequisite.
Complete mode continues refusing. 42 PDS +114 triangle host tests pass; no
hardware operation or references/ change occurred.
