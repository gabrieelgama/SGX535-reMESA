# Cycle04 preparation: separate boot and capture clocks

Preparation snapshot. Cycle04 subsequently ran and entered HOLD; see the
[current result](first-load-cycle-04-result.md). The preparation facts below are
historical.

2026-10-01. Preparation is authorized; **Cycle04 has not booted**. Gate B stays
BLOCKED; SGX whitelist `[]`. No fixed ioctl or triangle is authorized.

[Cycle03](first-load-cycle-03-result.md) is spent. Its experimental and recovery
captures never ran. No menu photo was captured. We are not extending its deadline
or supplying later stock data as missing experimental evidence.

## Fresh STOCK observation

[Preparation evidence](../hardware-evidence/MINI12-20261001T232417Z-CYCLE04-STOCK-PREPARATION/README.md)
records one authorized read-only connection. All 54 root guards passed: expected
Mini 12/i686/kernel, original loaded note, PCI/DRM/framebuffer/IRQ16/VT, slimski
and Xorg, complete kernel health, stock files/default and existing experimental
file identities. Creation device/inode receipts matched. No target file, service,
module, PCI, VT or boot setting was changed.

The running stock boot ID was `0103242d-9a46-4d66-8a61-42b0c44e00c6`.
The original loaded note SHA-256 was
`484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7`.
The kernel was `5.10.240-antix.1-486-smp`, architecture `i686`.
Automatic capture uptime went from 1,430.18 to 1,436.84 seconds. The host
connection took 9.757 seconds, within its 40-second bound. Kernel health passed
with the existing bounded stock diagnostics and no selected faults.

Process-start records were 0.09 seconds for PID 1, 31.55 for the observed slimski
process and 43.40 for Xorg. They are process launch times in this boot, not login,
usable display or completed boot times. The operator reports that HDD stock boot
can exceed 120 seconds. That remains OPERATOR-REPORTED; this inspection neither
confirms nor contradicts it. We did not reboot to measure it.

The observed time namespace had zero monotonic/boottime offsets. UTC is recorded
for audit, not compared between machines to establish a deadline. A later boot
must provide its own clock and identity evidence.

This fresh observation closes the lack of an independently captured **current
stock state** at the end of Cycle03. It does not close LIVE FIRST OWNER or full
three-boot LIVE RECOVERY: Cycle03's experimental ID/note/trace are still missing.

## Successor timing

The [v2 plan](cycle04-first-load-procedure.json) and
[wrapper/controller](../../tools/psb-dri-re/frozen_first_load_capture_v2.py)
are separate from v1. The [v2 evidence validator](../../tools/psb-dri-re/frozen_first_load_procedure_v2.py)
reuses v1's unchanged stock/module/health identities. It does not inject a fake
elapsed time into the old validator.

| Limit | Meaning |
| --- | --- |
| 600 seconds | Operator boot watch from kernel handoff; a hang/fault/HOLD stops sooner |
| 1,200 seconds | Latest completed cycle capture on the target's kernel boot-time clock |
| 40 seconds | Maximum duration of the single host connection/capture |
| 35 seconds | Maximum root child/target capture duration; reduced by remaining cycle budget |

These are prospective operating bounds for separate authorization. They are not
measured safe GPU timeouts. The earlier 180-second proposal was not accepted or
executed. Given the operator's slow-HDD context, v2 leaves a ten-minute boot
watch and a further ten minutes before the absolute kernel-clock capture ceiling.
That avoids requiring login, local sudo, a context switch and communication in
the same 120-second interval. Longer exposure of the experimental kernel is a
new review/authorization condition, not an architectural fact.

The boot watch remains an operator observation for hang handling. It is not
silently reconstructed from a process-start timestamp. The operator need not
type an exact elapsed number to race the capture. Record whether normal userspace
appeared before the boot-watch limit, with any approximate time labelled as such.
If that cannot be established, boot-watch qualification remains NOT ESTABLISHED.

Capture timing is automatic. The wrapper prints unprivileged boot ID, kernel,
architecture, uptime and UTC before attempting `sudo -n`. It reads the same boot
ID and uptime at completion and measures its own monotonic duration. The host
measures connection duration independently. Uptime/monotonic disagreement over
one second rejects; the allowance covers sample/quantization overhead, not a
permission to ignore suspension or unexplained clock changes. The target duration must also fit inside the enclosing host interval; no extra
tolerance is allowed for that check. Negative/nonfinite clocks, changed boot IDs,
late completion, partial records and stderr reject.

Normal STOCK preparation has a distinct `stock_preparation` mode with no boot-age
limit: an already-running stock machine is not a new boot experiment. That mode
still has the 40/35-second execution bounds and cannot qualify first ownership
or recovery. Experimental and stock-recovery modes enforce the 1,200-second
kernel-clock limit. The v2 record gates reject preparation receipts in their place.

V1, its 120-second constants and all Cycle03 files stay unchanged. V2 needs a new
explicit cycle authorization. The image and module need no rebuild for this
capture-only change.

## Exact next live procedure, not executed here

1. Obtain new authorization naming these bounds and exactly one non-SGX
   experimental/stock cycle. Confirm physical presence, normal stock display,
   visible GRUB/manual power controls and data-loss risk. No saved-default change.
2. Run one fresh STOCK preflight using the reviewed read-only root program and
   v2 preparation wrapper. Pin its source/hash and retain output. Require all
   stock guards, preserved stock files/default and exact existing creation
   receipts. A mismatch or privilege failure is STOP before boot. Do not restage.
3. Save work. The operator uses the reviewed local machine boundary to GRUB.
   **Before selection**, hold at the visible menu, photograph both exact titles
   with STOCK still default, and actually supply the photo for review. No blind
   selection or entry edit. Missing photo/menu/default means select STOCK and stop;
   do not attempt another experimental selection under that authorization.
4. Manually select `EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)` once.
   Watch boot for at most 600 seconds from handoff. A hook HOLD, fault, unusable
   display or hang stops the path immediately; use only the authorized recovery
   boundary. Do not wait for the capture deadline to act on a fault.
5. At normal userspace, run `sudo -v` locally. Send one short reply confirming
   normal physical display/userspace, successful local sudo in this boot and
   userspace reached before the boot-watch limit. No password, exact elapsed
   typing or graphics commands. The actual `sudo -n` result must still pass.
6. Make one passive capture, with an exclusively created evidence directory
   before connecting. No retry. It must finish within the automatic bounds.
   Bind the root script to this cycle's fresh prior stock ID, not an old ID.
   Require selected image/file receipts, exact ordered trace, loaded derivative
   note, new boot ID, PCI/DRM/framebuffer/IRQ16, services, complete kernel-health
   receipt and physical display evidence. Supplied-record PASS is not raw provenance.
7. Preserve evidence. No triangle, fixed ioctl, SGX request/MMIO experiment,
   unload, PCI/VT control, original insertion or hot recovery. Use the reviewed
   operator machine boundary, then manually select only STOCK. Preserve selection
   evidence; a reset is not proof of recovery.
8. Apply the same boot watch and current-boot local sudo/readiness steps. Make
   one automatic bounded stock capture. Require original loaded identity,
   complete stock ownership/services/health, stock bytes/default and normal
   physical display. All three linked boot IDs must exist and be distinct for
   full recovery PASS. Missing experimental data stays missing even if stock
   itself verifies. No second experimental boot or recovery loop.
9. End at stock, record actual results and reevaluate Gate B. A perfect first-load
   cycle still grants no SGX execution or display experiment.

The unchanged existing image is 50,804,481 bytes, SHA-256
`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`, at
`/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01`.
The existing `/boot/grub/custom.cfg` is 974 bytes, SHA-256
`181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612`.
The embedded lifecycle derivative remains
`91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`.
The stock title remains `antiX-26 Stephen Kapos, 5.10.240-antix.1-486-smp`.
Stock files/default and defective hot-removal avoidance are mandatory throughout.

## Qualification and boundary

The actual wrapper is tested with positive and negative clocks, sudo refusal,
partial timeout, changed identity, missing readiness and no credential stdin.
The actual single-attempt controller refuses duplicate directories before another
connection. V2 record tests retain photo/trace/service/ownership/health/default,
one-boot and three-boot requirements. None is a live first-owner claim.

The STOCK connection exercised the initially qualified v2 wrapper. Later offline
checks added stricter clock/identity receipt checks and the reusable controller.
The actual raw STOCK data also passes those stricter receipt checks. The final
successor code has not been run in an experimental boot; do not claim otherwise.
The final code, source pins and checks must be recorded before its future use.

**SINGLE SMALLEST NEXT STEP — TARGET-STATE-CHANGING:** explicitly authorize ONE
Cycle04 non-SGX experimental/stock recovery cycle under this procedure and its
600/1,200/40/35-second bounds, after reviewing the offline qualification. No
restaging, overwrite, hot transition, retries, SGX or triangle. Preparation
permission does not authorize that boot.
