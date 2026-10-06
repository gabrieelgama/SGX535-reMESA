# First-load capture timing: proposed successor

2026-10-01. **DRAFT FOR REVIEW. Not an executable procedure or authorization.**
Gate B remains BLOCKED; SGX whitelist `[]`. The current 120-second guards stay
unchanged. No image, module, controller or policy file was changed for this
proposal. [Cycle03](first-load-cycle-03-result.md) is spent; this proposal cannot
be applied retrospectively to it.

The operator asked us to remove the rush from the next separately authorized
cycle without bypassing evidence requirements. The goal is still one non-SGX
first-load boot, complete first-owner evidence, and one stock recovery boot.
No triangle, fixed ioctl, hot transition or retry belongs in this procedure.

## What failed

The existing limit covers both boot and completed capture. Cycle03 reached
experimental userspace at approximately 120 seconds, according to the operator.
That left no established capture budget. The operator then described the manual
boot, userspace check, local sudo, context switch and reply as too much to fit
reliably into that window. The stock reply did not establish the remaining time
or all readiness fields. Neither boot capture ran. The menu photo was also
missing; that is a separate evidence gap, not a timing result.

The guard behaved as specified. `require_ready` refuses elapsed time at or above
120, missing time, invalid numbers and missing current-boot sudo/userspace
statements. The wrapper independently rejects an expired kernel clock and late
completion. Its tests passed without changing those rules.

## Recommended next design

Separate watching the boot from collecting its evidence. Proposed limits for
review are **180 seconds to reach normal userspace** and **600 seconds from
kernel boot to finish the one passive capture**, for each boot. Each SSH/capture
operation still has a **40-second maximum** and no retry. These are proposed
operational limits, not established hardware-safe times.

The observed experimental arrival was approximately 120 seconds; the earlier
stock arrival was reported as 90 seconds. A 180-second boot watch adds a limited
60-second margin to the old ceiling. A 600-second capture limit leaves up to
seven minutes after that boot-watch limit for local authentication and reporting.
It is a usability budget, not a GPU timeout or proof that a longer-running
experimental kernel is safe. Approval would explicitly accept the longer
observation exposure. If those limits are rejected, do not silently substitute
other numbers.

A fault, contradictory ownership, hook HOLD or unusable display still stops the
experiment immediately. The larger evidence budget never permits waiting out a
fault, driver manipulation, a second boot or another capture. If normal userspace
is not reached by the boot-watch limit, use only the separately authorized
operator machine boundary. Do not begin a capture after either deadline.

Before any experimental selection:

1. Complete the fresh stock preflight and existing-file receipt checks.
2. Hold at the visible GRUB menu. Capture and actually supply the photo showing
   both exact titles and STOCK as default. Review it before selection; no kernel
   clock is running yet. If the menu cannot be held or photographed, stop.
3. Supply one short readiness form with named fields, rather than several long
   questions whose text may be truncated. Record exactly which confirmations
   are supplied. “Ready” must never manufacture missing fields.
4. Only then manually select EXPERIMENTAL once. Keep the boot-watch clock for
   the operator's hang decision. No automatic boot selection or reset.

After normal userspace appears, the operator confirms a normal display and runs
`sudo -v` locally in that boot. The operator then sends one current-boot readiness
reply. No password is transmitted. The actual `sudo -n` result must still pass;
a local sudo witness does not prove SSH privilege will work. If it refuses,
preserve the unprivileged identity prefix and stop. Do not retry authentication
or capture inside that boot.

Use automatic records for capture timing: boot ID, kernel, architecture and
`/proc/uptime` before privilege; the same boot ID and uptime at completion; and
host monotonic elapsed time around the one connection. The child timeout is the
smaller of 40 seconds and the target's remaining capture budget. The outer host
connection also has a fixed 40-second ceiling. No polling/reconnect loop.

`/proc/uptime` is a kernel boot-time clock. In the retained exact antiX source,
`fs/proc/uptime.c` obtains `ktime_get_boottime_ts64`, applies the time-namespace
adjustment and emits hundredths of a second. It is not the operator's GRUB
handoff stopwatch, UTC, or proof of when normal userspace appeared. Preserve
those clock meanings separately. Suspend or a changed clock/boot context must
not be normalized away. Capture must confirm the actual expected boot context.

The proposed 600-second check uses that kernel clock directly. It does not
require the operator to type an exact elapsed value while racing the capture.
Record the operator's boot-arrival measurement as a separate observation, with
its actual precision. Approximate arrival is not independent timestamp proof.
If the boot-watch qualification remains ambiguous, report it as such even if
the later raw ownership capture succeeds.

All first-owner evidence stays mandatory: selected image/entry and existing-file
receipts, new boot ID, exact ordered trace, loaded derivative note, Live state,
PCI/DRM/framebuffer/IRQ16, services, complete kernel log and health receipt,
normal physical display, and zero experimental SGX/hot-module actions. Capture
completion within the proposed bound is only one predicate. It does not replace
any of the others.

After the one experimental capture or its failure, use only the authorized
operator boundary and one manual STOCK selection. Apply the same proposed
boot-watch and capture limits to stock recovery. Require all three distinct boot
IDs for full cycle qualification. If the experimental ID is missing, an otherwise
valid stock capture can establish the observed stock state, but cannot repair
the missing experimental ledger. No hot restoration.

## Alternatives considered

Keeping the combined 120-second limit and using a short local command removes
some communication overhead. It still leaves no capture budget when boot itself
takes approximately 120 seconds. It also needs a separately reviewed local
capture route; it is not permission to improvise one in cycle03.

A root startup collector could record evidence before a user logs in and avoid
sudo timing. That would change the image or boot integration, require a newly
pinned artifact and complete qualification, and introduce automatic privileged
execution. It must not submit SGX work or persist credentials. It is a larger
change than the proposed wrapper/procedure revision and is not selected here.

## Work required before this design can be used

This document alone qualifies nothing. Review and approve the new limits and
clock semantics first. Then implement a separate version of the plan, validator,
wrapper and controller; preserve the v1 files/evidence. Do not change old records
or edit v1's constants to make their outcomes pass.

Tests must run the actual successor code and cover: missing/ambiguous readiness;
exact deadline edges; NaN/infinite/negative times; kernel-clock regression;
changed boot ID; stale boot data; timeout with partial output; sudo refusal;
wrong loaded note/trace/ownership; a fault at any time; missing menu photo; duplicate
capture and second selection; recovery with and without experimental identity;
and zero credential stdin. Include controls proving v1 still refuses at 120.

Requalify the unchanged image/candidate identities and all existing offline
guards. Review the new executable procedure and capture-source hashes. Only
after that review may a new, separately scoped live authorization name the
successor procedure and its limits. No staging, image rebuild or SGX action is
proposed as part of this timing change.

**SINGLE SMALLEST NEXT STEP — OFFLINE:** review this timing design and its
proposed 180/600/40-second bounds before implementing the successor procedure.
No target operation is authorized by this document.
