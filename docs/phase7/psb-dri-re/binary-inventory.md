# Binary inventory — checkpoint A

The exact artifact is `references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so`; `realpath` resolves inside this repository. `stat` reports 2,921,100 bytes, mode 0644, and the retained file mtime 2009-05-26 17:25:54 UTC. `file` and `readelf -h` independently report a stripped, dynamically linked ELF32 little-endian Intel 80386 shared object (`ET_DYN`). SHA-256: `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`. This is a file identity, not a hardware result.

The recovered package is `xpsb-glx` 0.18, [spec](../../../references/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx.spec) release `4.1` with i686 build architecture. Its spec labels the package `Redistributable, no modification permitted`; the spec's referenced `COPYING` was not present in the extracted files inspected. The retained source archive SHA-256 is `da180da15c38bb3dec5d70adf263c9953d63e4f88a01726f0fc867fdb7e9fab7`; spec SHA-256 is `60062e6850f6780e0c9a5d458b380f188188ed3ea2b58a0f5651dcd41b64c895`. Distribution metadata and date do not authenticate the binary's original build source.

The reproducible [read-only parser](../../../tools/psb-dri-re/elf_static.py) emits [ELF summary](analysis/elf-summary.json), [dynamic symbols](analysis/dynamic-symbols.csv), [relocations](analysis/relocations.csv), and filtered [strings](analysis/interesting-strings.csv). ELF VAs and file offsets are separate fields. The existing GUI's addresses are displaced by `0x10000` from ELF VAs in the checked examples; scripts retain both addresses and verify the headless image delta independently.

| ELF property | directly parsed value |
|---|---|
| file size | 2,921,100 bytes |
| program headers | 6, including two LOAD segments; executable segment VA `0x00000000`, writable segment VA `0x002b2b04` |
| sections | 26, including `.text` VA `0x0001ea20`, `.rodata` `0x00255000`, `.data.rel.ro` `0x002b2b20`, `.data` `0x002bb1a0` |
| dynamic symbols | 1,741 entries: 104 named undefined imports and 1,636 named exports, plus null entry |
| relocations | 3,925 total: 3,820 `R_386_RELATIVE`, 98 `R_386_JUMP_SLOT`, 7 `R_386_GLOB_DAT` |
| DT_NEEDED | `libm.so.6`, `libpthread.so.0`, `libexpat.so.1`, `libdl.so.2`, `libdrm.so.2`, `libc.so.6` |
| build ID / compiler note | none established by `readelf -n` or section inventory |

Exports include `__driDriverExtensions` at ELF VA `0x002bb1bc` and `__driConfigOptions` at `0x0025de60`, plus many `_mesa_*` and `_ae_*` symbols. The lack of obvious exported `psb_*` symbols is consistent with stripping of local symbols; it does **not** mean the driver-specific code is absent. The dynamic imports include `drmCommandWriteRead`, `drmCommandWrite`, `drmBOCreate/Map/Unmap/SetStatus/WaitIdle`, fence helpers, `mmap`, `dlopen` and `dlsym` (see symbol CSV). Imported names prove linkage, not which requests reach the kernel.

The retained string table includes `Intel(R) GMA500`, `poulsbo`, `psb_ioctl.c`, `psb_outbuf.c`, `psb_use_compiler.c`, `psb_ta.c`, `psb_vs.c`, and assertions referring to relocations, PDS data and generated USSE code. It also includes `2.1 Mesa 7.4` and related version strings. These are binary bytes and strong locators for analysis; a function's behavior is not CONFIRMED until its references and data flow are checked. No matching Poulsbo firmware or source package is implied by these strings.
