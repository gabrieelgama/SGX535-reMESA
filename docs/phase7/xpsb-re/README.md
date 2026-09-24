# Xpsb.so and historical PSB ABI: static investigation

This continues the [psb_dri.so analysis](../psb-dri-re/final-report.md) from the current Phase 7 worktree. The retained `xpsb-glx` 0.18 package is evidence, not executable test material. No historical binary, ioctl, MMIO operation or GPU command is run here.

The follow-up resolves the [static-buffer source and its composite-path accessor](static-buffer.md), identifies a stronger [5.0.1.0046 PSB kernel-source family match](version-pairing.md), and narrows the [external firmware inventory](bootstrap-dependencies.md). A later [XHW init trace](xhw-init-frozen-draw.md) resolves the operation-zero request's consumed fields and scene-info/bind-fire operation mapping within the candidate family. It does not authenticate a complete built stack or change Gate B.

Checkpoints:

1. Identify and hash `Xpsb.so`; inventory its ELF symbols, imports and package provenance.
2. Use a disposable Ghidra project to trace its DRI record and shared structures back to the `psb_dri.so` screen initializer.
3. Correlate BO, fence, lock, vblank and command paths with historical libdrm and PSB kernel sources, keeping binary observations separate from version matches.
4. Update the existing P7B map, evidence matrix and unknowns; review the remaining bootstrap and implementation gaps.
5. Rehash both retained binaries and validate the documentation and data. Gate B remains BLOCKED unless separate hardware-safety evidence changes it; this analysis does not seek such authorization.
