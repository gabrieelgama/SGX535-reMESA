# First SGX MMIO experiment result: NOT EXECUTED

**Gate B: BLOCKED. Whitelist: `[]`. Experiment status: OPERATOR-AUTHORIZED despite unresolved Gate B requirements; NOT EXECUTED.** The exact single-read candidate was prepared, but the retained SSH user lacked read permission for both candidate raw mapping routes. The stop condition was applied before opening any MMIO resource. No write or SGX MMIO read occurred.

| Required report field | Result |
| --- | --- |
| FIRST MMIO ATTEMPT | **NOT EXECUTED**; attempt count `0` |
| REGISTER | Proposed `CORE_ID` only; `CORE_REVISION` untouched |
| TARGET | Fresh pinned-SSH DMI/CPU/PCI fields matched H0/P1 Dell Inspiron 1210, `0000:00:02.0`, `8086:8108`, PCI revision `0x06`, subsystem `1028:02b1` |
| BAR / SGX APERTURE | BAR0 `0xd8100000–0xd817ffff`; installed-module SGX map base BAR0 + `0x40000` = `0xd8140000`, length `0x8000` |
| ADDRESS DERIVATION | `0xd8100000 + 0x40000 + 0x10 = 0xd8140010`, checked 4-byte aligned and inside both BAR0 and SGX aperture |
| WIDTH | Proposed 32 bits, from historical `ioread32`/`PSB_RSGX32` source; target revision access validity UNKNOWN |
| EXACT READ MECHANISM | Conditional read-only `resource0` page mapping at BAR offset `0x40000`, one aligned native 32-bit load at page offset `0x10`, no write or range dump. **No read program was compiled or run; mapping and generated instruction were not verified because access was unavailable.** `/dev/mem` was not opened as a fallback. |
| RESULT | No MMIO value; no hardware transaction attempted |
| EXIT STATUS | MMIO program: N/A. Read-only preflight SSH exit `0`, stdout 2300 bytes, stderr 0 bytes; exact argv, source hash, output hashes and timestamps in `raw/preflight.meta.json`. |
| TARGET RESPONSIVE AFTERWARD | **YES after preflight**: SSH returned normally. No claim about responsiveness after an MMIO attempt, because none occurred. |
| HW-OBSERVED FACTS | **None from the intended SGX MMIO transaction.** The preflight returned OS-visible DMI/CPU/PCI, module/driver, PM and permission text only. |
| INFERENCES NOT YET ALLOWED | No SGX revision, errata, power/clock/reset semantics, register safety, or general MMIO safety inference. |
| GATE B STATUS | **BLOCKED**, whitelist `[]`; rows 03–10 and 12–16 remain target-qualified unresolved. |
| NEXT ACTION | Obtain an explicitly arranged read-capable execution path and independently verify its one-load/no-write mechanism before any later authorized attempt. **Not performed here.** Do not use a failed permission probe as the one read or retry automatically. |

## Captured pre-experiment state and timestamps

The retained pinned SSH endpoint matched Dell Inc., Inspiron 1210, board `0X605H`, BIOS A02, i686 Atom Z520 and exact PCI identity. BAR0 resource text matched P1/H0; `gma500` remained bound, `gma500_gfx` live, loaded Build-ID note unchanged. PCI `enable=1`, `power/runtime_status=active`, `power/control=on`; `/proc/fb` reported `gma500drmfb`, and IRQ 16 text named `gma500, eth0`. These are OS-visible snapshots, not SGX internal-state proof.

The capture host recorded `2026-09-28T03:05:59.618471+00:00` to `03:06:00.639278+00:00`; the target emitted `2026-09-28T00:56:24.475795+00:00` to `00:56:24.507691+00:00`. The host-target clock offset remains uncalibrated. Raw output and per-file hashes are preserved in [the manifest](manifest.json). H0 and P1 were not changed. No post-MMIO aftermath exists because no MMIO attempt occurred.
