# Mini 12 sudo credential transport for Attempt 02

Attempt 01 stopped at stage 0 because a cached sudo timestamp left a
credential line unread; the following remote `sh -s` interpreted that line
as a command. [Attempt 01](../hardware-evidence/MINI12-20260930-TRIANGLE-ATTEMPT-01/RESULT.md)
remains unchanged. The failure was in credential delivery, before artifact
transfer or graphics-state change.

The replacement transport in
[`mini12_sudo_transport.py`](../../tools/psb-dri-re/mini12_sudo_transport.py)
uses **two distinct pinned SSH processes**:

1. Run `sudo -S -p '' -v` with the credential obtained interactively and
   sent only on that process's stdin. No shell or target script is attached.
   Require exit 0 and empty stdout/stderr. Never persist its stdin.
2. Only then run `sudo -n -p '' sh -s` in a new SSH process, sending only the
   exact reviewed script bytes. If authentication is unavailable or expires,
   `-n` refuses instead of prompting; no credential is offered to this
   process. Require the script's expected exit, stdout and empty stderr.

The credential is absent from command arguments, environment, repository
files and retained capture. The caller must not log the authentication
process's stdin or reuse the Attempt 01 combined input stream. This change
does not alter the approved display/module/SGX operation.

## Read-only validation, 2026-09-30

The [credential-transport capture](../hardware-evidence/MINI12-20260930-CREDENTIAL-TRANSPORT-TEST/RESULT.md)
used the retained pinned SSH identity and a root script limited to `id -u`
and staging-path absence checks. Initially `sudo -n -v` returned 0, showing
cached authentication. The separated transport returned `uid=0`, all three
paths absent, exit 0 and empty stderr. `sudo -k` then invalidated the sudo
timestamp; `sudo -n -v` returned 1. The same separated transport again
returned `uid=0`, the same absent paths, exit 0 and empty stderr. No artifact
was transferred or staged. No graphics service, driver, DRM node, MMIO or
SGX execution state was touched.

The focused host test checks that the credential and script enter different
subprocesses, and that failed authentication never starts the script.
Cached and uncached target cases both passed. This establishes the corrected
credential routing for the scoped read-only test; it is not an active
transition result. Attempt 02 still requires renewed explicit authorization
and all live stop/HOLD guards from the existing Gate B review.
