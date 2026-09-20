# Phase 4.1 — Test Vector Zero evidence

2026-09-19. **Gate B: BLOCKED. No SAFE-CANDIDATE approved.** I incorporated the real TVZ report for the Dell Inspiron 1210 without performing another hardware operation.

The report confirms BDF `0000:00:02.0`, vendor/device `8086:8108`, PCI revision `0x06`, subsystem `1028:02b1`, class `030000`, IRQ 16, and the four measured BARs. It also reports kernel `5.10.240-antix.1-486-smp`, i686, driver `gma500`, DRM `card0`, and runtime status `active`.

The report performed no MMIO reads or writes, reset, firmware load, MMU initialization, command submission, or power/clock manipulation. PCI revision `0x06` is not an SGX core revision. The physical SGX revision remains UNKNOWN. `runtime_status=active` does not prove SGX clocks are enabled.

BAR sizes are recorded as measurements. Their functions are not inferred from size or address alone. They do not resolve the MMIO, power, clock, ownership, failure, or revision gates.
