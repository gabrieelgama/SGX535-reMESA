# Diagnostic first-load staging

The operator confirmed physical presence, normal STOCK display/userspace, working GRUB/power controls and local `sudo -v` success before staging.

- Fresh STOCK preflight: **PASS, 36/36 guards**. Boot ID `d78d349e-daac-43aa-b7f9-156485506ca5`; the original `gma500_gfx` was live and owned PCI/DRM/framebuffer/IRQ16.
- Stage readiness: **PASS, 18/18 guards**. New diagnostic paths were absent; `/boot` and the incoming filesystem had the required free space.
- The pinned diagnostic image and entry were copied into a new private incoming directory. The root staging script checked their hashes and current STOCK state before its first `/boot` write.
- Staging: **PASS**. The new initramfs was created exclusively, synced and read back at `/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-diagnostic-01`: 50,805,273 bytes, SHA-256 `376da01e11c3fd7079f348c0b10c1ea4d09c58dd917fb8be4dda52a7d805abae`.
- The prior two-entry `/boot/grub/custom.cfg` was preserved as `/boot/grub/custom.cfg.pre-diagnostic-01`: 2,040 bytes, SHA-256 `269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8`. The published three-entry `custom.cfg` preserves those exact bytes as its prefix and adds only the separate diagnostic entry: 3,042 bytes, SHA-256 `a2d2b6bd33868deefe64e5e1cb2cd9ecedbfd1fe0e78370af983a92a2ec655c5`. The entry is non-saving; STOCK remains the saved GRUB selection.
- Independent post-stage capture: **PASS, 38/38 guards**. STOCK kernel/initramfs, original installed module, `grub.cfg`, `grubenv`, Cycle06 and corrected images, and the Cycle06 config backup retained their pinned hashes. The original driver remained active on the same STOCK boot. Kernel-health classification was `PASS WITH BOUNDED STOCK DIAGNOSTICS`; no rejected fault line. The pending config path was absent.

No reboot, manual experimental selection, module operation, DRM open, ioctl, MMIO access or SGX action occurred during staging. Gate B remains **BLOCKED** and the whitelist remains `[]`. The diagnostic first-owner boot and STOCK recovery have not occurred.

## Raw evidence hashes

| File | SHA-256 |
| --- | --- |
| `prestage-stdout.json` | `aee04b9d6e33ee4b040be51d7725964ae6c0b502138dab3579124feb947a65ac` |
| `stage-readiness-stdout.json` | `b3a5ae9bed22e8a1a5cbc58294973ef82ad2d77a35c0a59a5ea379d5d19cdf25` |
| `stage-stdout.json` | `ee9a241b30c02cf0845a758030892c25e01c128b839e336517bff59c48d5d54f` |
| `poststage-stdout.json` | `4287eab4cfd2b877ca984b03ea2c063327f4dfcf44d2adbf701f75664793d1f9` |
| `stage-root.py` | `f54e701f32dda4694221912f3f43f97c4ad9a0c0d0904395a83e26e3a7eef049` |
| `diagnostic-experimental-root.py` | `ec3e75f0d4d6dd516a0aea9a082157ff079d79f63f10c977d2e5c1da57eccdf3` |

The stage and post-stage command files record exact SSH argv, stdout/stderr and exit status. The prepared diagnostic first-owner capture is read-only and will be used only after one manual diagnostic boot reaches userspace with local `sudo -v` success inside the existing timing bounds.

## Diagnostic boot and STOCK recovery

The operator reported that the manually selected DIAGNOSTIC boot reached normal display and userspace in about 120 seconds and that local `sudo -v` succeeded. The one bounded passive capture recorded boot ID `69847808-be83-4a28-88b1-96688f0b0d66`. Its 66/66 root guards passed. The loaded module's build-ID note matched diagnostic Build ID `594030ac153ce3c6c7dec748025142c92d90dd0c`. The volatile hook log contains exactly one ordered `BEGIN`, `FILES-VERIFIED; INSERTION-POSSIBLE`, and `PASS: derivative first owner; boot may continue`. PCI, DRM, framebuffer and IRQ 16 ownership, services and kernel health passed. Capture uptime was 643.28–655.58 seconds, within the 1,200-second capture ceiling. The operator's boot-watch report and automatic capture clock measure different events.

The operator then crossed the reviewed machine boundary and manually selected normal STOCK once. They reported normal display/userspace, successful local `sudo -v`, and about 120 seconds to userspace. The one read-only recovery capture recorded a new boot ID, `bce184cb-f5d1-40a2-8591-bd800f4679cc`, distinct from the preceding STOCK and DIAGNOSTIC boots. Its 39/39 root guards passed: original module Build ID `d8dcb4d38b774ad64799d5e13aaedede069371f3`, original PCI/DRM/framebuffer/IRQ ownership, running display services, accepted kernel health, unchanged STOCK and staged file hashes, saved STOCK GRUB default, and no diagnostic initramfs hook log on this STOCK boot.

The recovery wrapper wrote `STOP: capture kernel drift` even though the root capture completed with exit status 0 and empty stderr. This is a receipt-schema mismatch: the unprivileged record uses `kernel`, while this recovery root script uses `kernel_release`. Both captured values are `5.10.240-antix.1-486-smp`. The raw output and STOP verdict remain unchanged. `stock-recovery/offline-adjudication.json` records an offline check that aliases only that field in an in-memory copy and then passes the existing receipt validator. It does not represent a second target capture. The raw root guards and automatic same-boot timing remain the primary recovery evidence.

Diagnostic first-owner observation and STOCK recovery are supported by this cycle's evidence. The machine is now on STOCK, so Gate B for a live diagnostic workload is **BLOCKED** and the current whitelist is `[]`. No SGX ioctl, TA/raster submission or diagnostic workload ran in this cycle. Attempt05 remains `HELD_AFTER_FAILURE`; its consumed module was not retried. Another DIAGNOSTIC boot is outside this cycle's one-boot authorization.

| Additional evidence | SHA-256 |
| --- | --- |
| `diagnostic-experimental/stdout.txt` | `ca8666a4a1313c3c890175392bd32e16f6e53092e61d5fc4f6cc5340ab796bfb` |
| `diagnostic-experimental/decoded-records.json` | `6fd9d69225998f4604539c11ed304b558bc400020d29156f80508c3d8517e90d` |
| `stock-recovery/stdout.txt` | `5fe7efc5374be6ad7013764afaae83b835d8842fe53cca0ac2cf6061a8fc2964` |
