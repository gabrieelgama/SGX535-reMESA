# Generic Mesa and Poulsbo boundary

The binary is not a small standalone SGX layer. Of 1,636 named dynamic exports, 1,070 start with `_mesa_`; other exported namespaces include `_tnl_`, `_swrast_` and `_math_` (parsed [dynamic symbol list](analysis/dynamic-symbols.csv)). Source strings include common Mesa module names and functions, as well as the Poulsbo-specific `psb_*` file and assertion names in [filtered strings](analysis/interesting-strings.csv). No named dynamic export starts with `psb` or `intel`; stripped local symbols must be recovered by their addresses and references.

| region or evidence | classification | confidence and limit |
|---|---|
| named `_mesa_*`, `_tnl_*`, `_swrast_*` exports | GENERIC_MESA | CONFIRMED symbol namespace; individual call behavior still requires inspection |
| `__driDriverExtensions` and its ABI callbacks | DRI_GLUE | CONFIRMED descriptor roles from [historical ABI](dri-extension-map.md); much of the wrapper resembles Mesa 7.4.4 `dri_util.c` |
| `psb_ioctl.c`, `psb_outbuf.c`, `psb_scene*.c`, `psb_use_compiler.c` assertion XREFs and reviewed bodies | POULSBO_USERSPACE / DRM_PSB | CONFIRMED selected functions in the [function map](function-map.csv); unreviewed neighbors remain UNCERTAIN |
| `Intel(R) GMA500` and `poulsbo` strings | POULSBO_USERSPACE locator | CONFIRMED strings; no hardware measurement follows |
| `psb_vs.c`, `psb_ta.c`, `psb_use_compiler.c`, `usc/hw.c` XREFs and reviewed generation paths | SGX_SPECIFIC | CONFIRMED historical PDS-related and USC/USSE generation behavior in [the focused report](sgx-specific-findings.md); target revision and full encodings UNKNOWN |
| remaining stripped functions | UNCERTAIN | proximity to a string or address range alone is insufficient |

`DT_NEEDED` contains `libdrm.so.2` but no named PowerVR Services library. An optional `libtxc_dxtn.so` string appears in Mesa texture-compression error text; whether that path is used in any particular application is UNKNOWN. This says only what the ELF declares and strings mention. It does not establish that the binary contains every component needed for the historical graphics stack: the package also contains `Xpsb.so`, and the PSB kernel ABI and version compatibility remain separate evidence questions.

The clean-implementation boundary is specification and observed data layout. Decompiled proprietary functions remain research evidence; this investigation will not translate them into a new Mesa source tree.
