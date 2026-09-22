# Minimum authoritative disclosure for Gate B

Gate B does not require publication of a full EDS/TRM. It needs a traceable answer for the exact read under review.

| required disclosure | Gate requirement(s) | acceptable source scope |
|---|---|---|
| `CORE_ID`/`CORE_REVISION` access type, width, and side effects | 03–05 | SGX535 hardware reference covering the relevant revision |
| required SGX power, register-interface clock, and reset state; verification method | 06–08 | Poulsbo integration reference, or an explicitly applicable vendor statement |
| firmware, BIF, MMU, or microkernel prerequisites | 08 and pre-initialisation scope | SGX535/Poulsbo contract |
| physical revision coverage and relevant BRNs | 09–10 | SGX535 errata plus an authoritative identification/mapping source |
| CPU-visible outcome when the aperture is unavailable and supported recovery | 15–16 | Poulsbo chipset/integration documentation |
| gma500-owned serialization against PM, suspend/resume, IRQ, and removal | 11–14 | exact-kernel lifecycle analysis, or a reviewed implementation design; not an SGX TRM alone |

An authoritative answer that omits unavailable-aperture failure behaviour or revision scope cannot close the whole gate. Likewise, a valid hardware contract would still need a target-kernel ownership and synchronization design before a read could become a `SAFE-CANDIDATE`.
