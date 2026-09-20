# Passive hardware identification

## Identidade aceita

The probe accepts only these pairs:

| vendor | device | classification | evidence |
|---|---|---|---|
| `0x8086` | `0x8108` | Poulsbo, SGX535 according to the Linux table | CONFIRMED `P4-001` |
| `0x8086` | `0x8109` | Poulsbo, SGX535 according to the Linux table | CONFIRMED `P4-001` |

The source table also declares GMA 500/Atom Z5xx. It does not map revision or
subsystem ID for US15W, US15WP, US15WPT or specific SGX stepping. This
subclassification is **UNKNOWN**. Revision and subsystem are recorded, never
used to promote it.

## Chosen passive surfaces

**CONFIRMED — P4-002:** the Linux PCI documentation marks `vendor`, `device`,
`revision`, `subsystem_vendor`, `subsystem_device`, `class`, `irq` e `resource`
as read-only ASCII attributes. `resource` contains start/end/flags, from the
quais o tamanho pode ser calculado inclusivamente
(`Documentation/PCI/sysfs-pci.rst:7-75`; `drivers/pci/pci-sysfs.c:40-76,163-195`).

They are collected:

- BDF, vendor/device, revision, subsystem, class e IRQ;
- entries from the text file `resource`, including indexes 0–5 as BARs;
- alvo do symlink `driver`;
- we `cardN`, `renderDN` or `controlDN` in `/sys/class/drm` whose symlink
`device` resolves to the same PCI function;
- kernel release/version via procfs and architecture via `uname(2)`;
- public DMI fields of system/placa/BIOS in `/sys/class/dmi/id`, when
available; serial, UUID, and asset tags are not collected;
- `power_state`, `power/runtime_status` and time counters, when present.

**CONFIRMED — P4-019:** Linux exports vendor/nome/system version and board and
vendor/version/BIOS date with `0444` mode; serial and UUID use `0400` mode
(`drivers/firmware/dmi-id.c:22-62,188-224`). The probe selects only the fields
audiences without unique identifiers.

**CONFIRMED — P4-003:** `config`, `enable` and `resourceN` do not belong to the array:
`config` is a binary configuration space RW, `enable` is RW and `resourceN` may
be the device programming mmap. The ROM usually requires write to be
habilitada (`sysfs-pci.rst:36-87`).

**CONFIRMED — P4-004:** `runtime_status` pode retornar `active`, `suspended`,
`suspending`, `resuming`, `error` or `unsupported`; their implementation only
formats the state maintained by the core PM. The probe neither reads nor writes
`power/control`, whose write to `on` can wake the device
(`sysfs-devices-power:35-52,264-306`; `drivers/base/power/sysfs.c:123-179`).

**CONFIRMED — P4-022:** o atributo PCI read-only `power_state` formata
`pci_dev.current_state` (`drivers/pci/pci-sysfs.c:154-161`). It also does not describe
os clocks internos da SGX.

## DRM and memory

**CONFIRMED — P4-005:** o gma500 atual anuncia `DRIVER_MODESET | DRIVER_GEM`,
has its own empty ioctl table and does not announce `DRIVER_RENDER`
(`psb_drv.c:91-95,493-518`). The DRM core creates a render node only with
`DRIVER_RENDER` (`drm_drv.c:771-785`). Thus, `cardN` is expected; absence of
`renderDN` is not a probe failure.

The PCI file `resource` exposes resources, but not the derived values
`gtt_phys_start`, `mmu_gatt_start`, `gatt_start` ou o tamanho stolen calculado
by the driver. The gma500 internally derives GTT/GATT and reads BSM to calculate
stolen (`gtt.c:185-253`; `gem.c:331-361`, CONFIRMED `P4-011`). No attribute
Stable gma500 for these derivatives was located; the probe prints
“not exposed by selected passive interface”. This absence of interface is a
inventory result, not a statement that another kernel will never expose it.

No specific gma500 debugfs file was found in the directory of
current driver. The probe does not use the generic DRM debugfs: it does not add a
field required for the Zero Test Vector, is not a stable ABI and would expand the
observation surface with no demonstrated benefit.

## Closed failure

- no exact target: exit `2`;
- BDF requested with a different ID: exit `2` before SGX completion;
- multiple targets: exit `2` until explicit selection;
- mandatory attribute unreadable/malformed: exit `2`;
- no attempt to 'guess' by PCI class, name, or CPU.

## Limit of the expression “passive”

**CONFIRMED — P4-006:** `drm_dev_register()` occurs after the entire sequence
of `psb_driver_load`; therefore observing a `cardN` gma500 also implies that the
driver has already had the opportunity to modify hardware (`psb_drv.c:450-479`). The
report guarantees only `state_modified_by_probe: false`.
