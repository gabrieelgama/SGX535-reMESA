# Renewed read-only capture — connection STOP

The operator reports that the Mini 12 was powered off during capture 01,
is now powered on, and authorizes continuation of the same read-only task.
Capture 01 remains unchanged. The exact script and pinned transport were
copied unchanged into this separate evidence directory; the renewed plan
was recorded before contact. No deployment or SGX action was authorized.

One SSH connection to the reviewed `gama@192.168.18.90:22` endpoint was made
at **2026-09-30T18:35:10.590002Z**. It ended at
**2026-09-30T18:35:12.930228Z**, exit **255**, before authentication or remote
execution:

```text
ssh: connect to host 192.168.18.90 port 22: No route to host
```

The connection STOP guard applied. No target script ran, no build material
was captured, and no retry, alternate-address probe, scan, target-state
repair or SGX operation occurred. This network error does not establish that
the target is off or that its display/driver state differs from the operator
report. The target's current IP/network connection has been requested from
the operator; no endpoint substitution is authorized or performed here.

## Operator network follow-up

The operator subsequently supplied current address `169.254.77.254`.
Local-only `ip -4 route show` displays only the connected
`192.168.18.0/24` route, source `192.168.18.84`, on `wlan0`;
`ip -4 -brief address` shows no link-local IPv4 interface address. This does
not provide a qualified route to the supplied target address. The local
`/proc/net/route` read was permission-denied; no privilege escalation occurred.
Exact subsequent local route/interface outputs are in `local-network.json`.
No SSH connection to the new address, address scan, local route/interface
modification, or target configuration operation was performed. The operator
was asked to provide the Mini 12's address on the same LAN before capture
continues. Any new connection must retain the known Mini 12 host-key pin;
an address change alone does not qualify a different machine.

Local-only route diagnostics reported interface `wlan0` up with IPv4 address
`192.168.18.84/24`. `ip route get 192.168.18.90` itself emitted a diagnostic
(`Not a route ... An error :-)`) rather than a usable route result. Neither
local observation establishes target reachability or target state. No local
network configuration was changed.

## Evidence

| Artifact | SHA-256 |
| --- | --- |
| Identical script `capture.sh` | `b9591dab67bedfab789bd82d1c228da0d76201eeccfe9389728bc8a0c5166003` |
| Empty `stdout.txt` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| Raw `stderr.txt`, 62 bytes | `388e01bafb72ba1971cc36d06e8560deaf97c7595a260bf0d2bcb58299be3bb2` |

`capture.json` records exact argv, script/output hashes, timestamps and exit
status. SHA256SUMS covers all files in this capture. Local `sh -n` and
capture/manifest integrity checks passed. Raw stderr CRLF is preserved.
The 187-test offline baseline remains the previous verified result; no
implementation changed and the suite was not rerun just for this connection
failure. No kernel candidate was rebuilt.

Post-reset state remains independently unverified; no new Module.symvers,
config, generated headers or compiler metadata was obtained. Target ABI and
gma500 transition remain BLOCKED. Known preserved/candidate/old-table
`module_layout` CRCs remain `0xb84efb99`/`0x995e9910`/`0x995e9910`; the old
150 CRC mismatches and nine unqualified imports are unchanged, not newly
observed target results. Gate B BLOCKED, active whitelist `[]`.

No blocker closed. **Single next step: READ-ONLY TARGET OBSERVATION** —
confirm the Mini 12's current network address/connection, then continue the
same pinned-host capture under an explicitly confirmed endpoint/scope.
Do not scan, silently substitute an address, retry automatically, deploy,
or perform SGX work.
