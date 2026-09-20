# GO / NO-GO / BLOCKED Matrix of Phase 4

The states apply to the current sources and harness. `GO` does not authorize a step
implicit posterior.

| item | state | basis; exact requirement to unlock |
|---|---|---|
| A. Passive hardware probe | GO | Exact IDs and documented RO textual attributes; implementation does not open device or BAR |
| B. Controlled MMIO reads | BLOCKED | missing read-side-effects contract, power/clock per register, stepping and locking; required source: datasheet/register reference SGX535 Poulsbo + errata/power sequence Intel/IMG |
| C. Controlled SGX reset | BLOCKED | official sequence missing due to review, pre/post state, impact on display/IRQ/MMU and recovery; required source: init/reset authenticated SGX535 Poulsbo script or Intel/IMG documentation |
| D. Clock/power manipulation | NO-GO | current gma500 has declared broken PM runtime, empty Poulsbo callbacks, and shared PCI/display function; no external manipulation is acceptable |
| E. BIF observation | BLOCKED | divergence of layout/contextos and semantics of status/fault; source required: SGX535 Poulsbo BIF register reference + revision errata |
| F. MMU initialization | BLOCKED | formula of contexts and addresses GATT/MMU diverges; official formato/flush/fault and limits are missing; source needed: complete Poulsbo DDK and BIF/MMU documentation from the review |
| G. SGX address-space creation | BLOCKED | depends on F and the GTT/stolen model confirmed in hardware; lacks modern memory ABI and cache/coherence rules |
| H. Firmware/microkernel loading | NO-GO | firmware verified, license, hash, target revision, and bootstrap protocol are missing |
| I. CCB allocation | BLOCKED | CCB format, memory/coherence, produtor/consumidor and compatible firmware are not established for target Poulsbo |
| J. EVENT_KICK | NO-GO | is a submission action; address/evento/firmware and recovery are not verified |
| K. PDS execution | NO-GO | binary, ISA, bases, stepping, and nonexistent sandbox |
| L. USSE execution | NO-GO | ISA/encoding and SGX535 limits were not rebuilt with compatible source |
| M. First GPU workload | BLOCKED | depends on B–L and on hang mechanism detection/recovery; required source: complete and reproducible Poulsbo SGX535 stack |
| N. First rendering workload | BLOCKED | depends on M, command formats stream/render target and validation; required source: documentation/compilador/driver userspace with proper provenance |
| O. Mesa integration | BLOCKED | modern architecture can only be defined after memory, submission, sync, firmware, and minimum workload are confirmed |

## Decision

Only A is `GO`. D, H, J, K, and L are `NO-GO` with the current evidence. The
the other items are `BLOCKED`, never promoted due to similarity with SGX540/544,
OMAP ou EMGD.

