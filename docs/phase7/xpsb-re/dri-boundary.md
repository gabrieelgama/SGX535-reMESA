# DDX, Xpsb and DRI client boundary

The `PsbDRIRec` producer is the **open Xorg DDX**, not `Xpsb.so`. In the public `xserver-xorg-video-psb` 0.32.0 source archive ([provenance](source-provenance.md)), `src/psb_dri.h:43–54` declares eight 32-bit fields, and `src/psb_dri.c:625–654` allocates and assigns that record to `DRIInfoRec.devPrivate`. `src/psb_accel.c:370–376` later fills `exaBufHandle` from the EXA BO. These source facts are CONFIRMED for **that source release** (P7C-003). Pairing the release byte-for-byte with retained `xpsb-glx` 0.18 remains UNKNOWN.

| offset | DDX `PsbDRIRec` field | retained `psb_dri.so` observation |
|---|---|---|
| `0x00` | `lockSAreaSize` | no confirmed use in screen initializer |
| `0x04` | `lockSAreaHandle` | no confirmed use in screen initializer |
| `0x08` | `sAreaSize` | no confirmed use in screen initializer |
| `0x0c` | `sAreaPrivOffset` | copied to screen-private `+0x20` |
| `0x10` | `pciVendor` | no confirmed use in screen initializer |
| `0x14` | `pciDevice` | copied to screen-private `+0x0c` |
| `0x18` | `cpp` | multiplied by eight for visual selection |
| `0x1c` | `exaBufHandle` | copied to screen-private `+0x28` |

The client initializer at retained `psb_dri.so` ELF VA `0x0004be92` tests `devPrivateSize == 0x20` before these reads (P7B-003). The eight-field source record is also `0x20` bytes on historical i386. Field-name correlation is **INFERRED** from the separately recovered DDX source plus binary offsets; byte-for-byte pairing is not confirmed (P7C-004).

`src/psb_dri.c:577–582` sets the DRI client driver name only when acceleration and `Xpsb` loading are enabled. `src/psb_driver.c:642–652` loads the `Xpsb` Xorg submodule. Separately, `src/psb_driver.c:1027–1047` maps PCI BAR0 through historical Xorg APIs, then `:1554–1559` calls `XpsbInit(pScrn, regMap, drmFD)`. The retained ELF's `XpsbInit` at `0x2690` consumes that three-argument shape, uses `drmFD` for libdrm calls, and derives an SGX register pointer by adding `0x40000` to the supplied mapping (P7C-005). This is evidence of **historical behavior**, not a safe present-day mapping method or hardware contract.

The historical paths therefore fork at the DDX:

```text
Xorg psb DDX (source 0.32.0, exact binary pairing UNKNOWN)
  ├─ DRIInfoRec.devPrivate = PsbDRIRec (8 × 32-bit fields)
  │    └─ DRI loader → retained psb_dri.so InitScreen (0x0004be92)
  └─ BAR0 mapping + DRM fd → retained Xpsb.so XpsbInit (0x2690)
       ├─ Xorg composite/video callbacks
       └─ historical XHW kernel-client thread
```

The DDX public header `src/Xpsb.h:61–88,90–117` describes `XpsbSurface` and exported composite/video entry points. `src/psb_composite.c:289,297` calls the Xpsb composite functions. This is a separate X-server 3D/composite path, not the DRI client's `PsbDRIRec`. The source's requested loader string `XpsbTakedown` (`src/psb_driver.c:268–275`) differs in spelling from the ELF export and C call `XpsbTakeDown`; runtime resolution for this source/binary combination is **UNKNOWN** and prevents claiming an exact matched DDX (P7C-006).
