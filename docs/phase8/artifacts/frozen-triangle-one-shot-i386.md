# Fixed one-shot i386 client artifact

The [client binary](frozen-triangle-one-shot-i386) is the **static ELF32
Intel 80386** build of the current reviewed
[`frozen_triangle_one_shot.c`](../../../tools/psb-dri-re/frozen_triangle_one_shot.c)
(source SHA-256 `4271a62846a0f02694e574161658920987ee89ad69e3bf5b2be79fc83f5634d2`).
The UAPI header SHA-256 is
`04dd2080deeb0056a366546fe27faddbdeff6c1657a2471c96e39749dc080420`;
the header copied into the candidate antiX module build has the same hash.

**Artifact SHA-256:** `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`.
Size: 771,320 bytes. `readelf` reports ELF32, little-endian, Intel 80386,
statically linked, Linux ABI 3.2.0. There is no `PT_INTERP`, dynamic section,
or userspace shared-library requirement. Target execution remains untested;
the Mini 12 was not contacted for this build.

Built twice, byte-for-byte identical, with local Clang 21.1.8 targeting
`i386-linux-gnu`, LLD, `-static -std=c11 -O2 -Wall -Wextra -Werror
-pedantic`. The temporary sysroot came from configured Debian trixie
`libc6-dev-i386-cross` and `libc6-i386-cross` 2.41-11cross1,
`linux-libc-dev-i386-cross` 6.12.38-1cross1, and
`libgcc-s1-i386-cross`/`libgcc-14-dev-i386-cross` 14.2.0-19cross1.
No package was installed on the Mini 12.

An i386 ABI check compiled against this header and run under offline
`qemu-i386` printed `sizeof=4268`, request prefix 16 bytes,
`operation_errno` offset 16, row counts offset 44, color bytes offset 172,
and ioctl `0xd0ac6440`. `_Static_assert` checked the sizes and offsets.
The candidate module and client use the identical UAPI header.

Under offline `qemu-i386`, no arguments and a wrong flag both returned 2
with the source's `No action` refusal. The wrong-flag syscall trace showed
no `open` or `ioctl`; no DRM node was opened and no color output was
created. The reviewed flag was **not** exercised. The binary has not run on
the Mini 12 or against any DRM device.
