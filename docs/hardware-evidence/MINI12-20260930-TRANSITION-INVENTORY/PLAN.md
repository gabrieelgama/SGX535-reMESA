# Approved root read-only transition inventory: pre-contact record

Scope approved in the current operator instruction: identify only (1) the
live `slimski` supervisor and exact stop control, (2) all root-visible DRM/
framebuffer FD holders, and (3) the privileged deployment route and available
module/service tools. No display, driver, DRM, MMIO, or SGX operation is
authorized by this inventory.

One pinned-key SSH connection as `gama@192.168.18.90:22` will invoke
`sudo -n sh -s` with the exact
[inventory script](../../phase8/mini12-transition-inventory-proposed.sh)
on standard input, SHA-256
`1bfdb9f2d4fbef46fd0dcda703dd1dd78c704b23a88733aac06c54734910ff27`.
`sudo -n` cannot prompt for a password. A refusal or incomplete output stops
the inventory; no alternate root route or retry is in scope.

SSH arguments: `-T -o BatchMode=yes -o ConnectTimeout=8 -o
ConnectionAttempts=1 -o StrictHostKeyChecking=yes -o IdentitiesOnly=yes -o
PasswordAuthentication=no -o KbdInteractiveAuthentication=no -o
UserKnownHostsFile=/home/gama/sgx535-gfx/docs/hardware-evidence/MINI12-20260927-H0/ssh_known_hosts
-i /home/gama/.ssh/id_ed25519_sgx535_h0 -p 22`.

Expected output sections, in order: `PID1`, `DISPLAY_PROCESSES`,
`DISPLAY_MANAGER_SELECTOR`, `SERVICE_DEFINITIONS`,
`DEPLOYMENT_TOOL_PATHS`, `GRAPHICS_DEVICE_FD_HOLDERS`. The script checks
effective UID 0 before any inventory output. It reads `/proc` process and
FD symlinks, selected `/etc` and runit service files, and command paths.
It never opens `/dev/dri/card0`, `/dev/fb0`, PCI config, a BAR or MMIO.
It does not write a target file, stop a process, invoke module tools, or
alter display/SGX state.

Capture locally: exact SSH argv, local UTC start/end, exit status, untouched
stdout/stderr, byte counts, and SHA-256. A successful command may still leave
the transition review blocked if no service definition or privileged route
is established or if holder enumeration is ambiguous.

## Second approved authentication attempt

The first `sudo -n` attempt was refused before script execution. The operator
then supplied the sudo credential for this same root read-only inventory.
The second attempt will keep the same pinned-key SSH target and exact script,
but invoke `sudo -S -p '' sh -s`. The credential is sent separately through
stdin, never in a command argument, environment variable, project file,
capture, or final report. Only after sudo accepts it will the script bytes
be sent on that same stdin. The script remains read-only and refuses a
nonroot effective UID. Authentication failure or incomplete output stops
the attempt; there is no module/display/SGX action or alternate root route.

Expected sections and captured evidence remain exactly as above. The
second attempt is separately logged; the first refusal remains immutable.

The first interactive transport opened with a closed stdin pipe. No
credential bytes could be sent, and sudo reported `no password was provided`;
stdout was empty and the script did not run. This was a local transport
failure, not a wrong-password result. The corrected transport uses a local
PTY with echo disabled, still requests **no remote PTY**, and retains the
same `sudo -S -p '' sh -s` command and unchanged read-only script. Only one
corrected credential delivery is planned. The transport refusal bytes are
retained separately; no target state-changing action occurred.

## Supervisor-parent completion read

The successful root inventory captured `slimski` with parent PID 1377, but
its process filter did not include that parent's name. To close the same
approved supervisor-identification question, perform one additional pinned
SSH read of public `/proc` process metadata only:

`ps -p 1377,1409,1766 -o pid=,ppid=,comm=,args=`

The expected result names PID 1377 and shows whether the originally captured
`slimski` and Xorg PIDs still have the same parent chain. A missing/reused PID
or changed chain remains a snapshot change, not a reason to modify the target.
This operation does not require sudo or open a graphics device. It has no
write, service control, DRM, MMIO, ioctl, or GPU action. Capture raw output,
stderr, exit code, and hashes locally. No other target command is in scope.
