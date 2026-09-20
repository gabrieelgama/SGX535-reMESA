# Linux gma500 state for Poulsbo

Scope: local Linux checkout `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f`. This document does not state that the kernel running on the target is this one. The compiled set is given by [references/linux/drivers/gpu/drm/gma500/Makefile:1-47](../references/linux/drivers/gpu/drm/gma500/Makefile#L1).

## CONFIRMED — o que existe

| Area | Observed Implementation | Source |
| --- | --- | --- |
| Identification | Intel 8086:8108 and 8109 → psb_chip_ops; SGX535 comment | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:43-59](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L43) |
| PCI/MMIO | SGX window mapping; chip ops selects Poulsbo offset | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:261-264](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L261); [references/linux/drivers/gpu/drm/gma500/psb_device.c:272](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L272) |
| KMS | init modeset, polling, vblank; Poulsbo inicializa LVDS/SDVO | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:366-405](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L366); [references/linux/drivers/gpu/drm/gma500/psb_device.c:18-23](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L18) |
| GEM | dumb buffers, mmap/fault, pin/unpin, pages and stolen | [references/linux/drivers/gpu/drm/gma500/gem.c:29-107](../references/linux/drivers/gpu/drm/gma500/gem.c#L29); [references/linux/drivers/gpu/drm/gma500/gem.c:140-200](../references/linux/drivers/gpu/drm/gma500/gem.c#L140); [references/linux/drivers/gpu/drm/gma500/gem.c:255-300](../references/linux/drivers/gpu/drm/gma500/gem.c#L255) |
| GTT | resources, table and insertion/removal/restoration | [references/linux/drivers/gpu/drm/gma500/gtt.c:78-157](../references/linux/drivers/gpu/drm/gma500/gtt.c#L78); [references/linux/drivers/gpu/drm/gma500/gtt.c:185-287](../references/linux/drivers/gpu/drm/gma500/gtt.c#L185) |
| MMU SGX | PD/PT, flags, insertion/removal of pages, invalidateidation | [references/linux/drivers/gpu/drm/gma500/mmu.c:44-210](../references/linux/drivers/gpu/drm/gma500/mmu.c#L44); [references/linux/drivers/gpu/drm/gma500/gem.c:56-59](../references/linux/drivers/gpu/drm/gma500/gem.c#L56); [references/linux/drivers/gpu/drm/gma500/gem.c:96-98](../references/linux/drivers/gpu/drm/gma500/gem.c#L96) |
| Basic reset | soft reset of the blocks, clear BIF fault and 2D base | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:103-125](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L103) |
| Init SGX parcial | zera bancos BIF, remove bypass, cria PDs, mapeia stolen, programa bases PDS/3D | [references/linux/drivers/gpu/drm/gma500/psb_drv.c:127-163](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L127); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:340-362](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L340) |
| IRQ | vblank/page flip; IRQ SGX 2D e fault BIF; acknowledge | [references/linux/drivers/gpu/drm/gma500/psb_irq.c:115-129](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L115); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:151-230](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L151); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:286-289](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L286) |
| PM | save/restore display and PCI D3hot/D0, reconstruction GTT/GEM | [references/linux/drivers/gpu/drm/gma500/power.c:95-203](../references/linux/drivers/gpu/drm/gma500/power.c#L95) |

Do not summarize this as 'Linux does not touch SGX': it does. Also, do not call it a functional 3D driver just because it contains registers and an MMU.

## CONFIRMED — limites da interface e do fluxo

`psb_ioctls[]` is empty; registered features are `DRIVER_MODESET | DRIVER_GEM`, without DRIVER_RENDER. There is `dumb_create`, but no proper SGX submission interface. [references/linux/drivers/gpu/drm/gma500/psb_drv.c:91-95](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L91); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:493-510](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L493). The Kconfig describes experimental 2D KMS framebuffer, not 3D acceleration. [references/linux/drivers/gpu/drm/gma500/Kconfig:16-19](../references/linux/drivers/gpu/drm/gma500/Kconfig#L16).

`gma_sgx_interrupt` reads blit status and prints faults; there is no completion of 3D jobs in this function. Normal enablement only turns on TWOD_COMPLETE and BIF_REQUESTER_FAULT. [references/linux/drivers/gpu/drm/gma500/psb_irq.c:151-196](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L151); [references/linux/drivers/gpu/drm/gma500/psb_irq.c:286-289](../references/linux/drivers/gpu/drm/gma500/psb_irq.c#L286).

The inventory of uses of EVENT_KICK, USE_CODE_BASE, and slave port 2D in the directory finds definitions, not an emission path; a search for `drm_sched`, `dma_fence`, and `request_firmware` does not find a local implementation. This is evidence limited to the directory and combined with the interface/fluxo above, not universal proof by keywords. [evidence.txt](evidence.txt).

Presence of `dma_resv_lock` on the GEM pin does not equate to a scheduler and rendering fences. [references/linux/drivers/gpu/drm/gma500/gem.c:39-65](../references/linux/drivers/gpu/drm/gma500/gem.c#L39). 2D package defines and scene cookies also do not demonstrate an active acceleration path. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:165-190](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L165); [references/linux/drivers/gpu/drm/gma500/psb_reg.h:522-538](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L522).

## Power and recovery: concrete limitations

**CONFIRMED:** `psb_power_up/down` return zero without operations. The common framework manages PCI/display, and a comment declares broken runtime PM, keeping reference with `pm_runtime_get`. Do not inherit capability from other chip ops to Poulsbo. [references/linux/drivers/gpu/drm/gma500/psb_device.c:186-194](../references/linux/drivers/gpu/drm/gma500/psb_device.c#L186); [references/linux/drivers/gpu/drm/gma500/power.c:46-70](../references/linux/drivers/gpu/drm/gma500/power.c#L46).

**CONFIRMED:** the reset called in init does not contain the microkernel handshake, the CCB processing, or the DDK TI job recovery. Compare [references/linux/drivers/gpu/drm/gma500/psb_drv.c:103-163](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L103) with [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:467-665](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L467) and [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c:544-655](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxreset.c#L544). **UNKNOWN:** how much of the state required by 3D is already left by platform firmware.

## INFERRED — what is missing for SGX535 3D acceleration

The comparison of the interfaces above with the DDK allows identifying gaps, without choosing an implementation now:

- Reproducible SGX535 boot: identification/revision, clocks, reset, and complete scripts; firmware with proper source and license.
- Executable/controllable contexts and buffers with validation, lifetime, coherence, and isolation; GEM scanout alone does not provide this.
- Submission of jobs and ABI kernel/userspace, scheduler, completion, dependencies, timeout, and recovery.
- Command and state formats TA/3D/PDS, parameters/tile buffers, render targets, textures, and PBE.
- ISA/assembler/compiler USSE and issuance of shaders/estados in userspace; only then integration with a graphical/Mesa API.

Comparison sources: [references/linux/drivers/gpu/drm/gma500/psb_drv.c:91-95](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L91); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:505-510](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L505); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:70-201](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L70); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h:85-114](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgxinfo.h#L85); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c:2612-2629](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxinit.c#L2612). The need for each exact component may change if a new design dispenses with parts of the historical protocol; the possibility of this alternative is **UNKNOWN**.
