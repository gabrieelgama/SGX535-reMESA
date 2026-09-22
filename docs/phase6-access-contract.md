# SGX535/Poulsbo identification-read access contract

The question is narrow: what must hold before a CPU performs one 32-bit read of SGX535/Poulsbo `CORE_ID` at SGX-relative `0x0010` or `CORE_REVISION` at `0x0014`, and what happens if it does not hold?

Available historical code establishes the names, offsets, and 32-bit accesses. It also establishes that historical stacks performed reads after their own initialization paths. It does not turn these software observations into an architectural read contract.

## Contract reconstruction

| contract term | `CORE_ID` | `CORE_REVISION` | evidence state |
|---|---|---|---|
| identity, offset, width | historical software evidence | historical software evidence | PASS: P3-001/P3-011/P4-014/P4-015 |
| RO/RW/WO attribute | no explicit attribute | no explicit attribute | UNKNOWN |
| clear-on-read, latch, FIFO, destructive behavior | no positive contract | no positive contract | UNKNOWN |
| synchronization or exclusion | no complete gma500 protocol | no complete gma500 protocol | UNKNOWN |
| power prerequisite | no minimum SGX state | no minimum SGX state | UNKNOWN |
| clock prerequisite | no register-interface clock state | no register-interface clock state | UNKNOWN |
| reset prerequisite | no accessibility/order contract | no accessibility/order contract | UNKNOWN |
| firmware/MMU/BIF/init prerequisite | historical order exists, requirement does not | historical order exists, requirement does not | UNKNOWN |
| revision and BRN coverage | physical core and BRN set unknown | physical core and BRN set unknown | UNKNOWN |
| unavailable-block behavior | no Poulsbo CPU/bus contract | no Poulsbo CPU/bus contract | UNKNOWN |
| recovery after failure | no applicable recovery contract | no applicable recovery contract | UNKNOWN |

The source warning against dumping registers while SGX is unpowered is not enough to characterize either candidate register, but it prevents treating an unchecked power state as harmless. The missing access attributes and failure contract are independently blocking; a harmless-looking identification value would not resolve them.
