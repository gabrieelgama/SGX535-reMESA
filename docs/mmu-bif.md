# MMU e BIF

> Phase 2 update: local history contains `sgx535defs.h` and an explicit Poulsbo integration in DDK 1.14. The absence references below describe the Phase 1 master checkout. See [archaeology](source-archaeology.md), [recovered files](sgx535-missing-files.md), and [Poulsbo comparison](poulsbo-evidence.md) for the expanded state.

## CONFIRMED — format described by sources

The DDK defines 4 KiB pages (`PAGE_SHIFT=12`), 10-bit indices PD/PT, masks `0xffc00000` and `0x003ff000`. The branch without `SGX_FEATURE_36BIT_MMU` uses address PDE/PTE `0xfffff000` with no additional offset. The SGX535 selection declares a 32-bit VA and does not enable this 36-bit feature. Do not confuse virtual width with the physical capacity of the entire integration. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h:48-93](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h#L48); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:63-73](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L63).

Linux uses PD index `va >> 22` and PT index `(va >> 12) & 0x3ff`; composes PTE with shifted PFN and flags. [references/linux/drivers/gpu/drm/gma500/mmu.c:44-52](../references/linux/drivers/gpu/drm/gma500/mmu.c#L44); [references/linux/drivers/gpu/drm/gma500/mmu.c:146-158](../references/linux/drivers/gpu/drm/gma500/mmu.c#L146); [references/linux/drivers/gpu/drm/gma500/psb_drv.h:75-84](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L75).

| Field | DDK | Linux Poulsbo | Confidence |
| --- | --- | --- | --- |
| valid | bit 0 | bit 0 | CONFIRMED in both sources |
| write-only | bit 1 | bit 1 | CONFIRMED in both sources |
| read-only | bit 2 | bit 2 | CONFIRMED in both sources |
| cache | bit 3, CACHECONSISTENT | bit 3, CPU cache coherent comment | CONFIRMED as definition; unknown full coherence |
| EDM protect | bit 4 | no equivalent in these defines | CONFIRMED only in the generic TI header |

Table sources: [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h:66-93](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxmmu.h#L66); [references/linux/drivers/gpu/drm/gma500/psb_drv.h:81-84](../references/linux/drivers/gpu/drm/gma500/psb_drv.h#L81). The TI header also lists PDE sizes from 16 KiB to 4 MiB; **UNKNOWN** if they are usable in SGX535/Poulsbo. The presence of these defines in a common file does not prove core support.

**INFERRED:** with the 4 KiB format, 1024 PDEs × 1024 PTEs × 4096 bytes cover 4 GiB of VA, with each PT covering 4 MiB. It is arithmetic over the fields above, not the amount of RAM.

## Contexts and important divergence

**CONFIRMED:** the DDK SGX535 declares 16 directory lists; `SGX_BIF_DIR_LIST_INDEX_EDM` selects the last one, therefore index 15 in this configuration. The reset associates EDM and 2D to the kernel context; under BRN23410, it also associates TA. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:67-69](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L67); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:357-361](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L357); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:164-203](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L164).

**CONFIRMED:** Linux uses context 0 for standard PD and 1 for `pf_pd`; PDs allocated with `trap_pagefaults=1` use zero invalidateid entries. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:340-359](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L340); [references/linux/drivers/gpu/drm/gma500/mmu.c:160-210](../references/linux/drivers/gpu/drm/gma500/mmu.c#L160).

The formulas do not match:

| Source | Register selection |
| --- | --- |
| TI | base0 for 0; base1 + `4*(index-1)` for the others |
| Linux | base0 for 0; base1 + `4*hw_context` for the others |

[references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:121-135](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L121); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:183-203](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L183); [references/linux/drivers/gpu/drm/gma500/mmu.c:123-135](../references/linux/drivers/gpu/drm/gma500/mmu.c#L123).

In Linux, `BASE1=0x0c38`, so that context 1 writes in **0x0c3c**; this account is **INFERRED directly from the expression**, and not a suggested correction. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:126](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L126). **UNKNOWN:** convention difference, historical definition, or defect; do not choose the TI formula based on similarity. Phase 2 retrieved the SGX535 header and Phase 3 confirmed that the divergence remains; see `poulsbo-vs-ti.md` and `phase4-audit.md`.

## Addresses and mapping

**CONFIRMED:** Linux keeps GTT and SGX MMU separate. GEM pin gets pages, marks WC, inserts into GTT and SGX PD in `gatt_start + offset`. Unpin removes from both and restores WB. GEM storage is limited to DMA32 pages. [references/linux/drivers/gpu/drm/gma500/gem.c:29-107](../references/linux/drivers/gpu/drm/gma500/gem.c#L29); [references/linux/drivers/gpu/drm/gma500/gem.c:175-178](../references/linux/drivers/gpu/drm/gma500/gem.c#L175).

**CONFIRMED:** the initialization inserts stolen memory into the standard PD, writes `PDS_EXEC_BASE=0x20000000` and `BIF_3D_REQ_BASE=0x30000000`. `gtt.c` still holds `mmu_gatt_start=0xe0000000`, separate from `gatt_start` obtained from the PCI resource. Do not use these names as synonyms. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:352-362](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L352); [references/linux/drivers/gpu/drm/gma500/gtt.c:185-250](../references/linux/drivers/gpu/drm/gma500/gtt.c#L185).

**CONFIRMED:** the OMAP5 integration implements CPU ↔ system ↔ device conversions as identity, making the UMA assumption explicit. This does not establish a universal rule for SGX. [references/omap5-sgx-ddk-linux/eurasia_km/services4/system/omap5/sysconfig.c:827-935](../references/omap5-sgx-ddk-linux/eurasia_km/services4/system/omap5/sysconfig.c#L827).

The DDK separates heaps of data, 3D parameters, TA, sync, PDS code/data, kernel code/data, and shaders. In the 32-bit branch without BRN31620, examples are sync `0xef000000`, kernel code `0xf2000000`, kernel data `0xf4000000 + offset`, pixel shader `0xf9000000`, vertex shader `0xfe000000`. **CONFIRMED as DDK policy**, not fixed hardware physical addresses. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h:67-68](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h#L67); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h:195-248](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h#L195).

## Invalidation and synchronization

**CONFIRMED:** the TI MMU accumulates invalidateidation flags PD/PT in `ui32CacheControl`; the scheduler transfers them to the command and clears the accumulator. The flags are requested from the microkernel, not MMIO offsets. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/mmu.c:591-643](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/mmu.c#L591); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxutils.c:454-466](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxutils.c#L454); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:367-370](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L367).

**CONFIRMED:** Linux invalidateidates by `BIF_CTRL`, alternating INVALDC or FLUSH and performing a barrier and return read. It has CPU path `clflush`; this does not prove coherence of all shader data and caches. [references/linux/drivers/gpu/drm/gma500/mmu.c:54-120](../references/linux/drivers/gpu/drm/gma500/mmu.c#L54).

## Faults e recovery

**CONFIRMED:** the Linux handler reads the BIF status and fault address, distinguishes page fault/protection, and prints the requestor. Then it clears the events. There is no demand paging or job replay there. [references/linux/drivers/gpu/drm/gma500/psb_irq.c:151-196](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L151). The insertion of PTE in IRQ is only TODO in [references/linux/drivers/gpu/drm/gma500/mmu.c:35-42](../references/linux/drivers/gpu/drm/gma500/mmu.c#L35).

**CONFIRMED:** the non-MP TI reset drains requests through PD/PT/temporary page and repeats until there is no fault. The comment about 2 GiB relative address and bus-master MSB is in this specific context; **UNKNOWN** describes Poulsbo, so it should not become a PTE rule. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:544-618](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L544).

**UNKNOWN:** end-to-end ordering, invalidateidation security with active engines, isolation between contexts, EDM protect policy and real DMA limits on the target. Resolve before allowing SGX to touch new memory.

## Mapping layers in the DDK IT

**CONFIRMED:** the buffer manager reserves device VA via `pfnMMUAlloc` and chooses mapping functions according to the memory source. In the contiguous branch, it passes the converted physical address and destination VA to `pfnMMUMapPages`; there are separate branches sparse/shadow. This distinguishes GPU VA allocation from CPU mapping. [services4/srvkm/common/buffer_manager.c:2294–2380](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/common/buffer_manager.c#L2294).

**CONFIRMED:** `OSMapPhysToLin` requires `PVRSRV_HAP_KERNEL_ONLY` and uses ioremap/LinuxMemArea wrappers to produce VA CPU. It is not the function that creates the BIF translation. [services4/srvkm/env/linux/osfunc.c:1647–1687](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/env/linux/osfunc.c#L1647).

**CONFIRMED, OMAP integration:** `create_gem_wrapper`, in branch `SUPPORT_DRI_DRM_EXTERNAL`, converts LinuxMemArea into pages or physical region, translates flags cached/WC/uncached into `OMAP_BO_*` flags, and calls `omap_gem_new_ext`. It is a concrete dependency for TI integration within Linux environment code, not only in `services4/system/omap5`. [services4/srvkm/env/linux/mmap.c:372–482](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/env/linux/mmap.c#L372). **UNKNOWN:** equivalent policy to share rendering BOs with Poulsbo scanout. Do not port this wrapper to Intel just for using GEM.
