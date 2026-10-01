# One experimental first-load boot: staging, observation and stock recovery

**OFFLINE REVIEW ONLY — not an execution authorization.**
Image and procedure design are PASS OFFLINE. Live first load, display/SSH,
trace delivery and reset-to-stock recovery are UNKNOWN. SGX Gate B remains
BLOCKED; whitelist `[]`. This procedure contains **zero triangle operations**.

The required machine-readable bounds are in [first-load-operational-procedure.json](first-load-operational-procedure.json). Use the
[operator checklist](first-load-operator-checklist.md) during execution. [Qualification report](first-load-operational-review.md) separates design from
live evidence. Execute nothing from this document in the current review task.

## 0. Scope, identity and authorization

A future authorization must explicitly cover staging two files and a private
incoming directory, one manual experimental selection, passive observation, an
operator-controlled full machine reset/power boundary, one manual stock
selection and passive recovery checks. It does not authorize the fixed client,
DRM opens, SGX/TA/raster work, new MMIO operations, driver unload/reload, PCI/VT
controls, boot-default changes or a second experimental boot.

The stock original remains active throughout staging. No service is stopped; no
module is removed. This procedure never uses its defective removal path. The
derivative probe performs the same stock KMS/MMIO initialization already
reviewed. Loading it changes device state. Kernel/module ABI qualification does
not prove early probe or reset safety.

Before any live stage the operator must confirm physical presence, ability to
observe GRUB/LCD, local input, manual power controls and acceptance of losing
network/display and unsaved data. No serial, BMC, watchdog, SysRq keyboard
sequence or remote reboot is promised. The operator must confirm that the
physical controls work before proceeding. Do not improvise if they cannot.

Use only the previously reviewed authenticated transport. Credentials must never
enter shell/script stdin, arguments, logs or files. If noninteractive privilege
is unavailable, STOP before writes; local operator authentication is a separate
step. No credentials are requested or used by this offline review.

## 1. Fresh PRE-STAGE read-only guards — STOP on mismatch

Preserve a fresh capture with argv, stdout/stderr/exit codes, UTC time and
hashes. Do not reuse capture 02 as a fresh target-state assertion.

Read these paths and command outputs. Do not open DRM:

- `uname -r`, `uname -m`, `/proc/version`, `/proc/cmdline`,
  `/proc/sys/kernel/random/boot_id`, `/proc/sys/kernel/tainted`.
- DMI `/sys/class/dmi/id/product_name` and selected PCI vendor/device/subsystem
  files; PCI driver/module and DRM device symlink targets.
- `/proc/modules`, `/sys/module/gma500_gfx/initstate`, loaded Build-ID note,
  `/proc/interrupts`, PCI IRQ file, `/proc/fb`, framebuffer name/dimensions,
  vtconsole bind reads, `stat` of card0 only (never open it).
- `sv status /etc/runit/runsvdir/default/slimski` and passive Xorg process list;
  complete current-boot dmesg, current network/SSH availability.
- `lstat`, size and SHA-256 of the five stock paths in the identity table below;
  `/boot/grub/grubenv` decoded values; boot configuration and normal entry.
- `/proc/mounts` or mountinfo, `/boot` free space, ownership/mode and non-symlink
  parents; availability of the normal exclusive-copy/fsync/read-back facilities.
  Missing tools or insufficient privilege is STOP, not permission to install tools.

Require Inspiron 1210 / i686 / `5.10.240-antix.1-486-smp`, with the stock original Live.
Check its loaded note and installed hash, expected gma500 PCI/card0/fb binding,
and IRQ 16 handler. The stock display and userspace must be normal, slimski/Xorg
up, vtcon0=0, vtcon1=1, and taint 12289 with no new fault. Record current boot
ID, not an assumed historical ID. Do not require historical IRQ
counters/PIDs/uptime to repeat. Stock network and access must be adequate to
finish staging before reboot.

Use the qualified [kernel-health guard](kernel-health-guard-qualification.md). Real
BUG/oops/panic/traces/lockups/WARN/new graphics faults always reject. The guard
reports the retained PowerButton and zero-field BL diagnostics with bounded
counts. Changed, repeated or unrecognized diagnostics are rejected. This does
not suppress warnings generally or prove architectural safety. Use the same
classifier for fresh preflight, experimental observation and recovery; preserve
its raw receipt.

| Preserved target path | Size | SHA-256 |
| --- | ---: | --- |
| `/boot/vmlinuz-5.10.240-antix.1-486-smp` | 5984416 | `cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438` |
| `/boot/initrd.img-5.10.240-antix.1-486-smp` | 50863580 | `f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341` |
| `/boot/grub/grub.cfg` | 10438 | `396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f` |
| `/boot/grub/grubenv` | 1024 | `72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927` |
| `/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko` | hash guard | `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb` |

Original loaded Build ID: `d8dcb4d38b774ad64799d5e13aaedede069371f3`.
Read the loaded note, not `modinfo` of the installed file, for runtime identity.
Normal selected stock ID:
`gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1`.
No next_entry, prev_saved_entry or experimental default is permitted.

Require **at least 128 MiB free on the filesystem containing /boot** before
copy, plus space for the private incoming copy on its filesystem. This is a
conservative operational margin, not a hardware fact. Boot parents must be
trusted root-owned directories, not symlinks or writable by other accounts.
The exact experimental image destination, `/boot/grub/custom.cfg`, and incoming
directory must all be absent under `lstat`/lexists, including dangling symlinks.
A preexisting custom.cfg is STOP; do not append, replace or merge it.

## 2. STAGE — future ordinary file operations only

Pinned repository sources:

- `docs/phase8/artifacts/experimental-first-load-01-20261001/build-03/initrd.img-sgx535-firstload-01`:
  **50,804,481 bytes**, SHA-256
  `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`.
- `.../build-03/proposed-custom.cfg`: **974 bytes**, SHA-256
  `181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612`.

Create the future transfer directory `/home/gama/sgx535-firstload-incoming-01` exclusively, with mode
0700 and owner verified from `id -u gama`. It must contain exactly
`initrd.img-sgx535-firstload-01` and `proposed-custom.cfg`; neither is executed. No separate
module or triangle client is copied. Transfer failure is STOP. Source files must
be regular, single-link, no symlinks; verify size/hash from opened file
descriptors. Incoming identities do not replace destination checks.

Final boot destinations, **in this order**:

1. `/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01`
2. `/boot/grub/custom.cfg`

The future privileged writer must create files exclusively: open trusted
parents, create using `O_CREAT|O_EXCL|O_NOFOLLOW`, no overwrite or fallback rename over
an existing path; copy only the fixed hashed input bytes. Use initial mode 0600,
then root:root with mode 0644. The final inode must be a regular file with a
single link. On a write, short-copy, space or read error, STOP. Never publish
the GRUB entry before the image is fully verified. No `update-grub`,
grub-mkconfig, grub-install, grub-reboot, grub-set-default, grub-editenv write,
update-initramfs or depmod. The captured generated 41_custom already sources
this exact custom.cfg location.

For **each** completed file: fsync its file descriptor, fsync its containing
directory, close and reopen no-follow; read every byte from the destination;
verify exact size and SHA-256. Preserve actual sync/read-back exit results.
A successful copy command alone is insufficient. A failed sync is STOP.
Directory fsync unsupported by the observed filesystem is STOP, not a bypass.

After creating the entry, read back its exact bytes and confirm its initrd path
names the verified experimental file. Rehash all five stock paths and verify
same stock Live identity/binding/boot ID/service state as PRE-STAGE. Require
grubenv raw hash unchanged, only the captured saved stock ID, all stock entries
still present, and no default or command-line mutation. Check sufficient
remaining space and no target warning/oops. STOP before reboot if anything
differs.

### Partial-stage reversal

Before **any boot**, only newly created incoming/experimental files may be
removed, and only after rechecking recorded creation ownership/device/inode,
regular/single-link/no-symlink state and current bytes. Never blindly
`rm` a path whose identity is uncertain. Remove custom.cfg before
the experimental image; fsync parent directories and verify absence and
unchanged stock state. A partial file may be removed only when its creation
receipt and captured partial hash identify it. An unexpected inode/hash or
missing receipt means HOLD. Stop for manual review; do not clean up. No graphics
operation belongs to this reversal.

## 3. PRE-BOOT and one manual selection

Stop other work and save user data. Verify every STAGE guard, explicit future
authorization, the operator's physical readiness, the observation clock and the
stock entry title/ID. No fixed client or workload may be scheduled at boot or
executed afterward. Do not change services, module configuration or kernel
arguments for this experiment.

Operator initiates the **separately authorized full machine reset/power boundary**
using controls confirmed before staging. Software must not choose a new shutdown
command, SysRq recipe, kexec or platform reset. If a normal operator shutdown is
possible, save data and finish filesystem sync first; if a fault prevents it,
manual power-off is a separate accepted contingency with filesystem/data-loss
risk. No automatic reset is implemented or promised.

At GRUB, require visible stock title `antiX-26 Stephen Kapos, 5.10.240-antix.1-486-smp` and experimental title
`EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)`, with the saved stock selection still the default. Photograph
the menu before selection. The captured configuration has a menu timeout of 5
seconds (recordfail 30). Practical keyboard and menu availability is not yet
live-qualified. Use the visible menu controls only. No blind keystrokes, entry
editing, saved-default change or grub-reboot command.

Select experimental **once manually**. Its kernel and arguments remain the stock
ones; only the initrd path changes. If either entry is absent, the title/default
is wrong, the menu cannot be seen, or the operator cannot select stock independently,
**do not select experimental**. Allow/select untouched stock, verify it, and stop.
Do not reboot again to try experimental under the same authorization.

The entry omits savedefault/save_env/default/next_entry writes. Normal stock
entry calls savedefault with the same stock ID. The configuration alone cannot
prove which entry the operator selected or which bytes the kernel consumed.

## 4. Observe, capture, then STOP — NO SGX

Observation ceiling: **120 seconds from kernel handoff**, timed by the
physically present operator. This is a new observational limit, not a
kernel/probe timeout. Shell code cannot safely kill or time out a blocked probe.
Before the single root capture, require an explicit operator statement that
local `sudo -v` succeeded during this boot; a userspace/display answer
does not supply that fact. The actual sudo-n result remains mandatory and
failure does not permit a retry. Capture completion, not userspace arrival, must
be timed. See the cycle02 evidence-readiness follow-up. Do not extend this
window or repeatedly reconnect/fire/reset to obtain a PASS. One local login or
one bounded read-only capture may be used; failed SSH is not permission for
exploratory retries. Local evidence may substitute for SSH.

The hook logs to `/run/initramfs/sgx535-first-load.log`; only the outer HOLD message
is guaranteed by script placement to be sent to `/dev/console` (visibility itself
is UNKNOWN). It does **not** stream its successful trace to console/dmesg.
The log is volatile. Captured init moves sys/proc, and init-bottom udev moves dev;
post-pivot accessibility of this /run log is **not established**. If the log cannot
be retrieved, do not invent it or call first-owner proof complete. Preserve any
photo/kernel evidence and classify first ownership NOT ESTABLISHED, then recover.
No image logging mechanism was changed by this review.

Record `elapsed_seconds` from the operator's kernel-handoff clock at completion
of the required capture (finite, nonnegative and ≤120), not login time or uptime
from another boot. Missing, invalid or late timing is NOT ESTABLISHED/HOLD.
Preserve nonempty `prior_boot_id` (PRE-STAGE), experimental
`boot_id`, and later recovery `boot_id`; all three must be
distinct and linked by the capture receipts. The local validator checks supplied
records. It does not authenticate them as live evidence.

Capture all of the following before recovery for first-owner PASS:

- Menu-selection photograph/notes, stage source/destination hash receipt, start
  time, current boot ID different from PRE-STAGE and exact uname/cmdline/DMI.
- Raw hook log with exactly one BEGIN, one FILES-VERIFIED/INSERTION-POSSIBLE,
  one PASS in that order, no HOLD, error or unexplained additional line. A successful boot alone is insufficient.
- Loaded derivative note SHA-256
  `96eb5049d143a3c7a6e7d672aed1651fa51db069ea9efd3484388b3401f29eaf`
  (Build ID `074c650d48ccb463e37e90428b486dccdb23eb80`), Live state and module list.
  Installed root modinfo still describes original; it does not identify loaded code.
- PCI 0000:00:02.0 identity/driver gma500/module gma500_gfx, DRM card0 device link
  to that BDF, node metadata only, framebuffer gma500drmfb, PCI IRQ16 and gma500
  handler on that exact line. IRQ counters may change; zero is not ownership proof.
- Complete current-boot kernel log, including earliest probe through userspace;
  no warning/oops/panic/fault or new taint outside the known P/O/E mask 12289.
- Operator LCD/KMS observation, userspace/login and service state; evidence capture
  complete and attributable to this boot. Record SSH result separately.

The verified image contains no original and CONFIG_DRM_GMA500=m. Before
inserting the pinned payloads, the pre-udev hook checks that the module is
absent and the device unbound. The ordered trace, loaded note, binding, selected
image and complete logs are the evidence for that path. Name-only module/driver
checks, display or cmdline alone cannot establish it. This is operational
evidence, not secure-boot attestation or proof against malicious
filesystem/kernel modifications.

Permitted reads are the corresponding paths listed in section 1 plus the hook
log. Preserve raw stdout/stderr/status, file hashes, boot ID and photos off
target where access works, or operator-supplied local captures before reset.
Logs may be lost on a hang; opportunistic photographs are not substitutes for
mandatory trace. No debugfs/MMIO, DRM open, ioctl, PDS/TA/raster action,
triangle client, Mesa/test workload, unload, original insertion, PCI/console
unbind or second load is allowed. Normal boot's stock services/KMS continue; no
new workload is launched.

### Qualification levels

| Level | PASS criteria |
| --- | --- |
| BOOT-SELECTION | Exact staged bytes + visible manual selection evidence + matching new boot/trace |
| FIRST-OWNER | All trace/identity/binding/IRQ/no-fault evidence above; never name/display alone |
| DISPLAY/KMS | Expected fb/DRM state and physically normal usable display |
| USERSPACE | Expected normal userspace/slimski/Xorg reached; attributable local or remote capture |
| SSH | One successful ordinary bounded connection/capture; separate from local first-owner proof |
| Overall first load | BOOT-SELECTION + FIRST-OWNER + DISPLAY/KMS + USERSPACE; SSH may remain UNKNOWN/failed if local evidence is complete |

These levels do not establish triangle readiness, broader driver safety or
fault-free future operation. Missing mandatory evidence is NOT ESTABLISHED, not
success.

## 5. Failure/HOLD table

| Condition | Required response |
| --- | --- |
| PRE-STAGE/STAGE mismatch, existing destination/symlink, write/readback/sync failure | STOP before reboot; bounded file-only reversal if receipts prove ownership, otherwise HOLD |
| Menu/default/stock selection not independently observable | STOP without experimental selection; verify stock; no retry |
| Hook guard/payload/dependency/insertion/post-binding mismatch | Boot hook HOLD; no fallback original, unload or retry; photo/capture permitted evidence, operator recovery boundary |
| Probe hangs, panic/oops/fault, warning, unexpected original, contradictory ownership | HOLD immediately; no further driver action; operator recovery boundary |
| Black/unusable display, userspace not reached, network absent | Never treat as success. Local evidence if available within deadline; display/userspace failure or missing mandatory evidence → HOLD/recovery |
| SSH unavailable but local trace/binding/userspace/display evidence complete | SSH not established; no speculative network/graphics change; recover stock after capture |
| Missing/duplicate/reordered hook trace, wrong note, premature automatic reboot | First-owner NOT ESTABLISHED/HOLD; no retry; identify new boot separately |
| Deadline reached | HOLD; no kill/unload/reset script. Operator performs only authorized recovery boundary |
| Stock recovery mismatch | HOLD and stop; no hot load, repair, force, alternate kernel, reset loop or experiment retry |

HOLD retains ownership while the experimental kernel is running. Do not
interpret an error as proof nothing initialized. Only the authorized full
machine boundary ends that kernel lifetime. A hot release does not. This
experiment does not qualify any general safe reset method.

## 6. Reset to STOCK, never hot restoration

After success or failure: preserve reachable evidence, stop experimental
activity, and use **one operator-controlled reset/power boundary** as separately
authorized. Responsive system: finish evidence and user-data/filesystem sync
using the normal operator control already confirmed. Unresponsive system:
photograph symptoms; manual power-off may be required. It may lose logs/data or
damage the filesystem. No blind keyboard/SysRq commands, speculative SGX reset,
forced module action, automated power control or remote recovery is introduced.

At the next visible GRUB menu select only the normal **STOCK** title/ID above,
not experimental or an untested alternate init system/recovery entry. If that
cannot be done, STOP for operator recovery; do not try experimental again.
If timeout chooses stock, record that actual selection mode rather than claiming
manual selection evidence. Missing mandatory selection proof keeps full recovery
qualification NOT ESTABLISHED until explicitly reviewed; no extra reboot here.

Within **120 seconds after stock kernel handoff**, capture new boot ID
(different from both PRE-STAGE stock and experimental), exact stock
release/cmdline and staged-stock file hashes; original loaded Build ID/note and
installed original hash; Live original, expected
PCI/card0/fb1280×800/IRQ16/VT0=0/VT1=1; slimski/Xorg/userspace; physically
normal display; stock WLAN/SSH; complete new-boot dmesg and taint. Require saved
stock ID with no experimental next/default entry; inspect/report any grubenv
change rather than silently repairing it. Do not reuse old dmesg or call
power-on alone recovery. All required baseline elements must pass for **LIVE
RECOVERY PASS**, including stock access. Failure → HOLD/manual review.

## 7. Persistence and reversibility limits

Staging intentionally adds only private incoming files, one separate initrd and
one absent-before custom.cfg. It writes no stock kernel/initrd/module/generated
GRUB configuration, module policy, blacklist or boot default. No nonvolatile
GPU/firmware programming is introduced by the hook. Intrinsic driver probe
writes registers; their behavior across warm/cold boundaries is not fully
proved. Normal real-root boot can write logs, filesystem journals, service state
or GRUB environment through preexisting mechanisms. Forced power-off can corrupt
these. The design cannot rule out persistent filesystem changes or guarantee
platform recovery in every case. Rehash/reobserve after stock boot; unknown
effects remain UNKNOWN.

The experimental entry itself cannot become saved default through savedefault.
Any unexpected default/env mutation is STOP/HOLD. Do not edit it back to obtain
PASS. Existing configured menu timeout is not proof of future visible
menu/input.

After **verified stock recovery only**, a separately authorized cleanup may remove
custom.cfg, then experimental image, then the exclusive incoming files/directory,
checking exact hashes/inodes/ownership and syncing parents. Stock paths remain
untouched. Cleanup is optional and not part of this offline task or an automatic
boot action. Untouched stock selection can continue while files remain present;
the experiment is not selected by default.

## Decision and next action

**Staging procedure / manual selection / first-owner observation: PASS OFFLINE
DESIGN. Reset-to-stock procedure: PASS OFFLINE DESIGN, not live-qualified safety.**
Physical menu/control accessibility, post-pivot trace delivery, early probe,
experimental display/SSH and actual recovery remain UNKNOWN. These have mandatory
live STOP guards; no one may silently accept a failed guard.

The procedure is **READY TO REQUEST SEPARATE LIVE FIRST-LOAD AUTHORIZATION**, with
fresh stock identities and physical/menu prerequisites required before mutation
and experimental selection respectively. It is not authorized or executable now.
The existing SGX Gate B does not whitelist this new boot; a future authorization
must explicitly name its non-SGX scope. No old triangle authorization is reused.

**SINGLE SMALLEST NEXT STEP:** operator consideration of a separately scoped
TARGET-STATE-CHANGING authorization for staging, one manual first-load observation
and reset-to-stock recovery under this exact procedure. Do not execute it now.

## Appendix: exact proposed entry (974-byte artifact remains authoritative)

```grub
# OFFLINE PROPOSAL ONLY. Manual selection; leave the saved stock entry unchanged.
menuentry 'EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)' --class gnu-linux --class os --id 'sgx535-rev121-firstload-01' {
	load_video
	insmod gzio
	if [ x$grub_platform = xxen ]; then insmod xzio; insmod lzopio; fi
	insmod part_msdos
	insmod ext2
	set root='hd0,msdos1'
	if [ x$feature_platform_search_hint = xy ]; then
	  search --no-floppy --fs-uuid --set=root --hint-ieee1275='ieee1275//disk@0,msdos1' --hint-bios=hd0,msdos1 --hint-efi=hd0,msdos1 --hint-baremetal=ahci0,msdos1  6da9b4a7-ede2-4e27-bbfc-b537f568eaf1
	else
	  search --no-floppy --fs-uuid --set=root 6da9b4a7-ede2-4e27-bbfc-b537f568eaf1
	fi
	echo	'Loading Linux 5.10.240-antix.1-486-smp ...'
	linux	/boot/vmlinuz-5.10.240-antix.1-486-smp root=UUID=6da9b4a7-ede2-4e27-bbfc-b537f568eaf1 ro  quiet selinux=0
	echo	'Loading initial ramdisk ...'
	initrd	/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01
}
```

## Cycle02 evidence and a separately authorized successor

[Cycle02 result](first-load-cycle-02-result.md) records exact staging, one manual
experimental selection, failed privileged experimental capture and independently
verified normal stock recovery. This cycle is spent; no second experimental boot.

Under a new authorization, do not recopy or overwrite the existing staged
image/entry. Repeat the full fresh stock/default/health/parent/file checks using
the provided post-stage passive scope, then compare both exact file identities
AND device/inode against the recorded exclusive creation receipts. The offline
`validate_existing_stage_files` guard rejects substitution even with identical bytes,
missing/duplicate receipts, wrong ownership/type/mode/link/hash. This reuses the
already approved artifacts; it does not waive the original absence guard for a
new creation operation. Any mismatch means STOP. Do not overwrite or use cleanup
as a workaround. The fresh stock boot ID becomes the new cycle's prior ID.

The future capture wrapper is qualified offline. It requires explicit local sudo
readiness, preserves unprivileged boot identity before privilege in the same
connection, records same-boot uptime at start/end, bounds the read-only child,
and preserves partial timeout output. It changes no image/hook/driver or SGX
operation. It was not used in cycle02 and requires the successor's reviewed
controller to pin its root program/source. Physical kernel-handoff/completion
timing remains mandatory; do not substitute service-reported duration or
unsynchronized target/host UTC for that clock.

A successor must be explicitly authorized for exactly one manually selected
non-SGX experimental boot and operator-boundary stock recovery. No SGX whitelist
is generated by this design. No triangle/ioctl/TA/raster/hot restoration/retry.
