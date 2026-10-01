# Root read-only transition inventory result

**RESULT: COMPLETE for the three approved questions.** The successful
inventory exited 0 as root and returned every expected section. This is a
snapshot of service and file-descriptor ownership, not proof that a later
module transition will succeed.

The first pinned-key SSH attempt used `sudo -n sh -s` and was refused before
the script ran (`sudo: a password is required`). Its exact interval, status,
byte counts, and hashes remain in [capture.json](capture.json), with untouched
[stdout.txt](stdout.txt) and [stderr.txt](stderr.txt). A later local transport
attempt had closed stdin before credentials could be supplied, so its script
also did not run; its raw outputs are
[stdout-stdin-closed.txt](stdout-stdin-closed.txt) and
[stderr-stdin-closed.txt](stderr-stdin-closed.txt). Neither refusal changed
the target graphics state.

The operator then supplied the credential for the same approved read-only
scope. The corrected pinned-key SSH connection delivered it via stdin to
`sudo -S -p '' sh -s`; the value was not written to a file, command argument,
environment variable, or capture. The unchanged root-only
[inventory script](../../phase8/mini12-transition-inventory-proposed.sh) had
SHA-256 `1bfdb9f2d4fbef46fd0dcda703dd1dd78c704b23a88733aac06c54734910ff27`.
[capture-root.json](capture-root.json) records the transport, exit 0, byte
counts, script and output hashes, and local capture-file timestamp. The exact
UTC start time was not recorded and is not inferred. Untouched
[stdout-root.txt](stdout-root.txt) is 5,492 bytes, SHA-256
`227de5ee5fc6502413a49b8ed9d9e83726cde160f50192a6a16aa7e6638d5161`;
[stderr-root.txt](stderr-root.txt) is empty.

## Observed target facts

| Approved question | Root read-only observation |
| --- | --- |
| Display supervisor/control | PID 1 is `runit`. `/etc/runit/runsvdir/default/slimski` links to `/etc/sv/slimski`; its `run` script executes `/usr/bin/slimski`. A separate narrow read of public process metadata identified PID 1377 as `runsv slimski`, parent of `slimski` PID 1409 and grandparent of Xorg PID 1766. `/usr/bin/sv` exists. Runit's documented `sv stop <service>` requests down and waits up to seven seconds; down prevents supervision from restarting it. The target command was **identified, not executed**. |
| Root-visible DRM/framebuffer FD holders | The complete root `/proc` FD enumeration returned only `1766 Xorg 16 /dev/dri/card0`. It found no other `/dev/dri/*` or `/dev/fb*` file holder at capture. This does not enumerate kernel-internal fbcon references or guarantee the holder set later. |
| Privileged deployment route | Operator-supplied sudo authentication succeeded for the read-only script. `/usr/sbin/modprobe`, `/usr/sbin/insmod`, `/usr/sbin/service`, and `/usr/bin/sv` exist. Their presence does not prove a future unload/load succeeds. The root route is identified; no active command was executed. |

The supplemental parent read used only `ps -p 1377,1409,1766 -o
pid=,ppid=,comm=,args=` over the same pinned SSH route without sudo. It exited
0, returned the expected three-process chain, and had empty stderr. Its exact
UTC interval, command, hashes, and raw output are in
[capture-supervisor.json](capture-supervisor.json),
[stdout-supervisor.txt](stdout-supervisor.txt), and
[stderr-supervisor.txt](stderr-supervisor.txt). It changed no target state.

The target also has `/etc/init.d/slimski`, but stopping only that SysV script
would leave the observed runit supervision path active. The reviewed future
transition uses runit service control, with a fresh holder and service-state
check before any module removal.
The stop/start semantics are from the published
[`sv(8)` manual](https://manpages.debian.org/bullseye/runit/sv.8.en.html),
not from executing `sv` on this target.

No target file was transferred or written. No DRM node was opened, display
service stopped, module removed or loaded, ioctl issued, MMIO accessed, ISP
reset performed, or GPU work submitted. The candidate module and i386 client
remain local. This inventory does not authorize the active transition.
