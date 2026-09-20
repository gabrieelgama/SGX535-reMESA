# Phase 4.2 sources and method

These files were collected on 2026-09-19 without changing `references/`. [manifest.json](manifest.json) records the size and SHA-256 digest of source files and derived results. A digest proves copy integrity; it does not prove hardware equivalence or supply-chain authenticity.

## antiX source package

- Source: <https://antixlinux.com/testing/pool/main/l/linux-5.10.240-antix.1-486-smp/>
- Maintainer named in the DSC: anticapitalist / antiX project
- Version: `5.10.240-antix.1-486-smp-6`; changelog date 2025-08-07
- Intended platform: Linux i386/486 SMP, not a Poulsbo-only package

The tarball and diff checksums match the DSC retrieved over HTTPS. I did not verify an apt/InRelease signature or an independent signature on the DSC.

The 191,264,284-byte source tarball was kept only in the `/tmp` cache; its URL and digest are in the manifest. The repository archives 79 selected files under `antix-source/`. I did not extract, install, or execute the package on the target GPU, and no package script was run.

The decompressed packaging diff identifies 17 changed paths, including `.config.old` and metadata. It does not modify the gma500, PM, or PCI files compared here. All 71 gma500 paths and Git blob hashes were compared with the GitHub API view of gregkh/linux commit `d5eca7ebcf6f64c4aebf9684c365c130a3a069b3`; five additional PM/PCI files have copies under `upstream-source/`. This was not a complete comparison of all kernel, DRM, or ACPI source.

License terms remain those of Linux/antiX and each file's authors. SPDX tags, headers, and `COPYING` are retained. Several gma500 files are `GPL-2.0-only`; `power.c` has a permissive notice. The packaging declares GPLv2 in `antix-package.diff:62363–62381`. The package does not have one blanket permissive license, and GPL code cannot be moved into a permissive implementation without a compatible licensing basis.

## Other source artifacts

| Artifact | Origin, owner, and commit/date | Provenance and use |
|---|---|---|
| `DDK_osfunc.c.txt` | TI/IMG KM commit `cb46ba4d0c900f89f7ec0284f9803d476bfa98de`, `eurasia_km/services4/srvkm/env/linux/osfunc.c` | Extracted from local Git with its header retained; study only |
| `EMGD_osfunc.c.txt`, `EMGD_sgxerrata.h.txt` | Intel/IMG material in EMGD-Community mirror commit `e6884ec2eaaf1afe88d5ff9dd44d70403525be5b` | URLs in [external-urls.json](external-urls.json); historical provenance, not an official manual and not copied into an implementation |
| `PSB_psb_msvdx.c.txt` | gregkh/psb-kmp commit `98b5307e5158a9ac401b29128ddd1184ae06b4d7` | Used to keep MSVDX and SGX evidence separate; copyright and license remain in the file; not executed |
| `context-*.txt` | KM representative commits listed in `core-occurrences.csv` | OMAP4/5, JZ4780, TC3, and sunxi integrations; none proves Poulsbo behavior |
| `rev116-mail.html` | H. Nikolaus Schaller proposal discussed by Rob Herring on 2020-05-05; linux-arm-kernel archive | Binding proposal, not a measurement; patch header says GPL-2.0-only OR BSD-2-Clause; no conclusion about the whole message's license |
| `core-occurrences.csv`, `search-scope.json`, `*-identification-history.txt` | Read-only searches across all local KM/UM commits | Derived indexes, not substitute sources; each occurrence points back to original Git history |
| `upstream-gma500-tree.json` | GitHub API view of gregkh/linux at the pinned commit | Blob hashes for comparison; not hardware evidence |

TI/IMG licenses must be audited per file before reuse. These are study copies with headers retained, and no code was incorporated into a new driver. No firmware or executable binary from the historical material was incorporated.

The regular-expression audit of C/H files does not cover binary contents and cannot prove that hidden interfaces are absent. Repeated commits or imports are not independent confirmation. Intel documents from Phase 4.1 retain the same mirror and provenance limitations recorded there.
