# Fresh STOCK preflight after Attempt05

Read-only capture completed after the operator manually returned the Mini 12 to STOCK and confirmed normal display/userspace and local `sudo -v` success.

## Result

**PREFLIGHT: PASS (36/36 guards)**

- Target: Dell Inspiron 1210, i686, kernel `5.10.240-antix.1-486-smp`.
- Current boot ID: `d78d349e-daac-43aa-b7f9-156485506ca5`; it differs from Attempt05 boot `203a5b9b-5a90-4fd3-8001-3c92cdefeddd`. The ID was unchanged during capture.
- STOCK command line and taint value match the retained baseline (`12289`).
- PCI `0000:00:02.0` is the expected Intel 8086:8108 / Dell 1028:02b1 device, IRQ 16, bound to `gma500`; the active module is `gma500_gfx`, initstate `live`.
- The loaded module Build ID is `d8dcb4d38b774ad64799d5e13aaedede069371f3`, matching the retained original module. This confirms the Attempt05 diagnostic module is not the active module; no module was changed by this capture.
- DRM card0 points to the expected PCI device. The `gma500drmfb` framebuffer is 1280x800. VT bindings, IRQ handler, slimski and Xorg match the STOCK baseline. The DRM node was stat'ed only; it was not opened.
- Kernel-health classification: `PASS WITH BOUNDED STOCK DIAGNOSTICS`; no rejected fault lines. The classifier counted the retained ACPI power-button messages and zero-valued `BL bug` diagnostic within their documented bounds.
- Operator reports the physical display is normal. The capture independently confirms the expected framebuffer and graphical services, not the physical panel itself.
- STOCK kernel, initramfs, original module, GRUB files/environment, Cycle06 image, corrected image and Cycle06 backup all match their retained sizes/hashes. The saved GRUB entry remains STOCK. `custom.cfg` still has the two existing experimental entries and no saving commands.
- The diagnostic initramfs and pending diagnostic entry are absent. No diagnostic staging occurred.
- No target writes, reboot, module operation, service operation, PCI/VT operation, DRM open, ioctl, MMIO access or SGX operation occurred during this preflight.

## Capture files and SHA-256

- `root-source.py`: `e7016084d9b76115f62ffca7fcedc414455fbd9401f02f94b751ab335ddf265d`
- `command.json`: `91483eb652a2cdc8e53e8085b1dd7c35ec75f51e87ba688aedacfaf21d213fd9`
- `stdout.json`: `52b482613afee620972949b192641fb44856ac04aa7b2109cb1a3c1041ea0ca7`
- `stderr.txt` (empty): `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`
- `exit-code.txt` (`0`): `9a271f2a916b0b6ee6cecb2426f0b3206ef074578be55d9bc94f6f3fe3ab86aa`
- `timing.json`: `4126a6f28ce6af2c9e0ec3bfd775bbe349354e9157b075169292e4aee29de069`

The raw structured capture is `stdout.json`. `root-source.py` is the exact read-only script sent over the pinned SSH connection. The health classifier embedded in it came from `tools/psb-dri-re/frozen_kernel_health.py` with SHA-256 `5abe09ed15609cc2ad409d909d4785953275656690632700c869fc5a48d3b7ee`.

## Boundary

This result establishes a clean STOCK baseline for a separately authorized diagnostic qualification stage. It does not authorize staging or booting the diagnostic image, and it does not change Gate B or the whitelist.

Attempt05 remains `HELD_AFTER_FAILURE`; failing callback remains `UNKNOWN`; TA fire remains `UNKNOWN/POSSIBLE`; TA completion remains unobserved; raster submission remains `NO`; its one-shot module remains consumed and must not be retried. No triangle was established.
