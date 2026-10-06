# Reusable render frames: correctness versus historical policy

This document designs an eventual reuse path. It does not remove the current
one-shot limit or claim that frameN+1 is safe. See the
[capability matrix and next experiment](FIRST-REAL-3D-ROADMAP.md).

## Current implementation boundaries

| Mechanism | Category | Current source/evidence | Required evolution |
| --- | --- | --- | --- |
| Correct rev121 identity,bootstrapping,TA/raster/DHOST protocol | Hardware/software correctness | [contract](../../tools/psb-dri-re/frozen_kernel_contract.c),[service](../../tools/psb-dri-re/frozen_fixed_service.c),FIRE #3 completion | Retain; repeated completion must still consume all required DHOST fields |
| Exclusive mutex,owned/pinned BOs,power/translation holds | Resource ownership | [owner](../../kernel/sgx535_frozen/gma500_bo_owner.c),[entry](../../kernel/sgx535_frozen/gma500_fixed_entry.c) | Retain until no reader/writer can touch reused bytes |
| Startup reset/source-lifecycle boundary,continuous producer/reset/PM isolation | Correctness and attribution | [source guard](../../tools/psb-dri-re/frozen_source_guard.c),[observer](../../kernel/sgx535_frozen/gma500_capsule_observer.c) | Startup-only provider does not establish post-frame delayed-work exclusion; add a justified rearm provider before reuse |
| Accepted current-operation event ledger and capsule lifecycle | Attribution | [capsule](../../tools/psb-dri-re/frozen_capsule.c),fixed service | Per-frame fresh admission and immutable prior result; CLOSED is not reusable UNUSED |
| Keep resources after possible issuance/ambiguous completion | Correctness | Entry/service HOLD paths | Retain; no automatic reset/free/retry |
| Static `fixed_attempt_used` and `run_once` | One-shot policy plus protective stop | Entry sets consumed before execution; service rejects reuse | Replace only after authoritative rearm semantics exist; deleting the flag is insufficient |
| Historical FIRE card/manualFIRSTLOAD/quota/controller hashes | Experimental policy/integrity | [FIRE #3 procedure](FIRE3-TRIANGLE-REPRODUCTION.md) | Keep in diagnostic/reproduction layer; not a hardware requirement to repeat accidental controller bugs |
| Complete original response/readback/log seals | Evidence integrity | Sealed FIRE #3 archive | Retain per-frame capture during bring-up; never reuse mutable result buffers before capture |
| Fifteen-second Xorg server grab and exact restoration | Display experiment policy | [visible procedure](VISIBLE-TRIANGLE-REPRODUCTION.md) | Suitable for one reversible demonstration; not the permanent animation scheduler |

## Minimum future frame operation

`prepare frame → validate ownership/construction → establish source boundary and
continuous isolation → admit → possibly submit → accept attributable events →
retire → capture and seal output → present → request authoritative reuse boundary`.

Frame identity is an internal operation context. A new exported UUID/sequence,
broad tracer or modified fixed UAPI is not needed merely to design this seam.
The fixed API today still permits only one operation.

The [CPU-only policy model](../../tools/sgx535_demo/frame_policy.py) makes the
admission/terminal/rearm prerequisites explicit. Its12tests include clean
synthetic lifetime, each missing rearm premise, incomplete ledger,evidence loss,
interference,absence of authorization and repeated admission. It contains no
device adapter. Provider truth is outside the model; all-true synthetic booleans
are **not** authoritative SGX evidence and cannot enable live reuse.

## What must establish frameN+1

- All prior TA/raster/DPM work that can produce a later relevant event has ended;
  accepted events have been legitimately consumed under the normal protocol.
  Do not clear a completion to manufacture a successful observation.
- DPM resources and scene state can safely be rebound/reused;3D-memory-free alone
  is not a general CPU-publication/root-preservation theorem.
- Prior readback and provenance are immutable before color/response storage reuse.
- CPU edits of vertices/indices/program data are visible before the next GPU
  consumer; final GPU color is visible before readback. Distinguish these from
  the unresolved CPU visibility of TA-generated root bytes.
- Exclusive producer ownership,isolation,power/translation lifetime remain valid
  across the boundary,or a fresh authoritative boundary reestablishes them.
- No IRQ/service-delayed prior event can be attributed to the next operation;
  any evidence loss,reset/power transition,ownership loss or ambiguous operation
  terminates the sequence.

Current accepted retirement and a later `STATUS==0` are insufficient providers
for this entire set. No live rearm adapter is implemented in this task.

The smallest eventual reuse implementation should retain exclusive ownership,
use a validated post-retirement provider for source/DPM/event readiness,make a
new internal capsule context,and reinitialize only eligible owned inputs.
Reset/power establishment,if eventually necessary,must be separately justified
and preserve the preceding outcome; it must not manufacture a completed frame.
No unproven reset helper is proposed as current readiness.

## Buffers and presentation

Double buffering is not intrinsically necessary for a serialized initial
sequence: retain one color BO through retirement/CPU copy,then seal a separate
immutable host result before reuse. This is a design inference pending the
reuse/visibility provider. Double buffering becomes useful for overlap; it adds
ownership and memory-pressure contracts and should not precede basic repeatability.

An independent CPU image may be presented while later CPU geometry is prepared,
provided Xorg owns its destination and source bytes remain immutable. Overlap
with a second SGX operation is not currently authorized or implemented. KMS
page flipping and direct SGX scanout remain separate future architecture work.

The first sequence should be finite,with predetermined transforms and a per-frame
maximum admission count. Stop on the first ambiguous frame. A successful static
cube plus a safe bounded sequence would justify rotation; none exists yet.
