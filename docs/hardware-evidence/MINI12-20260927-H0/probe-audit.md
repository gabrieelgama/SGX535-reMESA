# Passive probe safety audit

Scope: [sgx535_probe.py](../../../tools/sgx535-probe/sgx535_probe.py) and its [synthetic tests](../../../tools/sgx535-probe/test_probe.py). This is a source audit, not target execution. Four tests passed on the workspace.

| Operation | Classification | Interface | Side effect and failure boundary |
| --- | --- | --- | --- |
| Enumerate PCI device directory, require exact `8086:8108` or `8086:8109`, optional BDF | READ_ONLY | sysfs path names and `vendor`/`device` text | No match, multiple matches, malformed selection or unsupported identity exits 2; no SGX inference. |
| Read PCI identity, IRQ, text `resource`, driver symlink and DRM node identity | READ_ONLY | sysfs text/symlink | `resource` is the text extent list; `resourceN` is never opened or mapped. Missing optional fields become null. |
| Read PCI/runtime power status and counters | READ_ONLY | sysfs text | A reported `active` state does not establish internal SGX power or clocks. `power/control` is not read or written. |
| Read public DMI names, proc kernel release/version and `uname` | READ_ONLY | sysfs/procfs/uname | Serial numbers, UUIDs and other unique DMI fields are omitted. |
| PCI configuration, DRM device open/ioctl, debugfs, MMIO, reset, power write, GPU submission | NOT IMPLEMENTED | none | No corresponding operation or recovery path exists in this tool. |

`read_text` handles missing, denied and decoding failures; mandatory attributes fail closed. Resource-list parsing rejects malformed and inverted ranges. The CLI emits JSON or human text; `--sys-root`/`--proc-root` are test fixtures, not hardware controls. No unbounded poll exists. The probe does not change device state, but a bound `gma500` driver may already have initialized hardware before the probe. The tool cannot establish safe SGX register access, core revision, recovery or any R1/R2/R3 postcondition.

The current workspace is not the target. Following a separate passive identity check over authenticated SSH, the probe ran **once** on the Mini 12 with exit 0; see the [result](results.md) and [raw capture](raw/passive-probe-stdout.json). This verifies the tool's reported read-only observations, not safe SGX register access or any R1/R2/R3 postcondition.
