# USSE / USE / PDS

> Phase 2 update: local history contains `sgx535defs.h` and an explicit Poulsbo integration in DDK 1.14. The absence references below describe the Phase 1 master checkout. See [archaeology](source-archaeology.md), [recovered files](sgx535-missing-files.md), and [Poulsbo comparison](poulsbo-evidence.md) for the expanded state.

## CONFIRMED

The SGX535 configuration declares two USE pipes; SGX543/544 enable flags like `USE_NO_INSTRUCTION_PAIRING` and `USE_UNLIMITED_PHASES` that do not appear in the SGX535 branch. This prevents automatically importing ISA assumptions from newer cores. [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h:63-118](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h#L63).

Linux defines 16 USE bases from `0x0a0c` to `0x0a48`, DM field in bits 26:25 with names VERTEX=0, PIXEL=1, RESERVED=2, EDM=3, base field in bits 24:0 and alignshift 7. The address comment contains a question mark; the macro value is confirmed, but the physical interpretation requires further confirmation. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:87-114](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L87).

The PDS_EXEC_BASE record is in `0x0ab8`, with shift/alinhamento declared as 20 bits; Linux writes `0x20000000` during init. This does not document PDS instructions. [references/linux/drivers/gpu/drm/gma500/psb_reg.h:116-118](../references/linux/drivers/gpu/drm/gma500/psb_reg.h#L116); [references/linux/drivers/gpu/drm/gma500/psb_drv.c:361](../references/linux/drivers/gpu/drm/gma500/psb_drv.c#L361).

The kernel command has `ui32ServiceAddress` described as the address of the USE handler. The host control uses names `PVRSRV_USSE_EDM_*`, and the DDK reserves distinct heaps for kernel code, pixel/vertex shaders, and PDS code/data. [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:70-75](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L70); [references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h:289-306](../references/omap5-sgx-ddk-linux/eurasia_km/services4/include/sgx_mkif_km.h#L289); [references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h:224-248](../references/omap5-sgx-ddk-linux/eurasia_km/services4/srvkm/devices/sgx/sgxconfig.h#L224).

## INFERRED

The microkernel of this model uses the USE/USSE/EDM execution infrastructure, instead of having evidence here of a separate firmware CPU. The inference relies on the USE handler address and the USSE_EDM fields above. It does not specify by itself how PDS triggers the program, which registers initialize it, or how instructions are encoded.

## UNKNOWN

No source for assembler/disassembler USSE, full opcode description, shader compiler, or source of programs microkernel/PDS was found in the two TI trees. The inventory in [evidence.txt](evidence.txt) allows repeating this search. `libusc` and `libglslcompiler` are UM artifacts; their name is not ISA documentation. The package describes itself as libraries/binaries for OMAP. [references/omap5-sgx-ddk-um-linux/README:20-25](../references/omap5-sgx-ddk-um-linux/README#L20).

Missing: size/effective encoding of SGX535 instructions; pairing and hazards; register banks and operands; branches/predication; task terminators; cargas/stores and addressing; cache synchronization; texture and sampler descriptors; export of vertex/pixel and PBE interface. We do not assign values to these fields.

## Next investigation

Search for authorized and specific publication SGX535 of USE/USSE and PDS, including review. First examine the KM contracts already available and the origin of `sgx535defs.h`; do not use SGX544 as a substitute specification. The UM metadata inventoried in [firmware.md](firmware.md) serve to identify components, not to import payload. The decision to write an assembler or compiler should wait for the minimum execution specification and an appropriate license for the new sources.
