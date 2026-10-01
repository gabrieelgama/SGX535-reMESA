#!/bin/sh
set -eu
stage=boot_guard
trap 'rc=$?; if [ "$rc" -ne 0 ]; then printf "READONLY_STOP stage=%s exit=%s\n" "$stage" "$rc" >&2; fi' EXIT
boot_id=$(cat /proc/sys/kernel/random/boot_id)
printf 'boot_id=%s\nkernel_release=%s\nuid=%s\n' "$boot_id" "$(uname -r)" "$(id -u)"
test "$boot_id" = 8ae37532-19d4-4ec7-9c29-a791acfd889f
test "$(uname -r)" = 5.10.240-antix.1-486-smp
stage=service_status
printf 'PID1\n'
ps -p 1 -o pid=,ppid=,comm=,args=
printf 'DISPLAY_PROCESSES\n'
ps -eo pid=,ppid=,comm=,args= | awk '$3 == "slimski" || $3 == "Xorg" || ($3 == "runsv" && $0 ~ /slimski/)'
service=/etc/runit/runsvdir/default/slimski
printf 'SERVICE_LINK\n'
ls -ld "$service"
readlink -f "$service"
printf 'SUPERVISE_METADATA\n'
for path in "$service/supervise" "$service/supervise/status"; do
    if [ -e "$path" ] || [ -L "$path" ]; then ls -ld "$path"; else printf 'ABSENT %s\n' "$path"; fi
done
printf 'SV_STATUS_BEGIN\n'
status_rc=0
/usr/bin/sv status "$service" || status_rc=$?
printf 'SV_STATUS_END exit=%s\n' "$status_rc"
test "$(cat /proc/sys/kernel/random/boot_id)" = "$boot_id"
printf 'SERVICE_DIAGNOSTIC_DONE\n'
exit "$status_rc"
