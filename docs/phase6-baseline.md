# Phase 6 baseline

Date: 2026-09-22

Phase 6 starts from [the Phase 5 final report](phase5-final-report.md), not from an assumed hardware state. The Phase 5 decision was:

| item | state |
|---|---|
| Gate A — passive probe | `GO` |
| Gate B — first controlled SGX observation | `BLOCKED` |
| `CORE_ID` | `SAFE-CANDIDATE NO` |
| `CORE_REVISION` | `SAFE-CANDIDATE NO` |
| whitelist | `[]` |
| Test Vector One | blocked, not designed or executed |
| hardware state modified | `NO` |

## Target identity

Test Vector Zero records the following measured identity in [TVZ-001](hardware-evidence/TVZ-001/operator-report.txt): Dell Inspiron 1210; Dell 0X605H; BIOS A02; graphics function `0000:00:02.0`; `8086:8108`; PCI revision `0x06`; subsystem `1028:02b1`; IRQ 16; gma500 on `card0`; and kernel `5.10.240-antix.1-486-smp` on i686.

`0x06` is the graphics function's PCI revision ID. It does not establish an Intel graphics or SCH stepping, a physical SGX core revision, a DDK `SGXCORE_REV` target, or a BRN set.

## Scope and safety boundary

This phase inspects checked-in sources, repository history, public documentation, and the preserved passive report. It performs no hardware operation. In particular, it does not map a PCI resource, use `/dev/mem`, access SGX MMIO, write PCI configuration space, change power or clocks, reset hardware, alter GTT/MMU state, load firmware, submit commands, or execute Test Vector One.

The two candidate observations remain historical 32-bit reads at SGX-relative offsets `0x0010` (`CORE_ID`) and `0x0014` (`CORE_REVISION`). Their identity, offsets, and historical width are documented software facts. They are not an access contract.

## Required evidence for a first observation

For each register, Phase 6 evaluates: identity, offset, width, read semantics, read side effects, power, clock, reset, physical-revision coverage, errata, ownership, mapping lifetime, locking, suspend/resume, failure behavior, and recovery. A non-`PASS` safety requirement prevents a whitelist entry.

The permitted work is evidence acquisition. All active bring-up remains prohibited while Gate B is `BLOCKED`.
