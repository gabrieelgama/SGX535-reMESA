# Power state safety

## What the current code really guarantees

**CONFIRMED — P4-012:** `gma_power_init()` declares runtime PM “broken”, takes
an unconditional reference with `pm_runtime_get()` and only returns it on uninit
(`power.c:46-87`). For Poulsbo, `psb_power_up()` and `psb_power_down()` return
zero without additional sequence (`psb_device.c:186-194`).

**CONFIRMED — P4-013:** o suspend de system desinstala IRQ, salva display,
disable the PCI function and select D3hot. The resume returns to D0, restores state
PCI/BSM/VBT, reabilita GTT/GEM/display e reinstala IRQ
(`power.c:95-203`). Preserved SGX 3D states are not documented.

**CONFIRMED — P4-016:** no bind Poulsbo, `psb_init_pm()` faz read-modify-write
from `PSB_CR_CLKGATECTL` to the 2D portion and placed the writing with a reading
(`psb_device.c:85-94`). This proves the driver's behavior, not that any
another SGX reading is valid.

## Respostas

| pergunta | resposta |
|---|---|
| When is SGX enabled? | **UNKNOWN** at the clocks/blocos internal level |
| when is it power-gated? | **UNKNOWN** |
| who controls? | function PCI/PM: gma500 + PM core; full SGX gate: **UNKNOWN** |
| which registers depend on clocks? | only 2D clock gate behavior appears; complete set **UNKNOWN** |
| is reading valid when turned off? | **UNKNOWN**; no audited source provides this contract |
| what can wake up? | `gma_power_begin(force_on=true)` calls `pm_runtime_resume_and_get()`; the probe does not call it (P4-021) |
| what can turn off? | suspend PCI calls disable + D3hot; the probe does not call it |
| preserved state? | display is salvo/restaurado; state SGX/BIF/USSE/PDS **UNKNOWN** |
| lost state in reset? | reset touches BIF/DPM/TA/USE/ISP/TSP/2D; full consequences **UNKNOWN** |

`power/runtime_status=active` is **CONFIRMED** as the state of the runtime-PM core,
but its equivalence to 'SGX ready for MMIO' is **UNKNOWN**. Clock is not inferred
de um status PCI.

## READ does not imply SAFE

**CONFIRMED — P4-020:** the historical DDK says that the register dump **does not
must** be done when the SGX is not powered (`INIT.txt:1320-1334`). This
refutes any premise that reading is safe with the power off. The passage does not
defines how to prove the powered state, which clocks are enough, or which registers
They have side effects. No MMIO reading is approved at this stage.

## Probe operations

Reading the chosen textual attributes does not call `gma_power_begin()` and the function
`runtime_status_show()` only checks the state of the PM core (CONFIRMED
`P4-004`). The probe does not open DRM. Therefore, it does not contain an intentional path
of wake; absence of effects from competing external agents is not promised.
