# First-load cycle02: stock restored, experimental proof unavailable

2026-10-01 UTC. [Raw evidence](../hardware-evidence/MINI12-20261001T062604Z-FIRSTLOAD-CYCLE-02/README.md).

FIRST TRIANGLE: NOT ATTEMPTED. SGX Gate B BLOCKED; whitelist `[]`.

No fixed client/ioctl/PDS/TA/raster request, hot unload/replacement or
speculative reset was performed. The physically present operator reported one
experimental selection and one stock recovery selection. There was no second
experimental boot.

## Guard correction and fresh preflight

The [cycle01 failure](first-load-cycle-01-result.md) remains unchanged. The [kernel-health correction](kernel-health-guard-qualification.md) fixes the
case-insensitive BUG substring match in BL bug. It checks the actual diagnostic
fields and bounded counts, with real fault controls. The ACPI diagnostic forms
remain explicit. Unknown, repeated or changed warnings and
BUG/oops/panic/traces/lockups/graphics faults are rejected. A baseline cannot
exempt an arbitrary fault.

Cycle02 preflight01 stopped before its script ran: sudo required local
authentication (06:26:26 UTC, exit1). The operator refreshed sudo locally. Fresh
preflight02 ran from the beginning (06:28:22–06:28:24): 49/49 PASS. No
credential was transmitted or recorded. The health receipt counted 2/2/1/1/1
stock diagnostics and no selected faults. The checks passed for the exact
original identity, ownership and services; five stock files; GRUB saved stock
selection; trusted parents, space and destination absence.

## Staging: PASS

At 06:31:53–06:32:44 UTC, staging repeated the full preflight, then exclusively
created the private incoming directory and two files. It created the boot image
and custom.cfg in that order.

The writer used no-follow opens and recorded inodes. It fsynced files and
directories, then closed and reopened the files for complete readback. The final
files were root-owned, mode 0644, with a single link. All post-stage guards
passed. The five stock files, inodes and hashes were unchanged, as were the GRUB
environment/default, loaded original and boot ID. No service, console or driver
operation occurred.

- Image: 50,804,481 bytes, `4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`.
- Entry: 974 bytes, `181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612`.
- Pre-stage stock boot: `8ae37532-19d4-4ec7-9c29-a791acfd889f`.

## Experimental boot: NOT ESTABLISHED

The operator reported that STOCK was still the default, EXPERIMENTAL was
manually selected exactly once, and normal userspace appeared within 120 seconds
with no visible hang/HOLD. These are operator observations. They do not prove
which code loaded or who first owned the device. No menu photograph was
supplied; it remains missing from the required selection evidence.

The sole passive root capture (06:39:19–06:39:21) failed before its script ran:
`sudo: a password is required`, exit1, empty stdout. The experimental reply did not
explicitly confirm local sudo-success. There was no retry.

We did not capture the loaded derivative identity, hook trace, experimental boot
ID, PCI/DRM/IRQ state or complete experimental kernel log. Mandatory capture
completion timing is UNKNOWN. Normal userspace is not enough for PASS. The
missing evidence put the operation in HOLD. No kernel-side hook HOLD was
reported.

## Stock recovery: current machine verified

The operator used the authorized machine boundary and manually selected STOCK.
They reported normal userspace, successful local sudo-v and 90 seconds before
capture. The single passive recovery capture (06:49:39–06:49:50) passed 54/54
guards, with exit0 and empty stderr. The operator then confirmed a normal
physical display and capture completion within 120 seconds.

| Fact | Result |
| --- | --- |
| New stock boot | `83fd48ed-7a27-4d4d-bd88-62880c3ffa78`, distinct from pre-stage |
| Original loaded driver | Live; exact original note SHA `484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7` |
| PCI/DRM | 0000:00:02.0 bound to gma500/gma500_gfx; card0 ownership restored |
| Display ownership | gma500drmfb 1280×800; VT0=0/VT1=1; gma500 on IRQ 16 |
| Services/access | slimski/Xorg running; passive SSH/root capture succeeded; physical display normal |
| Stock artifacts/default | Five stock identities/inodes unchanged; saved stock ID only; staged image/entry exact |
| Kernel health | No selected fault; typed stock diagnostics visible; taint 12289 |
| Full recovery contract | NOT ESTABLISHED: missing experimental boot ID prevents complete three-boot ledger |

The stock service status reports 174s; the operator reported 90s. Target and
host UTC are unsynchronized, and no same-boot uptime was captured. Both reports
are preserved. Independent deadline corroboration remains UNKNOWN.

The normal stock driver and display were verified, but full cycle qualification
is incomplete. No further target contact or recovery operation occurred. The
staged experimental files remain. There was no cleanup or default mutation.

## Offline follow-up

The future capture helper requires an explicit current-boot local sudo witness.
It preserves unprivileged boot identity before sudo in the same connection,
bounds read-only capture by same-boot uptime, and preserves timeout output.
Cached/uncached and adversarial actual-wrapper tests pass. This helper was
qualified offline only. It was not deployed and cannot recover the missing
historical experimental evidence. No pinned image or candidate was rebuilt or
changed.

LIVE FIRST LOAD / FIRST OWNER: NOT ESTABLISHED.
Full LIVE RECOVERY: NOT ESTABLISHED.
Current stock machine/driver/display state: PASS observed.
Gate B BLOCKED; SGX whitelist `[]`.
This first-load cycle never authorized SGX execution.

The single smallest next step is TARGET-STATE-CHANGING: obtain new explicit
authorization for ONE fresh, non-SGX manual first-load/recovery cycle. First
revalidate the already-staged exact files and creation identities, plus the
stock/default state. Use the corrected evidence-readiness/boot-ID/timing
capture. No restaging overwrite, hot transition, retry within this spent cycle,
or triangle permission.

## Final offline verification

304 repository tests passed with zero skips, along with 14 boot-analysis tests,
three strict UBSan harnesses, the generator, both actual candidate-import CRC
checks and two deterministic dry runs. Dry SHA remains
2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e. --complete
still returns the expected exit1: PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE. No
candidate, image, stock capture or historical attempt was rewritten.

Six raw-cycle receipt tests passed. The bundle includes the final link,
manifest, whitespace and preservation results. The four existing-file tests use
the actual recovery records and reject changed inodes even when the bytes match.
They grant neither boot nor SGX authorization.