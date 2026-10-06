# Corrected first-load cycle: first owner verified

The fresh STOCK preflight passed 57/57 guards. Its boot ID was `10d6abfa-e7a8-4311-97ae-213f503cee20`; the original driver and stock display path matched the retained baseline. The corrected image was staged at a distinct path. Cycle06's image remains unchanged, and its original GRUB entry was saved byte-for-byte before adding the corrected non-saving entry. The saved STOCK default and stock kernel/initramfs remained unchanged.

The operator reported that the corrected EXPERIMENTAL boot reached normal userspace quickly, with normal display/userspace and successful local `sudo -v`. The bounded passive capture passed 62/62 guards in boot `203a5b9b-5a90-4fd3-8001-3c92cdefeddd`. It captured the exact ordered hook trace, corrected module note hash `b78dc1ffde5cc661d076941e27e10b06edf62ad1da37651f3c38a98e622890c2`, expected PCI/DRM/framebuffer ownership and IRQ 16 handler. Kernel health passed with no selected faults.

The capture establishes first ownership for the corrected derivative under the reviewed first-load contract. The menu photo was not provided; the operator's boot report is retained as a note. No SGX ioctl or workload ran.

Gate B is PASS for readiness of the exact off-screen action `MINI12-SGX535-REV121-FROZEN-32x32-SEQ1`, pinned to this boot and corrected module. Whitelist contains that action. SGX execution still requires a separate explicit authorization. The frozen action remains sequence 1, one scene, at most one TA and raster fire, 5×HZ / 300,000 sample bounds, HOLD on fault/timeout/ambiguity, and no retry.

FIRST TRIANGLE: NOT ATTEMPTED. Physical LCD handoff remains outside this authorization.
