# First triangle: forward blocker burn-down

**Historical engineering chronology:** The [post-Attempt-03 checkpoint audit](post-attempt03-checkpoint-audit.md)
is authoritative for current blockers. Earlier statements below that a
provider/service does not exist describe their respective earlier checkpoints;
the fixed kernel implementation now exists, but its rejected module is not
target ABI-qualified and the hot transition is not currently qualified.

Target: Dell Inspiron Mini 12 / Poulsbo / SGX535 rev121. Phase 7 remains complete with an unresolved evidence boundary. This pass performed **offline analysis and host-model changes only**. No target contact or hardware action occurred. Gate B is **BLOCKED**; whitelist `[]`.

## 1. Observed revision through the historical bootstrap

The HW-observed CORE_REVISION is `0x00010201`; retained masks decode major 1, minor 2, maintenance 1. The historical Xpsb option function derives `100 + (raw & 0xff) + 10 * ((raw >> 8) & 0xff)`, giving CPU branch code **121**. This numeric branch selector is a historical Xpsb software value. Its equality to the project's rev121 label does not equate it to PCI revision or prove bootstrap safety.

The retained `Xpsb.so` (SHA-256 `da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f`) shows:

- `XpsbInit` at `0x28c0–0x28c7` allocates the `0x140c`-byte private state with `Xcalloc`; at `0x2c12` it calls `Xpsb_set_vopt`, then at `0x2c1a` calls `Xpsb_sgx_initialize`.
- `Xpsb_set_vopt` at `0x4b04–0x4b1a` reads the SGX-relative `+0x14` word and computes the selector. Code 121 is outside its 107–113 jump-table range; the path at `0x4c6f–0x4c7b` returns without option stores. There is **no historical rejection** at that branch.
- `Xpsb_sgx_initialize` at `0x3828`, `0x3893–0x38bb` selects its `+0xa74` and `+0x804` CPU write values from the zero option words. For this path the modeled values are `0x05188200` and `0x0000ffff`. This is CPU behavior, not a safe MMIO recipe or a ready-state postcondition.

The previous rejection came from `frozen_triangle_contracts.bootstrap()` admitting only 107/108/109/111/113. **Classification: C — PROJECT_CHECKER_RESTRICTION.** The corrected model admits only the separately observed raw `0x00010201` with qualified-revision provenance on the historical zero-option default path. Nearby unqualified revisions still fail closed; no wildcard fallback was added. The dry run records the CPU branch result and `ready=False`. This closes the **model admission mismatch**, not FT-SERVICE or R2 readiness.

## 2. FT-BO and target provider

| Obligation | Current evidence/status |
| --- | --- |
| BO sizes, domains, allocation flags, alignments, 10 roles | TESTED_OFFLINE; historical placement classes support the policy, not a live provider. |
| Synthetic GPU addresses, offset resolution, relocation, range and alias checks | TESTED_OFFLINE. Distinct user BO handles are now required; `LOCAL` CPU interfaces cannot be assigned a GPU address. |
| Live BO allocation, validated GPU VA, CPU/GPU mapping, USE register reservation | REQUIRES_LIVE_TARGET or a target-qualified implementation; no real values were supplied. |
| Residency, exclusive ownership, lifetime through completion, teardown | Required contract; no live provider enforces it. |
| Final payload and translation publication | R1/B3 CONDITIONAL: host ordering and abort policy are modeled; device visibility postcondition is UNKNOWN. |

The [offline BO resolver](../../tools/psb-dri-re/frozen_triangle_bo.py) constructs deterministic user BO bytes from caller-supplied addresses. It is **not** a kernel allocation/mapping adapter. Synthetic addresses and handles in the dry run are never target proof.

### Kernel provider boundary (retained source, no target contact)

The candidate legacy kernel registers `DRM_PSB_CMDBUF` at private index 0, `DRM_PSB_XHW_INIT` at 1, and `DRM_PSB_XHW` at 2 (`PSB_psb_drv_c.txt:85–109`). Its CMDBUF handler validates the BO list, fixes relocations, obtains the command/TA/OOM BOs, allocates or looks up a scene, queues a TA task and returns fence state (`PSB_psb_sgx_c.txt:1237–1430`, `PSB_psb_schedule_c.txt:1230–1295`). TA scheduling hands scene binding and fire to the XHW client (`PSB_psb_schedule_c.txt:141–203`, `PSB_psb_xhw_c.txt:146–176`); XHW_INIT itself requires an initialized shared BO and registered client (`PSB_psb_xhw_c.txt:416–470`). This is an integrated service, not a single command ioctl that can be copied in isolation. The exact historical kernel/binary pairing remains unproved.

The retained antiX `gma500` source registers **zero** private ioctls (`psb_drv.c:101–105,503–516`). Its dumb GEM creator calls `psb_gtt_alloc_range(..., PAGE_SIZE)` regardless of the `align` argument, and the allocation is from the GTT resource (`gem.c:50–63`, `gtt.c:324–356`). On a CPU mmap fault it pins the object; pinning inserts its pages into the default SGX MMU at `gatt_start + offset` (`gem.c:135–158`, `gtt.c:235–259`). The actual target `gatt_start` is not established by this offline source inspection. The legacy frozen plan requires separately validated PDS, RASTGEOM and MMU placements, a USE register reservation, scene/TA memory, relocation, device publication and fence-held lifetime. GEM mmap pinning does not establish those postconditions. Its unpin wait is for the display blitter (`gtt.c:267–300`), not proof of SGX task completion.

A minimum kernel-owned frozen-scene provider would have to bind and retain the exact ten BO roles in their qualified address domains, apply the fixed relocation set after validation, publish both payload and translation state, establish the XHW/scene/TA ready state, submit only the fixed control sequence, attribute completion, and retain mappings until completion or reviewed recovery. No such provider exists in the retained gma500 implementation. Adding a private ioctl that merely forwards the current synthetic packet would omit these dependencies. No kernel code or live provider was claimed by this pass.

## 3. FT-SERVICE, auxiliary state and L12

The historical `psb_dri.so` path issues a 144-byte index-0 CMDBUF argument; retained Xpsb uses XHW_INIT index 1 and XHW index 2 with a shared 132-byte communication object. The candidate legacy kernel names corresponding handlers, but exact paired-build compatibility and target execution are unproved. The retained antiX `gma500` source has an **empty `psb_ioctls[]`** and declares MODESET/GEM, not the legacy CMDBUF/XHW service (`docs/phase4-2-data/antix-source/drivers/gpu/drm/gma500/psb_drv.c:101–105,503–516`). Installed-module correspondence and the current live state remain separately unverified. No legitimate target submission adapter can be completed from the present interface evidence; the offline packet must fail closed.

Auxiliary CPU images are preserved: secondary, vertex/bounds fetch, state-upload, event and background families have recorded bytes and relocations. FT-AUX remains open because their eligible pre-definition source domains are not proved. This is the **same kind of R3 architectural eligibility gap** as primary L12, across different launch contexts; it is not erased by counting the programs together. This pass produced no new PDS source-selection evidence. L12 remains open, and source containment remains PARTIAL/UNPROVED.

## 4. Verification and exact-action gate

The current offline dry run reports all 51 scene objects, 10 BO roles, 49 wire relocation records, CPU bytes where established, unknown kernel/service payloads as null, and a synthetic non-executable submission template. Two serializations produced SHA-256 `2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e`. The complete serializer still correctly refuses `PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE`.

Gate B action reviewed: **one 32×32 frozen triangle via the historical TA/raster CMDBUF and XHW path**. Exact target interface, live BO addresses/bytes, service context, publication, bootstrap/table readiness, primary and auxiliary source containment, completion attribution, and applicable hang recovery remain unproved. The last passive capture had the active display on `gma500drmfb`; current state is UNKNOWN. **Gate B: BLOCKED. Whitelist: `[]`. First triangle: NOT ATTEMPTED.** No call may treat the CPU branch-model fix or synthetic command template as permission to submit.

The smallest remaining **forward engineering** obstacle is a target-qualified, kernel-owned BO/CMDBUF/XHW provider compatible with the selected frozen path. The smallest independent **architectural evidence** obstacle remains the complete pre-definition source bound for primary and auxiliary launches. Both must close, along with R1/R2 publication/readiness and recovery, before a future exact-action Gate B review could pass. Do not repeat closed Phase 7 archaeology without genuinely new evidence.

## Subsequent offline kernel-service reconstruction

[Kernel submission reconstruction](kernel-submission-reconstruction.md) traces the candidate historical CMDBUF handler through BO validation, relocation, scene scheduling, XHW bind/fire, IRQ completion and recovery, then compares each dependency with retained gma500. The XHW bridge is submission-critical; current retained gma500 has no equivalent scene/TA service. Exact pairing of the candidate historical kernel with the retained userspace binaries and identity of the retained gma500 snapshot with the installed target module remain unproved.

An offline fixed-scene C contract now validates a 16-byte request with no address, handle, command or relocation input; the ten internal BO descriptors; all 49 canonical relocation records; and the selected TA/raster register-offset sequences. It models publication, service and completion as separate claimed transitions and holds resources after a post-submission failure. These checks are host-tested only. They perform no kernel integration, device allocation, MMIO or submission and do not change FT-BO, FT-SERVICE, R1, R2, L12, recovery, Gate B or the empty whitelist. A real backend still needs a qualified SGX VA owner, scene/XHW service, payload and translation publication, attributable completion, and target-specific recovery.

The continued offline pass added a failure-unwinding scene owner, a bounded VA reservation ledger, and a one-time C relocation patcher. All six deterministic user BO images match the Python golden images under two synthetic VA maps. The retained gma500 pin path ignores SGX MMU insertion failure, and partial insertion can occur; the present primitive cannot be treated as an atomic scene mapping. The archived kernel snapshot is incomplete and is not proved to match the installed Mini 12 module. These facts leave live BO/VA ownership and the scene/XHW service conditional or blocked despite stronger offline tests.

The subsequent pass recovered the exact public antiX source package and built
an unregistered fixed-scene BO-owner draft into a temporary i386 gma500 module.
The draft reserves separate SGX VA domains, excludes the retained GTT range,
pins GEM shmem pages, inserts default-PD mappings with exact prefix rollback,
and releases them in reverse order. A generated sparse C image now reproduces
the six selected pre-relocation user BO images byte for byte after complete
zeroing. These are source-level and offline build/test results only. The
installed target module, full live VA ownership, USE reservation, publication,
scene/TA/XHW service, completion, and recovery remain unproved; L12 and
FT-AUX source containment remain unchanged. Gate B is BLOCKED and whitelist
remains `[]`.

The retained public Fedora `psb_regman.c` and `drm_regman.c` were then traced
for the exact USE base helper. A fixed offline planner now derives the
candidate first-use pixel/vertex register sequences 3/4 from the historical
3–15 free list and validates every selected USE relocation against the same
512 KiB base window. The kernel owner applies the 49 relocations to its
resolved CPU images after SGX VA assignment, and the i386 compile-only build
still links. Live register programming, fence ownership, GPU visibility and
submission remain unimplemented and unauthorized. FT-BO is still conditional;
FT-SERVICE, R1, R2, completion and recovery remain blocked or unproved.
