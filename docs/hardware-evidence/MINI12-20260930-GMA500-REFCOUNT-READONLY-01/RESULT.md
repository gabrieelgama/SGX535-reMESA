# gma500 residual reference: read-only target observation

Scope: pinned SSH reads of `/proc`, `/sys`, VT-console metadata and PCI
symlinks. No display-service stop, framebuffer/DRM open, sysfs write,
module operation, MMIO or SGX submission occurred.

The target was still running `5.10.240-antix.1-486-smp` with the reviewed
original gma500 Build ID. `/proc/modules` and
`/sys/module/gma500_gfx/refcnt` both reported **2** with the display service
running. `/proc/fb` contained only `0 gma500drmfb`; `fb0` and `card0` both
resolved to PCI `0000:00:02.0`. `vtcon1` identified itself as `(M) frame
buffer device` and reported `bind=1`; `vtcon0` was `(S) VGA+`, `bind=0`.
`/sys/module/gma500_gfx/holders` contained no dependent module.

These observations establish that the framebuffer console is bound to the
only registered framebuffer, owned by gma500. The exact-version antiX source
shows that fbcon takes a reference on `info->fbops->owner` while using a
framebuffer, and every gma500 framebuffer-ops variant sets that owner to
`THIS_MODULE`. The source path and the target binding account for one
gma500 reference. Attempt 02 separately observed refcount **1** after Xorg
and all root-visible graphics file holders disappeared, consistent with
that remaining fbcon reference. The VT binding was not sampled during the
stopped-service interval, so this last attribution is a source-supported
inference across the two observations, not a direct stack trace of a live
module reference.

The source also defines a normal VT-console unbind control, but no write to
it was authorized or performed. See
[the bounded source analysis](../../phase8/gma500-fbcon-refcount.md).

Raw output, exact SSH command, UTC interval and hashes are in this directory.
