# Passive verification of the installed kernel — planned, not executed

Objective: connect the TVZ report to the package, binary, and configuration actually installed on the Dell. This plan does not repeat the PCI probe, open DRM, call PM, read PCI configuration, or access MMIO.

Record, without changing device state:

1. `uname -a`, `/proc/version`, and `/proc/cmdline`.
2. The `/sys` driver symlink and PCI resource metadata.
3. The installed package version and build metadata.
4. `modinfo gma500`, including the exact module path and vermagic.
5. Hashes and sizes of ordinary files such as the module, kernel image, `System.map`, and configuration where readable.
6. Permission failures as results, not reasons to substitute another operation.

Do not install or update packages. Do not use `modprobe`, `ioctl`, `kexec`, reboot, reload, unbind/rebind, `/dev/mem`, PCI configuration writes, or MMIO. No command in this document has been executed on the target as part of this documentation pass.
