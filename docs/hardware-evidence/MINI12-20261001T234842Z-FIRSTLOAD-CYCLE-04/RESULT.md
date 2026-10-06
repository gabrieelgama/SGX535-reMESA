# Cycle04: experimental HOLD, STOCK restored

Cycle04 used the prepared v2 procedure. The fresh STOCK preflight passed all 54
guards. The saved STOCK default, original driver and stock/staged files matched,
including the existing creation receipts. Nothing was restaged or overwritten.

We opened the actual menu photo from shared storage and preserved it unchanged.
Both required entries were visible. The photo highlighted `(sysvinit)`; the
operator confirmed that normal STOCK was initially selected and that they moved
the highlight manually. Neither entry had booted at that point. The saved default
also matched the fresh preflight. The menu guard passed before experimental
selection was directed.

The operator selected EXPERIMENTAL exactly once. The screen stayed at
`VMX (outside TXT) disabled by BIOS`. They recognized this persistent state from
previous boots, including outside the experiment. A short power-button press did
not shut it down. That is OPERATOR-REPORTED history, not a diagnosis. The same
VMX message appears in the successful STOCK log. We do not know the stall's cause
or whether the derivative loaded.

The cycle entered HOLD under the existing hang condition. No experimental
connection or root capture ran. There is no experimental boot ID, loaded-module
note, hook trace or first-owner capture. Waiting out the maximum watch was not
required once the operator identified the hung state. No experimental retry was
made.

The operator used the authorized long-press power-off boundary and manually
selected normal STOCK, without a suffix. They reported normal physical display
and userspace, userspace reached well within the 600-second watch, and successful
local `sudo -v`. The single recovery capture passed all 55 guards:

- boot ID `22aae08e-fea5-492f-957b-5cffea6f33f4`, distinct from preflight
  `0103242d-9a46-4d66-8a61-42b0c44e00c6`;
- original loaded note SHA-256
  `484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7`;
- original Live, expected Poulsbo PCI/DRM ownership, gma500drmfb, raw dimensions
  `1280,800`, VT0=0/VT1=1 and gma500 handler on IRQ16;
- slimski/Xorg running, unchanged stock/staged files and saved STOCK default;
- complete current-boot health receipt with no selected faults and only the
  bounded stock diagnostics.

Automatic recovery-capture uptime was 162.93–170.02 seconds. The host connection
lasted 9.927 seconds. The 1,200-second kernel-clock ceiling and 40/35-second
execution bounds passed. These timestamps establish capture timing, not an exact
userspace-arrival or HDD boot duration.

Current STOCK restoration is independently verified. Full three-boot LIVE
RECOVERY remains NOT ESTABLISHED because the experimental boot ID is missing.
LIVE FIRST LOAD and FIRST OWNER are also NOT ESTABLISHED. The fresh stock data
cannot replace the missing experimental capture.

## Gate B and the smallest next experiment

Gate B remains BLOCKED; SGX whitelist `[]`. The concrete blocker is the missing
experimental first-owner capture: boot ID, loaded derivative note, ordered hook
trace and PCI/DRM/framebuffer/IRQ ownership. No offline analysis can supply those
observations for this boot. The failed cycle does not justify changing the image,
driver or SGX semantics, and it does not prove that EXPERIMENTAL caused the stall.

The minimum experiment is ONE separately authorized non-SGX first-load/recovery
cycle using the unchanged pinned files and prepared v2 procedure. Capture the
required experimental evidence, then cross the operator machine boundary and
verify STOCK recovery. No restaging, hot replacement, automatic retry or SGX
operation. This cycle is spent; another experimental selection needs new explicit
authorization. **Next step: TARGET-STATE-CHANGING.**

No fixed ioctl, controlled SGX/PDS/TA/raster submission, hot unload/replacement
or triangle operation occurred. FIRST TRIANGLE: NOT ATTEMPTED. Physical SGX
triangle display remains unestablished.

## Verification

Before the cycle: 329 repository tests passed with zero skips; 14 boot-analysis
tests, three UBSan harnesses, generator, complete candidate CRC checks and
unchanged-image verification passed. After recovery: all 22 v2 capture/procedure
tests and both actual raw-receipt checks passed. Original and final stock state
fields were compared directly. No production driver, candidate or image changed.
The task-start historical files and source pins remain unchanged.

Both deterministic dry-run hashes remain
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` remains `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.
Nothing was staged, committed or pushed.
