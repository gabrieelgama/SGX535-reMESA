# First two-indexed-triangle square — 2026-10-06

**MULTI_TRIANGLE_ESTABLISHED.** One authorized SGX invocation produced the exact
predicted32×32 readback:256words`0xffff00ff`,768words`0x00000000`, with all
foreground coordinates`8<=x<=23,8<=y<=23`. Every oracle pixel matches.
TA completion, end-render,3D-memory-free and retirement were accepted for the same
operation; complete response/capsule/readback and live source/boot bindings agree.

This establishes a fourth vertex and two indexed foreground contributions in one
draw. It does not establish interpolation, perspective, depth testing, per-draw
color rebinding, repeated frames, a cube or physical square publication by that rendering call. A later, separate
[CPU/Xorg display publication](square-display-result-20261006.md) succeeded.

## Immutable successful configuration

See the [full-hash manifest](MULTI-TRIANGLE-REPRODUCTION.json),
[qualified candidate](two-triangle-experimental-candidate-20261006.json) and
[original prepared card](next-3d-execution-card-20261006.json).

- Driver Build ID:`8be2777b2a79eaa6651b89d19faf4d68cdcdc460`.
- Observer Build ID:`f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
- Image SHA-256:`0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d`.
- Successful boot:`83ee4ff8-a7f1-4468-b348-d10627f38d17`.
- Readback SHA-256:`6e9af8e8b6576b979aa78816d44bd3b0ffd31b70e169a982e83736d63ab91729`.
- Indices:uint16`0,1,2,1,3,2`; vertices`(8,8),(24,8),(8,24),(24,24)`;
  fixedz=.5,w=1,RGBAwhite,vertexstride32.
- Fragment words remain`001f00ff fca7f1f1`.
- Client exit/ioctl/operation errno0;outcome2;phase9RETIRED;ledger0x7.

The known-good FIRE3 triangle and centered physical-display reproduction remain
separate immutable milestones. No display publication was performed during the square render. The later
[display-only result](square-display-result-20261006.md) preserves its physical publication separately.

## Actual successful procedure

The [archived live campaign](artifacts/MULTI-TRIANGLE-20261006/live-campaign/)
contains the actual sources, commands, receipts, corrected derived card and
[18 sealed originals](artifacts/MULTI-TRIANGLE-20261006/live-campaign/one-authorized-square-call/originals/originals.seal.json).
The [139-file archive seal](artifacts/MULTI-TRIANGLE-20261006/archive-seal.json)
protects the procedure and derived visualization. Scripts are historical records,
not reusable permission or current boot bindings.

1. Verify the original card's exact module/image/client/UAPI hashes and the
   retained offline construction/build qualification. Do not rebuild/substitute.
2. Recover to normal STOCK locally; authenticate using local`sudo -v`.
3. Capture complete privileged STOCK health/ownership/recovery/file guards.
   The original80guard continuation PASS used boot
   `4b6515d2-9e04-47e9-8e2f-5de7f3b38624`.
4. Audit phase bindings; create an exclusive incoming directory. Transfer the
   qualified50,816,648-byte image once with900-second upper deadline.
   Actual transfer25.573539469973184seconds. Require copy success/fsync,
   independent stable size/hash and no writer; preserve failed historical files.
5. Stage image/backup/pending exclusively and atomically publish the appended
   GRUB entry. Retain STOCK saved/default and all prior entries. Independent
   poststage82/82PASS. Actual commands are in the archived`stage/command.json`.
6. Manually select the exact square FIRSTLOAD entry once. Authenticate locally;
   confirm normal userspace/display/recovery controls and no client run.
7. Capture83/83FIRSTLOAD guards, including exact module notes, image/config,
   hook, UNUSED capsule, startup witness and continuous producer isolation.
8. Run passive protected preparation56/56 and independent final guards204/204.
   Exclusive root-owned client/evidence directories and future output absence
   must be established. Preserve raw preparation source/capsule/kernel evidence.
9. Bind the derived one-shot card to this fresh boot and creation receipts.
   After a controller STOP, renewed passive continuity42/42PASS preceded dispatch.
10. The launch controller durably saved source/capsule/kernel/card/authorization
    originals, checked bindings again, and launched exactly once:

    ```text
    /root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/frozen-triangle-one-shot-response-i386 --one-shot-sgx535-rev121 /root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17/color.original.bin /root/sgx535-square-seq1-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c/evidence-83ee4ff8-a7f1-4468-b348-d10627f38d17/response.original.bin
    ```

11. Preserve post source/capsule/kernel and streams on every outcome. Retrieve
    through the read-only pinned-directory helper; preserve18files exclusively
    and hash-seal originals **before interpretation**.
12. Run saved-byte capsule/response/readback agreement plus live origin binding
    checks and closed source validation. Compare all1024pixel values against the
    exact square oracle; process exit alone is insufficient.

## Controller history and STOP discipline

A preparation adapter first compared the archived pre-FIRE3 backup with the old
live inode2616686 instead of its receipt inode2616687.79/80STOP was preserved;
corrected fresh80/80preparation was separately authorized.

A second offline adapter STOP duplicated the same witness in`source_witness`
and the legacy launch`witness`dictionary. Full PRE07 had passed, but no client
was launched. The corrected card explicitly aliases`source_witness`to the unique
`witness`object. Seven CPU checks reject conflict/staleness/missing or unexpected
references. A renewed continuation and fresh passive continuity preceded the one
call. Neither bug is a GPU requirement; both historical STOPs remain preserved.

Future reproduction needs new explicit authorization and fresh bindings. Never
run these archived commands against the consumed boot/capsule. Identity,
ownership, isolation, health, output-preservation or possible-issuance ambiguity
must STOP; there is no automatic retry. Authorization was consumed by this call.

## Next capability

The next offline probe is two separately indexed draws with independent immutable
magenta/green fragment programs and a narrowly scoped primary-PDS state rebind.
It tests per-draw color selection before the CPU-transformed perspective cube.
No two-color invocation is authorized. See the [roadmap](FIRST-REAL-3D-ROADMAP.md).

## Subsequent square display-only milestone

One separately authorized [square publication](square-display-result-20261006.md)
used the sealed256pixel SGX source through CPU/Xorg.10×nearest-neighbor
presentation produced a centered160×160magenta square; operator saw and recorded
it. Machine readback matched25,600magenta pixels and exact restoration of all
409,600saved bytes. [Full result](square-display-result-20261006.json) and
[card](square-display-execution-card-20261006.json) preserve the actual procedure.
Display taskSGX invocations0; no directSGXscanout. Authorization consumed;
no further live display or SGX execution authorized. Historical first triangle
and centered first physical triangle records remain unchanged.
