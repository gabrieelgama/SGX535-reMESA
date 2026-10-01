# First-load cycle01 preflight scope (recorded before contact)

Run `preflight.sh` once through the pinned prior SSH argv with final command
`sudo -n -p '' sh -s`. Script stdin is program bytes only; no credentials.
Root Python uses -I -B -S and performs only ordinary reads/lstat/statvfs,
symlink resolution and the argv-preserved passive uname/sv-status/pgrep/dmesg/
grub-editenv-list/id calls. Exact paths are in the preserved script.
Expected: pinned kernel/machine/i686/original note and installed stock hashes,
PCI/DRM/fb1280x800/VT/IRQ16, slimski/Xorg, taint12289 and healthy logs; destinations
absent, trusted boot parents, >=128MiB free, writable filesystem, Python no-follow
exclusive/fsync primitives. Dynamic PIDs/counters/boot ID are recorded, not pinned.
Any mismatch, unavailable privilege/tool, error/stderr, timeout or identity ambiguity
STOPS before target writes. Physical/display/power/risk confirmation is separately
required before staging. No DRM open, ioctl, MMIO, module/service/console/PCI write.
