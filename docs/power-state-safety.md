# Power-state safety

## What the current code establishes

**CONFIRMED — P4-012:** `gma_power_init()` calls runtime PM "broken," takes an unconditional `pm_runtime_get()` reference, and releases it only during uninitialization (`power.c:46-87`). Poulsbo's `psb_power_up()` and `psb_power_down()` return zero without an additional sequence (`psb_device.c:186-194`).

**CONFIRMED — P4-013:** system suspend removes the IRQ, saves display state, disables the PCI function, and selects D3hot. Resume returns it to D0, restores PCI/BSM/VBT state, re-enables GTT/GEM/display, and reinstalls the IRQ (`power.c:95-203`). Preservation of SGX 3D state is undocumented.

**CONFIRMED — P4-016:** during Poulsbo bind, `psb_init_pm()` performs a read-modify-write of the 2D field in `PSB_CR_CLKGATECTL` and posts the write with a read (`psb_device.c:85-94`). This establishes driver behavior, not the validity of any other SGX read.

| Question | Current answer |
|---|---|
| When is SGX powered? | **UNKNOWN** at the internal block/clock level |
| When is it power-gated? | **UNKNOWN** |
| Who controls it? | PCI function/PM: gma500 and PM core; complete SGX gating: **UNKNOWN** |
| Which registers need clocks? | Only observed 2D clock-gate behavior is known; the complete set is **UNKNOWN** |
| Are reads valid while powered off? | **UNKNOWN**; no applicable source supplies this contract |
| What may wake the device? | `gma_power_begin(force_on=true)` calls `pm_runtime_resume_and_get()`; the passive probe does not call it (`P4-021`) |
| What may power it down? | PCI suspend disables the function and selects D3hot; the probe does not call it |
| What state is preserved? | Display state is saved and restored; SGX/BIF/USSE/PDS state is **UNKNOWN** |
| What is lost on reset? | Reset touches BIF/DPM/TA/USE/ISP/TSP/2D; complete consequences are **UNKNOWN** |

`power/runtime_status=active` is **CONFIRMED** only as PM-core state. Whether it means "SGX ready for MMIO" remains **UNKNOWN**. A PCI state does not establish SGX clock state.

## A read is not automatically safe

**CONFIRMED — P4-020:** the historical DDK says not to dump registers while SGX is unpowered (`INIT.txt:1320-1334`). This rejects any assumption that reads are safe with power off. The source does not say how to prove the powered state, which clocks suffice, or which registers have read side effects. No MMIO read is approved.

Reading the selected sysfs/procfs attributes does not call `gma_power_begin()`. `runtime_status_show()` only reports PM-core state (`P4-004`), and the probe does not open DRM. The probe therefore has no intentional wake path; it cannot promise that unrelated activity will not affect state concurrently.
