# FIRE #3: first established SGX535 triangle reproduction

Milestone: **KNOWN_GOOD_TRIANGLE_READBACK** / **TRIANGLE_ESTABLISHED**, 2026-10-06.
This is the procedure that actually produced FIRE #3, backed by copied original
scripts, command vectors, receipts and seals. It is not an execution authorization.
No physical-display triangle was claimed by FIRE #3. No repeat is authorized.

Subsequent visible milestone: the separately authorized
[centered CPU/Xorg publication](display-publication-centered-result-20261006.md)
showed these preserved pixels on the physical display and restored its original
contents. It made no new SGX invocation and did not change this rendering baseline.
The permanent [visible-triangle reproduction tutorial](VISIBLE-TRIANGLE-REPRODUCTION.md)
and [full-hash manifest](VISIBLE-TRIANGLE-REPRODUCTION.json) describe its actual
source-to-LVDS procedure, with the earlier `(64,64)` non-observation retained separately.

The [machine-readable manifest](FIRE3-TRIANGLE-REPRODUCTION.json) is the full
identity/dependency lock. [Historical successful procedure](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/PHASE8-HANDOFF-FIRE3-20261006.md)
and [sealed result](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/outcome-report.json)
are authoritative. The archival scripts retain historical `authorized=true` flags;
these describe a consumed permission and MUST NOT be executed as current permission.

## Immutable successful identities

Recorded repository HEAD: `f8565115977bda8a529401a7ac5cba822d1a0d36`. This was a dirty
working tree. The execution card did not separately record the commit; commit
alone cannot reproduce the build. The manifest locks the successful build inputs,
source snapshots and exact shipped binaries. Do not use a nearby rebuild.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| driver | 250788 | `c925caedebcc0699aef53227d29e64b47bf2833efd9d2e611cba276dd3a08ab3` |
| observer | 26284 | `2e35e863d51dbc1d407feef08f08bb2edc6390af621a0f173a9b1a73ec76158a` |
| image | 50816631 | `3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e` |
| client | 775264 | `2f84917f96db2859678797a327d40d9638325e5c5efb6e0dfccef44d756b4835` |
| uapi | 760 | `04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420` |

Driver Build ID: `85ec06b428c99fac7f9127919b7a488d204f4a77`.
Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
Client source SHA-256: `919c2611e048e4a660ecae3542264adf5a5367d9b0fbe7fbceff2f45f90adedf`.
Exact source/binary/UAPI copies, modules and the boot image are archived in
[known-good-artifacts](artifacts/FIRE3-TRIANGLE-20261006/known-good-artifacts/).
The readback SHA-256 is `52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5`.

Successful boot: `f1ab6606-0561-445f-a397-28a028516cd7`, Dell Inspiron1210 / Mini12, i686,
`5.10.240-antix.1-486-smp`. Established physical identity: `CORE_ID=0x01130000`,
`CORE_REVISION=0x00010201`, SGX535 rev121. This work made no new MMIO probe.
Physical evidence remains in the Phase7/core-identity corpus; revision identity is
not inferred from a filename. Future machine/boot/loaded notes must be freshly bound.

The exact i686 GCC14/binutils sysroot, antiX5.10.240 source/Kbuild output, fixed
build timestamp/user/host, ARCH and host compiler flags are recorded in
[driver-build-command.json](artifacts/FIRE3-TRIANGLE-20261006/build-qualification/driver-build-command.json).
[Client command vectors](artifacts/FIRE3-TRIANGLE-20261006/build-qualification/client-commands.json)
preserve its earlier approved build. These dependency archives are not replaced
by installing today's compiler. Any unavailable restricted/opaque entry input
must remain opaque: use the authorized archived binary or obtain legitimate
access, never reconstruct it. The permanent archive includes the known-good
binary/image, so using the exact successful artifacts does not require rebuilding.

## Exact experimental delta

Original foreground fragment words: `00000000 f8040140`.
Successful words: `001f00ff fca7f1f1`, eight bytes at USE+0..7, little-endian dwords.
Exact byte hex: `ff001f00f1f1a7fc`; SHA-256 `ce03eb07b128b67d091f5a1cbaaee9d40eff5d896476f81cfed6af5e06455e6e`.
Intended packed diagnostic color: **ARGB 0xffff00ff**.
`SGX535_EXPERIMENTAL_CONSTANT_FRAGMENT=1` was set in the preserved module Makefile.
Normal builds without it retain the original words. Geometry, indices, TA setup,
PDS launch, ISP, PBE/surface, observer, client and fixed UAPI were kept unchanged.
[Minimal-change verification](artifacts/FIRE3-TRIANGLE-20261006/build-qualification/minimal-change-verification.json)
and canonical scene/address-plan hashes in the manifest lock that scope.

This was initially **EXPERIMENTAL_HYPOTHESIS — NOT ESTABLISHED**. Its later result
is **CONSTANT_FRAGMENT_HYPOTHESIS_SUPPORTED**, not retrospective proof of FIRE #2
TA-output coverage or every internal shader/PBE detail.

## Actual successful reproduction sequence

Historical paths/UUIDs below identify the past attempt, not future destinations.
A future maintainer must have separate preparation and one-shot execution authority,
fresh first-owner lifetime, exact bytes and new exclusive evidence destinations.
Never copy the archived live controller and run it against its historical boot.

1. **Build modules offline.** The isolated `build/module` tree reused the qualified
   frozen path/opaque entry and observer. The explicit eight-byte variant was
   enabled in [driver.Makefile](artifacts/FIRE3-TRIANGLE-20261006/source-snapshot/driver.Makefile).
   Run the exact recorded make argv/environment only in a fresh output workspace
   with the identified prerequisites. Forced rebuilds were byte-identical;
   ELF32/i386, vermagic,253 driver imports,38 observer imports and CRCs passed.
   The current commit alone is not this build recipe.
2. **Verify artifact identities.** Compare full bytes/SHA-256/Build IDs with the
   manifest, approved client source/binary and UAPI; do not rebind old hashes to
   changed bytes. [Offline candidate receipt](artifacts/FIRE3-TRIANGLE-20261006/build-qualification/experimental-candidate.json)
   records the original pre-live state, not the final milestone.
3. **Construct and verify image offline.** The exact
   [builder](artifacts/FIRE3-TRIANGLE-20261006/source-snapshot/build_frozen_provenance_image.py)
   takes `--qualified-root` and `--output`, consumes the recorded base initramfs
   plus qualified module pair, binds `/init` to the final packaged hook and builds
   two images. Its CLI/output receipts establish the construction; a separate
   historical image-build launch receipt was not retained. Do not invent one.
   [Finished-image identity](artifacts/FIRE3-TRIANGLE-20261006/image-construction/identity.json),
   hook/init copies and reproducibility receipt establish exact packaged bytes,
   observer-before-driver insertion, hook hash binding and shell syntax. Use
   fresh output directories for any future build and require exact output hashes.
4. **Establish healthy STOCK.** The successful transport campaign continued on
   STOCK boot `fc623297-419e-4bce-85d1-271c1b937d0e` after corrected76/76 preflight
   and fresh20/20 continuity. Preserve normal STOCK recovery/default, authenticate
   locally with `sudo -v`; never send passwords over the controller.
5. **Transfer exactly once to a new incoming file.** The first transfer failed;
   its partial bytes were never repaired/reused. The authorized replacement used
   a distinct transfer2 destination, exclusive creation and900-second deadline.
   [Exact transfer command/receipt](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/image-transfer/receipt.json)
   records the actual argv and28.548412957927212-second successful duration.
6. **Independently verify transfer.** Completion, no remaining writer, stable
   size50,816,631, persistence, full hash and source identity were verified before
   publication. [COMPLETE_VERIFIED receipt](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/transfer-complete-verified.json)
   and independent verifier retain exact receipts. Timeouts were never success.
7. **Stage exact image and publish one reviewed GRUB entry.** Execute only under
   fresh authority using the audited stage source/argv and exclusive destinations;
   keep recovery/default intact. [Stage source](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/stage.audited.py)
   and [independent poststage receipts](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/independent-poststage/decoded.json)
   record bytes/config receipts and validation.
8. **One manual FIRSTLOAD selection.** The actual entry was
   `EXPERIMENTAL SGX535 rev121 FROZEN 85ec06b428c99fac7f9127919b7a488d204f4a77 CONSTANT-MAGENTA 3d9eb6ba FIRST-LOAD ONLY (no SGX)`.
   [Exact entry](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/candidate-entry.proposed)
   preserves root UUID/kernel/initrd lines. Boot reached normal userspace in about
   90seconds. Local keyboard/GRUB/power recovery remained available.
9. **First-owner qualification.** [First-owner source](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/first-owner.root-source.py)
   and [fresh82/82 receipts](artifacts/FIRE3-TRIANGLE-20261006/transfer-firstload-procedure/first-owner/decoded.json)
   established actual boot `f1ab6606-0561-445f-a397-28a028516cd7`, loaded driver/observer notes,
   FIRSTLOAD continuity, healthy kernel and no earlier invocation.
10. **Startup/source lifecycle and isolation.** Validate the qualified startup
    witness and passive interval; no submission/ACK/reset may manufacture a clean
    guard. Current witness hash is retained in the execution card and sources.
    A repeated witness byte hash does not independently identify a boot; surrounding
    boot/module/provenance bindings are essential.
11. **UNUSED capsule before invocation.** Validate complete producer-originated
    capsule and no unexpected admitted operation. A CLOSED/consumed capsule must
    never be reset/reused for another authorization.
12. **Correct controller and preserve continuity.** Structural hash validation
    passed28/28; existing boot reused with47/47 continuity. The original false STOP
    and late operator report remain immutable. This correction did not change GPU
    bytes, reboot or rearm the producer.
13. **Protected preparation and PRE07.** Exclusive protected client/evidence
    destinations, source/capsule/kernel originals and preserved context passed
    56/56; independent final guards204/204. [PRE07 evaluation](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/pre07-live-evaluation.json)
    established LIVE PASS. The copied controller/source receipts preserve the
    actual machinery, not a replacement idealized flow.
14. **Immediate pre-call bindings.** [Immediate42/42 source/receipts](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/immediate-fire-precheck/source.py)
    revalidated boot, loaded notes, source interval, capsule, health, protected
    directory/client inode and absent future outputs. Exact
    [execution card](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/one-shot-execution-card.json)
    bound that permission to this boot and exactly one launch/ioctl attempt.
15. **One authorized launch.** The archived
    [actual dispatch source](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/authorized-fire3.audited.py)
    created durable intent/authorization/evidence, redirected both streams into
    exclusive files and invoked the exact client argv below with30-second wait,
    no retry. The durable controller/remote intent consumed permission at possible
    issuance. The following is **historical, NOT permission to run now**:

    ```text
    /root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/frozen-triangle-one-shot-response-i386 --one-shot-sgx535-rev121 /root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/evidence-f1ab6606-0561-445f-a397-28a028516cd7/color.original.bin /root/sgx535-frozen-seq1-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/evidence-f1ab6606-0561-445f-a397-28a028516cd7/response.original.bin
    ```

16. **Preserve before interpretation.** Client saved full4268 response and4096
    image; producer capsule/source/kernel and process receipt were preserved on
    all outcomes. Originals were retrieved read-only into exclusive local files,
    hash/size checked and sealed before interpretation. See
    [18-file original manifest](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/originals-manifest.json)
    and [original seal](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/originals-seal.json).
17. **Seal all evidence.** The full successful controller/preparation/operation
    archive has a95-file seal; transfer/FIRSTLOAD preceding STOP has an81-file seal.
    These copies preserve every original byte; original archives remain unchanged.
18. **Interpret independently.** Complete response/capsule/color equality plus
    actual pre/post boot/module/source bindings established same-current-operation
    provenance. Return code alone was insufficient. Closed capsule is state4;
    source state3, reasons0; accepted ledger0x7 and terminal phase9 RETIRED agree.
19. **Validate exact spatial footprint.** Read the raw little-endian4096-byte image
    and compare every pixel, not an appearance heuristic. The copied
    [spatial analysis](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/spatial-analysis.json)
    and [reference comparison](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/diagnostic-reference-comparison.json)
    show zero mismatches. General SGX edge-rule semantics were not qualified.

## Successful reference result

FIRE #3 invocation count **1**; client exit **0**; ioctl **0**; operation errno **0**;
phase **9 RETIRED**; ledger **0x7**. TA completion, end-render,3D-memory-free and
retirement confirmed. Complete response/capsule/readback attributable to one
current operation. Readback **32×32 /1024pixels /4096bytes**: **120** pixels
`0xffff00ff`, **904** pixels zero. Coordinates: `y=8..22`, `x=8..(30-y)` inclusive;
bounding box `(8,8)..(22,22)`. Expected triangular footprint established. The
pixel-center reference for vertices `(8,8),(24,8),(8,24)` excluding hypotenuse ties
matches the complete image. **Triangle: ESTABLISHED** in GPU-memory/readback.
[Lossless derived view](artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/readback-nearest512.derived.png)
is a visualization, not the original evidence.

## GPU/rendering requirements

The known-good path uses the frozen 32×32 geometry/index/TA/PDS/ISP/PBE/surface
construction plus the explicit constant-color fragment sequence. Valid backend
ownership, source lifecycle/admission, accepted-event semantics and retirement-gated
color capture remain necessary to this qualified path. No claim is made that every
campaign guard or historical script is an intrinsic GPU programming requirement.

## Reproduction/integrity requirements

Exact source/artifact identity, loaded producer/boot bindings, fresh source exclusion
and isolation, operation-owned immutable capsule, full-response/color preservation,
exclusive no-overwrite destinations and no retry after possible issuance protect
attribution/reproducibility. Boot/readiness is not FIRE permission. A new reproduction
requires fresh qualification/explicit authorization; this successful boot is consumed.

## Historical preparation infrastructure

SSH transport/known-host pinning, sudo authentication, controller AST rebinding,
protected-root copies, manual GRUB recovery and three layers of guards were the
original campaign machinery. Their actual scripts/receipts are preserved. They are
not established SGX ISA/geometry requirements. Future authorized tooling may simplify
or replace them only while retaining their substantive integrity/ownership guarantees.

## Historical controller failures

- Stale `custom.cfg` profile binding: correct to the authoritative published receipt;
  never weaken another guard to fit old metadata.
- Historical `sys.argv` boot override: explicit current bindings must remain
  authoritative; CPU controller tests rejected stale/positional replacement.
- First incoming transfer incomplete (6,717,440 of50,816,631bytes): preserve partial
  bytes; never call a deadline success. A separately authorized exclusive transfer
  with suitable900-second deadline produced the actual successful image.
- Two expected references to the same source-witness hash: global literal uniqueness
  was wrong. Validate unique structural roles resolving to one same-provenance
  witness; reject conflicts, stale/malformed references and unexpected duplicates.
  The28/28 corrected regression suite is archived.

These are lessons, not GPU prerequisites. Previous STOPs are not retroactively
hardware faults or rewritten successes.

## FIRE #1 and FIRE #2 context

[FIRE #1 DHOST analysis](dhost-load-hold-fix-20261005.md) establishes a load-
completion-consumption defect: four load kicks included DHOST, but actions22–24
polled/acknowledged/verified only mask0x7. The surviving STATUS2 bit0x8
`DPM_DHOST_FREE_LOAD` hit the strict unknown-STATUS2 predicate at STATUS_PROCESS
stage21 and caused HOLD/-1. The correction consumed all four observed current-
operation load completions with mask0xf before INITEND/TA; it did not relax scene
acceptance. Ledger0 and `issued=1` did not prove TA submission. The stale checker
also expected capsule CLOSED=6 instead of the producer's actual CLOSED=4.

FIRE #2 used the corrected lifecycle and retired with ioctl/client success,
TA/end-render/3D-memory-free confirmed, ledger0x7 and full4096-byte all-zero color.
It retained no TA-output memory, so post-TA selected-triangle survival remains
unknown retrospectively. That motivated the one-variable constant-fragment test.
FIRE #3 supports that hypothesis; historical uncertainty is not rewritten as certainty.

## Reproduction STOP conditions

Identity/image/hash mismatch, unexpected boot, ownership failure, startup/source or
isolation invalidation, non-UNUSED/invalid capsule, destination/preservation failure,
possible prior issuance, unexpected lifecycle HOLD/fault or ambiguous outcome require
STOP and evidence preservation. No automatic retry, reset, reload or repair to obtain
PASS. These safeguards protect evidence integrity; they are not all intrinsic hardware
requirements. Specific hardware faults must be supported by their own evidence.

## Consistency check and current display boundary

CPU-only: `python3 tools/display/verify_fire3_reproduction.py` checks the archive
hashes/seals, immutable identity copies, source image and Markdown/manifest agreement.
It neither connects to hardware nor launches a client.

[Display publication plan](display-publication-20261006.md) keeps this rendering
baseline unchanged. Reusing the preserved pixels for CPU display publication requires
no new SGX render. Direct SGX scanout is a separate, unestablished milestone.
