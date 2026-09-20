# Source Archaeology — Phase 2

## Result

**CONFIRMED:** `sgx535defs.h` exists in the local Git TI KM: blob **`8039da4a73ef9ee3e929edb64244d2891bc9239e`**, 38,928 bytes, 739 lines, found in **255 of the 330 available commits**. The file identifies itself as SGX535 and declares a dual license MIT/GPLv2. [H535:1–43](archaeology-data/H535.txt#L1). The documented absence in phase 1 is true for the `master` examined, not for the entire repository.

**CONFIRMED:** there is also a Poulsbo D0 target with `SGXCORE := 535`, `SGX_CORE_REV := 121`, `PVR_SYSTEM := poulsbo` and `dc_poulsbo`. [PBUILD:42–52](archaeology-data/PBUILD.txt#L42). This allows studying an explicit Intel integration without carrying the OMAP integration.

Convention: **CONFIRMED** confirms the content/history of the sources, not hardware operation; **INFERRED** identifies a deduction; **UNKNOWN** is what has not yet been established. Source identifiers like H535 and PBUILD resolve to repository, full commit, file, and hash in [source catalog](source-archaeology.md#source-catalog). Document snapshots preserve the original lines and license notices; they are not implementation files.

## Examined universe and limitations

| Repository | Physical commits available / reachable by refs | Local, remote refs and tags | Shallow | Unique blobs / binaries omitted from textual search |
| --- | --- | --- | --- | --- |
| `omap5-sgx-ddk-linux` | 330 / 330 | 35 | false | 1082 / 0 |
| `omap5-sgx-ddk-um-linux` | 153 / 153 | 54 | false | 2021 / 1802 |
| `linux` | 1 / 1 | 3 | true | 95387 / 5 |

**CONFIRMED:** there were no additional unreachable commits in the enumerated local repository. Linux has only one commit: the shallow checkout stops the history; there is no local base to conclude when something entered or left gma500. “Not shallow” in both TI repositories does not guarantee coverage of remote branches never cloned, private releases, or history prior to imported root commits. Sources: [summary.json](archaeology-data/summary.json), `*-refs.tsv` and `*-commits.tsv` files in this evidence directory; for Git inventories, lines of code are **N/A**.

The content scope is the entire `/home/gama/sgx535-gfx` project, particularly the three reference repositories. The historical search includes all paths of all available commit trees, not just `hwdefs` or `gma500`. Phase 1 documents do not count as independent evidence. An auxiliary search by names in the home found Mesa and Minecraft files with no demonstrated relation; their content and the history of unrelated projects are not part of this result.

No fetch, checkout, switch, reset, rebase, ref changes, compilation, binary loading, or hardware access was performed. No external source was consulted. HEADs and clean states were checked before and after the search. Git queries and generated artifacts were limited to reading references and writing in `docs/`.

## Reproducible Method

1. List refs and all local objects with `git cat-file --batch-all-objects --batch-check`; select commits and compare with `git rev-list --all`.
2. Enumerate each tree with `git ls-tree -r -z COMMIT`, including directory renames; relate commit, path, and blob.
3. Read each unique blob once with `git cat-file --batch`; search for the eleven patterns case-insensitively. Files with NUL were classified as binary and not submitted to text interpretation. This prevents claiming that a closed blob was examined as source.
4. Save lines found in `*-matches.tsv` and all historical associations in `*-contexts.tsv`. The join by the blob column retrieves **all commits/arquivos** of each occurrence. Also save matches in paths and commit messages.
5. Complement literal search with semantic inspection of the build: `SGXCORE := 535` and `SUPPORT_SGX$(SGXCORE)` are examples that a search limited for SGX535 would miss.
6. Confirm file history by `git log --all --full-history --name-status` and boundary snapshots; record hashes and compare offsets numerically.

The [scan method](archaeology-data/scan-method.py.txt) was preserved as documentary text. It does not execute build scripts or binaries from the references. [Source and hash registry](archaeology-data/source-registry.json).

## Default Result

Counts of **lines per single blob**, not number of commits, word occurrences, or statements about hardware. Overlaps are possible; substring search is deliberately broad. On Linux, USSE/PSB may occur in words or subsystems unrelated to the GPU.

| Standard | IT KM | IT A text | Linux text |
| --- | --- | --- | --- |
| `sgx535defs.h` | 4 | 0 | 0 |
| `SGX535` | 35 | 0 | 3 |
| `SUPPORT_SGX535` | 0 | 0 | 0 |
| `SGX_CORE_REV` | 603 | 0 | 0 |
| `SGX_FEATURE_` | 2503 | 0 | 0 |
| `SGX_BIF` | 76 | 0 | 0 |
| `USSE` | 732 | 0 | 1801 |
| `EURASIA` | 431 | 0 | 0 |
| `Poulsbo` | 179 | 0 | 22 |
| `PSB` | 6568 | 0 | 4233 |
| `GMA500` | 4 | 0 | 28 |

**CONFIRMED:** the literal zero for `SUPPORT_SGX535` is not the absence of the option: the build emits `SUPPORT_SGX$(SGXCORE)`, and the Poulsbo target fixes 535. The resulting expansion is **INFERRED statically**, without running Make. [CORE:488–494](archaeology-data/CORE.txt#L488); [PBUILD:47–50](archaeology-data/PBUILD.txt#L47).

**CONFIRMED:** the textual references to the exact name of the header are included in variants of `sgxdefs.h`; no textual manifest list adding a file hash was found. Git, however, provides hash and unambiguous association to trees. [DEFS:57–61](archaeology-data/DEFS.txt#L57); [matches KM](archaeology-data/omap5-sgx-ddk-linux-matches.tsv); [contexts KM](archaeology-data/omap5-sgx-ddk-linux-contexts.tsv). The UM binary manifests were not fully converted; a negative result regarding their content remains **UNKNOWN**.

## Main historical discoveries

| Discovery | Commit/ref | File and lines | Confidence |
| --- | --- | --- | --- |
| Initial import 1.9 already references 535, but tree does not contain header | KM `1450ae2166ad952ef30197e79518b51577a629e6`; ancestor of master | `services4/srvkm/hwdefs/sgxdefs.h`:54–58; tree in boundary-trees | CONFIRMED |
| First H535 appearance found, experimental DDK 1.13 | KM `322bcda5f3076037e2e20ef9209f4f4d575a7d5f`; ancestor of `origin/dra7/experimental` | [H535:1–43](archaeology-data/H535.txt#L1); [VER13:51–60](archaeology-data/VER13.txt#L51) | CONFIRMED |
| Renaming with identical content for eurasia_km | KM `a24ae6b2573b7eb1dc94473aa9953b964079b5c6`; `origin/dra7/experimental` | H535, R100 paths; lines 1–739 preserved | CONFIRMED |
| Import 1.14 includes header and Poulsbo platform | KM `cb46ba4d0c900f89f7ec0284f9803d476bfa98de`; origin/img-sgx | [PBUILD:42–52](archaeology-data/PBUILD.txt#L42); [PSYS:46–111](archaeology-data/PSYS.txt#L46) | CONFIRMED |
| Another lineage reapplies import 1.14 without eurasia_km prefix | KM `7c89d3433bd96d8b2755ca172e99198fa4b69c05`; ancestor of origin/1.17.4948957/mesa/k6.1 | `services4/srvkm/hwdefs/sgx535defs.h`:1–739, same blob | CONFIRMED |
| Removal of the target and Poulsbo integration, not of H535 | KM `3b6ca1d1f47a951c1f93ba5f9b693b25474d0798` and replay `636e957a340ebba5c10fb8a5c5b3f30b85078c66` | PBUILD and PSYSC, entire file removed; H535 remains | CONFIRMED |

Sources of tree operations (lines of code N/A): [header-history.txt](archaeology-data/header-history.txt), [poulsbo-history.txt](archaeology-data/poulsbo-history.txt), [boundary-trees.txt](archaeology-data/boundary-trees.txt). Author and committer dates were preserved: an old author date in a reapplied commit does not prove that that tree was published in that year.

## Source Catalog

Each ID below is a complete citation retrievable with `git -C references/REPO show COMMIT:FILE`. The `ID:line` references in the documents use **original lines from the blob**, also preserved in the snapshot `.txt`. Confidence of the records: **CONFIRMED** for identity and content; inferences are marked in the consuming text.

| ID | Repository | Commit | Original file | Blob / snapshot |
| --- | --- | --- | --- | --- |
| H535 | `omap5-sgx-ddk-linux` | `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | `services4/srvkm/hwdefs/sgx535defs.h` | `8039da4a73ef9ee3e929edb64244d2891bc9239e` / [H535.txt](archaeology-data/H535.txt) |
| PBUILD | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/eurasiacon/build/linux2/pc_i686_poulsbo_d0_linux/Makefile` | `86c0adfe78965f7d304e912f90a302dcfd4f119c` / [PBUILD.txt](archaeology-data/PBUILD.txt) |
| CORE | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/eurasiacon/build/linux2/config/core.mk` | `55c58fc853dbf4e0c3804cbac3283f1e15e3eabc` / [CORE.txt](archaeology-data/CORE.txt) |
| PSYS | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/system/poulsbo/sysconfig.h` | `5a47b6bad85388eac97da74ba8ca3cf3b19471b0` / [PSYS.txt](archaeology-data/PSYS.txt) |
| PSYSC | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/system/poulsbo/sysconfig.c` | `9c252ce8ae5d5a2de681294ad8bfd338cea95469` / [PSYSC.txt](archaeology-data/PSYSC.txt) |
| PINFO | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/system/poulsbo/sysinfo.h` | `a5539cc7554fb34b283cc27a940096b54eb4ab82` / [PINFO.txt](archaeology-data/PINFO.txt) |
| FEATURE | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxfeaturedefs.h` | `de07cc890c0194d9bc02997176d6eb1eda048811` / [FEATURE.txt](archaeology-data/FEATURE.txt) |
| ERRATA | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxerrata.h` | `e423d4fbaa566fe710c7b44e8cc25efe4a847a8c` / [ERRATA.txt](archaeology-data/ERRATA.txt) |
| DEFS | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxdefs.h` | `7acba793124ebaa2c0c2478d321b030df5362cdc` / [DEFS.txt](archaeology-data/DEFS.txt) |
| MMU | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgxmmu.h` | `a6a907aecb69fecb2c8ba01fb213736943fa3588` / [MMU.txt](archaeology-data/MMU.txt) |
| RESET | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/devices/sgx/sgxreset.c` | `ecf0e6207ca8b104b648fc80924daaf7e184ae48` / [RESET.txt](archaeology-data/RESET.txt) |
| VER19 | `omap5-sgx-ddk-linux` | `1450ae2166ad952ef30197e79518b51577a629e6` | `include4/pvrversion.h` | `ca8bead1cb03245e15d97ed36e98e712d5fe7960` / [VER19.txt](archaeology-data/VER19.txt) |
| VER13 | `omap5-sgx-ddk-linux` | `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | `include4/pvrversion.h` | `f272e696dc577080de853df51ef16c524a896996` / [VER13.txt](archaeology-data/VER13.txt) |
| VER14 | `omap5-sgx-ddk-linux` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/include4/pvrversion.h` | `4fb45c1f330a33dca1be6b7b8785d6025864469d` / [VER14.txt](archaeology-data/VER14.txt) |
| VER17 | `omap5-sgx-ddk-linux` | `b630d462f5fbb86e5f98965ba1af35da1207822f` | `eurasia_km/include4/pvrversion.h` | `6f28525bf332bf7ea8929cf6ba46d3f555f76b49` / [VER17.txt](archaeology-data/VER17.txt) |
| LREG | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_reg.h` | `2a229a0ef36c0d76d9eea29ad75107b5784f4a08` / [LREG.txt](archaeology-data/LREG.txt) |
| LDRVH | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_drv.h` | `db197b865b90a020ac0975c4cc6a5bd00627a29a` / [LDRVH.txt](archaeology-data/LDRVH.txt) |
| LDRVC | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_drv.c` | `d17cb5b4a4bf2d80e8fa0e83dd09cabff5b20fe1` / [LDRVC.txt](archaeology-data/LDRVC.txt) |
| LMMU | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/mmu.c` | `4fbc22a59ac7a9e76a373b65cd7fcf97aa1605ba` / [LMMU.txt](archaeology-data/LMMU.txt) |
| LIRQ | `linux` | `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f` | `drivers/gpu/drm/gma500/psb_irq.c` | `c224c7ff353ce35ba36b3c2b0a4cff4bff478578` / [LIRQ.txt](archaeology-data/LIRQ.txt) |
| UM19 | `omap5-sgx-ddk-um-linux` | `225230e5dae17836a8acf6f6e9fb6e8200af4dea` | `README` | `5284065ebe40bbbfb044e720d325ed7b72390032` / [UM19.txt](archaeology-data/UM19.txt) |
| UMROOT | `omap5-sgx-ddk-um-linux` | `348ea7e08d8b6e7f46f73065ad3ba75614edc12c` | `README` | `9e40cefcb46480543c2413014a4407196c2c9539` / [UMROOT.txt](archaeology-data/UMROOT.txt) |

Snapshots are documentary copies with original notices, not new driver code. Licenses do not automatically propagate from the KM to the UM or from the header to other files. UM binaries remain outside these snapshots. The analysis records provenance, without declaring general permission for reuse.
