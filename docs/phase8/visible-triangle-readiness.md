# Visible triangle: current boundary

2026-10-01. Gate B BLOCKED; SGX whitelist `[]`. FIRST TRIANGLE: NOT ATTEMPTED.
No target contact, boot, module operation, DRM open or SGX action occurred in
this review. The starting tree was clean at `f856511`.

The goal now includes physical LCD output attributable to the SGX535 workload.
The existing 32×32 off-screen experiment remains a useful prerequisite. It does
not satisfy that goal by itself. No CPU-drawn substitute, software rasterizer,
pre-generated image or screenshot-only result counts.

## What we actually have

The [cycle02 result](first-load-cycle-02-result.md) is the latest live record.
The operator selected EXPERIMENTAL once and reported normal userspace. The
privileged experimental script never ran: noninteractive sudo refused it.
There is no experimental boot ID, loaded-module note, hook trace, ownership
capture or complete kernel log from that boot. First ownership is NOT
ESTABLISHED. That authorization is spent.

The subsequent stock capture verified the original driver, PCI/DRM/framebuffer,
IRQ, VT, services, staged files and stock/default state. The operator confirmed
a normal display. This is CONFIRMED for that retained observation, not a fresh
assertion about the machine now. Full LIVE RECOVERY is NOT ESTABLISHED because
the experimental boot ID is missing. Independent deadline corroboration also
remains UNKNOWN; the conflicting service/operator timing records are preserved.

Both candidates remain ABI-qualified offline. Candidate #1 covers 230/230
versioned imports; the lifecycle derivative covers 232/232. Neither has missing
imports or mismatched CRCs. Both retain `module_layout = 0xb84efb99` and their
recorded byte-identical repeat builds. No candidate was rebuilt.

The currently reviewed entry captures the completed color BO, then releases
ownership on the permitted success path. The client writes the returned 4,096
bytes to a new diagnostic file. Neither path hands the frozen render target to
KMS scanout. Stock KMS support in the derivative does not fill that gap.
See [the entry](../../kernel/sgx535_frozen/gma500_fixed_entry.c) and
[the client](../../tools/psb-dri-re/frozen_triangle_one_shot.c).

## One source-analysis correction

The earlier [handoff analysis](artifacts/first-load-operational-review-20261001/hook-log-handoff-evidence.json)
listed only `/sys` and `/proc` as moves performed by stock `/init`. That list
was incomplete. The actual stock archive contains this unconditional command
at line 281:

```sh
mount -n -o move /run ${rootmnt}/run
```

It follows init-bottom and precedes the final `run-init`. The experimental
image preserves it. `/run/initramfs` is created before the experimental hook,
which runs before init-top/udev. The separately extracted stock `/init` is
byte-exact with the archive member, SHA-256
`0a9bb34973c78987922b57f010e99c36ddf102e8a5a2fd198385566539f5d6d5`.

This ordering is SOURCE-PROVEN. It supports the intended log handoff; it does
not establish that the mount succeeded or that later userspace preserved the
log in an experimental boot. Live retrieval remains UNKNOWN and mandatory.
The earlier evidence file and reports were not rewritten.

Three new [source tests](../../tools/psb-dri-re/test_frozen_first_load_trace_handoff.py)
check the actual stock source, its preserved experimental transformation and
the command ordering. In-memory controls remove, duplicate or move the `/run`
handoff too early; all are rejected. These tests do not execute mounts or a boot.

## Smallest new authorization required

Authorize ONE fresh **non-SGX first-load/recovery cycle**, using the already
staged pinned files. This is a successor to cycle02, not a retry inside it.
The [reviewed procedure](first-load-staging-boot-recovery-procedure.md) still
sets the hardware and recovery bounds, with these existing-file and capture
requirements:

1. Confirm physical presence, a normal stock display, visible GRUB selection,
   manual power controls and the data-loss contingency. Use a fresh read-only
   stock preflight. Recheck the original loaded note, kernel/machine, ownership,
   services, typed kernel-health receipt, stock hashes and saved STOCK default.
2. Revalidate the already-created image and `/boot/grub/custom.cfg` against
   cycle02's creation receipts: path, bytes, SHA-256, device/inode, regular-file
   type, single link, ownership and permissions. Use
   `validate_existing_stage_files`; same bytes in a different inode must STOP.
   Do not rerun the old absent-destination staging procedure, overwrite or
   restage anything. A mismatch is STOP before experimental selection.
3. Photograph the menu with STOCK still the default and both exact titles
   visible. Manually select `EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)`
   once. No default change, blind selection, entry edit or automatic retry.
4. Before the single passive capture, require an explicit statement that
   normal userspace was reached, **local `sudo -v` succeeded in this boot**, and
   the handoff clock leaves capture time. `require_ready` checks that statement;
   the actual `sudo -n` result must still pass. A local witness does not prove
   that authorization will work from SSH. No password is transmitted.
5. Use the qualified capture wrapper in the same connection: preserve the
   unprivileged boot ID/kernel/architecture/uptime before privileged capture,
   keep stdin at `/dev/null`, and bound the capture by the remaining time.
   Capture must finish within 120 seconds of handoff, with same-boot uptime
   confirmation and the operator's completion measurement. No failed-capture
   retry or deadline extension.
6. Require the exact ordered hook trace, derivative loaded note, Live module,
   expected PCI/DRM/framebuffer/IRQ16 ownership, complete kernel log and health
   receipt, normal display/userspace, selection evidence and new boot ID.
   Fresh preflight identity supplies `prior_boot_id`; do not reuse cycle02's
   hardcoded boot ID. Pin the reviewed capture program before it is used.
7. Preserve evidence, then use the reviewed operator-controlled machine
   boundary and manually select only STOCK. Within its separate 120-second
   window, capture the original identity, full stock ownership/services/health,
   preserved files/default and physical display. Require all three boot IDs
   to be present and distinct before calling full LIVE RECOVERY PASS.

The pinned boot image remains at
`/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01`:
50,804,481 bytes, SHA-256
`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`.
The entry remains 974 bytes, SHA-256
`181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612`.
The embedded derivative remains
`91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74`.
These are freshly verified local artifact identities, not fresh target readbacks.

Guard failure, fault, wrong identity, missing trace, timeout or contradictory
ownership means STOP/HOLD under the existing procedure. No hot unload, PCI
unbind, original insertion, speculative reset or second experimental boot.
Only the explicitly authorized operator recovery boundary may end the
experimental kernel lifetime. A stock-recovery mismatch stops the cycle.

This request includes no fixed client/ioctl, PDS request, TA/raster fire,
experimental SGX MMIO or triangle. A successful first-load cycle must end with
stock recovery and an evidence-based Gate B review. It grants no SGX permission.
No live step in this list has been executed by this review.

## From that boundary to the LCD

The shortest defensible order is:

1. Collect the missing first-owner and three-boot recovery evidence under a new
   non-SGX authorization. Failed cycle02 evidence cannot be recovered offline.
2. Finish an offline review of the fixed SGX result's display handoff. It must
   specify actual render-target/scanout addresses, format/pitch, ownership,
   completion, display synchronization and lifetime. The current success-path
   BO release cannot be treated as a retained scanout allocation. Any code or
   artifact change needs its own complete qualification.
3. Reevaluate Gate B for the exact combined SGX/display action, then obtain
   explicit authorization for that action. Existing off-screen assumptions and
   historical acceptances do not authorize new display programming.
4. Attribute completion and the resulting pixels to the frozen SGX workload,
   establish their passage through the qualified display path, and capture the
   physically present operator's LCD observation and relevant evidence. No CPU
   framebuffer drawing may substitute for this chain. Stop on ambiguity; there
   is no automatic retry.

Display handoff is not implemented/qualified for this frozen result. It is not
an irreducible technical blocker; it follows the present first-load evidence
and authorization boundary. No display addresses or packets were guessed here.

## Fresh offline verification

[New evidence](artifacts/visible-triangle-readiness-20261001T222342Z/README.md)
records 307 repository tests with zero skips, 14 boot-analysis tests, six actual
cycle02 receipt tests and three UBSan harnesses. The generator and complete
candidate import/CRC checks passed. Independent inspection passed for the
unchanged pinned experimental image. Both dry runs retained SHA-256
`2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`.
`--complete` still exits 1 with
`PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

Those architectural evidence boundaries and recorded scoped risk acceptances
remain unchanged. Tests establish offline obligations only. They do not supply
missing experimental observations, authorize SGX or prove a visible triangle.

**SINGLE SMALLEST NEXT STEP: new explicit authorization for the ONE non-SGX
first-load/recovery cycle above. TARGET-STATE-CHANGING.** Gate B remains BLOCKED;
SGX whitelist `[]`; no triangle was attempted.
