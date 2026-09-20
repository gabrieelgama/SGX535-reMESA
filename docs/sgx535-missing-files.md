# SGX535: retrieved files and remaining gaps

Convention: **CONFIRMED** confirms the content/history of the sources, not hardware operation; **INFERRED** identifies a deduction; **UNKNOWN** is what has not yet been established. Source identifiers like H535 and PBUILD resolve to repository, full commit, file, and hash in [source catalog](source-archaeology.md#source-catalog). Document snapshots preserve the original lines and license notices; they are not implementation files.

## sgx535defs.h: answer to the five questions

| Question | Answer | Evidence / confidence |
| --- | --- | --- |
| Did it exist in commit? | Yes, in 255 available commits | H535, blob `8039da4a73ef9ee3e929edb64244d2891bc9239e`; [H535:1–43](archaeology-data/H535.txt#L1); CONFIRMED |
| Was it removed? | No header removal found; it was not present in import 1.9/master | header-history and boundary-trees; Git operation, lines N/A; CONFIRMED in this universe |
| Is it in branch/tag? | Present in the tip of 13 remote-tracking branches; absent in the tip of all 12 available KM tags | header-by-ref, inventory lines; CONFIRMED |
| Is it referenced by build? | Dispatcher includes the header; Poulsbo target defines core 535/rev121; build generates SGX535 and SUPPORT_SGX535 | [DEFS:57–61](archaeology-data/DEFS.txt#L57); [PBUILD:44–50](archaeology-data/PBUILD.txt#L44); [CORE:488–494](archaeology-data/CORE.txt#L488); CONFIRMED in the code, expansion INFERRED |
| Can it be identified by hash or file list? | Yes: Git tree, single blob, SHA-256 and size | source-registry.json, H535; CONFIRMED. Corresponding external manifest UNKNOWN |

Inventory sources: [by ref](archaeology-data/header-by-ref.tsv), [history](archaeology-data/header-history.txt), [summary](archaeology-data/summary.json). No hash was invented for a missing version: this hash is of the actually available content.

## Branches with the header in the tip

| Local ref | Full commit | Path |
| --- | --- | --- |
| `refs/remotes/origin/1.17.4948957/mesa/k6.1` | `9ae0fa4998b1c624408945e062bf8fb0ea7efb9d` | `services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/devel-next` | `62f31de3ceed156bff32abb3dd03693a02df117e` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/dra7/experimental` | `a24ae6b2573b7eb1dc94473aa9953b964079b5c6` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/img-sgx` | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.1` | `fed0756f1b8b9d526da2821635c7d742989d47c3` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.14` | `76da7d73976f0a5dc04fdc84a3af899d6c2b1fe2` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.4` | `fd47e44b18944cf7ade480ac67a9c0172619ff7e` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.14.3699939/k4.9` | `0086977380d3320d70a3abc78b95fa0641427073` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k4.14` | `c545bf1c937b6067d27b7a268093baa3b8091185` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k4.19` | `2a777b8fb72a89d299b82845d42b63b2a2618daa` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k5.10` | `eda7780bfd5277e16913c9bc0b0e6892b4e79063` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k5.4` | `bfe83bbabb3849c24b03d5172cf678e7c5915e04` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |
| `refs/remotes/origin/ti-img-sgx/1.17.4948957/k6.1` | `bfd9edef3f976906fb7fececbfa29d16801154f5` | `eurasia_km/services4/srvkm/hwdefs/sgx535defs.h` |

All the lines above have confidence **CONFIRMED** by `ls-tree`; code lines: H535:1–739. `origin/*` are refs already present in the clone, not a server query. There is no reason to switch branches to read the file: `git show` solves it.

## What was actually removed

The DDK 1.17 series initially still loaded inherited Poulsbo files. The commit `3b6ca1d1f47a951c1f93ba5f9b693b25474d0798` removed `eurasia_km/eurasiacon/build/linux2/pc_i686_poulsbo_d0_linux/Makefile` and the integration `eurasia_km/services4/system/poulsbo`; the unprefixed lineage contains the corresponding operation in `636e957a340ebba5c10fb8a5c5b3f30b85078c66`. **CONFIRMED:** the removal records do not refer to `sgx535defs.h`, which remains in the trees. [Poulsbo history](archaeology-data/poulsbo-history.txt), [boundaries](archaeology-data/boundary-trees.txt). Lines removed from the Makefile: 1–86; from sysconfig.c: entire file, viewable in the parent.

**UNKNOWN:** whether the removal meant discontinuation of validated support, cleaning up unused code, or another decision. The title “remove DDK 1.14 specific files” does not document the actual state of the hardware or the product.

## What the header solves and what it does not solve

**CONFIRMED:** adds specific SGX535 definitions for clock/status/override, events, timer, PDS invalidation, BIF, 16 directory lists, banks, and 2D. For example, list 1 = `0x0c38`, list 2 = `0x0c3c`; BANK0 and BANK1 have fields EDM/TA/HOST/3D/2D, [H535:45–88](archaeology-data/H535.txt#L45), [H535:375–458](archaeology-data/H535.txt#L375), [H535:539–638](archaeology-data/H535.txt#L539).

**UNKNOWN:** complete datasheet, reserved fields, all effects of read/write, all silicon revisions, and validation in Poulsbo. Having this file does not authorize an arbitrary register dump. The set of header statements is not a complete specification of the USSE ISA or the rendering streams.

| Artifact | State in this phase | Exact source / confidence |
| --- | --- | --- |
| Header SGX535 | Recovered from Git for documentation | [H535:1–739](archaeology-data/H535.txt#L1); CONFIRMED |
| Poulsbo build and wrappers | Located in DDK 1.14 | [PBUILD:42–86](archaeology-data/PBUILD.txt#L42); [PSYSC:145–217](archaeology-data/PSYSC.txt#L145); CONFIRMED |
| KM microkernel interface header | Already exists; not a firmware implementation | phase 1 `sgx_mkif_km.h`, [firmware.md](firmware.md); CONFIRMED for the contract |
| `psb_powermgmt.h` and `sys_pvr_drm_export.h` required by the external branch | Referenced; not found as paths in KM commits or in the Linux snapshot | [PSYSC:65–69](archaeology-data/PSYSC.txt#L65); path logs; CONFIRMED as missing dependency in this set |
| UM Poulsbo compatible initializer and filled scripts | Not found as source | global searches and target [PBUILD:62–66](archaeology-data/PBUILD.txt#L62); UNKNOWN regarding external availability |
| Microkernel source and PDS SGX535 programs | Not located | global inventory; UNKNOWN regarding implementation and external license |
| ISA/assembler/compiler USSE SGX535 | Not found as source | global search; found UM compilers are binaries named 530/544, not 535; CONFIRMED about the names, compatibility UNKNOWN |

Do not confuse names `sgx_ukernel_status_codes.h`, flags USSE, and constants EURASIA with the presence of a firmware source or compiler. The compilers published in commit UM `5830fc09a5f37ca89b718d7c60a838e0267a6371` are `glslcompiler_SGX544_116.exe` and `glslcompiler_SGX530_120.exe`, according to [Git record with paths and hashes](archaeology-data/um-offline-compilers.txt); lines of code N/A, confidence CONFIRMED for the artifacts. They have not been run or disassembled.
