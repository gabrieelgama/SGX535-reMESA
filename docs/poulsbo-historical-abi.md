# Poulsbo: Historical ABI and origin

Phase 3 — static analysis, 2026-09-17. `CONFIRMED` means the source declares/implements; `INFERRED` is interpretation; `UNKNOWN` is gap. P3 IDs refer to [matrix](evidence-matrix.csv), with repository, commit, file, and lines. The snapshots preserve the original numbering; see [provenance](poulsbo-data/sources.json). No procedure below was executed on the GPU.

**CONFIRMED [P3-040]** — The historic PSB header declares package 5.0.0.0045 and commands CMDBUF=0, XHW_INIT=1, XHW=2, SCENE_UNREF=3, KMS_OFF=4, KMS_ON=5, HW_INFO=6. The dispatch authenticates CMDBUF and restricts XHW to root. Sources: [PSB_psb_drm_h:32-40](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drm_h:338-361](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drv_c:86-111](poulsbo-data/PSB_psb_drv_c.txt).

**CONFIRMED [P3-041]** — drm_psb_cmdbuf_arg carries lists of buffers/cliprects, scene/fence, handles/offsets/tamanhos TA, OOM, command and relocations, engine and feedback. drm_psb_reloc contains operation, destination, mask, shift and parameters. Defines memory types MMU/PDS/APER/RASTGEOM, engines and fences TA/raster/scene. Sources: [PSB_psb_drm_h:47-54](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drm_h:120-184](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drm_h:206-263](poulsbo-data/PSB_psb_drm_h.txt).

**CONFIRMED [P3-042]** — XHW_INIT receives a buffer handle; the kernel performs lookup/map and calls it a communication buffer with the X server. There are operations fire raster, bind scene, TA memory, reset DPM, OOM, terminate, vistest, resume, and lockup; the ioctl waits for work and uses the shared buffer. Sources: [PSB_psb_drm_h:265-361](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_xhw_c:415-475](poulsbo-data/PSB_psb_xhw_c.txt); [PSB_psb_xhw_c:543-629](poulsbo-data/PSB_psb_xhw_c.txt).

**CONFIRMED [P3-043]** — psb_sgx.c validates the list of buffer objects and applies relocations, checking indices of origem/destino and specific operations PDS/USE. This documents the submission path, not a security audit of the ABI. Sources: [PSB_psb_sgx_c:410-464](poulsbo-data/PSB_psb_sgx_c.txt); [PSB_psb_sgx_c:631-779](poulsbo-data/PSB_psb_sgx_c.txt).

**CONFIRMED [P3-044]** — The historical PSB has a watchdog for lockup and reset workqueue; it queries the XHW channel and requests a DPM reset after MMU reconfiguration. Sources: [PSB_psb_reset_c:145-177](poulsbo-data/PSB_psb_reset_c.txt); [PSB_psb_reset_c:231-279](poulsbo-data/PSB_psb_reset_c.txt).

**CONFIRMED [P3-045]** — DDK 1.14 defines DRM commands Services/display/buffer-class/is-master/unpriv/debug as 0..5 in native mode; DRM_EXT uses DRM_PVR_RESERVED1..6. There are no universal numeric values for the external mode in this header. Sources: [DRMS:43-70](poulsbo-data/DRMS.txt); [DRMC:462-475](poulsbo-data/DRMC.txt).

**CONFIRMED [P3-046]** — PVRSRV_BRIDGE_PACKAGE carries BridgeID, size, ponteiros/tamanhos for input/output, and hKernelServices. ALLOCDEVICEMEM uses device/heap handles, attributes, size, alignment, and chunk information; it is not a modern GEM handle. Sources: [BRIDGE:312-322](poulsbo-data/BRIDGE.txt); [BRIDGE:482-497](poulsbo-data/BRIDGE.txt).

**CONFIRMED [P3-047]** — sgx_bridge.h defines DOKICK at SGX_CMD_BASE+3; TRANSFER +13; INFO_FOR_SRVINIT +15; DEVINITPART2 +16; context registers render +20, 2D +24, transfer +26; SUBMIT2D +23 and PROCESS_QUEUES +28, with build conditionals. Sources: [SBRIDGE:63-114](poulsbo-data/SBRIDGE.txt); [SBRIDGE:233-273](poulsbo-data/SBRIDGE.txt); [SBRIDGE:315-351](poulsbo-data/SBRIDGE.txt).

**CONFIRMED [P3-048]** — SGX_BRIDGE_INIT_INFO receives handles CCB/controle/event-kicker/host-control/TA3D, handler addresses, scripts, build options, struct sizes, data from clock/cache, and additional handles. SGX_CCB_KICK aggregates command, handle/offset CCB, and synchronization objects TA/3D. Sources: [INFO:83-153](poulsbo-data/INFO.txt); [INFO:181-251](poulsbo-data/INFO.txt).

**CONFIRMED [P3-049]** — SGXMKIF_COMMAND contains service address USE, cache control, and six data words; the kernel CCB contains 256 commands and read/write control offset. The producer checks space and advances the write offset modulo 256. Sources: [MKIF:70-96](poulsbo-data/MKIF.txt); [UTIL:236-254](poulsbo-data/UTIL.txt); [UTIL:552-558](poulsbo-data/UTIL.txt).

**CONFIRMED [P3-050]** — After publishing the command, the Services path updates the event kicker, uses a memory barrier, and writes EVENT_KICK2 under FIX_HW_BRN_26620 + SYSTEM_CACHE without bypass, and EVENT_KICK in the alternative branch. The cited SGX535 definition does not enable MULTI_EVENT_KICK. Sources: [UTIL:603-633](poulsbo-data/UTIL.txt); [FEATURE:76-88](poulsbo-data/../archaeology-data/FEATURE.txt).

**CONFIRMED [P3-051]** — SGXInitialise executes part1 script, reset, part2 script, kick, and waits for PVRSRV_USSE_EDM_INIT_COMPLETE in shared memory; the scripts come from SGX_BRIDGE_INIT_INFO. This evidence does not provide the content of the microkernel or the scripts filled by the UM. Sources: [INIT:207-220](poulsbo-data/INIT.txt); [INIT:534-574](poulsbo-data/INIT.txt); [INIT:637-730](poulsbo-data/INIT.txt); [INFO:83-100](poulsbo-data/INFO.txt).

**CONFIRMED [P3-052]** — The Poulsbo display-class defines ENTER_VT=1, LEAVE_VT=2, and CURSOR_LOAD=3, struct with cmd/dev-id and cursor of tamanho32/pointer64. Sources: [DCS:46-68](poulsbo-data/DCS.txt).

**CONFIRMED [P3-053]** — EMGD emgd_shared.h sets PVR_RESERVED1..5 in 0x12..0x16 and RESERVED6 in 0x1e. emgd_drm.h defines GMM_ALLOC_REGION=0x0e, ALLOC_SURFACE=0x0f, FREE=0x10, FLUSH_CACHE=0x11, DRIVER_PRE_INIT=0x23, START_PVRSRV=0x25, and PREINIT_MMU=0x39. Sources: [EMGD_drm_include_emgd_shared_h:50-59](poulsbo-data/EMGD_drm_include_emgd_shared_h.txt); [EMGD_drm_include_emgd_drm_h:679-763](poulsbo-data/EMGD_drm_include_emgd_drm_h.txt).

**CONFIRMED [P3-054]** — emgd_drm_start_pvrsrv_t contains xserver and rtn, described as the return of PVRSRVDrmLoad. The pvrversion.h of the mirror declares 1.5.15.3226: do not confuse this internal version of PVR with EMGD 1.14 or TI DDK 1.14. Sources: [EMGD_drm_include_emgd_drm_h:638-647](poulsbo-data/EMGD_drm_include_emgd_drm_h.txt); [EMGD_drm_pvr_include4_pvrversion_h:45-53](poulsbo-data/EMGD_drm_pvr_include4_pvrversion_h.txt).

## Three interfaces, not a single ABI

| Family | Entry | Objetos/controle | Reconstruction limit |
|---|---|---|---|
| PSB 5.0.0.0045 | DRM CMDBUF / XHW | BOs, relocations, scenes, fences; dependency on X server | private userspace handlers not rebuilt |
| DDK TI 1.14 target Poulsbo | bridge DRM Services | heaps, handles, contexts, syncs, scripts/CCB/host-control | incomplete DRM_EXT integration and missing UM |
| Mirror EMGD / PVR 1.5.15.3226 | IGD + PVR slots | display/GMM + Services/init | does not show binary ABI compatible with TI1.14 |

The numbers above are relative indices according to each definition; a command number is not the full Linux ioctl. Direction, size, DRM_COMMAND_BASE, native pointers, IMG_HANDLE, IMG_SIZE_T, and build macros are part of the ABI. Do not invent sizeof/packing, nor the equivalent x86/ARM or 32/64 bits.

**UNKNOWN:** complete snapshot of the PSB XHW userspace side; options generated from the installed build; exact layouts of firmware objects; content/license/revision of Poulsbo microkernels; filled scripts and full UM/KM handshake. The XHW channel should not be called firmware: it is kernel/X server communication, according to P3-042. The presence of MSVDX video firmware in other packages would not identify SGX firmware.

**INFERRED:** a modern driver would need to design its own validation, lifetime, VA isolation, fences, and recovery. This is not authorization to implement or reuse the historical ABI at this stage.

## License and provenance

Decisions below are the material separation policy of this project, based on notices per file. Do not assign a single license to the EMGD directory or infer rights from public availability.


**CONFIRMED [P3-055]** — sgx535defs.h preserves dual warning MIT/GPLv2, with MIT option and maintenance of the warnings. Sources: [H535:1-40](poulsbo-data/../archaeology-data/H535.txt).

**CONFIRMED [P3-056]** — gtt.c currently has SPDX GPL-2.0-only; psb_drm.h history carries GPL version 2. Do not transplant code from these files into a future Mesa implementation under a permissive license. Sources: [GTT:1-8](poulsbo-data/GTT.txt); [PSB_psb_drm_h:1-21](poulsbo-data/PSB_psb_drm_h.txt).

**CONFIRMED [P3-057]** — The headers EMGD emgd_drm.h and SGX535 contain permissive notices with a requirement to preserve copyright/license. License.txt also separates the DRM kernel and points to GPLv2; handle the combination by file and origin, without assuming a global license. Sources: [EMGD_drm_include_emgd_drm_h:1-30](poulsbo-data/EMGD_drm_include_emgd_drm_h.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:1-21](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt); [EMGD_License_txt:1-9](poulsbo-data/EMGD_License_txt.txt).

**CONFIRMED [P3-058]** — The userspace license in readme.txt EMGD allows unmodified binaries, restricts hardware, and prohibits reverse engineering/decompilation/disassembly; License.txt also contains conditions for binary redistribution. Sources: [EMGD_readme_txt:1-16](poulsbo-data/EMGD_readme_txt.txt); [EMGD_License_txt:32-42](poulsbo-data/EMGD_License_txt.txt).


| Class | Use in this phase | Policy for future implementation |
|---|---|---|
| Code with origem/commit/aviso preserved | software behavior documentation | review individual license before copying |
| dual IT MIT/GPLv2, permissive headers IMG/Intel identified | technical candidates, no port at this stage | potentially reusable by the permissive option, preserving warnings and checking origin |
| Linux GPL-only and PSB GPL | evidence and comparison | do not copy implementation to permissive Mesa; maintain documentary separation |
| EMGD with provenance only from mirror | evidence of the mirror's content | authenticity/license of the exact version still needs confirmation |
| ONE TI and ONE EMGD binaries | historical inventory, license, and metadata | do not copy payload, disassemble, execute, or load at this stage |
| Intel Documents | context and evidence of what they state | are not firmware license nor full core description |

The `.txt` in poulsbo-data are reference snapshots with original warnings, not new driver code. Their licenses are not replaced by the documentation license. No external binaries were imported.


**CONFIRMED [P3-064]** — The community mirror README presents itself as EMGD 1.18 and claims to gather binaries, sources, and patches; it is not an Intel release authenticated by the project. [EMGD README:1–7](poulsbo-data/EMGD_README_md.txt). Distinguish this label, the internal PVR 1.5.15.3226 (P3-054), and the official feature document EMGD1.14 (P3-060).
