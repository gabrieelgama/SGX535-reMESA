# Attempt 02 residual gma500 reference

**Live resolution, 2026-09-30:** The separately authorized
[console-release test](../hardware-evidence/MINI12-20260930-CONSOLE-RELEASE-01/RESULT.md)
observed the original module's refcount move `1 → 0` after one normal
`vtcon1` unbind, then `0 → 1` after one rebind. The original PCI/module
identity stayed fixed; service restoration returned refcount to 2 and the
operator reported a normal display. This closes the residual-reference
mechanism for the tested target state. It did not remove a module or authorize
a triangle attempt.

## Reference path

Attempt 02's [retained result](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-02/RESULT.md)
shows `gma500_gfx` falling from refcount 2 to 1 after `slimski` and Xorg
stopped and root found no graphics-device FD holders. The required ordinary
removal preview then refused because the module remained in use.

The exact-version antiX `5.10.240-antix.1-486-smp-6` source contains this
specific kernel reference path:

1. `drivers/gpu/drm/gma500/framebuffer.c:167-195,387-394` gives every
   selected gma500 framebuffer-ops variant `.owner = THIS_MODULE` and assigns
   one of them to the fbdev `fb_info`.
2. `drivers/video/fbdev/core/fbcon.c:945-979` uses
   `try_module_get(info->fbops->owner)` when fbcon starts on that `fb_info`.
   `con2fb_acquire_newinfo()` at `:717-751` does likewise for a later
   framebuffer mapping. The reference persists until release.
3. `fbcon_exit()` at `:3394-3432` calls
   `module_put(info->fbops->owner)` for a mapped framebuffer. A framebuffer
   switch can release through `con2fb_release_oldinfo()` at `:753-780`.
4. The [read-only Mini 12 capture](../hardware-evidence/MINI12-20260930-GMA500-REFCOUNT-READONLY-01/RESULT.md)
   found only `fb0=gma500drmfb`, associated with PCI `0000:00:02.0`, and
   `vtcon1=(M) frame buffer device` with `bind=1`. It also found no
   dependent module in `gma500_gfx/holders`. The target's refcount was 2
   after the original display service was restored.

Thus fbcon has a source-defined module reference on gma500 while bound to
this target's sole fbdev. It accounts for the one reference left in Attempt
02 after the userspace graphics FD disappeared. The exact stopped-state VT
binding was not captured, and the source package is not a bitwise proof of
the installed module's build; the observed count and unchanged fbdev path
support, rather than directly instrument, the attribution.

## Normal nonforced release path and boundary

The antiX config has `CONFIG_VT_HW_CONSOLE_BINDING=y`.
`drivers/tty/vt/vt.c:3923-3951` exposes the VT-console `bind` sysfs
attribute. A write of `0` to the framebuffer-console binding invokes
`vt_unbind()`, then `do_unbind_con_driver()` (`:3742-3819`). Its backup
console selection and switch invoke fbcon's deinitialization; once fbcon
is unbound, `fbcon_exit()` releases the framebuffer owner's module reference.
The target has an alternate `(S) VGA+` console registered as `vtcon0`.

This is a **normal kernel control path**, not force removal or PCI unbind.
It is also an **active display/console state change**, not covered by the
Attempt 02 authorization. `store_bind()` returns the write count without
propagating `vt_unbind()`'s result, so a successful write alone would not
prove release. A separately reviewed action would have to verify the
framebuffer console's `bind` readback, module refcount and unchanged original
module/PCI identity before trusting a new removal preview. If any check
fails or the display state becomes ambiguous, it must stop without forcing
removal. Rebinding/rollback effects also need their own reviewed guards.

**Current classification:** reference mechanism identified; a normal
nonforced release path exists in source. The stage-2 transition remains
**BLOCKED** because the target effect and safe stop/rollback behavior of
that new console operation have not been reviewed or observed. Attempt 03
is not ready for full first-triangle authorization. The smallest next work
item is a separate exact-action safety review of one VT-console unbind and
its readback/rollback guards; no write was performed in this pass.

The narrow question for that future review is whether, **after separately
authorized graphical-service teardown and confirmation of the original
module, `vtcon1` binding and refcount 1**, one normal write of `0` to
`/sys/class/vtconsole/vtcon1/bind` yields `bind=0` and module refcount 0
without an ambiguous display/platform state. The proposed observation is
readback of both values and the original PCI/module identity before any
removal preview. It is a target-state-changing console operation. A failed
or contradictory readback must stop; no force unload or speculative reset
follows. Rebinding and service restoration require a reviewed conditional
path of their own. This is an experiment specification, not authorization.
