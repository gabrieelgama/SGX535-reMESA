# Errata relevant to the first identification read

The review covered BRN and errata material that could affect register reads, clocks, power, reset, MMIO aperture, or recovery. No item was proven applicable to the measured Dell graphics function.

`BRN22693` remains **POSSIBLE** only as a historical clock-gating reference; it does not establish which clocks feed the register interface. EMGD document `445348-016US` is a bibliographic source, not a universal Poulsbo hardware contract. The SCH GCR table has an unresolved field/default inconsistency and was not used to select bits.

PCI revision `0x06` does not select SGX errata. No BRN currently closes a Gate B requirement.
