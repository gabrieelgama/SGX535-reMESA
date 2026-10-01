# Experimental first-load image — offline qualification, 2026-10-01

**Newest offline operational review (2026-10-01):**
[Procedure qualification](first-load-operational-review.md),
[exact procedure](first-load-staging-boot-recovery-procedure.md) and
[checklist](first-load-operator-checklist.md) complete the staging/manual-first-load/
operator-reset-to-stock design review. Pinned image/candidates unchanged; no target
contact or staging. **266 tests, zero skips; 14 boot-analysis tests; three UBSan
harnesses PASS.** The non-SGX procedure is **READY TO REQUEST SEPARATE LIVE
AUTHORIZATION**, conditional on fresh stock, privilege, physical-control and visible
menu guards. Live first ownership/display/SSH/recovery remain UNKNOWN; volatile
hook-log delivery after pivot is UNKNOWN and missing trace prevents qualification.
No hot transition/restoration or SGX action is authorized. **Gate B BLOCKED;
whitelist `[]`; FIRST TRIANGLE NOT ATTEMPTED.** Earlier next-step notices are historical.

**Image construction / first-owner design: PASS OFFLINE. Live first owner,
experimental display/SSH and reset/fallback qualification: UNKNOWN.
Gate B: BLOCKED. Whitelist: `[]`. FIRST TRIANGLE: NOT ATTEMPTED.**

This task constructed local artifacts only. No target contact or mutation,
module insertion/removal, candidate rebuild, DRM operation or SGX fire occurred.
Nothing was copied to the Mini 12 or installed in `/boot` or GRUB.

The [evidence index](artifacts/experimental-first-load-01-20261001/README.md)
contains actual inputs, commands, manifests, full CRC maps, tests and review
resolution. This report supersedes older “image not constructed” statements;
historical captures and conclusions remain intact.

## Exact inputs and artifact identity

| Input | Evidence class | SHA-256 / qualification |
| --- | --- | --- |
| Captured stock initramfs | RETAINED TARGET FACT | `f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341`, 50,863,580 bytes, initramfs-tools 0.148.3 |
| Lifecycle derivative | OFFLINE-TESTED | `91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`, 242,724 bytes; ELF32 little-endian i386; 232/232 versioned imports, no missing/mismatch |
| Target symbol table | RETAINED TARGET FACT | `faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca` |
| Captured target config | RETAINED TARGET FACT | `93f4d7a779f4be65097b5f26db6c9b10431907c6d417109719eb2f16d6def3d9`; boot-config/build__config byte-identical |
| Captured GRUB configuration | RETAINED TARGET FACT | `396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f` |
| Stock kernel | RETAINED TARGET FACT, hash-only | `/boot/vmlinuz-5.10.240-antix.1-486-smp`, 5,984,416 bytes; `cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438`. Binary not captured or modified; future staging must verify it again. |
| Candidate #1 control | OFFLINE-TESTED | `13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f`, 230/230 imports, unchanged; not the payload selected here |

Derivative vermagic remains `5.10.240-antix.1-486-smp SMP mod_unload modversions 486 `;
module_layout is `0xb84efb99`; Build ID is
`074c650d48ccb463e37e90428b486dccdb23eb80`. Both existing module artifacts and
all captured inputs are unchanged. No new module compilation occurred.

**Final experimental image:**
[initrd.img-sgx535-firstload-01](artifacts/experimental-first-load-01-20261001/build-03/initrd.img-sgx535-firstload-01)

- Size: **50,804,481 bytes**.
- SHA-256: **`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`**.
- Hook SHA-256: `e95e373d2f07740ce2ad6e90f4bfa31d4e25860ab7f1f67dda5cc62368d3c4ee`.
- Derivative inside image: exact selected hash above, one copy only.
- Two fresh corrected builds (`build-03`, `build-04`) are byte-identical.

## Dependency closure and loading method

The topological insertion sequence is fixed:

| Position | Internal name | Origin |
| --- | --- | --- |
| 1 | drm | Captured stock image |
| 2 | syscopyarea | Captured stock image |
| 3 | sysfillrect | Captured stock image |
| 4 | sysimgblt | Captured stock image |
| 5 | fb_sys_fops | Captured stock image |
| 6 | cec | Captured stock image |
| 7 | drm_kms_helper | Captured stock image |
| 8 | video | Captured stock image |
| 9 | i2c_algo_bit | Captured stock image, filename i2c-algo-bit.ko |
| 10 | gma500_gfx | Qualified lifecycle derivative, private payload |

The closure is derived from actual derivative `depends` and captured
`modules.dep`, checked against actual dependency metadata and retained hashes.
All nine dependencies already exist in stock. Every actual undefined symbol is
covered by a matching version record and target CRC: **1,076 versioned imports
across ten files / 1,066 actual undefined symbols; zero missing/mismatch**.
Every file has the expected vermagic/module_layout. Exact paths/hashes/complete
CRC maps are in `build-03/payloads.json` and `module-qualification.json`.
No encoded firmware request or softdep adds a file requirement. This does not
prove firmware/BIOS/resource state during a future early probe.

Use the captured BusyBox **normal fixed-path `insmod`**, once per file in this
order. No depmod, alias lookup, modprobe wrapper or synthetic symbol records are
needed. Existing module indices remain byte-exact. The private derivative is
outside the indexed stock module tree. Aliases/internal name are unchanged;
selected PCI identity is 8086:8108 / 1028:02b1 at 0000:00:02.0.

## Construction, preservation and independent inspection

The builder preserves the first **15,006,720 bytes** (early archive/microcode
and padding) exactly. It splices raw newc records in the existing gzip main
archive, preserving all stock records except `/init`, then gzip-compresses with
level 9 and mtime 0. It does not run target scripts or regenerate an installed
initramfs. The stock framework/scripts remain the captured 0.148.3 bytes.

Exact delta: **2,138 → 2,141 members**, none removed:

- Add `usr/lib/sgx535-first-load` (directory).
- Add `usr/lib/sgx535-first-load/gma500_gfx.ko` (private derivative).
- Add `scripts/sgx535-first-load` (hook).
- Modify only `init`, adding one guarded call before `run_scripts /scripts/init-top`.

All **2,132 unchanged main records** preserve headers, payloads, padding,
permissions and hardlink identities. `/init` preserves its original executable
metadata. Added inodes are disjoint, modes/ownership/nlink/type explicit.
The stock BusyBox tools are zero-payload hardlink entries, sharing the sole
ELF32 BusyBox payload at `usr/sbin/watchdog`. Their records remain unchanged;
no naive extraction/repacking loses that relationship.

Independent inspection uses GNU cpio 2.15 and the unchanged prior capture parser.
Both enumerate the expected members; cpio independently extracts derivative,
hook and init bytes. The full manifest and four-path delta are preserved.
New additions contain static guards/identities only, no credentials, live IP,
SSH configuration or mutable target session state. No triangle client is added.

## Actual pre-udev execution and guards

Captured `/init` mounts proc/sysfs/devtmpfs and creates `/run/initramfs` before
our insertion. The call precedes **all init-top scripts**, including the captured
udev trigger; `/conf/modules` would be too late and is not used. Stock init-top
ORDER remains byte-exact.

The actual hook requires:

- Inspiron 1210, i686, exact kernel release and PCI/subsystem identity.
- No other device matching any derivative PCI alias.
- No existing selected PCI driver, DRM card0, framebuffer fb0, any closure
  module, or gma500 IRQ handler on **any IRQ**.
- No conflicting kernel/module blacklist, nomodeset or break policy.
- All ten fixed file hashes verified **before any insertion**.

After normal insertion it requires Live gma500_gfx, exact loaded Build-ID-note
hash, PCI driver and driver-module links, DRM device association, expected
`gma500drmfb`, PCI IRQ **16** and a gma500 handler **on IRQ 16**.
Successful insmod alone is not successful ownership.

Hook/init checks include command exit status, not just matching checksum output.
Any guard/insertion/probe-return/post-binding error prevents udev/root continuation
and enters the fixed HOLD sleep loop. Already inserted resources are retained;
there is no unload, PCI unbind, hot replacement, retry, reset or SGX submission.
An insertion/probe that hangs does not return to shell: offline tests cannot
bound it; physical recovery is a separate authorization/qualification boundary.

## GRUB isolation

[Proposed entry text](artifacts/experimental-first-load-01-20261001/build-03/proposed-custom.cfg)
uses the same stock kernel and unchanged root UUID / `ro quiet selinux=0` arguments.
Only the label/entry ID and distinct initrd pathname change; `savedefault` is
removed. Future proposed pathname:
`/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01`.

No default/saved-entry write, next_entry, blacklist, automatic selection or retry
is introduced. It is visibly **EXPERIMENTAL FIRST-LOAD ONLY (no triangle)**.
The captured custom.cfg sourcing can host it separately in a future task.
This text has **not** been installed. The configured known-good saved stock entry
and stock kernel/initrd bytes remain untouched. The entry is not a replacement
for a full staging/boot/recovery procedure or live GRUB validation.

## Tests, review and failure controls

**249 scoped tests PASS, zero skips**, including **34 new image/hook tests**;
14 retained boot-analysis tests PASS; three freshly compiled strict UBSan C
harnesses PASS; generator check PASS. Two frozen dry runs retain SHA-256
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still exits 1 with **PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE**.
No architectural evidence boundary was changed. Final whitespace, evidence-link,
manifest and starting-file-preservation checks are recorded separately.

Tests execute the **emitted shell decisions**, replacing only external I/O and
insertion boundaries. Every insertion position 1..10 is failed in turn; none
continues or retries. Wrong machine/kernel/architecture/PCI, extra GPU, existing
owner/IRQ/display resource, absent dependency, bad hash, insertion failure and
wrong post-binding/note/Live/IRQ state reject. Actual hash and IRQ helpers run
against ordinary local fixtures. Removing the actual machine guard admits the
wrong machine (deliberate negative control). Init checksum/hook failures cannot
reach the simulated next udev stage, even when a failed checksum prints the
expected digest.

Independent review found two gaps, fixed with red→green tests: handler attribution
was too broad; image metadata validation omitted init mode/new inode collisions.
Four corrupted **actual image** controls now reject: nonexecutable init, colliding
private inode, corrupt module and changed early prefix. Failed/superseded images
and erroneous repeat-measurement evidence remain preserved, clearly distinguished
from final build03/04; `verified-corrected-repeat.json` is the authoritative repeat
measurement. No production driver source or binary was changed.

## Continuation, failure and recovery — proof classes

**SOURCE-PROVEN / OFFLINE-TESTED:** the already reviewed PCI core only probes an
unbound matching device; same-name Live module prevents a second original being
co-loaded. The derivative retains the stock probe/KMS/fbdev and owns/releases its
own IRQ correctly. See [source qualification](first-load-boot-qualification.md).
After our successful binding, later stock modalias discovery cannot replace that
owner via the normal bind/load path. Normal initramfs/root boot is then allowed.
This is not proof of all future userspace actions; an explicit later unload path
or competing resource remains a STOP if observed.

**INFERRED:** successful early first binding should avoid the defective original
lifetime altogether and allow normal stock continuation. The precise historical
root module requester remains UNKNOWN; it is not needed to race an already-bound
owner, provided no later mutation removes that owner.

**UNKNOWN / REQUIRES LIVE OBSERVATION:** actual early probe/resource/firmware
behavior; first-owner evidence; visible LCD/KMS; network/SSH; later boot behavior;
hang/fault effects; physically selecting unchanged stock and recovery after an
experimental reset. Current stock WLAN/display success is not future proof.

Recovery design is **no hot restoration**: preserve state/HOLD, operator recovery
across a separately reviewed reset/reboot boundary, manually choose untouched
stock GRUB entry and stock kernel/initramfs. No automatic reset/reboot/retry is
implemented. Stock artifacts are preserved offline; live recovery is not PASS.

| Predicate | Classification | Remaining boundary |
| --- | --- | --- |
| Image construction / dependency provenance | PASS OFFLINE | Exact manifest, ABI/file hashes and repeat |
| Pre-udev ordering / guards / no-hot-transition | PASS OFFLINE | Actual image/script checks and failure injection |
| GRUB isolation / stock preservation | PASS OFFLINE | Text only, no installed changes |
| Normal continuation model | SOURCE-PROVEN / INFERRED | Stock PCI/module rules; actual boot not observed |
| Live first-owner qualification | UNKNOWN | Separately authorized experimental boot/preflight |
| Experimental display/resources / SSH | UNKNOWN | Future live observation |
| Reset/fallback architecture | SPECIFIED OFFLINE | Separate exact recovery/selection review required |
| Live recovery / first-load alternative execution | UNKNOWN / BLOCKED | No experimental boot has happened |
| Original hot removal / hot restoration | BLOCKED | Not repaired or replayed |
| SGX Gate B / whitelist | BLOCKED / `[]` | No boot/SGX authorization follows from image construction |

**SINGLE SMALLEST NEXT STEP — OFFLINE:** review the exact staging, manual
non-saving experimental boot and reset-to-stock recovery procedure for this pinned
image/entry, including STOP/HOLD and preservation checks. Only then request a new,
separately scoped target-state-changing authorization. Do not deploy or fire now.
