set -u
printf 'UTC '; date -u +%Y-%m-%dT%H:%M:%SZ
printf 'KERNEL '; uname -r
printf 'SERVICE '; /usr/bin/sv status /etc/runit/runsvdir/default/slimski
printf 'MODULE '; awk '$1 == "gma500_gfx" {print $1 ":" $3 ":" $5; found=1} END {if (!found) print "ABSENT"}' /proc/modules
if [ -e /sys/bus/pci/devices/0000:00:02.0/driver ]; then
    printf 'PCI_DRIVER '; readlink -f /sys/bus/pci/devices/0000:00:02.0/driver
else
    printf 'PCI_DRIVER ABSENT\n'
fi
if [ -e /dev/dri/card0 ]; then printf 'CARD0 PRESENT\n'; else printf 'CARD0 ABSENT\n'; fi
printf 'VTCON0 '; cat /sys/class/vtconsole/vtcon0/bind
printf 'VTCON1 '; cat /sys/class/vtconsole/vtcon1/bind
printf 'CANDIDATE_SHA '; sha256sum /root/sgx535-frozen-seq1/gma500_gfx.ko
printf 'CANDIDATE_VERMAGIC '; /usr/sbin/modinfo -F vermagic /root/sgx535-frozen-seq1/gma500_gfx.ko
printf 'DMESG_TAIL_BEGIN\n'
dmesg | tail -n 100
printf 'DMESG_TAIL_END\n'
