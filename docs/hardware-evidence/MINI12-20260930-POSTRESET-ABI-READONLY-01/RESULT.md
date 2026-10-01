# Read-only post-reset / kernel-build capture — connection STOP

The operator authorized this single read-only observation. The exact
[plan](PLAN.md), [target script](capture.sh), and pinned
[local recorder](run_capture.py) were recorded before contact. Local shell
syntax and Python compilation checks passed. No target file, sudo credential,
candidate module or one-shot client was transmitted.

At **2026-09-30T18:00:02.465741Z**, one connection to the retained pinned
`gama@192.168.18.90:22` endpoint was attempted with BatchMode,
StrictHostKeyChecking and ConnectionAttempts=1. It timed out at
**2026-09-30T18:00:10.529998Z**, before authentication or remote script
execution. SSH exited **255**:

```text
ssh: connect to host 192.168.18.90 port 22: Connection timed out
```

The capture STOP condition applied. There was no retry, alternative endpoint,
network scan, privilege escalation, target-state repair, or hardware action.
The failure does **not** establish that the Mini 12 is off, that its IP changed,
or that its display state differs. It establishes only that this connection
did not succeed. The operator's normal-display report remains the latest
post-reset information; detailed target state remains UNKNOWN.

## Preserved evidence

| Artifact | SHA-256 |
| --- | --- |
| Executed local script input `capture.sh` | `b9591dab67bedfab789bd82d1c228da0d76201eeccfe9389728bc8a0c5166003` |
| `stdout.txt` (0 bytes; no remote output) | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `stderr.txt` (66 bytes) | `d179692f158a8070ea4fcec4e68046fe6a557012f223b546a5fe5755f866c321` |

[capture.json](capture.json) records exact argv, times, return code, byte
counts and hashes. Local integrity verification passed. The adjacent
SHA256SUMS covers the plan, scripts, result and offline comparison outputs.
No new target Module.symvers/config/generated header/compiler record was
captured. The script itself was **not executed on the target**.

## Offline checks after the connection failure

The current [module checker](../../../tools/psb-dri-re/frozen_module_versions.py)
was rerun only against already-preserved local artifacts:

- [Preserved-module comparison](preserved-module-comparison.json): exit 1,
  **REJECT**, 150 shared CRC mismatches, nine candidate-only imports unverified.
- [Old build-table comparison](old-build-table-comparison.json): exit 1,
  **REJECT**, 150 mismatches, zero missing reference imports. This table is
  the old public package table, **not a newly observed target table**.
- Reference/candidate/table `module_layout`: `0xb84efb99` / `0x995e9910` /
  `0x995e9910`. Both comparison stderr files are empty; every mismatched CRC
  is recorded individually in JSON.
- Original/candidate hashes remain
  `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb` /
  `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`.
  The old table hash remains
  `f4732fe605f0bda023e1f83d4a3ff0e61460f7c7e1eca92599235047a1b35edc`.
- Full scoped Python discovery after collection stopped: **187 tests PASS**
  (`python3 -m unittest discover -s tools/psb-dri-re -p 'test_*.py'`, exit 0).
  This includes the existing host C harness builds/runs and fixed-IO UBSan
  regression. No kernel module was compiled or linked during this task.
- `sh -n capture.sh` and local Python recorder compilation: **PASS**.
  `git diff --check`, authored document/script whitespace checks and capture
  integrity checks: **PASS**. The raw SSH stderr retains its original CRLF;
  it is not normalized to satisfy source-code whitespace rules.

No build became justified: the installed kernel's complete, attributable
ABI/build bundle remains unavailable. No CRC editing, compatibility bypass,
candidate rebuild, deployment or Attempt 04 occurred. The established
incorrect-table provenance is unchanged; why the public package table
differs from the installed kernel remains **UNKNOWN**. The gma500 hot-reload
lifecycle concern also remains unresolved; correlation with the IRQ oops
has not been upgraded to proven causation.

Gate B remains **BLOCKED**, active whitelist **`[]`**. No blocker was closed
by this failed connection. Next required fact: the reviewed endpoint is
reachable for the separately scoped post-reset/build-input observation.
**Next step classification: READ-ONLY TARGET OBSERVATION.** Do not retry
automatically, substitute another host, deploy, or perform SGX work.
