#!/bin/sh
# system one time initialization tasks

PATH=/usr/sbin:/sbin:/usr/bin:/bin
export PATH

if [ -f /etc/runit/native.boot.run -o -n "$bootrun" ] && [ -d /etc/runit/boot-run ]; then
	for f in /etc/runit/boot-run/*.sh; do
		[ -r "$f" ] && . "$f"
	done
else
	for script in /etc/rcS.d/S* ; do
		[ ! -x "$script" ] && continue
		path=$(realpath "$script")
		name=${path##*/}
		[ -e /etc/runit/override-sysv.d/"$name".block ] && continue
		[ -e /etc/runit/override-sysv.d/"$name".pkgblock ] && continue
		"$script" start
	done
	if ! /usr/bin/wtmpdb boot ; then
		echo "warning: failed to write a wtmpdb boot entry"
	fi
fi

# Now /run is mounted.
install -m000 /dev/null /run/runit.stopit
install -m000 /dev/null /run/runit.reboot

# That is it.  We do as little, as possible at stage1. Services, that do
# not have runscripts and invoked via init.d scripts are started at
# stage2. This way, daemon that hangs or otherwise misbehave do not
# block whole boot process.
#
# Thanks to Lorenz for suggestion.
