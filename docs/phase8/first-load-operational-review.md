# First-load operational review — 2026-10-01

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

## Original offline review

The rest of this report records the offline review before the live cycles.

**FIRST TRIANGLE: NOT ATTEMPTED. Gate B: BLOCKED. Whitelist: `[]`.**
**No target contact, staging, boot modification, reboot, module operation,
MMIO experiment, fixed ioctl or SGX fire occurred.**

Deliverables: [exact procedure](first-load-staging-boot-recovery-procedure.md), [operator checklist](first-load-operator-checklist.md), [machine-readable bounds](first-load-operational-procedure.json),
[review evidence](artifacts/first-load-operational-review-20261001/README.md). This review did not reconstruct the image or compile
candidates.

## Fresh artifact verification

All files match the preceding 145-file qualification manifest. Independent
inspection with GNU cpio and the preserved parser checked the image structure,
stock records and delta, single private derivative, ten-module dependency
closure and CRC maps, and hook/init bytes and ordering. No image was
regenerated.

| Item | Fresh result |
| --- | --- |
| Experimental image | 50,804,481 bytes; `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71` |
| Embedded derivative | `91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`; ELF32 i386; module_layout `0xb84efb99`; 232/232 imports, zero missing/mismatch |
| Candidate #1 control | `13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f`; unchanged, 230/230 |
| Hook | `e95e373d2f07740ce2ad6e90f4bfa31d4e25860ab7f1f67dda5cc62368d3c4ee`, before all init-top/udev |
| Proposed entry | 974 bytes; `181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612` |
| Stock initrd | 50,863,580 bytes; `f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341` |
| Stock kernel | Captured on-target hash only: 5,984,416 bytes; `cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438`; binary absent locally, not independently rehashed here |

Direct dependency order remains drm, syscopyarea, sysfillrect, sysimgblt,
fb_sys_fops, cec, drm_kms_helper, video, i2c_algo_bit, derivative. The nine
dependencies match the captured stock bytes. Complete ten-file CRC coverage
remains PASS. The earlier byte-identical repeat evidence is unchanged. No repeat
build was needed.

## Procedure decision

| Predicate | State | Limit / guard |
| --- | --- | --- |
| Pinned image / dependency provenance | PASS OFFLINE | No artifact drift; all embedded import CRCs checked |
| Staging procedure | PASS OFFLINE DESIGN | Exclusive no-follow create, no overwrite, root0644, file/parent fsync and destination read-back; fresh live checks required |
| Stock preservation / staging reversal | PASS OFFLINE DESIGN | Separate image/custom.cfg/private incoming only; remove created identities only; no stock replacement |
| Manual boot selection | PASS OFFLINE DESIGN | Visible entries, saved stock ID; one selection; no default/env mutation; practical menu/input UNKNOWN |
| First-owner observation plan | PASS OFFLINE | Trace plus selected image/new boot/loaded note/binding/IRQ/no-fault; KMS/name alone insufficient |
| Experimental display/KMS / userspace / SSH | UNKNOWN | Local evidence can substitute for SSH; display/userspace required for overall first-load PASS |
| Post-pivot hook-log delivery | UNKNOWN | Volatile /run output not proved to survive; missing trace → NOT ESTABLISHED, no false PASS |
| Reset-to-stock procedure | PASS OFFLINE DESIGN | Operator full boundary, manual untouched stock selection; no hot restoration or reset script |
| Physical controls / practical fallback | UNKNOWN until live prerequisite | Operator confirmation and actual visible independently selectable stock entry required |
| Live first load / live recovery | UNKNOWN | No experimental boot yet; reset/power-on alone cannot close recovery |
| Original hot removal/restoration | BLOCKED | Known defect remains; never used by this procedure |
| Non-SGX first-load permission | READY TO REQUEST SEPARATE AUTHORIZATION | No present permission/whitelist; authorization must explicitly cover staging/reset contingencies |
| SGX Gate B / execution | BLOCKED / NOT AUTHORIZED | No SGX whitelist, no triangle readiness implication |

Exact future final paths: `/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01` and `/boot/grub/custom.cfg`. Staging
does not write the stock kernel/initrd, generated GRUB configuration, installed
module or saved selection. The entry is the exact already-qualified non-saving
text. It has not been modified.

Before reboot, future staging must check the source and reopened destination
bytes and metadata, completed sync, symlink/hardlink handling, ≥128 MiB free
/boot space, unchanged stock files and live state, and the exact saved stock ID.
Do not merge an existing custom.cfg. A fresh scope or guard failure stops before
mutation.

## Problems checked during review

1. **Original/default collision:** existing custom.cfg or image path of any type
   must stop, including dangling symlinks. Do not use cp overwrite or merge entries.
   Create the entry only after image sync/read-back. Static record gates reject
   stock overwrite, bad owner/mode/hardlinks and saved experimental selection.
2. **Apparently correct display:** installed root modinfo describes original,
   and both boots share cmdline/kernel. Require the loaded derivative note, ordered
   hook trace and selection provenance. A module name or display is not enough.
3. **Lost trace:** actual hook redirects detailed output to volatile /run; only
   outer HOLD is written to console. Stock init visibly moves sys/proc and udev
   moves dev. The retained evidence does not prove that BusyBox run-init preserves the /run
   log. Delivery stays UNKNOWN. Reject a missing trace; do not modify the pinned
   image or claim success without it. Hang/reset may irretrievably lose that log.
4. **Automatic experimental boot:** the captured GRUB menu has timeout 5 and a saved
   stock default. The proposed entry omits savedefault. No grub-reboot/default changes. If menu/input
   unavailable, choose/allow stock and stop without experimental retry.
5. **Unexpected fault/reboot:** zero automatic retries/reset operations in our
   design. The captured kernel has panic timeout 0/panic-on-oops 0. Watchdog support is
   enabled; actual platform/runtime watchdog behavior is UNKNOWN. An unexpected reset
   is not success; capture distinct boot evidence and stop. No watchdog settings
   are changed or assumed safe.
6. **False recovery:** a new stock boot needs the exact original loaded identity,
   files/binding/IRQ/fb/VT/service/physical-display/network evidence and full new-boot
   kernel log. No hot restoration. Power-on alone is rejected.
7. **Persistence:** normal root boot writes logs/journals/service state; stock
   savedefault can write grubenv; forced power-off can lose data/corrupt files.
   Intrinsic graphics programming may affect the next boot. The design preserves
   stock selections/files, but universal device/filesystem reversibility is UNKNOWN.
   Verify after reset, never edit mismatches back to make recovery PASS.

These constraints do not accept any new hardware risk. The earlier scoped
publication/ISP/L12/FT-AUX decisions are historical. They do not authorize this
boot or extend its zero-SGX scope.

## Offline executable checks

Seventeen new tests run the actual local plan and observation-record validators.
They reject overwritten stock destinations, policy changes, incomplete
read-back/sync, symlinks/hardlinks/ownership/mode/hash mismatch, insufficient
space, changed stock identity or boot during staging, default/next_entry
changes, absent/duplicate/reordered/HOLD trace, wrong loaded
note/IRQ/BDF/fault/taint/second boot, and recovery without stock
identity/display/SSH/new boot. They accept complete local first-owner records
without requiring SSH. Synthetic records are not target observations: their
classification is **PASS PROVIDED RECORD**.

The validators only read local JSON and artifact bytes, compare values and print
results. They do not use remote transport, stage files, execute processes or
operate hardware. Red→green evidence is retained, including a corrected
test-indentation error and valid failing fresh-stock-identity control. No
production driver/image changed.

Independent review found five missing checks: boot references, unverified
services, unverified recovery defaults/framebuffer, unrecognized hook errors,
and unchecked deadlines. The added negative controls initially failed in 30
subcases. All 17 procedure tests now pass with the required linked three-boot
IDs, service checks, exact trace, stock GRUB/fb guards and finite ≤120-second
timing. The validators cannot authenticate a supplied record as hardware
evidence.

Final verification: 266 repository tests PASS, zero skips; 14 boot-analysis
tests PASS; three strict UBSan harnesses PASS; generator PASS. Both actual
candidate import sets PASS (230/230 and 232/232, zero missing/mismatch), as does
the independent image verifier. Two dry outputs hash to
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` intentionally exits 1 with PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE.
The evidence bundle contains the commands, exit codes and preservation/link/delta
checks. No candidate or image was rebuilt. Historical artifacts are unchanged.

## Single smallest next step

**Request a separate TARGET-STATE-CHANGING authorization** for the exact non-SGX
staging, one manually selected first-load observation and operator reset-to-stock
recovery. Fresh stock guards and physical/visible-menu prerequisites must pass
before the applicable actions. No present target action or SGX fire is authorized.
