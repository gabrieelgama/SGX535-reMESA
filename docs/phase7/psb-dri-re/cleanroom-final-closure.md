# Strong clean-room contracts for the final frozen-triangle rules

**P7H-060, 2026-09-27.** This pass replaces three historical *software* dependencies with stricter clean-room policies. It does **not** establish the three remaining SGX535 hardware implications. R1, R2 and R3 therefore remain OPEN; the static frozen triangle stays PARTIAL and `--complete` still refuses. The [historical PDF hunt](sgx535-pds-document-hunt.md) is finished for this path: the PDF was not recovered, may be useful future evidence, and is **not a project prerequisite**. No historical provider, stale-hole, PDS ISA or exact-read conclusion is rewritten.

The machine-readable [rule table](cleanroom-final-rules.csv), [eight GPU-facing publication domains](cleanroom-publication-domains.csv), and [five auxiliary families plus primary source-domain table](cleanroom-source-domains.csv) are emitted by the [shared closure checker](../../../tools/psb-dri-re/frozen_triangle_closure.py). The [BO resolver](../../../tools/psb-dri-re/frozen_triangle_bo.py) now explicitly clears an entire supplied user backing, even if it arrives dirty, before writing objects and applying relocations. These host checks are `ENFORCED_CONTRACT` in the **static model**, not an implemented device backend or authenticated register observation.

## Dependency replacement

| Rule | Historical dependency removed | Strong clean-room contract | Unavoidable hardware fact at present |
| --- | --- | --- | --- |
| R1 / B3 | Historical callers may continue after timeout; that behavior need not be copied. | Allocate/zero, populate, validate, relocate, finish writes, publish CPU data and translations, require three fresh successful invalidation/clear phases for **every** GPU-facing BO, then permit its first consumer. Any failure aborts. | Completed mapping/maintenance must make the selected payload and translation domains visible to their first consumers. The status `0x138:0x44`, `0x138:0x1`, and `0x12c:0x04000000` observations have no complete target-qualified payload-domain postcondition. |
| R2 / B4 | The retained `0x7` load poll is not the new readiness criterion. | Qualify the raw SGX revision; require fresh `0x118` completion mask `0xf`, INITEND and required table/service observations; abort on missing LOAD3 bit `0x8`, stale status, timeout, unsupported revision, or incomplete tables. | For that qualified revision, the fresh LOAD3/INITEND/table/service observations must imply that DHOST data and revision-conditioned state are consumable before the first selected TA operation. |
| R3 / B1+L12 | Recycled historical PDS-hole contents are not a new-driver input requirement. | Explicitly zero every byte of each CPU-controlled user BO, including all `0x20000` bytes of the PDS BO, then write known constants/programs and final relocations. | Every eligible pre-definition source for each selected program must be supplied by that initialized backing or deterministic launch control, or be deterministically defined before its first read. This includes the unknown DS preload mapping and any eligible temporary/implicit state. |

This removes reliance on *historical timeout continuation*, *historical mask `0x7`*, and *historical recycled-hole values*. It does not remove hardware facts by renaming them as policy. The historical provider identity and exact `0x07000345` read set remain UNKNOWN/CONDITIONAL as previously recorded.

## R3: maximal controlled storage, still an architectural boundary

The six selected launch contexts (five auxiliary stream families and primary) all reside in the clean-room `pds` BO in the current 10-role plan. Its size is `0x20000` bytes. The new initializer checks the exact sizes of all six user backings and zeros each entire allocation before object bytes and the 49 wire-relocation writes are applied. The resolver rejects a missing, immutable or truncated supplied backing. The pure-host test supplies dirty bytes in **every** user BO; after resolution, padding as well as the selected primary image has deterministic CPU contents. Kernel/service-owned scene and TA objects remain separate producers and are not silently declared CPU-zeroed by this function.

For every family, the known CPU candidate inputs—backing bytes and encoded launch controls—are deterministic when the symbolic addresses are resolved. The `source_dominance` checker can prove set inclusion **only after a complete candidate set is supplied**; a deliberately unclassified candidate causes it to reject. The real six rows retain `pds_architectural_source_state_not_proven_from_initialized_backing_or_controls`, so no family is promoted. This single label describes an **eligibility proof gap**, not a demonstrated outside read. Specifically, the CPU's full `0x20000` PDS allocation does not prove that DS0/DS1 entries map to it, that unpreloaded entries are inaccessible, or that temporary/implicit state is unreadable or deterministically initialized. None of the selected instruction words has an evidence-backed complete pre-definition source envelope. The same unresolved architectural rule applies separately to each auxiliary launch and to primary `0x07000345;0xaf000000`.

**R3 surviving fact:** for each selected instruction stream and launch context, its *complete architecturally eligible pre-definition input domain* must be contained in the state established by its fully initialized BO and deterministic launch controls, or in a state deterministically defined before first read. No exact read-set decode is required if this containment is established. Without that fact, neither B1 nor L12 closes. Enlarging a CPU BO or clearing an unspecified hardware register cannot establish it.

## R1: publication state and per-BO coverage

The clean-room order is:

```text
ALLOCATED -> CPU_ZEROED -> CPU_POPULATED -> VALIDATED_BOUND
          -> RELOCATED -> SCENE_TA_READY_JOIN -> LAST_CPU_WRITE
          -> CPU_MEMORY_PUBLICATION -> TRANSLATION_PUBLICATION
          -> DEVICE_INVALIDATION_COMPLETE -> FIRST_CONSUMER
```

Any incomplete/reordered sequence, failed poll/clear, or missing GPU BO coverage aborts. No later CPU write is allowed without renewed publication. The domain table covers eight non-LOCAL roles: `pds`, `use`, `vertex_ta`, `background`, `color`, `scene_hw`, `ta_page_table`, and `ta_parameter`. The first five have user CPU images; the last three have service/kernel producers. The table distinguishes relocation writes, mapping and translation domains, and first consumers. `control` and `xhw_comm` are CPU interfaces, not falsely added as GPU payloads. The existing 49 wire relocation records are reused rather than rediscovered.

The model proves the chosen **software order and failure policy** for supplied observations. It does not prove CPU cache writeback for every mapping, a selected translation postcondition, or that the three device-status completions cover every PDS/USE/TA/raster payload. A fresh successful poll can be narrower than visibility to all first consumers. Therefore B3 and GPU contents preservation remain CONDITIONAL. The minimum remaining R1 fact is the composition of the selected mapping/CPU-publication contract with the target's `0x138` and `0x12c` completion domains for these eight BO roles. A different future backend could discharge the CPU edge independently, but its use has not been implemented or proved here.

## R2: stronger observations, not inferred readiness

The selected TA path starts four loads. The retained poll covers `0x7`; the applicable public header names LOAD3/DHOST completion bit `0x8`. The clean-room policy requires a cleared pre-request baseline, fresh observation of all `0xf` bits, successful clear, INITEND, a qualified SGX revision and required table/service completion. The host checker rejects synthetic revision provenance in this policy check and rejects any missing bit or readiness input. Passing that check returns **`ENFORCED_CONTRACT_ONLY`**. It does not certify that the status bit is produced only after the hardware state is consumable; a caller-supplied boolean is not hardware evidence. No bit `0x8` wait was added to historical claims or executed on a device.

**R2 surviving fact:** for the positively qualified target, fresh bit `0x8` together with INITEND and the selected table/service observations must imply LOAD3/DHOST and revision-dependent state are ready before the first TA consumer. If a revision cannot be qualified, the clean-room implementation must reject it rather than choose a default. The current repository does not authenticate that postcondition; B4 remains OPEN.

## Recomputed closure

| Item | Result |
| --- | --- |
| B1 auxiliary input coverage | OPEN: five family contexts still need the same source-eligibility rule |
| B2 target layout | CLOSED, unchanged |
| B3 publication / GPU preservation | CONDITIONAL: software abort/order strengthened; hardware visibility postcondition unknown |
| B4 service/bootstrap | OPEN: fresh full-mask policy modeled; target readiness implication unknown |
| L12 primary source eligibility | OPEN: same source-eligibility rule in its separate launch context |
| FG-01 / FG-02 | CLOSED / OPEN |
| BO manifest | PARTIAL overall; 10 roles and six user validation entries remain modeled |
| Relocation ledger | 49 emitted CPU wire records COMPLETE; whole-path contract PARTIAL for service-generated state/publication |
| Static triangle / complete serializer | PARTIAL / REFUSES |
| Hardware executed / validated; Gate B | NO / NO; BLOCKED, whitelist `[]` |

The complete serializer was rerun and refused `L12, FT-AUX, FT-BO, FT-SERVICE`. The full host suite passed **156 tests: 42 PDS and 114 triangle**. New negative tests cover dirty/partial backings, missing BO publication, failed maintenance, LOAD3 absence, stale or incomplete bootstrap observations, unsupported revision provenance, and unclassified source inputs. This is static validation only; no SGX ioctl, command, MMIO or proprietary executable ran.

**Can the remaining facts be designed around? PARTIALLY.** Dirty backing, historical timeout continuation and mask `0x7` are eliminated as dependencies by software policy. The three remaining implications concern what the selected hardware exposes and when; no currently evidenced software action makes them true by construction. Full static closure would be unsound until those facts are supported or a different, fully specified backend/launch path removes the relevant hardware edge.

An adversarial check explains the boundary. Two hardware interpretations can
produce the same fresh maintenance bits while differing on whether the first
PDS/TA fetch sees the final BO bytes (R1). They can produce the same full
`0xf` load bits and INITEND while differing on whether LOAD3's result is
consumable at the first TA use (R2). They can receive identical fully zeroed
BOs and launch words while differing on whether a selected instruction may
read an undefined on-chip source (R3). These are logical countermodels to a
software-only proof, **not observed SGX535 failures or established outside
reads**. The narrowed hardware postconditions in the table exclude exactly
those countermodels.
