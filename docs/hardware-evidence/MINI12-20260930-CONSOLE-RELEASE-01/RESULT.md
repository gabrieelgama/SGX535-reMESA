# Console-release test 01 — 2026-09-30

The operator authorized only the guarded VT-console release test and was
physically present. Pinned SSH and the reviewed two-process sudo transport
were used. The credential was entered through a non-echoing terminal prompt;
it is absent from scripts, command arguments and captured evidence.

| Stage | Observation |
| --- | --- |
| Fresh guard | Original `gma500_gfx` Build ID and installed hash, PCI binding, `gma500drmfb`, `card0`, `slimski`, Xorg and sole DRM holder matched the retained baseline. `vtcon0=0`, `vtcon1=1`, module refcount 2. |
| Graphical service stop | One `sv stop` succeeded. `slimski` and Xorg were absent, with no root-visible DRM/fb FD holders. Original module and PCI binding were unchanged. `vtcon1=1`, refcount 1. |
| One VT unbind | One normal write of `0` to `/sys/class/vtconsole/vtcon1/bind` returned success. Immediate readback: `vtcon1=0`, `vtcon0=1`, refcount **0**, original loaded Build ID and PCI binding unchanged. |
| One VT rebind | One normal write of `1` to the same control returned success. Immediate readback: `vtcon1=1`, `vtcon0=0`, refcount **1**, original loaded Build ID and PCI binding unchanged. |
| Service restoration | One `sv start` succeeded. The final read-only guard found `slimski` and Xorg running, the sole `/dev/dri/card0` holder, `vtcon0=0`, `vtcon1=1`, original module and PCI identity, and refcount **2**. |

All five stage scripts exited 0 and captured empty stderr. The script and
output hashes, UTC intervals and exact pinned SSH route are in the adjacent
`*.capture.json` records. The retained readbacks establish the normal
fbcon reference-release/reacquisition behavior on this live target for this
one tested transition. They do not establish that module removal, candidate
loading, or any SGX operation is safe or authorized. The physically present
operator subsequently reported that the display looked normal after
restoration; software checks established the measured baseline state.

No module was unloaded or loaded, no PCI unbind or DRM client open occurred,
and no SGX ioctl, MMIO write, TA fire or raster fire occurred. Attempts 01
and 02 were not modified. A new attempt at the triangle requires its own
exact-action review and authorization.
