# The first SGX535 triangle

On 6 October 2026, the successful triangle experiment (historically **FIRE #3**)
produced a 32×32 color buffer containing 120 magenta pixels (`0xffff00ff`) and
904 zero pixels. The footprint has rows `y=8..22`, with `x=8..30-y` on each row.
The inclusive bounding box is `(8,8)`–`(22,22)`.

The client and ioctl succeeded. TA completion, end-render, 3D-memory-free and
retirement were recorded for the same operation as the response, capsule and
readback. This is a rendering result; the physical-display publication was a
separate experiment.

## Verify the preserved result

```sh
python3 tools/reproduction/verify.py triangle
```

The [full-hash manifest](../phase8/FIRE3-TRIANGLE-REPRODUCTION.json) identifies
all 223 archived files. The readback hash is
`52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5`.
The [original color buffer](../phase8/artifacts/FIRE3-TRIANGLE-20261006/successful-procedure/fire3-one-authorized-call/originals/color.original.bin)
is 4,096 bytes of little-endian ARGB8888, not a screenshot.

| Successful artifact | Identity |
| --- | --- |
| Driver Build ID | `85ec06b428c99fac7f9127919b7a488d204f4a77` |
| Observer Build ID | `f11d3abb072caa4e1d32836ef92ce201e9c9d126` |
| Boot image SHA-256 | `3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e` |

## Reproduce a new render

Read the [shared prerequisites and build limitations](README.md) first. The
[historical reproduction tutorial](../phase8/FIRE3-TRIANGLE-REPRODUCTION.md)
records the actual build, image construction, verified transfer, staging, manual
boot, preparation, immediate bindings, launch and evidence retrieval in order.
Its linked source snapshots and command receipts supply the exact build inputs;
there is no substitute generic `make install` procedure.

The minimal practical sequence is: obtain and verify the exact artifacts; prepare
the matching first-owner boot; verify current ownership, identities, lifecycle,
isolation and unused capsule; prepare new evidence destinations; perform one
explicitly authorized client call; preserve originals; compare the complete
readback and lifecycle. Never reuse the archived boot UUID or authorization.
The original one-shot client took `--one-shot-sgx535-rev121`, a color-output path,
and a response-output path. Use the launch controller's fresh bindings rather
than running the historical command against its consumed directory.

The successful rendering delta was just eight fragment bytes:
`00000000 f8040140` became `001f00ff fca7f1f1` (little-endian byte sequence
`ff001f00f1f1a7fc`). Other rendering state was retained from the previous candidate.
The previous render completed but returned all zero; the replacement strongly
supports the fragment-color hypothesis without proving the earlier TA coverage.

For physical presentation of this exact image, use the separately qualified
[CPU/Xorg publication path](DISPLAY.md). It does not run SGX again and does not
establish direct scanout. Each new live operation needs its own reviewed scope.
