# Retained Xpsb binary

The existing artifact is [`drivers/Xpsb.so`](../../../references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so) from the retained `xpsb-glx` 0.18 archive. `realpath`, `stat`, `file`, SHA-256, `readelf`, `llvm-objdump`, a byte-only parser and a **separate disposable** Ghidra headless project under `/tmp/sgx535-xpsb-ghidra` were used. No loader invoked the ELF. Its initial SHA-256 is `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f` (P7C-001). The final hash is checked in [the gap review](gap-analysis.md).

`realpath` resolved to `/home/gama/sgx535-gfx/references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so`. Initial `stat` reported mode `0644`, 58,888 bytes and retained modification time `2009-05-26 17:25:54 UTC`.

| property | byte-derived result |
|---|---|
| size | 58,888 bytes |
| format | stripped ELF32 little-endian Intel i386 shared object (`ET_DYN`) |
| direct `DT_NEEDED` | `libpthread.so.0`, `libc.so.6` |
| ELF symbols | 163 dynamic entries: 59 imports, 103 exports, one null entry |
| relocations | 73 |
| exported roots | `XpsbInit` `0x2690`, `XpsbTakeDown` `0x2f10`, `XpsbThread` `0x30f0`, `Xpsb_sgx_initialize` `0x3820`, `XpsbFlush3D` `0x9f70`, `XpsbModuleData` `0xf100` |

The ELF imports `drmBO*`, `drmFence*` and other DRM helpers without a direct `DT_NEEDED` entry for libdrm. Their runtime provider is therefore not identified by this ELF's dependency list alone; the exact paired libdrm binary remains UNKNOWN.

Addresses in this track are **ELF virtual addresses**. The executable segment uses matching file offsets; data in the second LOAD segment does not. The [machine-readable ELF inventory](analysis/elf-summary.json), [symbols](analysis/dynamic-symbols.csv), [relocations](analysis/relocations.csv), and [filtered strings](analysis/interesting-strings.csv) are reproducible with [`elf_static.py`](../../../tools/xpsb-re/elf_static.py). The original Ghidra GUI project was not used. [`DecompileXpsb.java`](../../../tools/xpsb-re/DecompileXpsb.java) decompiled 40 selected functions into `/tmp` for review; decompiled implementation text is not copied into this repository. The [range/XREF trace](../../../tools/xpsb-re/TraceStaticBuffer.java) reviewed 246 non-external functions sequentially.

The package [spec](../../../references/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx.spec) calls its license “Redistributable, no modification permitted.” I treat the binary as historical evidence, not implementation source. The source package's version/date do not authenticate a particular Xorg DDX, libdrm or kernel binary.
