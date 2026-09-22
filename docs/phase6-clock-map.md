# Poulsbo clock evidence map

Poulsbo does not have a single documented "GPU clock" in the available material. This table keeps configuration values, named clock-gating fields, and a proven physical clock contract separate.

| item | observed source behavior | consumer or target proved by source | confidence | what it does not establish |
|---|---|---|---|---|
| `gma_get_core_freq()` / historical `psb_get_core_freq()` | writes `0xD0050300` to root-bridge PCI config `0xd0`, reads `0xd4`, and maps bits `[2:0]` to 100, 133, 150, 178, 200, or 266 MHz | `psb_chip_setup()` calls it; `psb_backlight_setup()` uses `dev_priv->core_freq` to calculate `BLC_PWM_CTL` | CONFIRMED, P6-001/P6-002 | SGX execution frequency, SGX register-interface clock, or safe SGX accessibility. The operation is not passive because it writes PCI configuration space. |
| `SYS_SGX_CLOCK_SPEED` | Poulsbo DDK configuration defines `200000000` | timing information supplied by that historical DDK target | CONFIRMED, P6-003 | measured hardware frequency, clock source/divider, gating state, or access precondition |
| `SYS_SGX_CLOCK_SPEED_PLB` | EMGD community mirror supplies `200000000` to a platform interface and timing structure | historical driver configuration | CONFIRMED, P6-004 | a physical measurement or a documented register-access condition |
| `CLKGATECTL` | SGX535 headers name 2D, ISP, TSP, TA, DPM, and USE gate fields plus `AUTO_MAN_REG`; gma500 changes only the 2D field at bind | software-visible SGX clock-gating control and a historical driver action | CONFIRMED, P3-010/P4-016 | whether the SGX register interface remains clocked, whether identification reads are valid, or how to prove the state safely |
| PCI/runtime PM state | Test Vector Zero recorded runtime status `active`; Linux exposes PCI power state | PCI/device software state | CONFIRMED, TVZ-001/P4-022 | an internal SGX power or clock state |

## `psb_get_core_freq()` result

The historical name `psb_get_core_freq()` and the current `gma_get_core_freq()` describe the same style of Poulsbo helper in the recovered Linux line. The helper is called during Poulsbo chip setup. In the current checked-in source, its value is used by `psb_backlight_setup()` to calculate the backlight PWM programming value. This is the only direct Poulsbo consumer found in the target driver path.

The source does not name the host-bridge `0xd0` selector/data mechanism's underlying hardware clock field. It also does not state that `core_freq` is the SGX execution clock. Treating it as such would be an unsupported inference. Calling the helper as a new probe would violate the passive policy because it writes PCI configuration space.

## Missing clock contract

**UNKNOWN:** SGX execution-clock source and frequency on the Dell system; register-interface clock domain; whether automatic gating leaves the identification registers accessible; the minimum clock state for either candidate read; and a passive proof of that state. The SGX535 header provides bit locations, not the required semantics.
