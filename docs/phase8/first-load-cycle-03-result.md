# First-load cycle03: deadline STOP, no experimental capture

2026-10-01. Gate B BLOCKED; SGX whitelist `[]`. FIRST TRIANGLE: NOT ATTEMPTED.
The cycle is spent. No extra boot, capture retry, restaging or SGX action is
permitted by its authorization.

[Raw evidence](../hardware-evidence/MINI12-20261001T223542Z-FIRSTLOAD-CYCLE-03/RESULT.md)
keeps the exact commands, replies, preflight output and decisions separate from
this report.

The fresh STOCK preflight passed all 53 root guards. Machine/kernel/original
module, PCI/DRM/framebuffer/IRQ/VT, services, kernel health and stock/default
checks passed. The five stock file records matched retained recovery exactly.
The existing experimental image and entry matched their original creation
receipts, including device/inode. Nothing was restaged or overwritten.

The operator reported EXPERIMENTAL userspace and a normal physical display at
approximately 120 seconds after handoff. Local `sudo -v` had succeeded in that
boot. The procedure requires the capture to finish within 120 seconds; the
reply did not establish any time left. No experimental connection was made.
This was a deadline STOP before capture, not a sudo or SSH failure.

The operator initially said a menu photo was captured, then corrected that:
**no menu photo was captured**. Both replies are retained. The photo, experimental
boot ID, loaded derivative note, ordered hook trace, complete kernel log and
ownership capture are missing. Normal userspace/display remain operator reports.
They do not establish which module owned Poulsbo or that the hook passed.

The operator proceeded through the reviewed machine boundary and reported
“ready stock.” That reply did not supply the required elapsed time, current-boot
sudo success and all readiness fields. The stock capture also did not run.
The exact stock-recovery elapsed time is UNKNOWN; no measured late capture is
claimed. The operator later confirmed STOCK userspace and a normal physical
display. Current module/kernel/boot identity, ownership and kernel health were
not independently captured after that boot.

| Item | Result |
| --- | --- |
| Fresh STOCK preflight | PASS, 53 root guards |
| Existing staged-file receipts / stock preservation | PASS at preflight |
| EXPERIMENTAL userspace and normal display | OPERATOR-REPORTED |
| Menu photo | MISSING; explicit correction preserved |
| Experimental connection / privileged capture | NOT RUN |
| LIVE FIRST LOAD / FIRST OWNER | NOT ESTABLISHED |
| Final STOCK userspace and normal display | OPERATOR-REPORTED |
| Recovery connection / privileged capture | NOT RUN |
| Full LIVE RECOVERY | NOT ESTABLISHED; three linked boot IDs missing |
| Restaging / agent module or service operations | NONE |
| Fixed ioctl / SGX fire / triangle | NONE |
| Gate B / SGX whitelist | BLOCKED / `[]` |

The operator asked for a timing redesign for a separately authorized successor.
The [proposal](first-load-capture-timing-redesign.md) separates the boot watch
from passive capture, records capture timestamps automatically, and obtains the
menu photo before selection. Its proposed limits are unapproved. The existing
120-second plan, validators, capture wrapper and image remain unchanged.

## Offline verification

After the deadline STOP: 307 repository tests passed with zero skips; 14
boot-analysis tests, three UBSan harnesses, generator, procedure and both candidate
import/CRC checks passed. Eleven unchanged capture-wrapper/readiness tests and
six cycle-controller tests passed. Both dry runs retained SHA-256
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still exits 1 with
`PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

No candidate or image was rebuilt. The image remains
`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`;
Candidate #1 remains
`13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f`;
the lifecycle derivative remains
`91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`.
ABI qualification is still offline evidence, not loader/runtime or first-owner
proof.

The current display handoff gap was checked against source while waiting for
recovery. The fixed entry captures the retired color BO, then releases its owner
on permitted success. Its private SGX allocation is not a retained stock
`gtt_range` framebuffer. The stock display path pins that framebuffer in the GTT
and programs format/pitch/base. The client writes a diagnostic file, not scanout.
No CPU framebuffer substitute was proposed and no implementation changed.
Physical LCD output attributable to SGX is still unimplemented/unqualified.

**SINGLE SMALLEST NEXT STEP — OFFLINE:** review the successor timing proposal
before implementing and qualifying a new procedure. A new live cycle would need
separate authorization. No SGX authorization follows from this cycle.
