# DDK Versions Map and SGX535 Evidence

Convention: **CONFIRMED** confirms the content/history of the sources, not hardware operation; **INFERRED** identifies a deduction; **UNKNOWN** is what has not yet been established. Source identifiers like H535 and PBUILD resolve to repository, full commit, file, and hash in [source catalog](source-archaeology.md#source-catalog). Document snapshots preserve the original lines and license notices; they are not implementation files.

## Versions identified by content, not just by branch names

| IMG / build Version | TI KM representative Commit | Evidence | SGX535 / Poulsbo | Confidence |
| --- | --- | --- | --- | --- |
| 1.9, numeric branch 19, build 2253347 | `1450ae2166ad952ef30197e79518b51577a629e6` | [VER19:55–65](archaeology-data/VER19.txt#L55) | dispatcher/feature/errata 535 exist; header missing in this tree; no Poulsbo target found in this import | CONFIRMED about content; functional build 535 UNKNOWN |
| 1.13 build 3444720, experimental | `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | [VER13:51–60](archaeology-data/VER13.txt#L51) | first local entry found of header 535; does not contain target pc_i686_poulsbo_d0_linux | CONFIRMED about content; functional integration UNKNOWN |
| 1.14 build 3699939 | `cb46ba4d0c900f89f7ec0284f9803d476bfa98de`, origin/img-sgx | [VER14:51–60](archaeology-data/VER14.txt#L51) | target Poulsbo D0 selects SGX535 rev121; header and system/poulsbo present | CONFIRMED: target explicitly configured; execution not validated |
| 1.17 build 4948957, SGX_DDK_Linux_XOrg variant | `b630d462f5fbb86e5f98965ba1af35da1207822f` | [VER17:51–60](archaeology-data/VER17.txt#L51) | this commit still has target and inherited wrappers; removed in the cleanup child | CONFIRMED history; support validated in 1.17 UNKNOWN |
| 1.17 build 4948957, SGX_DDK variant | `01a9d880722e93d8f8321e4f9be64dfae19e85a3` | `eurasia_km/include4/pvrversion.h`:51–57, blob `86a733d2fc714eedd0dcea03d50dfce1947ab62e` | product identification differences do not prove core change | CONFIRMED on version; compatibility UNKNOWN |

Five distinct blobs of pvrversion.h were found, representing four version/build pairs; the two from 1.17 differ in product identification. Inventory with commit, path, hash, and lines: [version-headers.json](archaeology-data/version-headers.json). Do not interpret `PVRVERSION_BRANCH=19` as DDK 1.19: major/minor are 1/9 in the same source.

**CONFIRMED:** the available KM history starts with a 1.9 import; no source for a version earlier than 1.9 was found. This does not prove that older SGX535 versions do not exist externally. **INFERRED:** 1.14.3699939 is the best local anchor to investigate Poulsbo integration, as it combines explicit version, specific header, and Intel/target revision in the same commit. [VER14:51–60](archaeology-data/VER14.txt#L51); [PBUILD:44–52](archaeology-data/PBUILD.txt#L44).

## Tags and branches are not interchangeable versions

The tag `TI_LINUX_OMAP_SGX_DDK_1.9_2253347` points to `6668f4ea0bbf7e3df7f72dc2230ab8da1e748b2d`. `ti_imgddk_1.9.0.11` points to `7e084e0452bfaa495d2a9c06c7a420bac35d8d99`; `glsdk_7.01.00.03` and master point to `430673f78b79eccdf308a6bbfb524209b485d2cc`. **CONFIRMED as local refs**; lines of code N/A. All 12 available KM tags lack the header in the tip. [Refs KM](archaeology-data/omap5-sgx-ddk-linux-refs.tsv); [presence by ref](archaeology-data/header-by-ref.tsv).

`origin/dra7/experimental` ends in `a24ae6b2573b7eb1dc94473aa9953b964079b5c6`, with 1.13 moved to `eurasia_km`. `origin/img-sgx` anchors 1.14. Branches `ti-img-sgx/1.17.4948957/*` maintain the header, but their examined tips do not necessarily maintain the Poulsbo target. It is necessary to check the file and commit, not infer by branch name. **CONFIRMED:** [header-by-ref.tsv](archaeology-data/header-by-ref.tsv).

## History ONE

**CONFIRMED:** the root UM `225230e5dae17836a8acf6f6e9fb6e8200af4dea` declares TI DDK 1.9.11 on IMG 1.9 ED2253347 and package OMAP5/DRA7xx. [UM19:19–41](archaeology-data/UM19.txt#L19); [UM19:57–61](archaeology-data/UM19.txt#L57). This does not establish an ABI compatible with Poulsbo.

The other root, `348ea7e08d8b6e7f46f73065ad3ba75614edc12c`, has an author dated 2013 but a committer from 2023 and a README that declares 1.17. **CONFIRMED:** metadata in [commits UM](archaeology-data/omap5-sgx-ddk-um-linux-commits.tsv) and [UMROOT:33–36](archaeology-data/UMROOT.txt#L33). Therefore, sorting only by author date would produce a misleading chronology. “Root” here is the root of the available graph, not the birth of the DDK.

Refs UM name series 1.14.3699939 and 1.17.4948957; the searchable source content does not provide SGX535/Poulsbo. There are 1802 blobs classified as binaries, whose content was not used to infer support 535. The offline compilers SGX530/544 added in 2018 show distribution of these artifacts, not compiler source nor SGX535 support. [Refs UM](archaeology-data/omap5-sgx-ddk-um-linux-refs.tsv); [import inventory](archaeology-data/um-offline-compilers.txt), binary files: lines N/A, confidence CONFIRMED on names and hashes.

## Compatibility Completion Limit

The Poulsbo D0 target **selects** rev121; it does not show that every 8086:8108, 8109 unit or every stepping of GMA500 has this revision. The errata header deals with 121, 126, and HEAD. [PBUILD:47–50](archaeology-data/PBUILD.txt#L47); [ERRATA:152–178](archaeology-data/ERRATA.txt#L152). This build association is a factual advance, not a user hardware identification.

No DDK was built or executed. Kernel compatibility, userspace, firmware, build options, and structures remain **UNKNOWN** until the corresponding set is obtained and analyzed. No feature of SGX540/544 was assigned to 535.
