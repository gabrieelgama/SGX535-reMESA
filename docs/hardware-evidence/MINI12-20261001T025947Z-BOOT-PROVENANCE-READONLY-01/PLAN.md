# Authorized read-only stock boot/module provenance capture

Recorded before target contact, 2026-10-01 UTC.

## Authorization and exact transport

Operator authorizes this boot/module/fallback observation only. One pinned SSH
invocation to the previously verified `gama@192.168.18.90:22`, using the existing
key and known_hosts. Exact argv derives from the retained triangle passive
preflight. Remote command: `sudo -n -p '' sh -s`; stdin contains **only** the
reviewed capture script. No credentials are requested, embedded, sent or logged.
Unavailable noninteractive authorization stops before the script executes.
No retry, alternate endpoint, authentication workaround or target installation.

The exact commands are in `capture.sh`; the bounded local recorder is
`run_capture.py`. Both are preserved before execution and hashed by the record.
All operations are read-only. The target receives no output-file path and
creates no analysis/extraction tree. Binary files stream back through the
existing base64 capture mechanism. Extraction is exclusively on this host.

## Guards before boot inventory

Require root read privilege, exact Dell DMI bytes, release
5.10.240-antix.1-486-smp, i686 and retained taint12289. Require original module
Live, installed SHA7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb
and loaded Build IDd8dcb4d38b774ad64799d5e13aaedede069371f3. Require PCI8086:8108
bound to gma500, expected card0 node metadata/device link, fb0 gma500drmfb
1280x800/32bpp/8192 stride, vtcon0=0/vtcon1=1 and running slimski/Xorg.
Require the retained stock command line before reading the boot configuration.
Any mismatch stops the capture, preserving output. No DRM node is opened.

## Read paths and expected outputs

- Existing identity/display guards: uname/proc version/boot ID/taint/modules,
  selected text DMI/PCI/module-note/fb/VT attributes, PCI/DRM symlinks, DRM node
  stat, local modinfo and read-only runit status/process listings.
- /proc/cmdline, PCI modalias, /proc/interrupts, /proc/mounts, /sbin/init symlink:
  current stock selection, current owner/IRQ and init family.
- /boot shallow ordinary-file inventory; existing GRUB/syslinux/extlinux/loader
  configuration candidates, /etc/default/grub and /etc/grub.d ordinary files:
  actual framework, entries/default/fallback where attributable. Absence is
  recorded; no framework is assumed from BOOT_IMAGE alone.
- /boot/vmlinuz-<verified-release> metadata/hash, associated
  /boot/initrd.img-<release> metadata/hash/bytes and boot image symlinks.
  Associated initrd is not called selected until offline entry analysis agrees.
- Already-installed package metadata for exact kernel, bootloader, initramfs,
  kmod/eudev/runit/SSH; read-only dpkg ownership query.
- Module lists/modprobe config in /etc and /lib, module alias/dep/softdep/builtin
  tables, relevant eudev driver rules, initramfs configuration/hooks, existing
  selected root boot/runit/display/network/SSH startup scripts. Read files;
  never execute their contents.
- dmesg (without clear), existing ordinary boot/kernel logs, optional retained
  /run/initramfs diagnostics. No tracing/instrumentation enabled.
- Read-only ip address/route, ss listener and sshd process observations. No
  network or service configuration and no promise of future SSH behavior.

Ordinary files over8MiB retain metadata/hash but are not transferred, except
associated initrd up to256MiB. Missing/unreadable sources are explicit UNKNOWN.
No packages are installed. Final guards require the same boot/module/PCI/DRM/VT
and running display service as at capture start.

## STOP and boundary

Identity/state disagreement, unexpected permission/error, timeout, hash change
or ambiguous selected-entry relationship stops qualification. Preserve raw
stdout/stderr/exit status even on failure. Do not guess missing files or broaden
contact into interactive probing. No initramfs/boot/modprobe regeneration,
service control, sysfs write, module operation, PCI config, MMIO, DRM open,
ioctl, reset/reboot, SGX fire or candidate staging.

Expected successful marker: BOOT_PROVENANCE_READONLY_PASS. Expected installed
boot files are merely observations; other configured entries are not thereby
known-good. Gate B remains BLOCKED, SGX whitelist[]. After the single completed
capture all interpretation and extraction remain offline.
