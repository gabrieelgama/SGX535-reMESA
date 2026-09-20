# Register-read safety audit

## Criterion

`SAFE-CANDIDATE` requires, at the same time: offset Poulsbo/SGX535 confirmed,
explicit reading semantics, known absence of clear/ack/latch/FIFO,
power/clock known and ownership/locking resolved. No register applies
to all requirements. The table is not an executable whitelist.

| offset | name | purpose | platform | source | read-semantics | known-side-effects | power-requirement | ownership | confidence | phase4-status |
|---:|---|---|---|---|---|---|---|---|---|---|
| `0x0010` | `CORE_ID` | identification/configuration | Poulsbo | Linux `psb_reg.h:31-35`; historical PSB `PSB_psb_drv_c.txt:325-339`; DDK `INIT.txt:1320-1352` | historically read | contract missing | SGX powered; how to prove UNKNOWN | gma500 SGX mapping | offset CONFIRMED; safety UNKNOWN | UNKNOWN |
| `0x0014` | `CORE_REVISION` | designer/major/minor/maintenance | Poulsbo | same artifacts; EMGD `sysconfig.c:547-575`; DDK `INIT.txt:1320-1352` | historically read | missing contract | SGX powered; how to prove UNKNOWN | gma500 SGX mapping | offset CONFIRMED; safety UNKNOWN | UNKNOWN |
| `0x0018` | `DESIGNER_REV_FIELD1` | designer review | Poulsbo | Linux `psb_reg.h:47` | not located | UNKNOWN | UNKNOWN | gma500 | offset CONFIRMED | UNKNOWN |
| `0x001c` | `DESIGNER_REV_FIELD2` | designer review | Poulsbo | Linux `psb_reg.h:58` | not located | UNKNOWN | UNKNOWN | gma500 | offset CONFIRMED | UNKNOWN |
| `0x0080` | `SOFT_RESET` | controls SGX resets | Poulsbo | Linux `psb_reg.h:49-56`; `psb_drv.c:103-124` | read used as posted read | write changes multiple blocks | UNKNOWN | gma500 init | action CONFIRMED | UNKNOWN |
| `0x0110` | `EVENT_HOST_ENABLE2` | SGX2 IRQ mask | Poulsbo | Linux `psb_reg.h:60-65`; `psb_irq.c:250-305` | readback post-write | run with IRQ/mask | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNKNOWN |
| `0x0114` | `EVENT_HOST_CLEAR2` | acknowledge IRQ SGX2 | Poulsbo | Linux `psb_reg.h:64-65`; `psb_irq.c:192-195` | only post-write readback observed | write-to-clear; read effect UNKNOWN | SGX IRQ active | IRQ handler | use CONFIRMED | UNKNOWN |
| `0x0118` | `EVENT_STATUS2` | status IRQ SGX2 | Poulsbo | Linux `psb_reg.h:62`; `psb_irq.c:198-239` | read in handler | concurrency/latch UNKNOWN | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNKNOWN |
| `0x012c` | `EVENT_STATUS` | status IRQ SGX | Poulsbo | Linux `psb_reg.h:67`; `psb_irq.c:198-239` | read in handler | concurrency/latch UNKNOWN | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNKNOWN |
| `0x0130` | `EVENT_HOST_ENABLE` | SGX IRQ mask | Poulsbo | Linux `psb_reg.h:69`; `psb_irq.c:250-305` | readback post-write | concurrency with IRQ | SGX IRQ active | `irqmask_lock` | use CONFIRMED | UNKNOWN |
| `0x0134` | `EVENT_HOST_CLEAR` | acknowledge SGX IRQ | Poulsbo | Linux `psb_reg.h:71-84`; `psb_irq.c:192-195` | undocumented read | write-to-clear | SGX IRQ active | IRQ handler | use CONFIRMED | UNKNOWN |
| `0x0ab8` | `PDS_EXEC_BASE` | PDS base | Poulsbo | Linux `psb_reg.h:116-118`; `psb_drv.c:361` | not located | execution/address control | UNKNOWN | gma500 init | offset/use CONFIRMED | UNKNOWN |
| `0x0ac4` | `EVENT_KICKER` | kick address | Poulsbo | Linux `psb_reg.h:120-121` | not located | associated with submission | UNKNOWN | historical ABI | offset CONFIRMED | UNKNOWN |
| `0x0ac8` | `EVENT_KICK` | triggers event | Poulsbo | Linux `psb_reg.h:123-124` | not located | write starts action | UNKNOWN | firmware/command path | offset CONFIRMED | UNKNOWN |
| `0x0c00` | `BIF_CTRL` | fault clear/cache/TLB controls | Poulsbo | Linux `psb_reg.h:128-130`; `mmu.c:69-120` | RMW/readback used | flush/invalidate/clear fault on write; read side UNKNOWN | UNKNOWN | `mmu->sem` | use CONFIRMED | UNKNOWN |
| `0x0c38` | `BIF_DIR_LIST_BASE1` | MMU base/context | Poulsbo | Linux `psb_reg.h:126`; `mmu.c:123-135` | not located individually | write changes address space | UNKNOWN | `mmu->sem` | offset CONFIRMED | UNKNOWN |
| `0x0c04` | `BIF_INT_STAT` | fault type/requester | Poulsbo historical/Linux | Linux `psb_reg.h:133-147`; `psb_irq.c:159-188` | read in fault IRQ | clear/latch UNKNOWN | SGX IRQ active | IRQ handler | use CONFIRMED | UNKNOWN |
| `0x0c08` | `BIF_FAULT` | failure address | Poulsbo historical/Linux | Linux `psb_reg.h:135-147`; `psb_irq.c:159-188`; DDK `INIT.txt:1353-1364` | read on fault/debug | clear/latch UNKNOWN | SGX powered UNKNOWN | IRQ/debug owner | use CONFIRMED | UNKNOWN |
| `0x0000` | `CLKGATECTL` | clock gate | Poulsbo | Linux `psb_reg.h:13-29`; `psb_device.c:85-94` | RMW/readback | a write changes gating; read effect UNKNOWN | it is the clock control itself | gma500 PM | use CONFIRMED | UNKNOWN |
| `0x0e04` | `2D_BLIT_STATUS` | status 2D | Poulsbo | Linux `psb_reg.h:160-163`; `psb_irq.c:156-158` | read on 2D completion | clear/latch UNKNOWN | clock 2D | IRQ handler | use CONFIRMED | UNKNOWN |

**CONFIRMED — P4-014/P4-015:** three historic families read ID/revision: PSB,
DDK Poulsbo and EMGD. **INFERRED:** ID/revision are the best research targets
documentary. **UNKNOWN:** if they are safe for the first real access. Repetition
from a historical practice does not replace datasheet/errata or IP contract.

No explicit evidence of clear-on-read was found for the two IDs; also
no explicit evidence of the absence of this behavior was found. Therefore
there is no `SAFE-CANDIDATE`.
