# Fixed-scene active gma500 transition review

**Current classification: BLOCKED.** The
[post-Attempt-03 checkpoint audit](post-attempt03-checkpoint-audit.md) records
the incompatible candidate and failed original-driver hot restoration. The
operator reset the target; [capture 08](../hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/RESULT.md)
now independently confirms normal original driver/display operation and
installed export-table/configuration provenance. The old hot-transition
recovery path remains unqualified. No new transition is authorized. The
PASS below is the historical procedure review, not a current qualification.

**Recovery snapshot before the operator-reported reset:** The separately authorized
[original-driver recovery](../hardware-evidence/MINI12-20260930-ATTEMPT03-ORIGINAL-RECOVERY-01/RESULT.md)
failed during normal original-module insertion with a kernel oops in IRQ
registration. At capture the module was `Loading`, PCI had a driver link, and no DRM
card/framebuffer existed; slimski remained down. No retry or SGX scene action
occurred. The historical transition and clean-unbound rollback predicates
below no longer describe the current target state.

**Attempt 03 outcome, 2026-09-30:** [Retained evidence](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-03/RESULT.md)
shows that normal `vtcon1` release reduced the original module refcount to
zero and nonforced removal detached it, but the pinned candidate `insmod`
failed with `module_layout` symbol-version disagreement. At capture the target was in
HOLD with no gma500 module or DRM card; no ioctl or SGX fire occurred. The
single-attempt authorization is spent. No automatic rollback or retry is
covered by this review.

**Offline follow-up:** [Version comparison](attempt03-module-version-followup.md)
shows the source-package `Module.symvers` supplied the candidate's incompatible
`module_layout` CRC and 149 other mismatched shared import CRCs. The earlier
PASS classified the transition procedure and its stop guards; it did not
qualify this candidate's runtime ABI. The pinned candidate and the whitelist
entry naming its hash must not be reused. A compatible candidate and a
separate recovery/review/authorization are required before any new transition.

**Attempt 03 scope, 2026-09-30:** The operator separately authorized one
new execution of whitelist `MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`, adding
only the [live-tested normal `vtcon1` release](../hardware-evidence/MINI12-20260930-CONSOLE-RELEASE-01/RESULT.md)
between service stop and the existing removal preview. Before release,
require original loaded Build ID/PCI binding, zero root-visible graphics FD
holders, `vtcon0=0`, `vtcon1=1`, and refcount 1. After exactly one write of
`0`, require `vtcon0=1`, `vtcon1=0`, refcount 0, and unchanged original
module/PCI identity. If the removal preview refuses while the original
module is still cleanly bound, the tested one-write rebind must restore
`vtcon0=0`, `vtcon1=1`, refcount 1 before the reviewed `slimski` restart.
No force operation, PCI unbind, extra submission or reset is added.
Attempt 02 already staged the exact candidate/client files; this attempt
must verify their existing root-owned paths, modes and pinned hashes and
must not overwrite or substitute them.

**Console test update, 2026-09-30:** The independently authorized
[VT-console release test](../hardware-evidence/MINI12-20260930-CONSOLE-RELEASE-01/RESULT.md)
observed the normal `gma500_gfx` reference release and reacquisition, with
the original module and PCI binding unchanged and the graphical service
restored. No module transition or SGX action occurred. The test does not
extend a prior one-attempt authorization to another triangle attempt.

**Live update, 2026-09-30:** [Attempt 02](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-02/RESULT.md)
stopped at the required removal preview. After `slimski` stopped and no
root-visible graphics FD holders remained, `modprobe -n -v -r gma500_gfx`
exited 1 with `Module gma500_gfx is in use`; `/proc/modules` still showed
reference count 1. No module removal or SGX submission occurred. The
reviewed service-only reversal restored the initial passive state. The
historical procedure below remains the record of the approved attempt, but
its stage-2 prerequisite is **BLOCKED for another attempt** until the
residual module use and a nonforced release path are qualified and reviewed.
Do not repeat the stopped attempt under its one-attempt authorization.

The bounded [fbcon reference analysis](gma500-fbcon-refcount.md) and
[read-only target capture](../hardware-evidence/MINI12-20260930-GMA500-REFCOUNT-READONLY-01/RESULT.md)
identify the framebuffer console's retained module reference. The exact
antiX source provides a normal VT-console unbind path that should release
that reference, and the target shows the framebuffer console bound to its
sole gma500 fbdev. This **does not amend stage 2 automatically**: writing
the VT-console bind control is a new display-state-changing operation.
Its target effect, readback and rollback guards need separate review before
any subsequent attempt. No such write occurred in this investigation.

**Historical classification: PASS as an exact-action transition review, subject to the
listed stop guards at execution.** Four operator risk decisions are accepted,
the 2026-09-30 passive preflight matched 28/28 retained H0 checks, and the
separately approved root read-only inventory resolved the three missing
service/holder/privilege facts. This is a reviewed procedure, not proof that
the module switch will succeed. **No deployment or active SGX action is
authorized in the inventory turn; a new explicit operator authorization is
required before stage 0.**

## Exact artifacts and current ownership

- Original installed target module:
  `/lib/modules/5.10.240-antix.1-486-smp/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko`,
  SHA-256 `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`.
  Its loaded GNU Build ID was
  `d8dcb4d38b774ad64799d5e13aaedede069371f3` at preflight.
- Candidate module (still local):
  `/tmp/sgx535-antix-source-6/source/drivers/gpu/drm/gma500/gma500_gfx.ko`,
  SHA-256 `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`,
  GNU Build ID `d4cb8d750efa6eb401f82e6eaec7eb82ebdf9c3c`, target
  vermagic `5.10.240-antix.1-486-smp SMP mod_unload modversions 486`.
- [Static i386 fixed client](artifacts/frozen-triangle-one-shot-i386.md):
  SHA-256 `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`.
  Its only executable request is the root-only private ioctl with the
  16-byte `{1,1,0,0}` input; the reviewed flag is required even to open
  `/dev/dri/card0`.
- Live preflight: PCI `0000:00:02.0` bound to `gma500`, loaded
  `gma500_gfx` with reference count 2, `/dev/dri/card0`, active
  `gma500drmfb` 1280×800 framebuffer and one Xorg. The later root-only
  [transition inventory](../hardware-evidence/MINI12-20260930-TRANSITION-INVENTORY/RESULT.md)
  found runit PID 1, `slimski` PID 1409, child Xorg PID 1766, the
  `/etc/runit/runsvdir/default/slimski` service link, and one root-visible
  DRM/fb holder: Xorg FD 16 on `/dev/dri/card0`. These are snapshots;
  dynamic state must match again immediately before an authorized switch.
  A narrow supplemental read identified `runsv slimski` as PID 1409's live
  parent, closing the supervisor-to-process link.

The exact antiX source's `psb_pci_remove()` calls `drm_dev_unregister()`,
`psb_driver_unload()`, then `drm_dev_put()`. Unload tears down the fbdev,
modeset, SGX MMU/GTT and mapped registers. Its load path initializes MMU,
PDS/BIF bases, IRQ, modeset and fbdev. **Removing or loading this module is
already an active hardware/display transition; the first SGX action is not
deferred until the fixed ioctl.** A matching vermagic does not prove that
this transition will work while the display is active.

## Reviewed single transition, not executed

The privileged route is the pinned SSH login as `gama` followed by the
operator-authenticated sudo path that succeeded for the read-only inventory.
The credential must be supplied interactively, never embedded in a command,
script, environment, or log. After a **new explicit authorization**, the
proposed staging destination is a newly created root-owned mode-0700
`/root/sgx535-frozen-seq1/`, with the candidate module mode 0600 and i386
client mode 0700 as distinct files. Transfer to a temporary unprivileged path is only
an intermediate CPU file copy; the root-owned final files must be hashed
again against the pinned values immediately before any module operation.
If that directory or its files already exist unexpectedly, STOP rather than
overwrite. The original installed module remains untouched.

For a future stage 0, use the corrected
[two-process sudo transport](mini12-credential-delivery.md): one pinned SSH
process runs `sudo -S -p '' -v` with only the interactively supplied
credential on stdin and **no shell script**. After successful, silent
validation, a separate pinned SSH process runs `sudo -n -p '' sh -s` with
only the reviewed script on stdin and **no credential**. An expired sudo
timestamp rejects the second process before the shell starts. Do not use
the combined credential-plus-script stdin stream that stopped Attempt 01.

Every stage below has its own check; advancing past a failed check is outside
the review. The service stop, module removal, and module insertion are
state-changing operations and were **not** performed during inventory.

| Stage | Required operation | Stop / rollback / HOLD boundary |
| --- | --- | --- |
| 0. Revalidate and stage | Before writes, recheck the 28 passive preflight predicates and the root-visible service/holder facts against the captures. Confirm a physically present operator and an SSH/log path independent of the display. Preserve the original installed module. Stage the **two exact hashed files** into a new root-owned, non-world-writable directory without replacing the original. Re-hash both on target and verify candidate ELF32/i386 vermagic before driver change. | Any mismatch, unexpected holder, missing tool/privilege, absent operator, or failed log capture: STOP. No service stop or module operation. Staging is reversible; staged bytes must not be substituted. |
| 1. Release graphical client | Use the observed runit service path: `/usr/bin/sv stop /etc/runit/runsvdir/default/slimski` once. Check `sv status`, absence of `slimski` and Xorg, and a fresh root `/proc` enumeration showing no DRM/fb FD holders. Do **not** rely on `/etc/init.d/slimski stop` alone because runit supervises the service. | If it does not stop or respawns: STOP before module removal. With the unchanged original still bound and no ambiguous state, `/usr/bin/sv start /etc/runit/runsvdir/default/slimski` is the proposed service-only reversal. No force/kill fallback. |
| 2. Remove original module | After confirming only the reviewed original Build ID is loaded, inspect `/usr/sbin/modprobe -n -v -r gma500_gfx` and require its plan to remove **only** `gma500_gfx`, with no configured removal helper or other module. If that exact condition holds, use ordinary **nonforced** `/usr/sbin/modprobe -r gma500_gfx` once. Check status, module absence, expected PCI detach, and DRM node removal before any candidate load. No PCI sysfs unbind, forced unload, or alternate reset. | An unexpected dry-run plan: STOP before removal. Busy/refused unload with original still bound: STOP and retain the original. Partial or ambiguous detach or a different actual removal effect: HOLD. `modprobe -r` can remove unused dependencies, so an unreviewed removal plan is never accepted. |
| 3. Load candidate | Only after an unambiguous clean detach, use ordinary `/usr/sbin/insmod` on the root-owned staged file whose SHA-256 is `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`. Verify loaded Build ID `d4cb8d750efa6eb401f82e6eaec7eb82ebdf9c3c`, PCI `8086:8108` bound to `gma500`, and exactly the expected `card0` before opening it. | Any load failure, identity mismatch, unexpected display/binding, or ambiguous probe: HOLD before ioctl. The candidate's load path itself initializes hardware. No automatic unload/reload. |
| 4. One fixed request | After all post-load checks, run only the exact static i386 client SHA-256 `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf` once with `--one-shot-sgx535-rev121` and a new exclusive color-output path. It opens the verified DRM node and issues ioctl `0xd0ac6440`, input `{1,1,0,0}`. | Reject/fault/timeout/unknown outcome: HOLD; retain candidate module, BOs, USE and scene as implemented. No second invocation, reset, power/clock change or speculative original-module reload. |
| 5. Attribute result | Preserve operation errno, phase, event mask, row counts, FNV-1a, full color bytes when available, target logs and exact hashes. Require attributed TA and raster events plus image result before claiming a triangle. | Ioctl return alone is not completion; ambiguity is HOLD. No automatic second scene. |

The one-shot kernel path uses sequence 1, at most one TA and one raster fire,
`5×HZ` deadline, 300,000 status-sample limit, no retry, and HOLD on
contradictory status or any failure after possible fire. These are software
bounds, not proof that CPU MMIO or the whole machine cannot hang. The prior
operator accepted the possibility of display/platform failure, not an
automatic recovery procedure. A manual power-cycle remains an operator
contingency with unproved outcome and possible filesystem loss.

The preserved original module remains installed, but that fact alone is not
a qualified automatic rollback. Before driver removal, restoring runit's
confirmed `slimski` service is the only proposed reversal of a clean
service stop. If the original is cleanly removed and the candidate has not
been loaded, an operator-controlled ordinary load of the unchanged installed
original is the proposed rollback, conditional on an unambiguous unbound PCI
state and fresh original-file hash; this is **not** a recovery guarantee.
A partial unload, failed candidate probe, or any possible scene fire is
HOLD: no automatic original-module reload, retry, or speculative reset.

## Root read-only inventory and remaining runtime guards

The [raw capture and result](../hardware-evidence/MINI12-20260930-TRANSITION-INVENTORY/RESULT.md)
showed runit PID 1, the active `slimski` service link and script, Xorg as the
only root-visible graphics-device FD holder, and successful root execution
through the operator-supplied sudo route. No graphics service or module was
changed. The first passwordless sudo refusal remains in the record; a later
credential-backed invocation of the **same read-only script** succeeded.
The root inventory cannot see future holders or kernel-internal fbcon
references. They are checked as far as observable at stage 0/1; refusal to
unload leaves the original intact and stops the procedure.

The runit [`sv(8)` stop/down contract](https://manpages.debian.org/bullseye/runit/sv.8.en.html)
supports the selected supervisor control: `stop` requests `down`, waits up
to seven seconds, and does not restart the service; `start` requests `up`.
The [`modprobe(8)` removal contract](https://man7.org/linux/man-pages/man8/modprobe.8.html)
allows unused dependencies to be removed too, which is why the reviewed
plan demands a `-n -v -r` preview with only `gma500_gfx` before the actual
nonforced removal.
The exact target `sv` binary was not invoked, so this is a reviewed control
plan with a live status check, not a claim of tested shutdown. A future
operator must newly authorize the active sequence, verify presence and log
capture, and obey every stop/HOLD guard. **The active transition was not
performed by this review.**
