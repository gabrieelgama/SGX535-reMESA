# First-load boot alternative: offline qualification boundary

## Current result: cycle02 (2026-10-01)

[Cycle02 evidence](first-load-cycle-02-result.md) records a corrected and tested health guard, a fresh 49-guard
preflight and exact exclusive staging, all PASS. The operator reported normal
userspace after one experimental boot. The sole experimental root capture never
ran because sudo was unavailable. We have no loaded derivative identity, hook
trace or experimental boot ID. There was no retry.

After the operator crossed the machine boundary, the stock recovery capture
passed 54 guards. It verified the original note, PCI/DRM/fb/VT/IRQ ownership,
services, files and default. The operator confirmed that the display was normal.
Full first-load and three-boot recovery qualification remain NOT ESTABLISHED.
Capture-readiness, boot-ID/timing and existing-created-file guards are now
tested offline for a separately authorized successor.

304 tests passed with zero skips, along with 14 boot-analysis tests, three UBSan
harnesses and the generator/CRC/dry guards. Gate B BLOCKED; whitelist []; no SGX
action; FIRST TRIANGLE NOT ATTEMPTED.

The next step needs new explicit authorization for a non-SGX first-load/recovery
cycle. Preserve the staged files and revalidate their exact creation identities.
Do not overwrite them or use a hot transition. The notices below are historical.

### Earlier preflight: cycle01 (2026-10-01)

[Cycle01 result](first-load-cycle-01-result.md): the separately authorized non-SGX cycle STOPPED BEFORE
STAGING. The physical/operator prerequisites were confirmed. After local sudo
authentication, the root read-only preflight checked stock identity, ownership
and services, then rejected three preexisting diagnostics. The complete fresh
dmesg matches the retained log from the same stock boot. The OFFLINE guard and
baseline disagree; there is no evidence of a new SGX fault. No guard was waived
or changed. No staging, experimental selection, reboot, module/service/PCI/VT
mutation, fixed ioctl or SGX fire occurred. Live first load and recovery
remained UNKNOWN. Gate B BLOCKED; whitelist `[]`; FIRST TRIANGLE
NOT ATTEMPTED. The next step at that point was OFFLINE testing of precise
baseline-versus-fault health discrimination. No further target action occurred
in that stopped cycle.

### Earlier offline operational review (2026-10-01)

The staging, manual first-load and operator reset-to-stock design review is in
the [qualification report](first-load-operational-review.md), [exact procedure](first-load-staging-boot-recovery-procedure.md) and [checklist](first-load-operator-checklist.md). No pinned
image or candidate changed; there was no target contact or staging. 266 tests
passed with zero skips, along with 14 boot-analysis tests and three UBSan
harnesses. The non-SGX procedure was READY TO REQUEST SEPARATE LIVE
AUTHORIZATION, subject to fresh stock, privilege, physical-control and visible
menu guards. Live first ownership, display, SSH and recovery remained UNKNOWN.
Delivery of the volatile hook log after pivot was UNKNOWN; without the trace,
qualification could not pass. No hot transition, restoration or SGX action was
authorized. Gate B BLOCKED; whitelist `[]`; FIRST TRIANGLE NOT
ATTEMPTED. Earlier next-step notices are historical.

### Earlier offline image qualification (2026-10-01)

The [experimental first-load image report](experimental-first-load-image-qualification.md) and [evidence](artifacts/experimental-first-load-01-20261001/README.md) record PASS OFFLINE for
construction, guards, pre-udev ordering, exact dependency CRC coverage, stock
preservation and non-saving GRUB text. The final image is 50,804,481 bytes,
SHA-256 `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`. Two corrected builds are byte-identical. 249 tests
passed with zero skips, including 34 new image checks. The 14 boot-analysis
tests and three UBSan harnesses also passed. No module was rebuilt, no target
was contacted or changed, and no image/entry was installed. There was no SGX
fire. Early first ownership, display/SSH and reset/fallback remained UNKNOWN.
The next step then was the OFFLINE staging, experimental-boot and reset-to-stock
recovery review, followed by separate authorization if justified. Gate B
BLOCKED; whitelist `[]`. Earlier “no image constructed” and
next-step statements below are historical.

### Earlier read-only boot observation (2026-10-01)

[Stock boot and first-load design](first-load-stock-boot-observation.md) supersedes older statements below that the
boot/initramfs/fallback files were absent. The separately authorized capture
passed identity/end guards and found a normal stock display. It captured the
GRUB saved stock entry and exact image hashes. The stock initramfs contains no
gma500; its udev trigger runs before /conf/modules loading. Root eudev modalias
coldplug remains an INFERENCE: no requester PID was traced. GRUB's existing
custom.cfg sourcing supports a separate non-saving entry. No entry or image was
created in that task. The next step then was separately scoped OFFLINE
image/entry construction and qualification. Experimental first-load, display and
reset-fallback remained BLOCKED or UNKNOWN pending that work, new boot
authorization and live preflight. Gate B BLOCKED; whitelist `[]`;
target contact READ-ONLY only; SGX fire NO. Earlier no-contact and
missing-boot-file statements describe their historical turns.

## Original first-binding review

The rest of this report records the review before the boot-provenance capture.

2026-10-01 UTC. **First-load alternative/fallback: UNKNOWN/BLOCKED.**
**Gate B: BLOCKED. Whitelist: `[]`. FIRST TRIANGLE: NOT ATTEMPTED.**
No target contact, boot change, module build/load/removal or SGX fire occurred.
The [evidence index](artifacts/first-load-review-20261001/README.md) preserves
exact source contracts, retained observations and new test/guard results.

## Decision

**SOURCE-PROVEN / OFFLINE-TESTED:** the existing lifecycle derivative can
register the same PCI driver and probe an unbound matching device without an
original-driver object. It is a full display/KMS replacement, not an SGX add-on.
Making it the first Linux PCI driver owner would avoid the defective original
remove path entirely. Loading it after original ownership does not do this.

**UNKNOWN / REQUIRES LIVE OBSERVATION:** the target's selected boot entry,
initramfs contents/load hooks and known-good fallback have not been captured.
The actual first original-module loading trigger is not proved. Consequently
there is no qualified target-specific selection procedure to execute. No
configuration or initramfs was generated speculatively, and no blacklist was
installed. The single next step is the separately authorized read-only
boot-selection inventory specified below.

## Retained target facts, with limits

| Item | Proof class | Evidence / conclusion |
| --- | --- | --- |
| Original is a module | RETAINED TARGET FACT | Captured config has `CONFIG_DRM_GMA500=m`; original binary/loaded state also retained |
| Kernel supports initramfs | RETAINED TARGET FACT | `CONFIG_BLK_DEV_INITRD=y`; H0 boot log unpacks an initramfs and runs `/init` |
| Recorded command line | RETAINED TARGET FACT | H0: `BOOT_IMAGE=/boot/vmlinuz-5.10.240-antix.1-486-smp root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0`; no recorded blacklist/nomodeset argument |
| Early console | RETAINED TARGET FACT | Operator-provided H0 dmesg: VGA 80×25 at 0.083195 s; VGA arbitration assigns boot VGA to 00:02.0 at 0.339720 s |
| Loading chronology | RETAINED TARGET FACT | H0: initramfs unpack 0.488919 s; `/init` 4.112868 s; eudev 4.58 s; real-root ext4 mount 12.122778 s; later eudev 17.386963 s; gma500 fbcon 25.869134 s; DRM registration 26.179301 s |
| Exact original load trigger | UNKNOWN | Completion after root mount is consistent with later userspace loading, but does not prove which caller loaded it, absence from initramfs, or probe start time |
| Initramfs membership/hooks, root module lists/rules, external softdeps | UNKNOWN | Neither selected initramfs nor these files is in retained capture 08/H0 boot material |
| Last verified normal state | RETAINED TARGET FACT | Capture 08, 2026-09-30 19:10 UTC: original Live/known Build ID, PCI gma500, card0, fb0, vtcon1, slimski/Xorg; not a fresh state check |
| Operator reset recovered display | RETAINED TARGET FACT | Operator report followed by capture 08 independently verifying normal operation; not a tested experimental-entry fallback |

The H0 timeline is explicitly **operator-provided retained dmesg**, not a new
privileged capture. Full input hashes and exact selected lines are recorded.
`BOOT_IMAGE` alone does not establish the installed bootloader configuration.

## Module and first-binding contracts

**SOURCE-PROVEN; actual source executed where stated:**

1. Original and derivative internal module name is `gma500_gfx`, PCI driver
   name `gma500`. Both contain alias
   `pci:v00008086d00008108sv*sd*bc*sc*i*`. The retained Mini 12 modalias
   `pci:v00008086d00008108sv00001028sd000002B1bc03sc00i00` matches both.
   The static table selects `psb_chip_ops` for 8086:8108. Neither module has an
   encoded softdep; external loading policy is still UNKNOWN.
2. The PCI core emits that device modalias. A userspace loader must act on it;
   kernel source does not identify the target's actual rule or caller.
3. Module init registers `psb_pci_driver`. `__pci_register_driver` registers
   it even if no device is claimed, so insertion return 0 is not binding proof.
4. `__pci_device_probe` probes only if `pci_dev->driver` is NULL and there is a
   matching ID. `local_pci_probe` acquires the PM reference and sets the owner
   before calling probe; negative return clears that owner and PM reference.
   Tests execute these actual functions with kernel boundaries controlled.
5. Existing ownership is never replaced by this path. A bound original is a
   STOP condition, not a reason to unload it. The tested negative control
   removing the ownership guard is detected.
6. Kernel `module_blacklist=gma500_gfx` rejects **both** binaries by internal
   name in `load_module`, before initialization. Renaming the file does not
   distinguish them. The actual blacklist function and a mutation ignoring it
   are tested. This mechanism cannot select the current derivative.
7. `add_unformed_module` rejects another same-named Live module with `-EEXIST`.
   Once derivative ownership is established, a stock alias load cannot co-load
   a second `gma500_gfx`. This does not rule out an external unload script.
8. A userspace alias blacklist is different from the kernel blacklist. Without
   target initramfs/kmod rules and explicit-load inventory it is not a proved
   suppression mechanism. The exact gma500 init has no `nomodeset`/vgacon-force
   check or module parameter that provides a deterministic selective load gate.

These are **not** instructions to blacklist, bind, insert or reboot the target.

## Clean probe, resource acquisition and display

The derivative retains the stock full KMS/fbdev implementation and fixed-service
additions. Exact sequence in the preserved integrated `psb_drv.c`:

```text
PCI enable → allocate DRM device → attach pdev/private data
→ allocate fresh private state / select Poulsbo ops
→ PCI master / map VDC and SGX / BIOS-opregion-chip setup / power init
→ zero UC scratch page → GTT init → MMU/default and fault PDs
→ selected stock SGX initialization / stolen mapping / MMU contexts
→ vblank init → mask routing → checked drm_irq_install
→ modeset → fbdev → KMS polling → backlight
→ drm_dev_register → fixed private ioctl available
```

**SOURCE-PROVEN:** no acquisition refers to an original-driver object, old IRQ
cookie, or successful hot removal. Firmware/BIOS display state and PCI resources
still exist before Linux driver binding: "first owner" means first relevant
Linux PCI driver, not first agent ever to initialize the hardware. Module
insertion performs stock MMIO/display/SGX initialization; it is an active
operation even when the fixed ioctl is never called. The fixed scene does not
automatically fire at module initialization.

**OFFLINE-TESTED:** the existing exact-source lifecycle tests cover checked IRQ
install failure, late init error cleanup, pre-modeset and modeset IRQ teardown,
PCI remove and module exit. IRQ mask/uninstall precedes private/MMIO lifetime
end. The first-load tests additionally cover core owner/PM rollback on negative
probe. They do not emulate all hardware-dependent initialization failures or
prove a successful probe on a cold boot.

**SOURCE-PROVEN limitation:** `psb_do_init` has a direct error return rather
than `out_err`; its only negative condition is misaligned `mmu_gatt_start`.
Selected `psb_gtt_init` sets that field to `0xE0000000`, making this error
unreachable on its successful unchanged path. Other stock hardware-dependent
initialization behavior is not upgraded to universally safe failure unwind.
A probe failure/ambiguous state requires stopping the experimental boot path,
not trying the stock module in that same boot.

**Display consequences:**

- **RETAINED TARGET FACT:** this recorded boot starts with VGA text, then
  switches to `gma500drmfb` and fbcon. The target config enables vgacon,
  framebuffer console, DRM fbdev emulation and VESA/EFI/simplefb capabilities;
  `CONFIG_X86_SYSFB` is unset. No selected H0 evidence demonstrates a separate
  VESA/EFI/simple framebuffer actually registering.
- **SOURCE-PROVEN:** gma500 supplies KMS/CRTC/output/fbdev/backlight support;
  the derivative retains it. No gma500 conflicting-framebuffer removal helper
  was found in the exact selected source. Any earlier framebuffer, firmware
  graphics payload or resource overlap must be accounted for, not ignored.
- **INFERRED:** without gma500, boot/root/network are not intrinsically gated
  by this PCI driver. Recorded r8169 Ethernet initialization precedes gma500.
  Display may stay in text mode or legitimately blank; slimski/Xorg may fail.
- **UNKNOWN / REQUIRES LIVE OBSERVATION:** usable LCD, SSH availability and
  actual early framebuffer/resource ownership in a different boot. Recorded
  VGA support does not guarantee visibility after a changed boot entry.

The off-screen triangle does not require LCD output. Independent operator
access/capture is required before selecting an experimental boot.

## Smallest prospective mechanism — INFERRED, not target-qualified

The useful candidate is a **separate experimental boot image/initramfs**, with
the original installed module and known-good default boot left unchanged:

1. Attribute the selected stock boot entry, kernel, initramfs and root loading
   policy before choosing any modification mechanism.
2. In an offline copy only, replace the gma500 payload with the pinned
   lifecycle derivative and its existing qualified dependencies. Establish an
   actual target-supported fixed preload point **before** any device alias,
   explicit stock load or root userspace can acquire ownership. Merely placing
   the binary in an initramfs or an `updates` directory does not prove ordering.
3. The future reviewed preload must stop on an already loaded original, prior
   binding, missing dependency, signature/policy mismatch, probe/binding error,
   contradictory IRQ/fb ownership or incorrect loaded derivative identity.
   It must not fall through to loading the original after a failed probe.
4. Only a verified derivative-first binding could allow the later fresh
   fixed-action preflight. No boot hook may invoke the triangle client or retry.
5. An operator-selected, non-default experimental entry must preserve a
   independently selectable stock fallback. Do not alter the original image,
   stock module or boot default while claiming reset restores them.

This mechanism is **not yet chosen**: the actual initramfs format/hooks,
bootloader entry/default/fallback and load policies are missing. An earlier
preload than the recorded late registration changes timing; its dependency,
firmware and display conditions need review. Controlled manual first load is
another conditional possibility only if *all* original loading routes can be
excluded and the device stays genuinely unbound. That exclusion is unproved.

## Recovery boundary

**INFERRED proposed model:** stop/HOLD → preserve evidence/ownership → operator
selects full reset into untouched stock boot. No hot original restoration, no
module-load retry, no automatic client run after reboot, no kexec substitute.
This removes reliance on the demonstrated bad original unload/restore sequence.

**SOURCE-PROVEN:** selected `pci_device_shutdown` does not call PCI removal;
gma500 has no `.shutdown` callback. Normal reboot is not proof of the IRQ-release
invariant or safe experimental DMA termination. The core's firmware-reset
comment is not target reset qualification.

**UNKNOWN:** deterministic return to the stock entry/image and bounded physical
recovery after a future fault. Prior operator reset and capture 08 show one
successful recovery, not an experimental-entry fallback. Independent access,
physical presence, exact reset boundary and new state-changing authorization
would be required; none is established/consumed in this turn.

## Affected Gate B predicates

| Predicate | State | Evidence / remaining boundary |
| --- | --- | --- |
| Candidate ABI/source/provenance | PASS offline | Both hashes unchanged; previous full CRC/repeat qualification retained |
| Derivative IRQ/lifetime pairing | PASS offline | Existing 10 tests/13 scenarios plus selected clean core probe tests |
| Same alias / unbound first-binding capability | PASS offline | Both actual binaries and extracted PCI source tested |
| Actual original boot trigger / initramfs membership | UNKNOWN | Loading policy and image absent from retained evidence |
| Deterministic derivative-first selection | BLOCKED | No attributable target boot/initramfs sequence |
| Display/console/resource conditions of experimental boot | UNKNOWN | Stock boot observed; proposed boot not specified or observed |
| Stock fallback across reset boundary | UNKNOWN | Actual entry/default/image selection not captured |
| First-load alternative/fallback overall | UNKNOWN/BLOCKED | Source-feasible; no qualified executable target sequence |
| First original hot removal / post-removal insertion / hot restoration | BLOCKED | Previously proved missing IRQ release; no correction to resident original |
| Fresh live identity/binding/IRQ/service/operator/signature guards | UNKNOWN | No new target contact; capture 08 is dated |
| Current transition/recovery authorization | BLOCKED | Historical one-shot authorization does not authorize a new boot procedure |
| Publication/ISP/L12/FT-AUX accepted assumptions | Unchanged | Historical scoped decisions, not architectural proof or new authorization |
| Gate B / exact whitelist | BLOCKED / `[]` | No executable qualified first-load transition |

The other fixed-scene predicates remain as individually reviewed in
[the preceding Gate B table](lifecycle-first-removal-gate-review.md).
First-load capability does not authorize SGX fire, replace fresh guards, or
erase the known original hot-unload defect. No existing boot-observation
whitelist was established; the observation below requires separate scope and
authorization.

## SINGLE SMALLEST NEXT STEP — READ-ONLY TARGET OBSERVATION

Missing fact: **the actual selected stock boot/initramfs loading chain and its
known-good fallback**. Capture it once under separately approved read-only
scope, using the existing pinned evidence transport. Proposed reads only:

- Confirm `uname -r`, `/proc/cmdline`, `/proc/modules`, original module hash and
  PCI driver symlink against capture 08. A mismatch stops further collection.
- Read selected boot configuration. If present, `/boot/grub/grub.cfg`,
  `/etc/default/grub` and `/boot/grub/grubenv` are candidates, not an assumption
  that GRUB is installed. Record the actual bootloader/default/selected kernel,
  initrd, firmware graphics-payload options and available untouched fallback;
  ambiguous attribution stops the qualification.
- `readlink -f`, `stat` and `sha256sum` on the *selected* kernel/initramfs and
  installed original module. Preserve the selected initramfs ordinary file
  through approved capture; inspect/unpack its archive **offline**, or read its
  membership using `lsinitramfs` only if already installed. Never execute its
  scripts. Inspect gma500/dependency payloads, module lists, loading hooks,
  blacklist and alias/explicit-load policy before and after root handoff.
- Read `/etc/modules`, `/etc/modules-load.d`, `/etc/modprobe.d`,
  `/lib/modprobe.d`, and the relevant already installed eudev module-loading
  rules, recording absence. No installation or regeneration of initramfs.

Do not run update-initramfs, depmod, grub configuration tools, modprobe, module
operations, bind/unbind, service controls, DRM opens, MMIO or reboot. No write to
the target, boot alteration or SGX action belongs to this observation. Missing
permission/tool/artifact is evidence of UNKNOWN, not a reason to improvise.

## Verification and changes

Seven new tests execute retained PCI matcher/probe and kernel blacklist bodies
under strict native GCC/UBSan; two deliberate mutations fail as intended. Alias
and internal-name checks read both preserved binaries with local modinfo.
Existing actual-source IRQ lifecycle tests remain green. **215 tests PASS,
zero skips; three separate contract/service/IO UBSan harnesses PASS.**
Generator PASS; both deterministic dry runs remain
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still exits 1: `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

No production/kernel patch or candidate was modified/rebuilt. Previous evidence
is preserved; additions are this report, focused tests, source/evidence bundle
and current-document pointers. There is no current triangle completion or
readback result, and no new deployment authorization.
