# First-load cycle01 — STOP before staging

2026-10-01 UTC. **No experimental boot. No target file staging or graphics/driver
mutation. No SGX action. FIRST TRIANGLE: NOT ATTEMPTED. Gate B BLOCKED; whitelist `[]`.**

The operator explicitly authorized one non-SGX first-load cycle and confirmed
physical presence, normal display, local GRUB/input/power controls and understood
network/display/data-loss contingencies. The procedure nevertheless requires every
fresh guard to pass. This cycle stopped at a mandatory kernel-health guard.

[Raw evidence](../hardware-evidence/MINI12-20261001T055237Z-FIRSTLOAD-CYCLE-01/README.md).

## Contact and earliest failure

1. Root preflight01 at05:54:59–05:55:00 UTC: noninteractive sudo unavailable;
   exit1, empty stdout, `sudo: a password is required`. Script never ran.
   Failure is preserved. No credential was sent or recorded.
2. The operator ran local sudo validation and replied ready. A separate immutable
   preflight02 ran05:58:09–05:58:11 UTC through the same pinned SSH host/key/argv;
   root read-only script exited1 intentionally, with empty stderr and JSON evidence.
3. 23 machine/command guards passed before **current-boot kernel health** refused.
   No target contact followed this failure. No staging, reboot or guard bypass.

**Exact matches:**

```
[   13.421480] ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)
[   13.704693] ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)
[   19.356593] gma500 0000:00:02.0: BL bug: Reg 00000000 save 00000000
```

## Offline diagnosis — evidence, not a risk waiver

The fresh complete dmesg, after trimming only outer whitespace, is byte-identical
to the retained stock boot-provenance capture02 dmesg. Same boot ID:
`8ae37532-19d4-4ec7-9c29-a791acfd889f`. Both stripped logs hash to
`2d1c9401ec6a5c73d639ddb4bc77d791f3798c1b92276eee98587ff19f37de0c`.
All three matcher hits are preexisting; no new matcher-hit lines appeared.
This is **not evidence of a newly occurring SGX fault**.

The actual reviewed `bound_identity` health gate uses the case-insensitive pattern
`WARNING:|BUG:|Oops:|Kernel panic|general protection fault|Call Trace:`.
It rejects the captured known-good stock baseline as well as the fresh identical
log. In particular, substring `BUG:` matches the stock `BL bug:` text. The retained
exact gma500 source emits that text with dev_err in
psb_intel_lvds_get_max_backlight when its computed maximum is zero; it is not a
kernel BUG() invocation. ACPI PowerButton warnings are also genuinely present in
the historical stock log. Their general safety implications are not newly proved
or silently accepted here.

This exposes an **OPEN-OFFLINE acceptance-guard inconsistency**. It does not justify
ignoring warnings or changing health policy during live execution. The guard
remains unchanged, the authorization was not expanded, and no experimental boot
was selected. Six offline tests reproduce refusal of real captured baseline bytes,
verify exact raw hashes/same boot, and retain genuine fault negative controls.

## Result matrix

| Item | Result / evidence |
| --- | --- |
| Fresh preflight | STOP; root01 authentication, then root02 kernel-health refusal |
| Machine/kernel | PASS observed: Inspiron1210, i686,5.10.240-antix.1-486-smp; stock cmdline; same boot; taint12289 |
| Source image | PASS local:50804481 bytes;4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71 |
| Source GRUB text | PASS local:974 bytes;181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612 |
| Original loaded identity | PASS observed: exact original Build-ID-note hash; module Live; not a derivative |
| Current stock ownership | PASS observed: PCI0000:00:02.0 gma500/gma500_gfx; DRM card0; fb gma500drmfb1280x800; IRQ16 gma500; VT0=0/VT1=1 |
| Current services/display/SSH | slimski/Xorg running; physical display normal by operator; successful SSH read-only capture |
| Current kernel-health acceptance | BLOCKED: exact known-baseline matcher hits; no policy waiver |
| Target stock file hashes/GRUB/free space/destination absence | NOT REACHED; retained identities remain historical, not fresh verification |
| Staging/destination read-back | NOT PERFORMED; no experimental files created by this task |
| Experimental selection/image consumed/hook/guards/load | NOT REACHED; experimental boot count0 |
| Derivative first owner/PCI/DRM/IRQ/display/userspace/SSH/health | UNKNOWN live; no experimental boot |
| Boot observation deadline | NOT APPLICABLE; no kernel handoff occurred |
| HOLD | No possible-probe HOLD; STOP BEFORE STAGING, existing stock owner retained |
| Fixed client/ioctl/PDS/TA/raster/SGX fire | NONE; not authorized |
| Hot unload/replacement/PCI/VT/service mutation | NONE |
| Reset/reboot/stock selection/recovery boot | NOT PERFORMED; unnecessary because no experimental mutation occurred |
| LIVE FIRST LOAD / FIRST OWNER / LIVE RECOVERY | UNKNOWN / UNKNOWN / UNKNOWN |
| SGX Gate B/whitelist | BLOCKED / `[]`; no new execution permission |

The target's last observed state is the original stock driver with normal display
and services. No rollback/reset/load was attempted or needed. This is a current
stock observation, **not** successful reset-to-stock experimental recovery.

## Verification and preservation

266 repository tests, zero skips;14 boot-analysis tests;three strict UBSan harnesses;
generator and both candidate CRC checks PASS. Two deterministic dry outputs remain
2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e.
`--complete` retains expected exit1/PARTIAL:L12,FT-AUX,FT-BO,FT-SERVICE.
Six capture-specific tests PASS. Source images/candidates/prior manifests unchanged;
no rebuilding. Final whitespace/link/manifest/preservation results accompany raw
captures. Earlier preflight failures and all historical evidence remain preserved.

## Single smallest next step

**OFFLINE:** qualify a precise kernel-health discriminator against the actual
retained stock baseline and genuine kernel fault controls, reconciling the
procedure/validator mismatch before any new fresh live preflight. No blanket
warning suppression, new hardware-risk acceptance or live retry is authorized by
this report. Do not stage or boot until a corrected reviewed guard is established.
