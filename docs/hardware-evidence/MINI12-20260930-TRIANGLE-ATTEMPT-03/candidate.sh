set -u
service=/etc/runit/runsvdir/default/slimski
pci=/sys/bus/pci/devices/0000:00:02.0
module=/root/sgx535-frozen-seq1/gma500_gfx.ko
expected_note=040000001400000003000000474e5500d4cb8d750efa6eb401f82e6eaec7eb82ebdf9c3c
stop() { printf 'STOP: %s\n' "$1" >&2; exit 21; }
hold() { printf 'HOLD: %s\n' "$1" >&2; exit 30; }
test "$(/usr/bin/sv status "$service" | cut -d: -f1)" = down || stop service-not-down
test ! -e /sys/module/gma500_gfx || stop module-already-present
test ! -e "$pci/driver" || stop pci-already-bound
test ! -e /dev/dri/card0 || stop drm-card-already-present
test "$(stat -c '%U:%G:%a' "$module")" = root:root:600 || stop candidate-mode-changed
test "$(sha256sum "$module" | awk '{print $1}')" = 934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf || stop candidate-hash-changed
printf 'CANDIDATE_LOAD_POSSIBLE pinned-module\n'
if /usr/sbin/insmod "$module"; then load_rc=0; else load_rc=$?; fi
printf 'LOAD_RC=%s\n' "$load_rc"
test "$load_rc" = 0 || hold candidate-load-failed
test "$(od -An -tx1 -v /sys/module/gma500_gfx/notes/.note.gnu.build-id 2>/dev/null | tr -d ' \n')" = "$expected_note" || hold candidate-build-id-mismatch
test "$(readlink -f "$pci/driver" 2>/dev/null)" = /sys/bus/pci/drivers/gma500 || hold candidate-pci-not-bound
test "$(cat "$pci/vendor")" = 0x8086 && test "$(cat "$pci/device")" = 0x8108 || hold pci-identity-changed
test "$(readlink -f /sys/class/drm/card0/device 2>/dev/null)" = "$(readlink -f "$pci")" || hold candidate-drm-not-bound
test "$(stat -c '%F:%t:%T' /dev/dri/card0 2>/dev/null)" = 'character special file:e2:0' || hold candidate-card-node-unexpected
test "$(find /dev/dri -maxdepth 1 -type c -printf '%f ' 2>/dev/null)" = 'card0 ' || hold extra-drm-node
printf 'CANDIDATE_PASS loaded-build-id=%s pci-bound card0-only\n' "$expected_note"
