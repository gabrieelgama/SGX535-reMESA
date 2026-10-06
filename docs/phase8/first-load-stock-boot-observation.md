# Stock boot observation and proposed first-load architecture

**Newest offline image qualification (2026-10-01):**
[Experimental first-load image](experimental-first-load-image-qualification.md)
and its [evidence](artifacts/experimental-first-load-01-20261001/README.md) now
establish construction/guards/pre-udev ordering, exact dependency CRC coverage,
stock preservation and non-saving GRUB text **PASS OFFLINE**. The final image is
50,804,481 bytes, SHA-256
`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`;
two corrected builds are byte-identical. **249 tests, zero skips; 34 new image
checks; 14 boot-analysis tests; three UBSan harnesses PASS.** No module rebuilt,
no target contact/mutation, no image/entry installed, no SGX fire. Actual early
first ownership, display/SSH and reset/fallback remain UNKNOWN. The next step is
OFFLINE exact staging/experimental-boot/reset-to-stock recovery review, followed
by separate authorization if justified. **Gate B BLOCKED; whitelist `[]`.**
Earlier “no image constructed”/next-step statements below are historical.


2026-10-01 UTC. **Read-only observation: PASS. Gate B: BLOCKED. Whitelist: `[]`.**
**No target boot/service/module/PCI/VT mutation, candidate insertion or SGX fire.**
The [successful capture](../hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02/RESULT.md)
completes the observation requested by the [previous offline review](first-load-boot-qualification.md).
It specifies a future architecture; it does not create or qualify an experimental image.

## Capture, identity and limits

The first connection stopped at noninteractive sudo authentication before its
script ran; [capture 01](../hardware-evidence/MINI12-20261001T025947Z-BOOT-PROVENANCE-READONLY-01/RESULT.md)
is preserved unchanged. After the operator ran `sudo -v` locally and replied
“ready”, one new capture ran 03:07:54–03:08:59 UTC, exit 0, empty stderr.
Transport stdin contained only the frozen read-only script; no credential input.
No further target contact followed it. All 55 decoded ordinary files match
both the target-reported size and SHA-256. Raw capture and later interpretation
are separate. Captured scripts are data, never executed locally or remotely.

**FRESH TARGET OBSERVATION:** Inspiron 1210, i686, kernel
`5.10.240-antix.1-486-smp`, boot ID
`8ae37532-19d4-4ec7-9c29-a791acfd889f` (same as capture 08), taint 12289.
Original `gma500_gfx` is Live/refcount 2, Build ID
`d8dcb4d38b774ad64799d5e13aaedede069371f3`; installed module SHA-256
`7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`.
PCI 0000:00:02.0 is 8086:8108 bound to gma500; card0 and gma500drmfb
1280×800 are present. vtcon0=0/vtcon1=1; slimski/Xorg run.
End guards confirm unchanged boot, module, binding, VT and service state.
This is successful stock observation, not experimental-boot proof.

## Actual boot selection and untouched fallback

**FRESH TARGET OBSERVATION:** installed GRUB grub-pc/common 2.12-9+deb13u1.
The captured generated `/boot/grub/grub.cfg` loads `grubenv` and uses
`saved_entry` unless `next_entry` exists. The environment contains only:

```
saved_entry=gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1
```

This is the normal stock entry; its kernel arguments agree with `/proc/cmdline`:

```
BOOT_IMAGE=/boot/vmlinuz-5.10.240-antix.1-486-smp root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro quiet selinux=0
```

`GRUB_DEFAULT=0` near the top of `/etc/default/grub` is overridden by its
init-diversity logic; it is not the effective generated default. The saved
entry/current command line agree, but no runtime GRUB chosen-entry trace or
firmware boot-sector attestation was captured.

All six Linux entries (normal, sysvinit, s6-rc, s6-66, dinit, recovery) use the
same kernel/initrd pair. The alternatives are configured, not independently
proved functional. No older Linux kernel/image pair appears in the inventory.

| Stock input | Size | SHA-256 |
| --- | ---: | --- |
| `/boot/vmlinuz-5.10.240-antix.1-486-smp` | 5984416 | `cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438` |
| `/boot/initrd.img-5.10.240-antix.1-486-smp` | 50863580 | `f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341` |
| `/boot/grub/grub.cfg` | 10438 | `396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f` |
| `/boot/grub/grubenv` | 1024 | `72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927` |

The current image association comes from the captured GRUB entry and running
command line. File hashes identify current on-disk inputs; they are not kernel
attestation of the initrd bytes consumed at boot. The running stock path is
known-good from post-reset capture 08 and this independent observation. **UNKNOWN:** successful selection/recovery after a
future experimental fault. Existing alternatives are not substitutes for that proof.

The existing generated `41_custom` block sources `${config_directory}/custom.cfg`
(or `$prefix/custom.cfg`). No custom.cfg appears in the captured boot inventory.
**SOURCE-PROVEN from captured configuration:** a separate custom menu entry and
separately named experimental initrd can coexist without replacing the stock
kernel, initrd, module or generated grub.cfg. Nothing was created here.

**Mandatory future selection guard:** the experimental entry must NOT call
`savedefault`, `save_env`, change the global default, or set `next_entry`.
Normal stock entries call `savedefault`; blindly copying that behavior would
make the experiment the saved default. Keep the saved stock ID unchanged,
require deliberate manual selection, and never autorun the triangle at boot.

## Initramfs membership and exact loading order

**FRESH TARGET OBSERVATION / OFFLINE-TESTED:** the captured image consists of an
uncompressed early newc/microcode archive (5 members), padding, and a gzip main
archive (2133 members). Main gzip starts at byte 15006720; expanded SHA-256
`9aef8fc4f1ad279e382e36514c933d3935c32a6e7060afd681dd274a35c60117`.
Independent GNU cpio 2.15 listing agrees with all 2138 member names.
Offline parsing extracts only selected regular files beneath a local analysis
directory; nothing is unpacked over target files.

No member contains gma500, no gma500 module/alias is present, and no initramfs
alias matches the actual Poulsbo modalias. `/conf/modules` is absent.
The installed initramfs-tools/core version is 0.148.3; image `/init` is byte
identical to installed `/usr/share/initramfs-tools/init`, SHA-256
`0a9bb34973c78987922b57f010e99c36ddf102e8a5a2fd198385566539f5d6d5`.
The image has `MODULES=most`, `BUSYBOX=auto`, `COMPRESS=gzip` policy. This
attributes the selected framework, not every historical image-generation command.

The actual image calls `run_scripts /scripts/init-top` at `/init:222`, then
`load_modules` at line 227. `run_scripts` sources the captured `ORDER` file:
all_generic_ide → blacklist → keymap → udev. That udev hook starts udevd,
triggers add events and settles. Only afterward does `load_modules` read
`/conf/modules`. **Adding the derivative only to /conf/modules cannot prove
pre-udev ownership.** A future fixed preload must run before init-top/udev and
before handing control to real-root userspace.

All nine modules in the original root `modules.dep` dependency closure already
exist in the image: video, drm_kms_helper, cec, drm, fb_sys_fops, syscopyarea,
sysfillrect, sysimgblt and i2c-algo-bit. The offline checker finds each recorded
versioned import in the qualified captured target table with matching CRC:
70/306/81/358/5/2/6/4/12 respectively, `module_layout=0xb84efb99` throughout,
zero missing or mismatched records. This is versioned-import evidence, not
complete experimental-image or live-load qualification.

## Who loads the stock driver, and when

**FRESH TARGET OBSERVATION:** root modules.alias has exactly one matching rule:
`pci:v00008086d00008108sv*sd*bc*sc*i* gma500_gfx`. Root eudev
`80-drivers.rules` handles add events with `MODALIAS` through builtin
`kmod load`. The captured root udev startup script triggers device/subsystem
add events and settles. Captured module lists are comments-only or list
lp/fuse/tun; no selected gma-specific force-load, install/remove or softdep was
found. `/sbin/init` resolves to runit-init; the exact root rcS/native branch
and runtime requester PID were not observed.

**INFERRED:** real-root eudev/kmod modalias coldplug requests the original.
**UNKNOWN:** exact runtime caller identity/call and exact probe entry timestamp.
Timing is supporting correlation, not an execution trace:

| Captured dmesg event | Seconds after boot |
| --- | ---: |
| VGA 80×25 console | 0.083232 |
| Run `/init` | 3.752748 |
| Early udevd starts | 4.219074 |
| Real-root ext4 mounted | 6.316267 |
| Later udevd starts | 11.509008 / 12.082971 |
| gma500drmfb becomes primary | 19.297957 |
| Console switches to fb 160×50 | 19.503341 |
| gma500 DRM registration completes | 19.530798 |

PCI binding/probe precedes the last three events; they do not give probe start
time. The absence of the original from the initramfs and captured root loading
rules support later-root loading. They do not identify a historical PID.

## Future first-load design — specified, NOT executed or qualified

Use the same stock kernel, an offline copy of this exact initrd, and the pinned
lifecycle derivative SHA-256
`91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`.
Candidate #1 remains a distinct qualified artifact and is not the selected
lifecycle replacement. Preserve original images and early microcode bytes.

A separately scoped future offline task would have to construct and validate:

1. An experimental copy containing the derivative, qualified existing
   dependency bytes and consistent normal module indices; no CRC editing.
2. One fixed guarded preload before `/init` runs init-top/udev. It must verify
   the expected kernel/device, module absence and an unbound PCI function;
   load through normal mechanisms exactly once; verify the pinned loaded
   identity, successful binding, and expected DRM/IRQ ownership. A module-load
   return code alone is not binding proof. Signature/loading policy must remain
   compatible. No ioctl/client/SGX scene fire in the boot hook.
3. Failure before/after possible probe → stop/HOLD; no original fallback load
   in the same boot, no retry, no speculative reset or hot restoration.
4. A unique non-default custom-entry text using the same stock arguments and
   separately named initrd, omitting all saved-default mutation. Manual
   selection only; unmodified stock saved entry remains the recovery choice.
5. A later independently authorized experimental boot/preflight with physical
   operator access. Confirm derivative-first ownership after root starts and
   before any fixed one-shot authorization. Early probe can write stock
   display/SGX registers; it is not a passive operation.

**SOURCE-PROVEN from the prior review:** once the derivative is Live under the
same internal name, a second stock module cannot co-load; PCI probe does not
replace an already bound owner. This avoids defective original hot removal
only if the new before-probe guards really show it never owned the device.
Future live evidence is required; no present original must be unloaded.

## Display, access and reset-boundary recovery

Stock VGA → gma KMS/fbdev takeover is observed. The derivative retains full
stock KMS/fbdev, but early timing/resource/firmware conditions and usable LCD
in a changed boot are **UNKNOWN**. The experiment remains off-screen.

Current SSH works on wlan0 <private-target-address>; eth0 is down. Captured ssh service
startup has no graphics dependency. Network driver initialization precedes
stock gma registration. **INFERRED:** root/network need not depend on gma.
**UNKNOWN:** future SSH availability if early derivative probe fails or hangs
before real-root startup. Current remote access is not a future recovery guarantee.

Recovery must be operator-controlled full reset followed by the untouched
stock saved entry/image; never hot-restoring the defective original in the
experimental boot. Prior operator reset plus captures prove one stock recovery,
not this future fallback under every fault. No automatic reboot or retry is
introduced. Physical access, exact reset/selection decision and new boot
mutation authorization are required separately.

## Updated predicates and next boundary

| Predicate | State / proof class | Exact remaining boundary |
| --- | --- | --- |
| Stock identity/display state | PASS / FRESH TARGET OBSERVATION | Capture end guards; not future experimental state |
| Stock entry/image/module provenance | PASS / FRESH TARGET OBSERVATION | Paths, hashes, saved entry and contents captured |
| Stock loader attribution | INFERRED | Root modalias rule; historical requester PID UNKNOWN |
| Pre-udev first-binding architecture | SOURCE-PROVEN; sufficiently specified for offline construction | Actual experimental image/hook not built |
| Candidate ABI / own IRQ lifetime | PASS offline / RETAINED TARGET FACT + OFFLINE-TESTED | Existing qualification unchanged; no new build |
| Original hot removal/restoration | BLOCKED | Known defect unchanged; proposed architecture avoids it conditionally |
| Deterministic experimental selection | BLOCKED | Non-saving separate entry not yet constructed/verified |
| Experimental display/resources/access | UNKNOWN | No experimental boot/preflight |
| Known-good stock path | PASS for observed stock boot | Experimental fault/reset/fallback execution UNKNOWN |
| First-load alternative/fallback execution | BLOCKED | Image/entry qualification, boot/recovery authorization and live preflight outstanding |
| SGX exact-action Gate B | BLOCKED | No first-load execution proof or new SGX authorization |
| Whitelist | `[]` | Observation does not authorize deployment/fire |

The smallest next step is **OFFLINE**: separately scoped construction and
qualification of a distinct experimental initramfs copy and non-saving custom
menu-entry text using these captured inputs. Do not stage or boot it in this
observation task. No production source, candidate binary or historical evidence
was changed.

## Offline verification and preserved history

Final rerun: **215 repository tests, zero skips**, 14 analysis-helper tests,
generator check and three strict UBSan harnesses PASS. Both dry runs remain
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` deliberately exits 1 with
`PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`. No boundary was weakened.
Both pinned candidate sizes/hashes are freshly unchanged; no candidate rebuilt.
[Exact commands and results](../hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02/offline-analysis/final-checks/checks.json)
and [summary](../hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02/offline-analysis/final-checks/summary.json)
are retained. GNU cpio independently agrees with the image catalog.

Two local analysis failures are explicitly preserved: the decoder initially
rejected the valid plus sign in a captured memtest filename; a later verification
script initially used an underscored local artifact label instead of its actual
hyphenated label. Corrections affect only local decoding/lookup, with negative
controls and successful full rechecks. Raw capture and target files were untouched.
