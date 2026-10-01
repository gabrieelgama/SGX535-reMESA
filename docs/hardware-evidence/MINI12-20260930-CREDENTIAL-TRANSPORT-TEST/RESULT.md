# Mini 12 credential-transport test

Scope: authentication and a root **read-only** script. No artifact transfer,
root staging, display-service action, module operation, DRM open, ioctl,
MMIO access, or SGX work.

The pinned SSH route first ran `sudo -n -v`: exit 0, empty output, confirming
a cached sudo timestamp. The corrected two-process transport then ran a
separate `sudo -S -p '' -v` authentication process followed by
`sudo -n -p '' sh -s` with only a read-only script. The script returned exit
0, empty stderr, `uid=0`, and the proposed root staging directory and two
temporary artifact paths absent.

An auth-only `sudo -k` invalidated the timestamp. A following `sudo -n -v`
returned exit 1, confirming the uncached case. The same corrected transport
then returned exit 0, empty stderr and the same read-only output. The
credential was entered through an interactive prompt into only the
authentication process's stdin. It was not written to a command, argument,
environment variable, capture or repository file.

The `.capture.json`, `.stdout`, and `.stderr` files preserve each process's
nonsecret command identity, timing, exit status and output hashes. The
successful root script is limited to `id -u` and absence checks; these
results do not stage anything or qualify an active module transition.
