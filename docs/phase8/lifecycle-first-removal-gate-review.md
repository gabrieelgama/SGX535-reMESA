# First-original-removal lifecycle and Gate B review

**Subsequent offline first-load investigation:**
[First-load boot qualification](first-load-boot-qualification.md)
adds source-tested first-binding/blacklist evidence. The original hot
removal remains blocked; the actual boot selection/fallback is not yet
captured. No candidate rebuild or target action occurred.

2026-10-01 UTC. **FIRST TRIANGLE: NOT ATTEMPTED. Gate B: BLOCKED.
Active whitelist: `[]`. Target contact: NONE.**

This review verifies the existing artifacts without rebuilding them and extends
the actual-source regression through PCI remove and module exit. It does not
replace the existing transition with a new deployment procedure. Commands,
fresh checks and source contracts are in the
[evidence index](artifacts/lifecycle-gate-review-20261001/README.md).

## Artifact gate: no drift

| Artifact | SHA-256 | Size | Build ID | Target CRC coverage |
| --- | --- | --- | --- | --- |
| Candidate #1 | `13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f` | 242,364 | `d72f19c97c5fd1feffddad07634e12333562ae60` | 230/230; missing 0; mismatches 0 |
| Lifecycle derivative | `91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74` | 242,724 | `074c650d48ccb463e37e90428b486dccdb23eb80` | 232/232; missing 0; mismatches 0 |

Both freshly verify as ELF32 little-endian ET_REL Intel 80386, exact vermagic
`5.10.240-antix.1-486-smp SMP mod_unload modversions 486 `, and
`module_layout=0xb84efb99`. Every actual undefined symbol has a checked version
record; module_layout is the sole additional synthetic record. Both preserved
clean-repeat binaries are still byte-identical to their respective artifacts.
No repeat compilation was necessary. ABI qualification remains **PASS offline**.

The source difference is exactly the existing lifecycle patch to `psb_drv.c`
and `psb_irq.c`: quiesce KMS/vblank, release owned IRQ before resource teardown,
check install failure, unwind late init failure, and mask all routing on true
IRQ removal while retaining the original direct-PM mask behavior. The additional
imports are `drm_irq_uninstall` and `drm_kms_helper_poll_disable`.
The frozen service/UAPI/programs/relocations are unchanged. All 15 fixed inputs,
86 integrated module inputs, and 7,426 prepared config/generated-state hashes
match the previous qualified manifests. The four integration patches freshly
apply in the recorded order with zero fuzz, offsets or rejects and reproduce the
preserved derivative source exactly. Target table SHA-256 remains
`faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca`.
Attempt 03's rejected artifact and all earlier evidence remain unchanged.

## Full selected lifetime: confirmed source and artifact evidence

1. Original `psb_driver_load` calls `drm_irq_install`. DRM requests an ordinary
   shared IRQ with handler `psb_irq_handler`, device cookie `dev`, and name
   `dev->driver->name`. `request_threaded_irq` stores these pointers in an
   `irqaction`; this is not a device-managed registration.
2. Module exit calls `pci_unregister_driver → driver_unregister →
   bus_remove_driver → driver_detach → device_release_driver_internal →
   pci_device_remove → psb_pci_remove`. The selected remove executes
   `drm_dev_unregister → psb_driver_unload → drm_dev_put`.
3. `drm_dev_unregister` does not release this IRQ for the MODESET/GEM driver:
   it has no `drm_driver.unload` hook and is not a LEGACY driver. Original
   `psb_driver_unload` tears down MMU/GTT, unmaps registers and frees private
   state without `drm_irq_uninstall`.
4. The surrounding core does not repair this omission. On this exact x86
   source, `pcibios_free_irq` is the empty weak implementation (no x86 override).
   `devres_release_all` cannot release an ordinary non-devm `request_irq`.
   PCI/sysfs unbinding reaches the same remove path; it is not a correction.
5. The legacy `DRM_IOCTL_CONTROL` helper checks `DRIVER_LEGACY` and returns
   without uninstall for this driver. It is not an existing normal release
   mechanism. No ioctl was issued to investigate this.
6. `drm_irq_uninstall` is the required pair: clear `irq_enabled`, invoke the
   driver mask callback, then `free_irq(dev->irq, dev)`. `free_irq` removes the
   matching action and waits for executing handlers. Its shared-IRQ contract
   requires the caller to disable interrupts on its own device first.

The retained H0 `/proc/interrupts` observation names **gma500 and eth0 on shared
IRQ 16**, even though counters are zero. A zero count, zero module refcount,
successful unbind, or successful `rmmod` is not evidence of IRQ-action release.
The original binary has no `drm_irq_uninstall` import. Its `free_irq` calls are
in Oaktrail HDMI-I2C init/exit, not selected Poulsbo teardown; the complete
previous disassembly/call-site review remains preserved.

The required invariant before either replacement or restoration is: the old
action removed and all running handlers drained **before** old handler/name,
DRM/private state or MMIO lifetime ends. The original violates that pairing;
the derivative supplies it for **its own** selected teardown only. Installing
the derivative afterward cannot run its fix inside the already loaded original.
The existing transition cannot honestly be qualified by offline rebuilding.

**INFERRED:** a stale action/name is consistent with the retained original-load
oops in `strcmp → register_handler_proc → __setup_irq → drm_irq_install`.
**UNKNOWN:** the exact faulting pointer and complete crash causation. The crash
occurred restoring the original after original removal; neither qualified new
artifact has been loaded, and neither caused that retained failure.

## Fresh regression and verification

Three additional UBSan cases execute extracted actual `psb_pci_remove` and
`psb_exit` code, including actual unload, with only kernel boundaries doubled:

- PCI removal unregisters DRM first, quiesces KMS/vblank, unregisters/masks IRQ,
  then frees resources and finally drops the DRM reference.
- Module exit dispatches the selected PCI removal once; owned IRQ is released
  once before final reference destruction.
- The immutable original is a negative control: private teardown/final put
  finish while `irq_enabled` remains true and no release occurred. No stale
  handler is invoked and no hardware is simulated.

Two temporary mutations (omitting IRQ release or omitting unload in PCI remove)
both fail the new safe-removal scenario. Production source and artifacts are not
mutated. Native harness strict compilation and UBSan pass. These establish
driver ordering at the documented boundaries, not real IRQ delivery, arbitrary
hot-unplug safety, or successful restoration of the active original.

**208 scoped tests PASS, zero skips; 10 lifecycle tests/13 scenarios PASS.**
Three freshly compiled strict service/IO/contract UBSan harnesses PASS. Generator
check PASS. Both dry runs hash to
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still exits 1 with
`PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
No production change, new module build, target action, force or retry occurred.

## Every existing exact-action Gate B predicate

PASS below is explicitly scoped to evidence/implementation or a recorded risk
decision; it is not a claim of live execution or architectural proof. Dynamic
guards are separate rows and remain UNKNOWN until freshly observed under an
authorized, reviewable procedure.

| Predicate | State | Supporting evidence / exact limit |
| --- | --- | --- |
| Exact target/rev121 guards | PASS | Retained identity and fixed kernel guard; no rediscovery |
| Expected kernel ABI/config/generated layout | PASS | Captured target table, successor equivalence, fresh unchanged-state checks |
| Candidate artifact identity and complete imports | PASS | Both frozen binaries freshly checked; derivative is the lifecycle-corrected choice |
| Integration/source/toolchain provenance | PASS | Strict four-patch replay, input manifests, normal Kbuild evidence and unchanged generated state |
| Static i386 client/UAPI/operation flag | PASS | Existing artifact/input and inert-flag regressions, unchanged pinned client |
| Ten private BOs/VA/MMU and 49 relocations | PASS | Tested fixed ownership/bounds/golden equivalence; live postconditions remain unobserved |
| USE 3/4 reservation/programming/lifetime | PASS | Fixed service tests and ownership retention; no live reservation claimed |
| Publication decision for this frozen scene | PASS | Recorded operator acceptance only; architectural visibility remains UNKNOWN |
| Historical operational bootstrap predicate | PASS | Bounded selected init/TA-load executor and failure injection; live ready status unobserved |
| Fixed scene/TA/XHW/raster ordering | PASS | Narrow kernel-owned plans and one-shot tests; no general userspace command path |
| Attributed TA/PBE/DPM events and selected fault mapping | PASS | Status-ledger/IRQ capture tests; DPM_TA_MEM_FREE nonterminal; no live event claimed |
| ISP bracket risk decision | PASS | Recorded scoped acceptance; no new reset/platform safety proof |
| L12 risk decision | PASS | Recorded scoped acceptance; architectural source containment remains UNKNOWN |
| FT-AUX risk decision | PASS | Recorded scoped acceptance; auxiliary source containment remains UNKNOWN |
| One scene, sequence 1, at most one TA and raster fire | PASS | Existing fixed UAPI, latch and failure injection; no automatic retry |
| 5×HZ/300,000-sample limits and HOLD | PASS | Existing bounded executor/ledger; not a whole-platform hang bound |
| HOLD BO/USE/scene/module ownership | PASS | No release after ambiguity; held attempt retains module reference; PCI unbind prohibited |
| Completion/readback success criterion | PASS | Requires attributable events plus defined color output; no current readback, GPU-to-CPU visibility proof or triangle result |
| Derivative IRQ/modeset/error unwind | PASS | Exact-source tests, core contracts, compiled relocations and paired release; scoped offline qualification |
| Original preserved recovery artifact | PASS | Preserved original hash/evidence; availability is not hot-restoration safety |
| Original display/console/service teardown facts | PASS | Retained console-release/service observations only |
| First hot removal of active original | BLOCKED | Missing IRQ release before private/MMIO/module lifetime ends |
| Replacement insertion after that removal | BLOCKED | Old-action-drained invariant cannot be established by the existing procedure |
| Normal hot original restoration | BLOCKED | Retained restoration oops; clean PCI unlink does not prove IRQ cleanup |
| First-load alternative and original fallback | UNKNOWN | Selected boot/initramfs/loading/fallback provenance absent; not an invented replacement procedure |
| Fresh live kernel/module/PCI/DRM/fb/VT/FD/service guards | UNKNOWN | Capture 08 is dated; no target contact permitted while this gate is blocked |
| Fresh loaded candidate identity/binding | UNKNOWN | Never inserted; only meaningful after a qualified transition |
| Runtime module-signature enforcement | UNKNOWN | Both unsigned; config does not force signing; original unsigned load is retained observation, not a fresh policy check |
| Physically present operator/independent capture/root staging | UNKNOWN | Future action must newly establish them; prior attempt presence is not current proof |
| Current exact transition and recovery authorization | BLOCKED | Existing hot procedure is disproved; conditional execution request cannot bypass its missing predicate or authorize an invented route |
| Current Gate B and exact-action whitelist | BLOCKED | No qualified executable transition; whitelist stays `[]` |

Software containment does not prove a GPU-only/platform-safe failure boundary.
The historical accepted assumptions are preserved without upgrading UNKNOWN
semantics or consuming them as a new automatic retry authorization.

## Boundary and single smallest next step

**The single concrete blocking transition is removal of the active unpatched
original without the required IRQ-release pair.** No ordinary existing release
interface was established, and an offline artifact cannot change that resident
module. This is not a request for more SGX archaeology or another candidate build.

**READ-ONLY TARGET OBSERVATION, separately scoped/authorized:** identify the
normal first `gma500_gfx` loading source and existing known-good boot fallback.
Minimum proposed read paths are `/proc/cmdline`, the selected bootloader entry
and default configuration, `/boot` image/initramfs paths for the verified release,
the installed original-module identity, and initramfs membership for gma500 and
its dependencies (`lsinitramfs` only if already available). Preserve/hash those
ordinary files through the reviewed capture mechanism; do not install tools,
change boot files, reboot, unbind, unload, load or submit. STOP if the normal
identity/state differs or selected boot entry/fallback cannot be attributed.
This observation is **not performed** here and does not itself authorize a
cold-start deployment. It supplies the missing facts for a separately reviewed
first-load route avoiding the unsafe original teardown.

No Mini 12 contact, original removal, replacement insertion, ioctl, SGX fire,
completion/readback, triangle pixels or recovery action occurred in this turn.
