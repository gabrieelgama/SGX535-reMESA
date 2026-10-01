# Final three rules — external evidence acquisition

**P7H-058, 2026-09-27: no architectural rule closes.** Six new public source
records were reviewed; nine downloaded artifacts have SHA-256 records in the
[source registry](final-rules-external-sources.json). The
[search ledger](final-rules-external-search.csv) distinguishes duplicates,
negative bounded results, and access failures. This is a bounded external
search, **not proof that no usable public source exists**. No internal corpus,
provider survey, Q analysis, or first-use model was regenerated.

## Acquisition and qualification

| Record | Public source | Narrow result and applicability |
| --- | --- | --- |
| E01 | [Official Native SDK R14.2-v3.4](https://github.com/powervr-graphics/Native_SDK/tree/8350f668a443f88c22107c4b81d79675e3d6b536) | Complete recursive manifest: 11,443 entries, no truncated response. No pathname contains `pds`, `eurasia`, or `assembler`, case-insensitively. Top-level license is MIT. The import commit is dated 2019-05-29; this is **not** the original release date. This excludes this manifest as an identified encoder lead, not hidden contents or all SDK history. |
| E02 | [Official SDK snapshot c1605c9](https://github.com/powervr-graphics/Native_SDK/tree/c1605c99281797e5cd4c8439e1bc679706bbb311) | 8,722 entries, not truncated. No PDS/Eurasia path; `assembler` matches Vulkan input-assembler documentation. Snapshot was located through [SwiftShader's public submodule](https://swiftshader.googlesource.com/SwiftShader/+/29cc245860746cb9ec174eba4b5f50bc8c6e790d/third_party/PowerVR_Examples). No source-level Series5 rule identified. |
| E03 | [Imagination patent US8669987B2](https://patents.google.com/patent/US8669987B2/en) | Followed the GB2449399 memory-management patent family. Describes macrotile/free-list storage management, but supplies no SGX535/DHOST mapping or LOAD3 completion dependency. Public patent disclosure is not evidence that a particular product implements every described mechanism. No implementation imported. |
| E04 | [AOSP Samsung SGX535 header](https://android.googlesource.com/kernel/samsung/+/android-samsung-2.6.35-gingerbread/drivers/gpu/pvr/sgx535defs.h) | Explicit GPLv2 header, Git blob `04f43be12b136acaac3515449f826075c9d6d00c`. Independently assigns status2 DHOST load bit `0x8`, with corresponding host-enable/clear bits. Read rendered source; direct raw download returned HTTP503, so no SHA-256 is invented. This corroborates register identity, not completion scope or consumer ordering. |
| E05 | [Linux v6.12 DMA guide](https://www.kernel.org/doc/html/v6.12/core-api/dma-api-howto.html) | Defines coherent DMA update visibility and the continuing need for ordering barriers; streaming mappings have synchronization/ownership requirements. Applies when the backend actually implements the documented API contract. Does not certify the selected legacy mappings or the scope of SGX internal cache invalidation. |
| E06 | [Linux v6.12 memory barriers](https://www.kernel.org/doc/html/v6.12/core-api/wrappers/memory-barriers.html) | Separates host ordering from DMA cache coherency. A barrier cannot substitute for required cache maintenance on a noncoherent path. Supplies no meaning for SGX status `0x138` masks `0x44/0x1`. |

The tag listing exposes 28 tags ending at R14.2-v3.4; older SDK releases are not
thereby proved nonexistent. Only manifests and license/metadata were acquired,
not arbitrary SDK code or embedded archives.

The exact/normalized Eurasia title searches returned the already known forum
trail, not a downloadable qualified manual. Internet Archive's **item metadata**
query `"Eurasia" AND "SGX535"` returned zero results; it is not a search of every
Wayback capture. The prior Sourcegraph/Debian/Wayback work remains in the
[existing external report](series5-pds-external-evidence.md); it was not rerun.

Other routes did not supply a new distinguishing claim:

- [US8046761B2](https://patents.google.com/patent/US8046761B2/en) describes task
  dispatch and shared processing storage. That does not identify the selected
  PDS operand stores or selectors. It revisits the generic architecture-patent
  route already considered; it is not counted among the six new source records.
- [Mesa's CSBGEN documentation](https://docs.mesa3d.org/drivers/powervr/csbgen.html)
  concerns Rogue. Modern PDS encoder hits lack an established SGX535 encoding
  equivalence and cannot constrain these words.
- [TI's hardware-definition addition](https://git.ti.com/cgit/graphics/omap5-sgx-ddk-linux/commit/?id=d654d4cd09ecb72847b61d2b58b6b0ca7ecf2283)
  was indexed, but direct retrieval returned403. Android raw requests returned503;
  attempted mirror paths returned404. Unread files were not treated as searched.
- [Yocto's EMGD1.14 announcement](https://docs.yoctoproject.org/pipermail/yocto/2012-July/007979.html)
  corroborates public kernel-source distribution. A file list does not specify
  maintenance completion or a PDS source domain.
- [The 2025 vendor forum reply](https://forums.imgtec.com/t/request-for-powervr-sgx-535-linux-driver-source-code/4136)
  discusses availability, not any of the three architectural implications.
- Exact register searches also returned SGX530/540/544 headers and an SGX544
  reverse-engineered register list with unknown descriptions. Similar names do
  not prove target compatibility. Intel enclave-SGX and unrelated PDS acronym
  results were discarded.

A search result explicitly marked **“PowerVR Series6 Graphics Overview —
Confidential and Subject to NDA”** was excluded unopened. Its locator is
`https://imgtec.eetrend.com/sites/imgtec.eetrend.com/files/download/201403/1600-2648-00.pdf`.
No contents or implementation claims were used. Existing quarantined PSP2/Intel
material remained excluded. Public indexing alone is not permission or provenance.

## R1 — publication: CONDITIONAL

**Rule:** fresh successful selected maintenance must cover every required
initialized/relocated payload domain through its first consumer.

**Model A:** host stores and translations are published, and completed SGX
maintenance covers all selected instruction/data consumers.

**Model B:** the same observed completion bits can occur while at least one
required payload/cache domain lacks that guarantee. This is an unresolved model,
not a demonstrated stale read.

**New checks:** E05/E06, exact PDS_INV1/PDS_INV_CSC/MADD searches, public kernel
and mailing-list searches, plus E04 register corroboration. The host API sources
exclude the inference “a barrier alone supplies all cache semantics.” They do
not choose A or B. A DMA-API backend could satisfy a host edge, but merely naming
that API does not implement it or establish the SGX-internal edge.

**Single distinguishing fact still needed:** the applicable completion-domain
implication for `0xAD4 -> status0x138:0x44`, `0xAE0 -> status0x138:1`, and
`0x804 -> status0x12c:0x04000000`, composed with the selected mapping contract.
A successful poll must cover the required payload domains, not just translation
or command receipt. Abort-on-timeout remains valid clean-room policy; it cannot
supply this missing implication.

**Most likely legitimate evidence:** target-qualified cache/register documentation
or publicly released initialization/maintenance source with the hardware
postcondition, including the otherwise undescribed status0x138 fields. Generic
Linux DMA documentation alone is insufficient. No backend was silently replaced.

## R2 — bootstrap/LOAD3: OPEN

**Rule:** the selected initialization/load sequence establishes every required
ready state, including the fourth DHOST load, before its first consumer.

**Model A:** INITEND and subsequent selected scene preparation order every
required load/state producer, including LOAD3.

**Model B:** those observations can precede LOAD3 availability or another
revision-conditioned prerequisite. No actual premature consumption is proved.

**New checks:** E03/E04, exact DHOST/status/host-clear terms, public kernel
hardware-definition and initialization leads. E04 independently corroborates the
fourth bit instead of explaining it away. E03 cannot turn the DHOST name into a
particular deallocation mechanism. No new evidence excludes implicit ordering,
separate hardware consumption, selected-path irrelevance, or an incomplete
interpretation. The existing scoped absence of a later explicit CPU wait is
unchanged.

**Single distinguishing fact still needed:** the target-qualified readiness
implication connecting DHOST load completion/consumer dependency and the
revision-conditioned initialization state to the first selected draw.

**Most likely legitimate evidence:** the DPM DHOST free-load/INITEND hardware
contract, or applicable publicly released bootstrap source documenting that
postcondition. Requiring fresh bits `0xF` is a possible stronger policy, **not
proof** of applicability, completion semantics, or the rest of initialization.
The existing ready observations and LOAD3 inventory are unchanged.

## R3 — source eligibility: OPEN for B1 and L12

**Rule:** each selected stream/context's complete pre-definition input domain is
contained in deterministic initialized launch state.

**Model A:** initialized prefixes and established launch controls supply all
pre-definition inputs.

**Model B:** a further launch/state producer is required but absent from the
clean-room contract. This does not assert that any selected word reads outside
its prefix or that state persists across launches.

**New checks:** E01/E02, patent/encoder searches, exact-title/archive queries,
public developer discussions, and current official documentation. These exclude
specific candidate *sources* as usable proofs, not architectural source classes.
No DS/temp/implicit class can newly be excluded; no finite accessible bank range
or complete initializer was recovered. No selected input model closes.

| Obligation | Context | Outcome |
| --- | --- | --- |
| FAMILY-1 | AUX-01/02 vertex/bounds | Complete pre-definition source eligibility UNKNOWN |
| FAMILY-2 | AUX-03/04/05 state | Complete pre-definition source eligibility UNKNOWN |
| FAMILY-3 | AUX-06 event | Complete pre-definition source eligibility UNKNOWN |
| FAMILY-4 | AUX-07 background | Complete pre-definition source eligibility UNKNOWN |
| FAMILY-5 | AUX-08/09/10/11 secondary | Even the single-word `0xaf000000` form lacks an established no-input rule |
| L12 | primary `0x07000345;0xaf000000` | Complete pre-definition source eligibility UNKNOWN |

The exact instruction streams, data, launch words and inherited constraints
remain in the [canonical family table](frozen-triangle-pds-families.csv). No new
controlled variation was found, so that corpus was neither rebuilt nor reinterpreted.

**Single distinguishing fact still needed:** an SGX535-applicable source/preload
eligibility rule covering these instruction forms in their separate launch
contexts, including any implicit inputs and read-before-definition behavior.
An encoder defining only explicit bit positions would narrow the problem but
would not alone prove the absence of implicit inputs.

**Most likely legitimate evidence:** a publicly released Series5 PDS instruction/
launch specification, or an applicable public assembler/encoder plus its complete
operand and launch-state contract. Such an encoder **could independently provide
this rule** if it covers source classes, bounds, implicit behavior and applicability;
one matching emitted word is insufficient.

The unseen **Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf** is
**VERY LIKELY to contain the needed class of information**, based only on the
[2009 public discussion](https://forums.imgtec.com/t/pds-programming/333) describing
PDS programming and constant/temporary stores. This is an inference about subject
matter, not knowledge of its contents, a promise that it resolves every form, or
proof of legitimate public distribution. No confidential copy is needed or accepted.

## Propagation and stopping boundary

No acquired source provides the missing architectural implication. Therefore the
shared [three-rule ledger](frozen-triangle-rule-discriminators.csv), CPU serializers
and refusal conditions remain unchanged. B1/L12 remain separate context obligations
under one R3 architectural family. R1 and R2 cannot merge merely because both poll
status bits: their postconditions differ.

B2 and FG-01 stay CLOSED; FG-02 remains OPEN for B1+B3+B4+L12. BO manifest and
whole-path relocation contract remain PARTIAL; all49 emitted wire records remain
covered. Static triangle PARTIAL; complete mode REFUSES. Nothing demonstrates a
new counterexample or hardware malfunction.

The bounded search covered independent vendor SDK, public kernel/header, official
host-contract, patent, forum/mailing-list, and archive-index routes. Repeated
unqualified query variations are not a rational next step. Transport-blocked
TI/Android source retrieval remains an access limitation, not exhaustion of those
repositories. The highest-value next acquisition is a **qualified PDS launch/
operand contract**; for R1/R2 it is the exact cache/DPM register postcondition.
A vendor public clarification could be sufficient; no message was sent.

## Reproduction and validation

Downloaded bodies remain only under `/tmp/sgx535-final-external/`. Every retained
body's URL, byte count and SHA-256 is persistent in the registry; SDK commits are
pinned. Re-fetch to a new cache and compare hashes before reuse. Dynamic HTML/API
metadata may change; a mismatch is not permission to substitute new contents.
E04 has a Git blob identity and URL but **no downloaded byte hash**. Failed
requests and excluded locators have no artifact. `references/` is unchanged.

Run the existing checks without executing historical binaries:

```sh
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_*pds*.py'
python3 -B -m unittest discover -s tools/psb-dri-re -p 'test_frozen_triangle*.py'
python3 -B tools/psb-dri-re/frozen_triangle_image.py --bundle
python3 -B tools/psb-dri-re/frozen_triangle_closure.py --complete
```

The last command must exit nonzero while the three rules remain unresolved.
No rule closed and no implementation code changed, so no fabricated architectural
regression was added. Existing negative/refusal tests retain their force.
Fresh validation: **42 PDS +108 triangle tests passed**. All four complete modes
(`image`, `bo`, `contracts`, `closure`) refused; partial bundle generation passed.
Both retained ELF hashes,20 prior public-source hashes and9 new artifact hashes
matched. Both SDK manifest predicates passed.43 changed/untracked CSV schemas,
586 local Markdown links and unique new evidence ID P7H-058 validated. Existing
Python tools compiled; no Python implementation changed. `git diff --check`
passed; references/ unchanged; index empty. No hardware was executed or validated;
Gate B BLOCKED, whitelist `[]`, hardware functionality UNVERIFIED.

Verify the ephemeral acquisitions without downloading or executing anything:

```python
from pathlib import Path
import hashlib, json
m = json.loads(Path("docs/phase7/psb-dri-re/final-rules-external-sources.json").read_text())
for a in m["artifacts"]:
    data = (Path(m["cache"]) / a["file"]).read_bytes()
    assert len(data) == a["bytes"]
    assert hashlib.sha256(data).hexdigest() == a["sha256"]
```

The next session must reacquire missing cache files from recorded URLs before
running this check; the repository does not contain their full bodies.
