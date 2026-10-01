# Frozen triangle Attempt 03 — HOLD before ioctl

The operator authorized one execution of whitelist
`MINI12-SGX535-REV121-FROZEN-32x32-SEQ1` with the separately tested
`vtcon1` release. The operator was physically present. No credential was
stored in commands, scripts or evidence. The reviewed two-process sudo
transport and pinned SSH identity were used.

## Stage results

- Fresh unprivileged preflight exited 0 with empty stderr and matched
  Attempt 02's 28-field capture byte-for-byte (SHA-256
  `222999f8362ba55bdfe0f98692f7c775a20d0434793dcd9047063ce6740e7779`).
- Root stage 0 verified original module identity, refcount 2, bound console,
  sole Xorg DRM holder, and the existing root-staged candidate/client paths,
  ownership, modes, pinned hashes and candidate vermagic.
- One reviewed `sv stop` passed: no `slimski`, Xorg or graphics FD holders;
  original module and PCI binding unchanged; refcount 1.
- One normal `vtcon1` unbind passed: `vtcon1=0`, `vtcon0=1`, refcount 0,
  original Build ID and PCI binding unchanged.
- The nonforced removal preview exited 0 and printed only
  `rmmod gma500_gfx`.
- One nonforced `modprobe -r gma500_gfx` exited 0. Its immediate guards found
  the module absent, PCI unbound and `/dev/dri/card0` absent.
- One `insmod` of the pinned candidate returned 1 with `Invalid module
  format`. The stage entered **HOLD**. No client was invoked.

The subsequent read-only kernel log identified the candidate rejection:
`gma500_gfx: disagrees about version of symbol module_layout`. Matching
vermagic and the pinned candidate hash did not establish this symbol-version
compatibility. The same log also contained warnings during removal of the
original driver: `drm_mode_config_cleanup` reported a leaked `LVDS-1`
connector, and `ida_free` reported ID 0 not allocated. These warnings were
observed in the post-failure log, after the immediate detach guards had
passed; they are not evidence of a safe cleanup state.

## State at HOLD

At the read-only capture, `slimski` was down, `gma500_gfx` was absent,
PCI `0000:00:02.0` was unbound, `/dev/dri/card0` was absent, `vtcon0` was
bound and `vtcon1` no longer had a sysfs `bind` file. The candidate file
still had SHA-256
`934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`
and reported the expected vermagic. No normal-state restoration was
attempted: the reviewed procedure assigns a failed candidate load after
clean detach to HOLD, with no automatic original-module reload or reset.

The ioctl was **not reached**. No TA or raster fire occurred; there is no
color result or triangle observation. No module insertion retry, PCI unbind,
force operation, speculative reset or service restart followed the failed
candidate load. Attempts 01 and 02 remain unchanged. Stage scripts,
stdout/stderr and per-stage hash/time records are retained alongside this
result. Any recovery or new submission requires a separate review and
operator decision.

The later [offline module-version follow-up](../../phase8/attempt03-module-version-followup.md)
establishes that the rejected candidate used `module_layout` CRC
`0x995e9910` from the public source package's `Module.symvers`, while the
preserved installed target module expects `0xb84efb99`. It records the wider
import CRC mismatch and the missing complete target symbol table. This does
not change the captured Attempt 03 state or authorize recovery or retry.

## Separately authorized original-driver recovery

The later [original-driver recovery](../MINI12-20260930-ATTEMPT03-ORIGINAL-RECOVERY-01/RESULT.md)
confirmed the recorded HOLD state and original installed hash, then attempted
one normal original-driver load. A kernel page fault during IRQ registration
killed `modprobe` (exit 137). The latest target state is now partial
`gma500_gfx` **Loading**, with a PCI driver link but no DRM card or framebuffer;
slimski remains down. No retry, console/service restoration, candidate load or
SGX scene submission followed. Recovery evidence is separate from Attempt 03.
