# Two indexed triangles: the first square

The next successful render added a fourth vertex and used indices
`0,1,2,1,3,2`. The two foreground triangles form a square covering **every pixel
from `(8,8)` through `(23,23)`**, inclusive. The complete 32×32 readback contains
256 pixels `0xffff00ff` and 768 pixels `0x00000000`, exactly as predicted.

TA completion, end-render, 3D-memory-free and retirement were confirmed, with the
response, capsule and readback attributed to the same operation. This establishes
multiple indexed triangles in the reconstructed path. It is not a 3D object,
depth test, interpolated-color result or repeated-frame demonstration.

## Verify it locally

```sh
python3 tools/reproduction/verify.py square
```

The [milestone manifest](../phase8/MULTI-TRIANGLE-REPRODUCTION.json) records the
successful identities and [139-file sealed campaign](../phase8/artifacts/MULTI-TRIANGLE-20261006/archive-seal.json).
The [raw readback](../phase8/artifacts/MULTI-TRIANGLE-20261006/live-campaign/one-authorized-square-call/originals/color.original.bin)
has SHA-256 `6e9af8e8b6576b979aa78816d44bd3b0ffd31b70e169a982e83736d63ab91729`.

| Artifact | Successful identity |
| --- | --- |
| Driver Build ID | `8be2777b2a79eaa6651b89d19faf4d68cdcdc460` |
| Observer Build ID | `f11d3abb072caa4e1d32836ef92ce201e9c9d126` |
| Image SHA-256 | `0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d` |

## Inputs and construction

Vertices are `(8,8,0.5,1)`, `(24,8,0.5,1)`, `(8,24,0.5,1)`,
`(24,24,0.5,1)`, with white vertex color and 32-byte vertex stride.
Indices are six uint16 values. The fragment program remains constant magenta.
The intentional delta from the triangle is geometry/index count and the associated
stream placement/relocations, not a new PDS, ISP or PBE experiment.

[Candidate construction](../phase8/two-triangle-experimental-candidate-20261006.json)
and [offline results](../phase8/first-3d-offline-qualification-20261006.json)
record the inputs and already-completed CPU/native/UBSan/i386 checks.
Read the [shared build limitations](README.md): a source checkout alone does not
reconstruct the qualified opaque input or the historical dirty build tree.

## Reproduce on the tested machine

The [historical square procedure](../phase8/MULTI-TRIANGLE-REPRODUCTION.md)
contains the actual deployment, boot, FIRSTLOAD/PRE07 and one-shot launch sequence.
Its [campaign directory](../phase8/artifacts/MULTI-TRIANGLE-20261006/live-campaign/)
retains commands, receipts, failed preparation records and the corrected final
card. They document the past run; they are not current execution permission.

1. Verify the exact modules, image, client and UAPI against the candidate manifest.
2. On the supported kernel, stage verified bytes and manually enter the matching
   first-load environment, retaining normal boot/recovery.
3. Bind the new boot, module identities, startup witness, isolated owner, unused
   capsule and new output destinations. Complete the required preparation checks.
4. Launch the qualified client once through the freshly bound one-shot path.
5. Preserve response, capsule, source/kernel records, client streams and all
   4,096 readback bytes before interpretation. Seal original files.
6. Require both same-operation lifecycle agreement and the exact pixel predicate:
   magenta iff `8 <= x <= 23` and `8 <= y <= 23`; zero elsewhere.

Steps 1–3 contain correctness and integrity checks. The campaign's numerous
receipts and named checkpoints provided additional research attribution; its
stale-inode and duplicate-witness controller bugs were not GPU prerequisites.
There is not yet a portable safe live wrapper that removes that controller work.
Missing or ambiguous evidence is not a successful reproduction and must not cause
an automatic retry.

## Show the already-rendered square

A later, separate [display publication](../phase8/square-display-result-20261006.md)
showed these preserved pixels through Xorg. The 32×32 source was enlarged 10×;
the 320×320 affected rectangle was centered at `(480,240)`, producing a visible
160×160 magenta square at `(560,320)`–`(719,479)`. It remained for 15 seconds;
all 409,600 saved destination bytes were restored exactly. The maintainer saw
and recorded it; the camera file itself is not part of the archived evidence.
See [display reproduction](DISPLAY.md). No SGX call was needed for that publication.
