set -u
printf 'UTC '; date -u +%Y-%m-%dT%H:%M:%SZ
printf 'KERNEL '; uname -r
printf 'SERVICE '; /usr/bin/sv status /etc/runit/runsvdir/default/slimski
printf 'PROCESSES\n'; ps -eo pid,comm,args | awk '$2 == "slimski" || $2 == "Xorg" || $2 == "modprobe" {print}'
printf 'MODULE '; awk '$1 == "gma500_gfx" {print; found=1} END {if (!found) print "ABSENT"}' /proc/modules
if [ -e /sys/bus/pci/devices/0000:00:02.0/driver ]; then printf 'PCI_DRIVER '; readlink -f /sys/bus/pci/devices/0000:00:02.0/driver; else printf 'PCI_DRIVER ABSENT\n'; fi
if [ -e /dev/dri/card0 ]; then printf 'CARD0 PRESENT\n'; else printf 'CARD0 ABSENT\n'; fi
for p in /sys/class/vtconsole/vtcon0/bind /sys/class/vtconsole/vtcon1/bind /sys/class/graphics/fb0/name; do
 printf '%s ' "$p"; if [ -r "$p" ]; then cat "$p"; else printf 'ABSENT\n'; fi
done
printf 'DMESG_BEGIN\n'
dmesg
printf 'DMESG_END\n'
