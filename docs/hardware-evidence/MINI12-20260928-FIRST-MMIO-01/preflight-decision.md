# Candidate and exact address derivation before target contact

**Gate B: BLOCKED. Whitelist: `[]`. Experiment: OPERATOR-AUTHORIZED despite unresolved Gate B requirements.** This is not a gate pass.

| Field | Proposed exact value | Evidence |
| --- | --- | --- |
| Target | Dell Inspiron 1210, PCI `0000:00:02.0`, `8086:8108` / subsystem `1028:02b1` | [H0 identity and PCI text](../MINI12-20260927-H0/results.md), [P1 re-verification](../MINI12-20260928-GATEB-P1/results.md); fresh preflight still required |
| PCI BAR | BAR0, start `0xd8100000`, end `0xd817ffff` | H0 `raw/baseline/pci_text.stdout`; P1 resource text matched byte for byte |
| SGX aperture | BAR0 + `0x40000` = `0xd8140000`, mapped length `0x8000` | [installed-module static check](../MINI12-20260927-H0/installed-module-static-check.md): `8086:8108` selects `psb_chip_ops`, offset `0x40000`, `ioremap` size `0x8000` |
| Register | `CORE_ID` | Retained SGX535 header in `docs/archaeology-data/H535.txt:89–94`; retained historical PSB use in `docs/poulsbo-data/PSB_psb_drv_c.txt:325–340` |
| Register offset | SGX-relative `0x0010` | SGX535 header; archived antiX `psb_reg.h:31` |
| Access width | 32 bits, historical/software evidence only | Historical `PSB_RSGX32`, archived antiX `psb_drv.h:895–905` expands to `ioread32`; target-revision validity remains UNKNOWN |
| Derived physical address | `0xd8100000 + 0x40000 + 0x10 = 0xd8140010` | Checked arithmetic; 4-byte aligned, within BAR0 and the mapped SGX aperture |

The preferred candidate mechanism, **if available and verified**, is a read-only mapping of exactly one BAR0 page at resource offset `0x40000` through `/sys/bus/pci/devices/0000:00:02.0/resource0`, followed by one native aligned volatile 32-bit load at mapping offset `0x10`. `resource0`, not `resource0_wc`, would be used. The mapping path and generated machine code must be checked before execution. `open(O_RDONLY)`, `mmap(PROT_READ, MAP_SHARED)`, one load, `munmap` and close require no intentional device write. A mapping is not a range dump. No other offset may be dereferenced. This is a **conditional mechanism**, not yet approved for execution; target permission, tool availability and generated instruction remain to be checked.

Known unresolved assumptions include register read attributes and side effects; SGX internal power, clock and reset state; physical core revision/errata; driver mapping lifetime; concurrency and PM exclusion; CPU-visible MMIO failure bound; and validated recovery. Healthy SSH and PCI `active` do not close them. The operator has explicitly authorized one attempt despite this blocked gate, but no additional SGX operation.

## Preflight decision

The fresh pinned-SSH [preflight](results.md) reverified the target and current BAR/driver/module state. It reported `euid=1000`; BAR0 `resource0` is owned by uid/gid `0:0`, mode `0600`, `read_access=false`; `/dev/mem` is owned by `0:15`, mode `0640`, `read_access=false`. The user groups reported by the target do not include gid `15`. Both routes therefore lack read access for the retained unprivileged SSH user. No file open, mapping, compilation or MMIO access was attempted. Trying an unverified alternative, elevating privilege, or changing permissions would exceed this prepared one-load mechanism. **Stop without executing.** The hardware-read attempt count is zero.
