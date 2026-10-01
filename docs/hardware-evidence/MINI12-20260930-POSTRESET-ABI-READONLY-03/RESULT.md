# Operator-requested connection — STOP before remote execution

In response to the request to reconnect the Mini 12 to the same LAN, the
operator requested "try now". Before contact, this capture recorded that it
would use the original reviewed endpoint `192.168.18.90:22` only, with the
unchanged pinned host/key transport, read-only script and STOP guards.

One connection began at **2026-09-30T18:43:39.634630Z** and ended at
**2026-09-30T18:43:41.985217Z**, exit **255**:

```text
ssh: connect to host 192.168.18.90 port 22: No route to host
```

No authentication or target script execution occurred. Stdout is empty.
No target artifact, state, or kernel identity was captured. No alternate
address, scan, further retry, network configuration change, module operation,
DRM open, ioctl, MMIO or SGX action followed. This failure does not disprove
the operator's report that the machine is powered on/display restored.

The last supplied target address was `169.254.77.254`; local read-only
diagnostics in capture 02 show this host on `192.168.18.84/24`, with only
the connected `192.168.18.0/24` IPv4 route displayed. The exact current
reachable target address/network relationship is still missing. No silent
substitution or host-key relaxation is permitted.

## Artifact integrity and status

- Identical script SHA-256:
  `b9591dab67bedfab789bd82d1c228da0d76201eeccfe9389728bc8a0c5166003`.
- Empty stdout SHA-256:
  `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`.
- Raw 62-byte stderr SHA-256:
  `388e01bafb72ba1971cc36d06e8560deaf97c7595a260bf0d2bcb58299be3bb2`.
- `capture.json` retains exact argv, times, exit code and hashes.
  SHA256SUMS verifies the capture files; raw CRLF remains unchanged.
- Local shell syntax and capture integrity checks pass. No implementation
  changed, candidate build occurred, or offline test suite was rerun.

No blocker closed. Target ABI/build provenance and gma500 transition remain
BLOCKED; Gate B BLOCKED; active whitelist `[]`. No SGX action was performed.
**Single next step: READ-ONLY TARGET OBSERVATION** — obtain the Mini 12's
current reachable LAN address/network connection and perform the same
host-key-pinned capture, with explicit endpoint confirmation. Do not retry
or try the reported link-local address automatically.
