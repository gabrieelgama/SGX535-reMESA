# Cycle05: derivative observed, capture stopped, STOCK recovered

The fresh STOCK preflight passed 54 guards. The actual menu photo passed review.
The operator selected EXPERIMENTAL once and reported normal display/userspace
within the 600-second watch, with local sudo authentication ready.

One passive capture ran. It recorded experimental boot ID
`5f99efa2-21ac-481d-819d-5bf3499bfbd6` and the expected derivative loaded note
SHA-256 `96eb5049d143a3c7a6e7d672aed1651fa51db069ea9efd3484388b3401f29eaf`.
The module was Live. Expected PCI, DRM, framebuffer, VT and IRQ16 ownership,
slimski and Xorg passed. These are observed facts, not just operator reports.
The capture passed 23 guards, then stopped at kernel health. Its exit status
was 1, stderr was empty, host duration was 3.860184947028756 seconds and prefix
uptime was 153.93 seconds. The hook trace and final timing receipt were not read.
The original STOP and raw output remain unchanged. There was no retry.

## The signature notice

The only selected fault was:

```text
drm: module verification failed: signature and/or required key missing - tainting kernel
```

The exact antiX `kernel/module.c` emits this notice once per boot after a
permissive signature check. Enforced signature rejection returns an error before
that path. The captured configuration has MODULE_SIG=y, MODULE_SIG_FORCE unset
and SECURITY_LOCKDOWN_LSM unset. The derivative has no signature trailer.

The notice names the first unverified module, here the captured stock `drm`
dependency. The preflight STOCK log names `fjes`; recovered STOCK names `video`.
All three captures have taint 12289. The notice itself is an expected loader/taint
diagnostic, not a GPU fault, ABI rejection or proof of successful probe. Successful
derivative loading and ownership are supported by the separate observations above.
We do not know why the first notice names different stock modules across boots;
that distinction is not needed to classify the source-defined notice.

The guard matched `drm` plus `failed` and incorrectly called this a graphics
fault. The narrow fix recognizes only this exact `drm` loader notice, at most once,
and leaves it visible in the receipt. Changed or repeated notices and real
warnings, oops, panic and graphics faults still reject. Module identity, CRC,
taint and ownership checks are unchanged. This is not a signature-check bypass.
Offline reclassification of the preserved complete log finds no selected faults;
it does not turn the failed live capture into PASS.

## STOCK recovery

The operator crossed the reviewed machine boundary and affirmed manual normal
STOCK selection, normal display/userspace, successful sudo and arrival within the
600-second watch. The literal affirmative reply was `conceded`; no exact arrival
time is inferred from it. The single recovery capture passed 55/55 guards.
Its boot ID is `8c89c9c9-d8c5-48b8-bfad-a4afa0442257`, distinct from both the
preflight ID `22aae08e-fea5-492f-957b-5cffea6f33f4` and the experimental ID.
Capture uptime was 226.35–233.22 seconds; host duration was 9.814524943940341 seconds.
The existing automatic timing bounds passed.

The original loaded note, PCI/DRM/fb/VT/IRQ16 ownership, services and complete
kernel-health receipt matched STOCK. Stock files, staged hashes/creation receipts
and saved STOCK default matched preflight. STOCK restoration and the observed
three-boot machine boundary are established. The controller's archived ledger
has a null experimental ID because it looked only for a successful capture;
`recovery-offline-ledger-review.json` accounts for the actual partial-capture ID
without editing that ledger. The incomplete experimental capture still prevents
full first-load-cycle qualification.

## Minimum remaining blocker

Gate B stays BLOCKED; whitelist `[]`. FIRST OWNER is INFERRED from the selected
pinned image, early derivative initialization and normal boot continuation, but
NOT ESTABLISHED under the current direct-evidence contract. The missing ordered
hook trace is the remaining first-load evidence blocker. It cannot be reconstructed
from this dmesg: the hook writes to volatile `/run`, and the experimental boot has
ended. The capture also lacks its successful end receipt.

Another rebuild, image change, timing redesign, restaging or new gate is not
needed. The existing 600/1200/40/35-second bounds and source-proven guards suffice.
The shortest next action requires separate authorization for one unchanged
EXPERIMENTAL boot and the existing bounded passive capture with the corrected
health source embedded and pinned. Capture the hook trace and complete receipt.
No SGX operation, hot transition or retry. Reuse this verified STOCK recovery;
do not add a separate rehearsal merely to repeat it. Any proposal to remain in a
healthy experimental boot while requesting later SGX authorization must explicitly
cover that stopping point; the completed Cycle05 authorization grants no such new
boot or SGX permission. Failure recovery remains the reviewed machine boundary.

That observation would close the current first-load gate. A later fixed SGX
operation still needs an evidence-based Gate B decision and explicit authorization.
The current result is off-screen; physical LCD handoff remains unimplemented and
unqualified. Neither an off-screen result nor successful first loading proves a
visible SGX triangle.

## Verification and preservation

330 repository tests passed with zero skips, including the regression that first
failed on the notice and actual Cycle05 log. Fourteen boot-analysis tests, three
UBSan harnesses, generator and both candidate CRC checks passed (230/230 and
232/232, no missing/mismatch). Independent unchanged-image inspection passed.
Both dry runs retain SHA-256
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still refuses with `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

The image remains `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`;
its embedded derivative remains
`91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`.
All 145 image-qualification manifest files and all 46 Cycle04 manifest files are
unchanged. No driver, module, image, entry or staged file changed. No restaging,
hot unload/replacement, fixed ioctl, controlled SGX/PDS/TA/raster submission or
triangle occurred. Nothing was staged in Git, committed or pushed.
