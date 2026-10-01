# Attempt 03 original-driver recovery — failed, HOLD

The operator explicitly authorized minimum normal recovery of the preserved
original driver and display service, with no candidate deployment, force,
reset, reboot, or fixed-scene operation.

The passive precheck passed: target kernel/revision namespace and PCI identity
matched, `slimski` and Xorg were down/absent, gma500 and card0 were absent, PCI
was unbound, vtcon0 was bound, vtcon1 bind was absent, and root-visible graphics
FD holders were absent. The installed original module's SHA-256 matched
`7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`.

The normal load preview showed exactly the original module plus its required
`i2c-algo-bit` and `drm_kms_helper` dependencies. Their installed paths,
vermagic and hashes were recorded. After rechecking the HOLD guards, exactly
one normal `/usr/sbin/modprobe gma500_gfx` was performed. It was killed with
exit 137. Recovery stopped at `normal_original_insert`; neither console
rebind nor service restart was executed.

The one subsequent read-only failure snapshot captured a new kernel oops:
`BUG: unable to handle page fault`, with EIP `strcmp+0xa/0x20`. The call trace
passes through `register_handler_proc`, `__setup_irq`, `request_threaded_irq`,
`drm_irq_install`, and `psb_pci_probe [gma500_gfx]`. This establishes failure
during original-driver IRQ setup; it does not establish the underlying stale
state/pointer cause. The complete new log is in
[recovery-kernel-log-delta.txt](recovery-kernel-log-delta.txt).

## Final observed state (2026-09-30T07:50:17Z)

- `gma500_gfx`: `Loading`, refcount 1; successful original-driver load and
  loaded Build ID verification were not reached.
- PCI `0000:00:02.0/driver`: link to `gma500`, but probe did not complete.
- `/dev/dri/card0`: absent.
- `fb0`: absent.
- `vtcon0`: bound; `vtcon1/bind`: absent.
- `slimski`: down; Xorg absent.
- SSH/read-only capture remained available; usable display was not restored.

No second load, unload, force, reset, reboot, console write, service start,
candidate load, fixed ioctl, TA/raster submission or triangle attempt occurred.
The target is now in a **partial original-module Loading HOLD state**. Further
recovery is outside this failed normal-load procedure and requires a separate
operator decision. No automatic recovery is proposed. Attempts 01–03 raw
evidence remains unchanged. Stage scripts, output/error bytes, UTC times and
hashes are retained here using the reviewed two-process credential transport.
