# Ownership do hardware

## Dono atual dos recursos

| resource | current Linux owner | observed synchronization | Phase 4 result |
|---|---|---|---|
| PCI function | `psb_pci_driver`/DRM gma500 | PCI lifetime + devm DRM | exclusive owner |
| VDC and SGX MMIO | `drm_psb_private.vdc_reg` and `.sgx_reg` | no global SGX lock | do not access externally |
| GTT MMIO | `gtt_map` | `gtt_mutex` | do not access externally |
| page tables SGX | `psb_mmu_driver` | `sem` + page-table spinlock | do not access externally |
| GEM/GTT mappings | GEM objects | `dma_resv` + `gtt_mutex`; `mmap_mutex` no fault | do not access externally |
| PCI IRQ | `gma_irq_handler`, `IRQF_SHARED` | `irqmask_lock` for mask/handler | shared with display/SGX |
| runtime PM | gma500 + PM core | runtime-PM count | use the driver's own API |

**CONFIRMED — P4-007:** o driver mapeia VDC e SGX a partir do recurso MMIO e
keeps the pointers in `drm_psb_private`; unmounts and unmaps on unload
(`psb_drv.c:254-264,166-210`; `psb_drv.h:377-423`).

**CONFIRMED — P4-008:** insertion/removal of GTT uses `gtt_mutex`; pin/unpin GEM
usa `dma_resv_lock`; faults de mmap usam `mmap_mutex`
(`gtt.c:70-125`; `gem.c:29-108,270-300`).

**CONFIRMED — P4-009:** the MMU maintains a `rw_semaphore` for structures
driver/PD is a spinlock for tables; flush and PD change take the semaphore in
write (`mmu.h:11-24`; `mmu.c:97-135`).

**CONFIRMED — P4-010:** a IRQ agrega identidade VDC, display, hotplug e SGX. O
handler takes `irqmask_lock`, reads SGX status when indicated, and acknowledges events
writing `EVENT_HOST_CLEAR{,2}`; the installation uses `IRQF_SHARED`
(`psb_irq.c:151-247,250-333`).

## Respostas diretas

1. **O gma500 pode acessar SGX simultaneamente a um experimento nosso?** Sim,
Your IRQ, PM, and MMU paths remain active. There is no exclusion for code
external. An independent parallel access is **UNSAFE**.
2. **Which accesses require locks?** GTT: `gtt_mutex`; MMU/PD: `mmu->sem` and, in the
internal routines, page-table spinlock; IRQ/mask: `irqmask_lock`; objects:
`dma_resv`; mmap fault: `mmap_mutex`. A general lock was not found that
   autorize qualquer offset SGX.
3. **Which require GPU powered?** The answer per register is **UNKNOWN**. The API
`gma_power_begin()` only guarantees what the code calls display power
island; there is no audited contract for all SGX clocks.
4. **Is it possible to observe SGX without interfering with KMS?** By the PCI identity and
state already exported: yes. By MMIO SGX: **UNKNOWN/BLOCKED**.
5. **Would a separate module be safe?** **NO-GO**. It would not have the
   `drm_psb_private`, os locks, a contagem PM, o IRQ ou o lifetime dos mappings.
6. **Is performing gma500 safer?** **INFERRED: yes**, for the future
limited access, because this place has ownership and PM. It still requires proof
of read-safety and a whitelist per review.
7. **Is there a risk to the display?** **CONFIRMED:** SGX and display share the
PCI function, BAR0/VDC and the attached handler. Suspend uninstalls the IRQ and puts
the entire function in D3hot; therefore interference is plausible and should be addressed
as a real risk (`power.c:178-203`; `psb_irq.c:198-247`).
8. **Shared resources?** PCI function, base MMIO resource, IRQ, runtime PM,
   GTT/GATT/stolen e lifetime DRM.

## Preferred Future Architecture

For any Phase 5 that is ever unlocked, the security order is:

1. temporary instrumentation compiled within the gma500;
2. specific read-only debug point in the driver itself;
3. userspace via specific ioctl and without offsets only if necessary;
4. separate module — rejected under current conditions.

No option authorizes a generic primitive `read_mmio(offset)` or any
write. The preference is ownership inference, not MMIO approval.

