# Minimal clean-room 3D path: readiness decision

**MINIMAL CLEAN-ROOM 3D USERSPACE PATH NOT YET SPECIFIABLE.** The [indirect mode-transition edge](draw-to-submit.md) closes an important control-flow gap in the historical DRI binary. It does not close the program, packet, exact ABI, bootstrap or hardware-safety contracts. This is a specification decision; no driver code or hardware test was made.

The smallest hypothetical path would draw one primitive in one context, finalize its scene, submit it and wait for completion. Each component below needs an independently reproducible description; finding the corresponding historical routine is not enough to reproduce it without translating proprietary implementation.

| dependency order | required behavior | present evidence | readiness |
|---:|---|---|---|
| 1 | DRI screen/context and X-server device-private record | Legacy DRI callback graph and 32-byte `PsbDRIRec`; [Xpsb/DDX boundary](../xpsb-re/dri-boundary.md). | INFERRED for a matched running stack; exact DDX loader mismatch persists |
| 2 | kernel/DRM bootstrap, XHW state and SGX availability | Retained Xpsb `XHW_INIT`/XHW paths and candidate kernel handlers; [dependency inventory](../xpsb-re/bootstrap-dependencies.md). | UNKNOWN for exact pre-first-submit state and payload requirements |
| 3 | BO manager, static/program/scene resources and mapping | Binary BO imports, Xpsb static BO copy, private scene creation and candidate kernel scene pool. | INFERRED as a family; exact allocation flags, address rules and lifecycle incomplete |
| 4 | minimal render state and shader/program bytes | Binary USC/USSE generator, PDS construction and dirty-state callbacks. | UNKNOWN as a clean-room, revision-applicable word-level specification |
| 5 | primitive/index and TA/scene records | 20-byte index record producer and finalization record; [format inventory](format-readiness.md). | INFERRED at selected field level; remaining bit semantics and required state records UNKNOWN |
| 6 | relocations, BO validation and 0x90-byte command argument | 40-byte relocation producer, validation-list construction, candidate `5.0.1.0046` kernel header/handler. | INFERRED cross-artifact pairing; exact libdrm/kernel build and every field/flag still UNKNOWN |
| 7 | scene finalization, command submit and fence handling | Mode `3` exit calls renderer `+0x28` → scene finalizer → `drmCommandWrite` index `0`; selected fence update paths. | CONFIRMED historical binary control flow; successful kernel/GPU execution and complete sync semantics UNKNOWN |

The strongest new result is **P7E-002**: the binary itself connects the renderer callback table to scene finalization through the frame tracker. It supports several submission triggers, including swap, a flush helper, render-target changes and capacity retry. It does **not** show one submit per successful draw. [The source candidate](../xpsb-re/version-pairing.md) remains a strong `5.0.1.0046` release-family match, not an exact five-component build pairing; the DDX `XpsbTakedown`/`XpsbTakeDown` disagreement and older libdrm PSB header remain open.

## Smallest static work that could change this decision

1. **Program and packet contract.** Extract a bounded minimal PDS/USSE/TA example from the retained binary, identify every producer and consumer field, and cross-check against an authorized SGX535 register/ISA/packet reference if obtainable. Static bytes alone may still leave hardware meaning UNKNOWN. This blocks dependencies 4–5.
2. **Matched bootstrap.** Find the exact `5.0.1.0046` DDX/libdrm/kernel build manifest or binaries and trace the XHW shared request/reply sequence to the first TA submit. Search the identified package family before more general web archaeology. This blocks dependencies 1–3 and 6.
3. **Validation/fence ABI.** Complete binary-side BO-validation entry fields, relocation variants, scene flags and fence transitions and compare them against that matched build. This blocks dependencies 3, 6–7.
4. **Frontend dispatch.** The `0x0002d124` renderer selector is the callback in a 16-byte dirty-state atom (P7F-010), with an indirect dispatcher at `0x0004f3ef`; `0x0002b3d4` copies its static pointer array into the context list at creation (P7F-011). Trace the actual GL/TNL/VBO entry into clipping and complete dirty-state preparation. The selected private renderer's `+0x18/+0x1c` invocation and mode entry/exit are already located; the remaining frontend edge blocks dependency 4's ordering.

Those are **static archaeology** targets. A functioning clean-room implementation would additionally need hardware validation of SGX revision applicability, power/clock/reset, faults/recovery and real command effects. Those hardware questions cannot be settled by replaying binary structure in prose. Gate B is still **BLOCKED**, the identification-register whitelist is `[]`, and hardware functionality is **UNVERIFIED**.

## Later bounded-path reassessment (P7F)

The [eight-position scene branch](bounded-path-closure.md) now fixes its 128 copied vertex bytes symbolically, six indices, two 8-byte USE/USSE instruction slots, selected 20-byte TA record literals, and some scene-state fields. The [interface/bootstrap review](validation-bootstrap-closure.md) tightens the 0x88-byte validation-node and 0x90-byte command envelope, the candidate kernel's relocation/fence handling, and the conditional Xpsb init order. It also corrects the earlier PDS label on the `0x00030ffd` instruction buffer: that buffer is a USE/USSE builder; its caller creates separate PDS-related output.

The selected branch is conditional on scene dirty state and a changed key; it is not proven sufficient for one valid GL draw. Required fragment/general-state generation, some vertex-output bytes and relocation values, exact bootstrap prerequisites, and complete synchronization remain UNKNOWN. The strict result is unchanged: [**NOT YET SPECIFIABLE**](preimplementation-decision.md). No implementation-grade `docs/phase7/minimal-clean-room-3d-spec.md` was created. Gate B remains BLOCKED; hardware functionality is UNVERIFIED.


For the later single-triangle reassessment, see [P7G frozen draw](frozen-draw-closure.md) and its [blockers](frozen-draw-blockers.md). The GL-entry and terminal-padding questions are closed; Result B remains.
