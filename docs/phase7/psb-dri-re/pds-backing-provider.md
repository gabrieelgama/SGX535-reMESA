# PDS backing-provider audit (P7H-032–035)

**Route C decision (2026-09-27):** historical target membership below remains
CONDITIONAL, but no longer gates the restricted new implementation. The
[clean-room contract](cleanroom-backing-contract.md) separates deterministic CPU
bytes from complete PDS launch-state coverage and concrete GPU contents
preservation. This audit is retained as source-scoped evidence, not deployment
proof or automatic certification of a new provider.

**Decision: CONDITIONAL.** The recovered historical provider family supplies
fresh zeroed PDS backing and preserves its contents. This now covers the actual
OBS source lineage of the retained userspace package, several Ubuntu branches,
and the earlier public PSB snapshot—not just one convenient RPM source tree.
What remains unestablished is **membership of the frozen target's supplying
BO implementation in that audited family**. No package dependency or target
artifact binds that implementation to the target. The existing target report
instead names modern `gma500`, whose catalogued source is not a direct provider
of this legacy PDS BO/submission interface.

This is not a discovered non-zero PDS counterexample. It is a bounded positive
source result with an unclosed target-membership edge. PDS read semantics remain
UNKNOWN. FG-02 stays OPEN. Gate B is BLOCKED, whitelist `[]`, hardware UNVERIFIED.
No historical executable, package installer, kernel module or device was run.
The prior corpus, ABI layout checks and first-use timelines were not rederived.

## Auditable inputs and scope

- [Provider candidates](pds-backing-candidates.csv): 12 rows, including exclusions
  and the unidentified target provider.
- [Placement/backing paths](pds-backing-paths.csv): 10 rows separating selected
  allocation, eviction, rejected placement alternatives and recycled operation.
- [File comparisons](pds-backing-file-comparison.csv): 84 hashed source-file
  observations. Byte equality and reviewed differences are separate results.
- [Acquisition log](pds-backing-acquisition.json): 117 URL attempts with retrieved
  SHA-256/size or the actual failure. API responses listing no surviving files
  are recorded as empty lists, not as proof that a version never existed.
- [Verification tool](../../../tools/psb-dri-re/pds_backing_audit.py): checks these
  local artifacts and the placement-mask constraint; never discovers a target
  identity from matching hashes alone.

All acquisitions remain in `/tmp/sgx535-backing-provider`. Public kernel source
files carry permissive Tungsten/Intel notices and/or GPL notices; the 4.42 Debian
copyright explicitly provides a permissive grant. These notices were inspected
before implementation analysis. OBS specs/patches are public package provenance
and modifications to that public source. No proprietary PDS definitions or the
quarantined Intel Confidential DRI were consulted. No payload was added under
`references/`. Download hashes establish the retrieved bytes; no claim is made
that Debian signatures or an installed kernel binary were authenticated.

## A package lineage, not merely contemporary filenames — P7H-032

The public [OBS project](https://api.opensuse.org/public/source/home:lkundrak:poulsbo)
still lists `xpsb-glx`, `libdrm-poulsbo`, `xorg-x11-drv-psb` and `psb`.
The retained xpsb-glx spec is byte-identical to
[xpsb-glx revision 3](https://api.opensuse.org/public/source/home:lkundrak:poulsbo/xpsb-glx/xpsb-glx.spec?rev=3):
SHA-256 `60062e6850f6780e0c9a5d458b380f188188ed3ea2b58a0f5651dcd41b64c895`.
Revisions 1 and 2 do not have that digest. Its retained original archive has
MD5 `9d14c7e16a72db1923c1a7801cf347c7`, matching the OBS inventory; the stronger
retained SHA-256 remains
`da180da15c38bb3dec5d70adf263c9953d63e4f88a01726f0fc867fdb7e9fab7`.

The [psb history](https://api.opensuse.org/public/source/home:lkundrak:poulsbo/psb/_history)
contains seven revisions. Every revision's inventory names the same 4.41.1
original tar with MD5 `5ca47555507dec0318a6ed4128f73146`. A fresh download from
[revision 7](https://api.opensuse.org/public/source/home:lkundrak:poulsbo/psb/psb-kernel-source_4.41.1.orig.tar.gz?rev=7)
has SHA-256 `12fe32e61b313882bd72a1eeda87afceb5b5b326761392edddf294b404efb5cd`:
**exactly the prior RPM Fusion candidate's original source**. The last revision's
source ID is `f9d9498beee28ac72ca14a4b562e2942`. This is direct source identity,
not an inference from dates or driver names.

All nine distinct patch contents across those seven revisions were inspected.
They change bus naming, IRQ compatibility, credentials, I2C, AGP field names,
build warnings and 2.6.32 compatibility. The `drm_vm.c` patch changes the AGP
fault path, not the selected TTM fault path. None changes `drm_ttm_alloc_page`,
PDS memory type, selected movement, or MMU page population. Different versions
of the warnings patch only change that patch's paths/hunks. No spec performs a
separate backing-allocator transformation. Thus the seven **published source
recipes** form a bounded equivalent class for this contract; this does not
assert all seven recipes successfully build on every configured kernel.

The dependency arrows stop short of selecting a target kernel:

| Arrow | Evidence and limit |
| --- | --- |
| retained xpsb-glx → OBS revision 3 | Exact spec SHA and original-archive inventory identity |
| OBS project → PSB 4.41.1 source lineage | Exact original-tar SHA; seven source revisions and patch audit |
| DDX → xpsb-glx/libdrm-poulsbo | Current DDX 0.31.0-11.2 spec has unversioned Requires |
| DDX → particular kernel build | **Absent**; `Provides: psb-kmp` / `psb-kmod-common` is not a requirement for a particular module implementation |
| xpsb-glx → particular kernel build | **Absent** from retained spec; no kernel digest/version pin |
| frozen target → OBS or Ubuntu provider | **Absent** from the target record |

The PSB spec requires firmware and builds against selected kernel flavors.
Ubuntu packaging uses DKMS. Neither supplies a target module build identity.
RPM automatic ELF dependencies on a library SONAME would still not identify
the code servicing BOCreate. Co-location in OBS does not prove co-installation.

## Independent branch and patch checks — P7H-033

The public [gregkh/psb-kmp snapshot](https://github.com/gregkh/psb-kmp/tree/98b5307e5158a9ac401b29128ddd1184ae06b4d7)
is a 4.40 import. Its generic header, TTM allocator, BO implementation, move
implementation, TTM fault source, PSB buffer backend and MMU source are all
byte-identical to the 4.41.1 candidate. The initial import
`0c4490c22402ba9fdc31a763c02e1948ec10f47d` also has the identical TTM file.
The eight-commit history concerns import, a display-mode diagnostic patch,
file removal and Makefile/ignore changes. The private package string is
`5.0.0.0045`, so this is not asserted to be a complete private-ABI match.

The Ubuntu archive directory returned HTTP 403. Following the
[Launchpad publication API](https://api.launchpad.net/1.0/~gma500/+archive/ubuntu/ppa?ws.op=getPublishedSources&source_name=psb-kernel-source&exact_match=true)
and each publication's `sourceFileUrls` recovered source packages instead:

| Source package | Original/archive SHA-256 | Backing result |
| --- | --- | --- |
| 4.37-0ubuntu1~810um1 original | `acec3193bdad233a87bbefd9c6a01873abe1fbf1c3a701a399072eb74f8807ab` | Same zero allocator; see old cache caveat below |
| 4.41.1-0ubuntu1~904um1 original | `12fe32e61b313882bd72a1eeda87afceb5b5b326761392edddf294b404efb5cd` | Identical original; Debian patch changes sysfs/udev |
| 4.41.6-0ubuntu1ppa9.10+3 original | `daf02e159dab223ca14545a4e7af7e91f09f65272a5dbd34891b9a50af2bdd45` | Same allocator/move/backend/MMU; AGP field-name change in fault source |
| 4.42.0-0ubuntu2~1004um2+karmic | `5d0750fdef9e272b699a325e40dc0cecbd6cfcb91b841656453025c4ce8ba6a4` | Same selected backing after patch review |
| 4.42.0-0ubuntu2~1004um3.1 | `37d5c6718ed6585de8c73fc1906820e4647ff9e0c6c2240ce224a176730ebb0b` | Same selected backing after patch review |
| 4.42.0-0ubuntu2~1004um3.2 | `70cef3de5d7f38c65ca5fc09aeaedc585bd63e2bc5aaa033bf98e0706548101b` | Same selected backing after patch review |
| 4.42.0-0ubuntu2~1010um5 | `56f04bec8bd8625a13acb6e456dc403adfc7dcb16d0d5e8b5108ba6ba48609bc` | Same selected backing after patch review |

All seven unpatched generic `drm.h` files equal the already checked header;
the 115 layout measurements were not repeated. `.dsc` checksums and retrieved
source hashes are checked independently of signature authentication.

For 4.42, reading only the original files would be inadequate: an enabled
`10_change_prefix.dpatch` touches the backing functions. Every hunk reduces to
symbol-prefix changes. Raw diff application in disposable trees, without running
any dpatch script or package build, confirms the allocator/BO/move/backend/MMU
and header equal the candidate after reversing those symbol prefixes. The TTM
fault portion also remains unchanged; its other difference is the AGP rename.
Other patches address module naming, display, IRQ, firmware declarations,
locking initialization and ACPI registration, not selected PDS payload storage.
One pre-applied I2C hunk was rejected in each disposable patch run; this is
recorded, not presented as a successful complete package build. Relevant
backing patches applied successfully. The RT patch even contains a suspicious
unrelated lock change; **backing equivalence is not certification of the whole
driver**.

The 4.37 allocator still explicitly requests zero pages. Its TTM caching code,
however, comments out `flush_agp_mappings`, whereas the later source invokes it
for kernels below 2.6.25. No full visibility equivalence is claimed for that
older configuration. For kernels >=2.6.25 that particular operation is absent
in both versions. This is a scoped caveat, not evidence of dirty PDS allocation.

No dirty-page pool, reused kernel-BO cache or alternate PDS allocator appeared
in these checked backing implementations. Missing later Launchpad file lists,
and HTTP 403 from the Yocto historical recipe endpoints, are logged. A failed
retrieval is not included as a proven equivalent provider. Existing EMGD/TI
material does not establish this old BO/provider ABI and is not silently added
to the equivalence class because it supports the same GPU family.

## Reachability: no stolen-memory fallback — P7H-034

Reuse the established retained request `mask=0x20000001`, `buffer_start=0`.
In the candidate and checked equivalent sources, `drm_bo_type_flags(type)`
selects bit `24+type`. `drm_bo_mt_compatible` rejects a type unless its memory
bit intersects both the request and `DRM_BO_MASK_MEM`. For this mask, only
**type 5 (PDS)** survives. Both the preferred and busy/eviction-assisted loops
in `drm_bo_mem_space` repeat this filter. A list containing VRAM or TT before
PDS is not permission to place this BO there.

This closes a concrete alternative: **PDS allocation cannot silently fall
back to unzeroed stolen memory in the audited provider**. Fixed VRAM type 1,
generic TT type 2 and APER type 7 are excluded by the mask. PDS's declaration
has TTM/CMA/mappability and no FIXED flag, independently of the host-MMU-access
conditional affecting other memory types. Unavailable PDS space causes eviction
attempts, retries or errors, not successful placement into an unrequested type.

Eviction of a PDS BO uses LOCAL. It preserves the original requested mask;
non-fixed PDS/local movement uses `drm_bo_move_ttm`, retaining the TTM payload.
Revalidation returns to the permitted PDS mapping. This is residency change,
not a second dirty payload allocation. The already checked CPU fault and GPU
MMU binding use the same page array. No fixed-memory copy, bounce allocation,
user-page import or metadata-in-payload path is introduced by these alternatives.
The [path table](pds-backing-paths.csv) cites the exact functions/lines.

## Physical reuse and the historical Linux zero operation

Fresh physical memory need not mean a page frame never previously used by
Linux. In [Linux v2.6.32 page_alloc.c](https://github.com/torvalds/linux/blob/v2.6.32/mm/page_alloc.c),
`prep_new_page` tests `__GFP_ZERO` and calls `prep_zero_page`; that function
clears each page with `clear_highpage`. Thus buddy/per-CPU free-page reuse does
not evade zeroing for this request. It is not necessary to assume a modern
security policy or that every userspace-visible mapping is automatically zeroed.
Ordinary `GFP_KERNEL` alone does not provide this proof. The PSB source contains
other plain-GFP allocations, such as its communication page, but these are not
the selected type_dc PDS payload route.

The relevant freshness transitions are:

1. A new context creates a new userspace pool/BO request, not merely a free slot.
2. The checked provider creates a new kernel BO and TTM page array.
3. Each absent payload page is allocated with the explicit zero flag—even if
   its physical frame came from a previously used Linux free page.
4. Mapping and PDS/local residency retain those contents. No step re-zeroes
   an already populated page or recycled userspace slot.
5. P7H-030 supplies the existing selected-range non-overlap proof at 0x160 or
   0x1c0. It is cited, not rerun or strengthened by additional correlations.

This proves the contract for the checked implementation path. The generic DRM
wire ABI does not require an implementation to contain that allocation call.
Security intent cannot convert an unauthenticated implementation into this one.

## The target-membership boundary — P7H-035

The 2026-09-27 [restart discriminator check](pds-target-provider-identity.md)
inventories the available target artifacts and separates implementation selection
from package acquisition. It also reconciles the TVZ report's original manifest
digest with a later language-note-only commit. No provider membership is promoted.

Let **S** be the explicitly audited historical sources/patches above, with the
4.37 old-kernel visibility caveat excluded from unconditional preservation.
For the selected successful PDS route, every checked member of S has fresh
zero pages and retained contents. The source equivalence result is substantive.

Let **K** instead mean *every implementation consistent with retained ABI and
available target provenance*. The evidence does **not** establish `K = S`, or
even that the actual target's supplying implementation is a member of S.
An ABI header cannot constrain the private page allocator's flags, and no
installed old-provider binary/source binding occurs in the target record.
It would be incorrect to declare an exhaustive equivalence class by redefining
K as only the conveniently recovered archives. No real ABI-compatible non-zero
provider was found; none is invented to justify this distinction.

The existing [TVZ operator report](../../hardware-evidence/TVZ-001/operator-report.txt)
names `5.10.240-antix.1-486-smp`, driver `gma500`. The catalogued matching-family
[source](../../phase4-2-data/antix-source/drivers/gpu/drm/gma500/psb_drv.c)
has an empty private ioctl table (104–105), GEM/MODESET features, and GEM file
operations (491–517). It does not establish the retained private CMDBUF/PDS
provider. The original installed-module hash is absent, as the prior provenance
audit already states. Modern GEM zeroing would not establish legacy PDS placement
or SGX visibility. No live target query was made in this pass.

**Smallest remaining fact:** identify/bind the target's implementation servicing
the legacy PDS BO request to an audited provider contract, or supply the concrete
replacement provider's allocation-and-SGX-mapping implementation for that same
request. The unknown is not another first-use offset, a missing TTM zero flag,
or a PDS source-selector bit. It is the **target → supplying implementation**
edge. Package lineage cannot recover an absent deployment/implementation choice.

## Per-hole and final decision

| Selected offset | Audited source-family first use | Frozen target application |
| --- | --- | --- |
| +0x08 | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x0c | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x10 | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x14 | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x18 | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x1c | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x24 | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x28 | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |
| +0x2c | ZERO_FROM_FRESH_BACKING_CONFIRMED | CONDITIONAL on provider membership |

| Required matrix | Result |
| --- | --- |
| TARGET BACKING PROVIDER | **CONDITIONAL**; bounded historical source class proven, target membership absent |
| FRESH BACKING ZERO | **CONDITIONAL** for target; CONFIRMED for audited selected source routes |
| CONTENTS PRESERVED TO FIRST GPU USE | **CONDITIONAL** for target; CONFIRMED source contract for K-equivalent routes |
| FIRST-USE NO-OVERLAP | **CONFIRMED**, existing P7H-030 retained |
| NINE HOLES ON RESTRICTED FIRST-USE PATH | **CONDITIONAL** |
| 0x07000345 READ SET | **UNKNOWN** |

**Does the restricted implementation still require the exact read set?**
**CONDITIONAL**: the dependency is removable once the target provider is bound
to the proven fresh-zero/preserved-contents contract. It is not removed for an
unidentified provider. This does not prove that holes are dead, establish reads
outside the modeled allocation, or make recycled operation deterministic.
FG-02 therefore remains OPEN; its earlier suffix/secondary facts are preserved.
No implementation-blocker closure or subsequent FG-02 completion is claimed.

## Reproduction and validation

The acquisition JSON records exact URLs and digests; public source payloads
remain disposable. Source comparisons use the prior 4.41.1 original as baseline.
For 4.42 patch review, extract the source tar, read `debian/patches/00list`, and
apply only raw text after `@DPATCH@` using `patch -p1 --batch --forward`; do not
execute dpatch scripts. Preserve/report the already-applied I2C rejection.
The comparison CSV records hashes of the resulting relevant files as well as
unpatched files. Namespace equivalence removes only the reviewed `psb_` prefix
before `drm_`/`ati_pcigart_`; it does not ignore allocation flags or statements.

```sh
python3 tools/psb-dri-re/pds_backing_audit.py \
  --artifacts-root /tmp/sgx535-backing-provider
```

The negative regression changes an authorial allocator-string fixture by
removing `__GFP_ZERO` and requires digest rejection. Another test adds the VRAM
placement bit and verifies that the allowed-type set broadens. These tests
validate audit safeguards; the modified fixture is **not** a historical
counterexample and does not execute any kernel code.

Validation completed: both retained ELF hashes and the retained archive/spec
identity were reverified; five prior source-package hashes, the prior libdrm
files and candidate baseline files matched their recorded digests. The new
audit verified 195 acquisition/derived-file hashes and 30 Debian `.dsc`
size/digest rows. All 35 Phase7/evidence CSV files passed width/header checks;
new table keys and P7H-032–035 defining evidence rows are unique. All 456 local
Markdown file links in Phase7 resolved. The new script parsed/compiled and its
positive/negative checks passed. `git diff --check` passed after correcting
new CSV line endings. `references/` has no Git changes and the index is empty.
These checks authenticate the analysis artifacts, not a deployed provider or
hardware operation.
