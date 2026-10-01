#!/bin/sh
# Read-only target inventory, scoped to the operator-approved transition
# review. Requires root to make the DRM-holder list complete.
# It never opens /dev/dri, /dev/fb0, PCI config, or an MMIO resource.
set -eu

test "$(id -u)" -eq 0 || {
    echo 'REFUSE: root read-only inventory required for complete FD ownership' >&2
    exit 2
}

printf '%s\n' 'PID1'
ps -p 1 -o pid=,comm=,args=
printf '%s\n' 'DISPLAY_PROCESSES'
ps -eo pid=,ppid=,user=,comm=,args= | awk '$4 == "slimski" || $4 == "Xorg" { print }'

printf '%s\n' 'DISPLAY_MANAGER_SELECTOR'
if test -r /etc/X11/default-display-manager; then
    cat /etc/X11/default-display-manager
else
    printf '%s\n' 'UNAVAILABLE'
fi

printf '%s\n' 'SERVICE_DEFINITIONS'
for path in /etc/init.d/slimski /etc/init.d/slim \
            /etc/sv/slimski /run/runit/service/slimski \
            /etc/runit/runsvdir/default/slimski; do
    if test -e "$path" || test -L "$path"; then
        ls -ld "$path"
        readlink -f "$path" || true
        if test -f "$path"; then
            sha256sum "$path"
            cat "$path"
        elif test -f "$path/run"; then
            sha256sum "$path/run"
            cat "$path/run"
        fi
    fi
done

printf '%s\n' 'DEPLOYMENT_TOOL_PATHS'
for tool in modprobe insmod service sv; do
    command -v "$tool" || printf '%s=UNAVAILABLE\n' "$tool"
done

printf '%s\n' 'GRAPHICS_DEVICE_FD_HOLDERS'
for process in /proc/[0-9]*; do
    test -d "$process/fd" || continue
    for fd in "$process"/fd/*; do
        target=$(readlink "$fd" 2>/dev/null) || continue
        case "$target" in
            /dev/dri/*|/dev/fb*)
                printf '%s %s %s %s\n' "${process#/proc/}" \
                    "$(cat "$process/comm" 2>/dev/null || printf '?')" \
                    "${fd##*/}" "$target"
                ;;
        esac
    done
done
