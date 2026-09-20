# Passive hardware identification

## Accepted identity

The probe accepts only these pairs:

| vendor | device | classification | evidence |
|---|---|---|---|
| `0x8086` | `0x8108` | Poulsbo, SGX535 according to the Linux table | CONFIRMED `P4-001` |
| `0x8086` | `0x8109` | Poulsbo, SGX535 according to the Linux table | CONFIRMED `P4-001` |

The source table also names GMA 500 and Atom Z5xx. It does not map PCI revision or subsystem IDs to US15W, US15WP, US15WPT, or an SGX stepping. That finer classification remains **UNKNOWN**. The probe records revision and subsystem values but never uses them to strengthen the identification.

## Passive interfaces

**CONFIRMED — P4-002:** Linux PCI documentation defines `vendor`, `device`, `revision`, `subsystem_vendor`, `subsystem_device`, `class`, `irq`, and `resource` as read-only ASCII attributes. `resource` supplies start, end, and flags; the inclusive range yields the size (`Documentation/PCI/sysfs-pci.rst:7-75`; `drivers/pci/pci-sysfs.c:40-76,163-195`).

The probe collects:

- BDF, vendor/device, revision, subsystem, class, and IRQ;
- entries 0–5 from the text `resource` file, reported as BARs;
- the target of the `driver` symlink;
- `cardN`, `renderDN`, or `controlDN` entries whose `/sys/class/drm` `device` symlink resolves to the same PCI function;
- kernel release and version from procfs, plus architecture from `uname(2)`;
- public system, board, and BIOS fields from `/sys/class/dmi/id`, when available; serial numbers, UUIDs, and asset tags are excluded;
- `power_state`, `power/runtime_status`, and runtime counters, when present.

**CONFIRMED — P4-019:** Linux exports the selected DMI vendor/name/version and board and BIOS fields with mode `0444`; serial and UUID fields use mode `0400` (`drivers/firmware/dmi-id.c:22-62,188-224`). The probe includes only public fields without unique identifiers.

**CONFIRMED — P4-003:** `config`, `enable`, and `resourceN` are outside this set. `config` is binary PCI configuration space with write support, `enable` is read/write, and `resourceN` may map device registers. PCI ROM access normally requires a write to enable it (`sysfs-pci.rst:36-87`).

**CONFIRMED — P4-004:** `runtime_status` can report `active`, `suspended`, `suspending`, `resuming`, `error`, or `unsupported`. It only formats PM-core state. The probe neither reads nor writes `power/control`; writing `on` there can wake a device (`sysfs-devices-power:35-52,264-306`; `drivers/base/power/sysfs.c:123-179`).

**CONFIRMED — P4-022:** the read-only PCI `power_state` attribute formats `pci_dev.current_state` (`drivers/pci/pci-sysfs.c:154-161`). It does not describe SGX internal clocks.

## DRM and memory

**CONFIRMED — P4-005:** this gma500 driver advertises `DRIVER_MODESET | DRIVER_GEM`, has an empty private ioctl table, and does not advertise `DRIVER_RENDER` (`psb_drv.c:91-95,493-518`). DRM creates a render node only for `DRIVER_RENDER` (`drm_drv.c:771-785`). A `cardN` node is expected; absence of `renderDN` is not a probe failure.

The PCI `resource` file exposes resource ranges, but not the derived `gtt_phys_start`, `mmu_gatt_start`, `gatt_start`, or stolen-memory size. gma500 derives GTT/GATT internally and reads BSM to calculate stolen memory (`gtt.c:185-253`; `gem.c:331-361`, CONFIRMED `P4-011`). No stable gma500 attribute for those derived values was found, so the probe reports "not exposed by selected passive interface." This describes the selected interface, not every possible kernel.

No gma500-specific debugfs file was found in the current driver directory. The probe does not use generic DRM debugfs because it adds nothing required by Test Vector Zero, is not a stable ABI, and would expand the observation surface.

## Fail-closed behavior

- No exact target: exit `2`.
- Requested BDF has another ID: exit `2` before SGX-specific reporting.
- Multiple targets: exit `2` until one is selected explicitly.
- Mandatory attribute is unreadable or malformed: exit `2`.
- Never guess from PCI class, a display name, or CPU model.

## Meaning of "passive"

**CONFIRMED — P4-006:** `drm_dev_register()` runs after the complete `psb_driver_load` sequence. Seeing a gma500 `cardN` therefore means the driver has already had an opportunity to modify hardware (`psb_drv.c:450-479`). The report guarantees only `state_modified_by_probe: false`.
