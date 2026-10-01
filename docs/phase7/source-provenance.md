# Phase 7 source provenance

[source-index.json](source-index.json) is the machine-readable index for the 29 sources and 36 source-specific matrix rows used here. Each row records its source key, repository, commit or package version, original path, retained local path when applicable, exact line range, SHA-256 and evidence ID. The index is an audit aid; the original source and its stated platform determine what a claim establishes.

| source family | provenance and platform | use and limit |
|---|---|---|
| antiX 5.10.240 source package | Existing [antiX package audit](../kernel-5.10.240-antix-audit.md) and [manifest](../phase4-2-data/manifest.json); 71 retained gma500 files match archived upstream commit `d5eca7ebcf6f64c4aebf9684c365c130a3a069b3` by Git blob ID | Closest available source for the TVZ-reported kernel. Equality with the installed module and surrounding PCI/DRM/ACPI build remains UNVERIFIED. Per-file Linux notices apply. |
| historical PSB | [source catalog](../poulsbo-data/sources.json), `gregkh/psb-kmp` commit `98b5307e5158a9ac401b29128ddd1184ae06b4d7`; retained snapshots and hashes indexed | Historical software precedent, not a hardware access guarantee or proof of the Dell's configuration. Per-file GPL notices must be respected. |
| TI kernel DDK | [source catalog](../poulsbo-data/sources.json); original Git commit `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` for the Poulsbo-capable DDK 1.14 tree; SGX535 header at `322bcda5f3076037e2e20ef9209f4f4d575a7d5f` | Build and source branches are scoped separately. TI/OMAP integration is never equated to Poulsbo. Source notices are retained; no code is imported into a new implementation. |
| EMGD community mirror | [source catalog](../poulsbo-data/sources.json), `EMGD-Community/intel-binaries-linux` commit `e6884ec2eaaf1afe88d5ff9dd44d70403525be5b` | Publicly retained source snapshots, not an authenticated Intel release or official hardware manual. Mixed license notices; evidence only here. |
| TI user-mode package | Original repository `omap5-sgx-ddk-um-linux`, commit `b6801bf89e00d69893c2957455bbeb3195c0ed52`; [README](../../references/omap5-sgx-ddk-um-linux/README) and ARM ELF `targetfs/lib/libsrv_init.so.1.9.6.0` | OMAP5/DRA7xx artifact. ELF metadata was inspected without copying or executing program contents. SHA-256 of the existing binary: `a1a3572ee4ae6104505808d171d6b4cd023309959588f890b2192aea44fafdda`. License/reuse requires separate review. |
| Test Vector Zero | [TVZ-001 report](../hardware-evidence/TVZ-001/README.md) and [manifest](../hardware-evidence/TVZ-001/manifest.json), reported Dell hardware | Inherited passive observations only. The original terminal stdout, complete package identity and installed binary hashes were not supplied. This run made no new target observations. |

The Phase 7 index checks retained snapshot hashes against existing source catalogs and original Git objects where the objects are available. That verifies transcription within this repository, not vendor authenticity or hardware behavior. The historical OMAP ELF's symbol values are metadata, not GPU addresses. No new restricted manual, firmware payload, or vendor binary was added.

License labels in the index describe the retained notice or repository context; they do not grant reuse rights. Future Mesa code must have an independently compatible provenance. References under `references/` remain untouched.

## Later xpsb-glx binary evidence

The separately recovered `home:lkundrak:poulsbo` `xpsb-glx` 0.18-4.1 package contains [psb_dri.so](../../references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so), SHA-256 `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`. Its [spec](../../references/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx.spec) says `Redistributable, no modification permitted`; this file is evidence only. [The new static-analysis track](psb-dri-re/README.md) keeps raw ELF offsets, reproducible scripts and address-level P7B entries in the project evidence matrix. Neither the original Phase 7 source index nor P7-001–P7-022 was renumbered. The package date/version does not authenticate its original build inputs or prove a match to the Dell's installed stack.

The retained package also contains [Xpsb.so](../../references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so), SHA-256 `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`. [The Xpsb follow-up](xpsb-re/source-provenance.md) records the public `libdrm-poulsbo` 2.3.0 and Xorg DDX 0.32.0 source RPM URLs, archive hashes, licensing and version mismatch. P7C evidence IDs extend the matrix; they do not renumber P7 or P7B or authenticate an exact paired stack.

The subsequent [static-buffer trace](xpsb-re/static-buffer.md) and [version-pairing review](xpsb-re/version-pairing.md) add P7D evidence without replacing earlier claims. Public RPM Fusion PSB kernel-source 4.41.1 contains a `5.0.1.0046` header matching both retained ELF version strings and direct request sizes. The companion public firmware package contains the MSVDX/video payload only. [The added provenance table](xpsb-re/source-provenance.md) records exact RPM/tar/header hashes and per-package licensing limits. No source archive or payload was added under `references/`.

The P7E [draw/format pass](psb-dri-re/draw-to-submit.md) reuses the same retained ELF hashes and 4.41.1 source candidate. It additionally compares the private renderer table with official MesaLib 7.4.4 source downloaded only to `/tmp` (tar SHA-256 `eaf73d7a3a2dc959ddc0753abaa18160c64bec00b35bf4a0c96040b2072918ec`, release MD5 `b66528d314c574dccbe0ed963cac5e93`). [The detailed provenance row](xpsb-re/source-provenance.md) links the public release. Mesa source is a generic comparator, not evidence of the exact proprietary build or a license to translate decompiled code.

The P7F [bounded-path pass](psb-dri-re/bounded-path-closure.md) adds no new historical artifact. It reads the same hashed `psb_dri.so` and `Xpsb.so`, public 4.41.1 PSB kernel source, recovered DDX/libdrm source and Mesa comparator. Its selected Ghidra decompilations and i386 disassembly remain under `/tmp`; only derived field evidence and reproducible function addresses enter the repository. P7F-003 records an explicit correction to the earlier P7B-009 buffer classification. No proprietary program or decompiled function was copied into implementation code.

The P7G [single-triangle trace](psb-dri-re/frozen-draw-closure.md) adds no binary or source package. Its [source checks](psb-dri-re/frozen-source-checks.json) hash the exact candidate kernel/libdrm files used for the USE-base, XHW and fence comparisons. The selected Ghidra decompilations remain under `/tmp/sgx535-psb-decompile`, from a read-only disposable project; only address-level observations, derived values and explicit uncertainty enter documentation. P7G-001/002 correct two P7F interpretations without changing the older evidence IDs or source bytes. The package's redistribution restriction still applies; no decompiled implementation was added to a driver.

The P7H [compiler checkpoint](psb-dri-re/frozen-fragment-compiler-checkpoint.md) and [XHW trace](xpsb-re/xhw-init-frozen-draw.md) use those same retained ELFs. Focused headless decompilation used the disposable `/tmp/sgx535-ghidra-re/PsbDriStatic` project in read-only mode; decompiled bodies and Ghidra databases were not added to the repository. [Source checks](psb-dri-re/frozen-source-checks.json) now also name and hash the candidate `psb_drm.h` and `psb_scene.c` used for the XHW layout/operation comparison. P7H entries are new scoped claims; they do not alter P7G evidence or authenticate exact binary/kernel pairing.

The later [FG-01 selected compiler trace](psb-dri-re/frozen-fragment-exact-output.md) uses the same retained `psb_dri.so` SHA-256 and disposable read-only Ghidra project. P7H-010/P7H-011 record only a derived CPU-side result for the frozen MOV/END input; no new historical artifact was acquired, no compiler code was executed, and the linked pixel program remains unresolved.

The [FG-02 continuation](psb-dri-re/frozen-fragment-link-progress.md) reuses the same retained DRI and Xpsb ELFs. P7H-012–015 add ELF-addressed link/PDS producer facts and Xpsb constant-store arithmetic, without claiming PDS instruction read semantics. Headless decompilation stayed in disposable `/tmp/sgx535-ghidra-re/PsbDriStatic` and `/tmp/sgx535-xpsb-ghidra/XpsbStatic` projects; derived descriptions, not decompiled implementation, were added here. The [public Imagination forum thread](https://forums.imgtec.com/t/pds-programming/333) is a document locator only, as already classified in P5-003.

The public [PVR_PSP2 repository](https://github.com/GrapheneCt/PVR_PSP2/tree/ae4df4b723f3f1aa629b1f3b276b9cca6d663891) was checked as a possible PDS decoder locator. Its stated platform is PSP2, and the inspected `include/gpu_es4/eurasia/hwdefs/sgxpdsdefs.h` bears an Imagination all-rights-reserved notice prohibiting redistribution without permission. It was not imported, treated as SGX535/Poulsbo evidence, or used to decode selected instruction words. The missing [PDS read-set contract](psb-dri-re/selected-pds-hard-boundary.md) remains UNKNOWN.

The subsequent [PDS search ledger](psb-dri-re/pds-readset-search-ledger.md) adds P7H-016 from the same hashed DRI ELF, using the authorial read-only immediate survey. A disposable clone of `freemangordon/mesa` branch `mesa-pvr-ti` at `5bd40a453e1484efea842be9a3f388adb5fac93c` and the archived PowerVR GNU instruction-encoding page were inspected as public leads; neither supplied a Series5 PDS decode. No source from either was imported. The comparative source-operand hypothesis remains INFERRED and is not a gate-changing claim.

P7H-017 traces the same literal in the hashed retained `Xpsb.so`; `Xpsb_emit_pixel_event_program` was also checked as a neighboring but different program. These are static CPU-emission facts. [USITC Publication 4374, volume 2, PDF page 124](https://www.usitc.gov/intellectual_property/documents/pub4374_vol_2_of_2.pdf) independently lists an “Eurasia 3D Input Parameter Format” exhibit (181C) as withdrawn. The public list is document metadata only: no exhibit content was retrieved, and neither that title nor the forum's SGX535-specific title establishes the selected PDS instruction fields or reuse rights.

P7H-018 checks the retained DRI outbuf reset, `driBOData`, and map wrappers at ELF `0x3865f`, `0x44c90`, and `0x43ffe`. This establishes that the inspected userspace path can retain a BO and does not clear it when passed a null source. It does not determine the actual selected BO contents or GPU consumption.

P7H-019/020 use the same hashed retained ELFs for a [curated PDS word corpus](psb-dri-re/pds-instruction-corpus.csv) and [authorial arithmetic check](../../tools/psb-dri-re/pds_field_constraints.py). The script verifies literal bytes directly and writes only a field-constraint regression CSV; it does not execute either ELF or import proprietary decoder code. Public [TI SGX530 documentation](https://e2echina.ti.com/cfs-file/__key/telligent-evolution-components-attachments/00-120-01-00-00-04-96-30/sprugz7b.pdf) and [current Imagination instruction documentation](https://docs.imgtec.com/reference-manuals/powervr-instruction-set-reference/html/index.html) were checked as scope comparators, not used to decode Poulsbo Series5 PDS. No new historical artifact was added to `references/`.

The later [external-evidence pass](psb-dri-re/series5-pds-external-evidence.md) used Sourcegraph (including indexed archives/forks), GitHub/Codeberg repository search, Debian Code Search, Wayback availability, public SGX540 reversing-project metadata, the TI OMAP5 user-mode package README, official architecture overviews and public patents as **locators or scope comparators**. The SGX540 project explicitly leaves PDS reverse engineering unfinished; TI's public package describes libraries/binaries. Exact Series5 identifiers in the inspected code index led only to the already rejected PSP2 source. No new proprietary artifact was retrieved, no restricted source detail was imported, and no independent SGX535 PDS rule was established. Search coverage is not positive hardware evidence; the selected read set and `0xaf000000` semantics remain UNKNOWN.

P7H-021/022 use no new historical artifact: the [authorial semantic-constraint script](../../tools/psb-dri-re/pds_semantic_constraints.py) reclassifies the prior 42-row corpus and checks arithmetic candidate models against already statically inspected DRI/Xpsb builders. The mixed-event correlation and terminal CPU grammar are recorded in [the hypothesis audit](psb-dri-re/pds-hypotheses.md). This is a word-construction analysis only; no retained binary was executed and no proprietary source or decoder was imported into implementation code. Neither evidence ID establishes an SGX535 PDS GPU read set.

P7H-023/024 are the [Experiment #43 static-producer audit](psb-dri-re/pds-experiment43.md).
The retained DRI/Xpsb hashes above were reverified. A related older Canonical
`libgl1-mesa-dri-psb` 0.24+repack+0038.1 package (SHA-256
`14e870f4273e84759630499efeda4948116c29f3ba7d08e2b5f028a5da3ef8a2`)
was located through the public Canonical hardy-dell-mini package index; its
extracted ELF SHA-256 is
`427627909b0c528d154d1b247688f16536162a520d43d472e8a4f404c02e2a6e`.
The original-content copyright notice states Intel Confidential/all rights
reserved, so the package remains in disposable `/tmp/sgx535-exp43-quarantine/`
and is **not** added to `references/` or used for implementation details.
Only static occurrence/provenance checks were made. Its same three `0x45`
variants provide no new ISA constraint, and no historical binary was run.

P7H-025/026/027 use the original retained DRI hash and the existing candidate
4.41.1 PSB kernel source for the [PDS buffer lifecycle](psb-dri-re/pds-buffer-lifecycle.md).
New bounded Ghidra exports of owner, outbuf, batch-pool and BO-wrapper
functions remain in `/tmp/sgx535-psb-decompile/`; their exact ELF entry
addresses are added to [targets.txt](../../tools/psb-dri-re/targets.txt).
The [per-dword table](psb-dri-re/pds-buffer-byte-provenance.csv) and
[call graph](psb-dri-re/pds-allocation-callgraph.csv) are authorial derived
records. The kernel relocation code is candidate-family evidence, not an
authenticated build pair or a PDS instruction decoder. No historical ELF
was executed and `references/` was not modified.

P7H-028 follows the already acquired candidate kernel through `drm_bo.c`,
`psb_buffer.c`, `drm_ttm.c` and `drm_vm.c`, and the existing candidate
libdrm's `xf86drm.c`. The PDS memory type is non-fixed/TTM-backed in that
source; new TTM pages use `__GFP_ZERO`. This is a conditional first-use
zero-image witness, not proof of the retained ELF's exact runtime pairing
or zeroed recycled suballocations. Per-file hashes and the complete
conditional chain are in [the lifecycle report](psb-dri-re/pds-buffer-lifecycle.md).

P7H-029–031 use only the same retained two ELFs and previously catalogued
libdrm-poulsbo2.3.0 / PSB4.41.1 sources. The common drm.h SHA-256 is
`e4e0c5dce5558d4aa09f33fa236832e354275ccc1612a7c4a1a906758bb51f41`
in both source packages. The [pairing report](psb-dri-re/pds-stack-pairing.md)
records source equality separately from unauthenticated deployed pairing.
New CSVs and arithmetic/ABI checking tools are authorial analysis. The ABI
tool compiles public source declarations only into a temporary constants
object and reads it without execution; no proprietary implementation code
is copied to the repository. Bounded decompilation text remains in /tmp.
No new source was acquired and references/ remains unchanged.

P7H-032–035 add a [backing-provider audit](psb-dri-re/pds-backing-provider.md).
Public OBS revision metadata identifies the retained spec exactly and supplies
the same original PSB4.41.1 tar as the prior RPM candidate. Public Launchpad
API source-file recovery adds4.37,4.41.6 and four4.42 source packages; pinned
gregkh files and Linuxv2.6.32 allocation source are comparison inputs. Their
per-file public notices were reviewed; the [acquisition log](psb-dri-re/pds-backing-acquisition.json)
records URLs, failures and SHA-256 digests. All acquired payloads and disposable
patch trees remain under /tmp/sgx535-backing-provider, outside references/.
No dpatch/package script or historical executable was run. Source equivalence
is not deployed-provider authentication; target membership remains conditional.

## Route C implementation policy and proof aid (P7H-036/037)

The [clean-room contract](psb-dri-re/cleanroom-backing-contract.md), image and
transition tables, Python serializer and regression tests are project-authored
static artifacts. They use established P7H-014/025/030/031 facts; no new source
package or ISA definition was acquired. The user selected Route C on2026-09-27.
That decision removes historical target-provider identity as a prerequisite
for the new construction; it does not authenticate an old installation.

The code initializes ordinary host bytes and parameterizes the resolved USE
word. It is not copied provider implementation, a PDS decoder, a kernel module
or an SGX mapping/visibility implementation. Its CPU proof does not establish
complete PDS launch-state coverage or GPU-observed zeros. All historical
UNKNOWN/CONDITIONAL and quarantined-material restrictions are preserved.

## Selected launch-state envelope (P7H-038/039)

The [launch report](psb-dri-re/pds-launch-state-coverage.md) reuses retained
DRI/Xpsb bounded exports and existing public SGX535 register snapshots. Only
the launch serializer was cross-checked in original x86 bytes with llvm-objdump;
no PDS corpus extraction or historical executable execution occurred. Exact
CPU offsets/formulas and snapshot hashes are preserved in the report. Temporary
Ghidra exports remain disposable; the existing export procedure reproduces
them. New Python is project-authored proof-model code, not a PDS decoder or
GPU backend. No source was acquired; no quarantined implementation definitions
were used; references/ was not modified. Register names do not establish L12.

P7H-040's [narrow launch-count analysis](psb-dri-re/pds-launch-count.md) uses
retained ELF-addressed resource arithmetic, DRI background0x2ee34 and Xpsb
0x71e0/0x7560 packing. Existing ephemeral export0002eef4 duplicates the containing
0x2ee34 function and is not independent evidence. Public source searches were
limited to already acquired PSB/DDX/libdrm and permitted SGX kernel/header
material; no symbolic architectural field definition was adopted. The new
checker is authorial constraint code with explicitly synthetic serializer
cases, not translated driver implementation, a hardware emulator or decoder.
No new acquisition, references change, confidential-source use or hardware work.

P7H-041 [availability analysis](psb-dri-re/pds-selected-source-availability.md)
rechecks the already catalogued public SGX535 forum thread specifically for
preload/temporary statements (2026-09-27). It is a user description, not an
authenticated instruction or launch contract. Only a short public quotation
is retained; the unavailable referenced document was not acquired. The patent
locator failed through the web tool and supplies no new evidence. Existing
public PSB reset source and retained Xpsb0x3df0 export were checked only for
whether they establish selected store initialization; they do not. No reset or
register operation occurred. No confidential source or new binary was used;
references/ was not modified. Authorial checker changes test conditional model
integrity and keep architectural envelope evidence UNKNOWN.

## P7H-042–046: independent frozen triangle CPU specification

No new material acquired. Existing retained DRI/Xpsb exports and targeted static
LLVM disassembly supplied the target/state/submission formulas in the
[frozen triangle specification](psb-dri-re/frozen-triangle-spec.md). No restricted
ISA headers or quarantined binaries were consulted. The already qualified
candidate PSB4.41.1 kernel and Mesa7.4.4 sources were reused within their original
scope; [file hashes](psb-dri-re/frozen-triangle-source-checks.json) record the eight
public source files used. These hashes identify local derived files, not a new
signature authentication or proof of historical deployment. Decompiled bodies
remain in disposable/tmp directories and are not checked into the repository.

## P7H-047–051: BO, surface and bootstrap constraints

No new acquisition. Targeted read-only Ghidra/LLVM analysis of the same retained
DRI recovered shared-surface mapping and ARGB8888 spans; retained Xpsb exports
supplied revision and TA-cookie arithmetic. The qualified PSB4.41.1 kernel and
public DDX0.32.0 sources supplied mapping/manager and stride corroboration.
[File hashes](psb-dri-re/frozen-triangle-source-checks.json) pin19 existing
public files now used; hashes do not authenticate archive signatures or target
deployment. [Report](psb-dri-re/frozen-triangle-burn-down.md) separates source
facts from clean-room grouping/initialization policy. No restricted material,
historical execution or device interface was used; references/ unchanged.

P7H-051 additionally reuses the already cataloged permissively licensed SGX535
register snapshot from EMGD-Community commit e6884ec2eaaf1afe88d5ff9dd44d70403525be5b,
within its existing provenance qualifications. Register-name correlation is not
a new architectural cache-domain guarantee. No PDS instruction definitions were used.

P7H-052–054 use the same retained DRI/Xpsb and the19 already hashed public files in `psb-dri-re/frozen-triangle-source-checks.json`. No acquisition or references/ change. The SGX535 register snapshot is used only for status2/DPM register correlations, not PDS ISA. Raw Xpsb0x3f49–0x3fd4 corroborates the fourth-load poll-mask arithmetic; decompiler return-type guesses are not evidence. Temporary exports remain outside the repository.

P7H-055–057 reuse the same qualified sources and retained ELFs. The existing candidate `psb_irq.c` is added to the per-file hash ledger; no new acquisition. A read-only no-analysis Ghidra export restores Xpsb0x4030 under `/tmp/sgx535-xpsb-decompile/`; the existing target list already contained it. No decompiled body or confidential ISA material is imported. Source-domain joins consume prior corpus/constraint artifacts without regenerating them.


## P7H-058: external three-rule acquisition

[Registry](psb-dri-re/final-rules-external-sources.json) records six new public
source reviews and nine SHA-256-pinned downloads, original URLs, dates where
known, scope and license qualifications. Official SDK MIT manifests, a public
Imagination patent, an independently published GPL SGX535 header and Linux host
API documentation supply no missing architectural implication. The Android
header was read through the web renderer but raw retrieval failed; its Git blob
is recorded and no SHA-256 is invented. Failed retrievals are not evidence of
absence. A search result explicitly marked Confidential/NDA was excluded
unopened; its locator is documented solely as a non-implementation reference.
No quarantined contents were used. Downloaded bodies remain in ephemeral
`/tmp/sgx535-final-external/`; nothing was added to references/.
