# Hardware ownership

## Current owners

| resource | current Linux owner | observed synchronization | Phase 4 result |
|---|---|---|---|
| PCI function | `psb_pci_driver` / DRM gma500 | PCI lifetime plus managed DRM lifetime | exclusive owner |
| VDC and SGX MMIO | `drm_psb_private.vdc_reg` and `.sgx_reg` | no global SGX lock | do not access externally |
| GTT MMIO | `gtt_map` | `gtt_mutex` | do not access externally |
| SGX page tables | `psb_mmu_driver` | `sem` plus page-table spinlock | do not access externally |
| GEM/GTT mappings | GEM objects | `dma_resv` plus `gtt_mutex`; `mmap_mutex` on faults | do not access externally |
| PCI IRQ | `gma_irq_handler`, `IRQF_SHARED` | `irqmask_lock` for mask/handler paths | shared by display and SGX |
| runtime PM | gma500 plus PM core | runtime-PM reference count | use the driver's PM path |

**CONFIRMED — P4-007:** the driver maps VDC and SGX from the PCI MMIO resource, stores both pointers in `drm_psb_private`, and unmaps them during unload (`psb_drv.c:254-264,166-210`; `psb_drv.h:377-423`).

**CONFIRMED — P4-008:** GTT insertion/removal uses `gtt_mutex`; GEM pin/unpin uses `dma_resv_lock`; mmap faults use `mmap_mutex` (`gtt.c:70-125`; `gem.c:29-108,270-300`).

**CONFIRMED — P4-009:** the MMU has an `rw_semaphore` for driver/PD structures and a spinlock for page tables. Flush and PD changes take the semaphore for writing (`mmu.h:11-24`; `mmu.c:97-135`).

**CONFIRMED — P4-010:** one IRQ aggregates VDC identity, display, hotplug, and SGX. The handler takes `irqmask_lock`, reads SGX status when indicated, and acknowledges events by writing `EVENT_HOST_CLEAR{,2}`. Registration uses `IRQF_SHARED` (`psb_irq.c:151-247,250-333`).

## Consequences for a future read

1. gma500 may access SGX concurrently with an external experiment through its IRQ, PM, and MMU paths. No exclusion protocol exists for outside code, so independent parallel access is **UNSAFE**.
2. Known locks are resource-specific: `gtt_mutex`, `mmu->sem` and the page-table spinlock, `irqmask_lock`, `dma_resv`, and `mmap_mutex`. No general lock authorizes arbitrary SGX offsets.
3. Per-register power requirements remain **UNKNOWN**. `gma_power_begin()` documents display-island handling, not a contract for every SGX clock.
4. PCI identity and kernel-exported state can be observed without interfering with KMS. SGX MMIO observation remains **UNKNOWN/BLOCKED**.
5. A separate module is **NO-GO**. It would lack `drm_psb_private`, the driver's locks, PM reference, IRQ relationship, and mapping lifetime.
6. If a limited read is ever approved, placing it within gma500 is **INFERRED** to be safer because ownership and PM are already there. Read safety and a revision-specific whitelist would still have to be proven.
7. SGX and display share the PCI function, BAR0/VDC resource, and IRQ handler. Suspend removes the IRQ and puts the complete function into D3hot (`power.c:178-203`; `psb_irq.c:198-247`). Interference with display is therefore a real risk.
8. Shared resources include the PCI function, base MMIO resource, IRQ, runtime PM, GTT/GATT/stolen memory, and DRM lifetime.

## Preferred architecture

If a later gate authorizes a read, the preferred order is temporary, fixed-purpose instrumentation within gma500; then a similarly narrow read-only driver interface if required. A separate PCI module remains rejected. No option permits `read_mmio(offset)`, arbitrary offsets, or writes. This ownership preference does not authorize MMIO.
