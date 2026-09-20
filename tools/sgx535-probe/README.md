# sgx535-probe

`sgx535-probe` implements **Test Vector Zero**: a passive inventory of an exact
Intel `8086:8108` or `8086:8109` PCI function. It reads a fixed set of text
attributes from sysfs/procfs and calls `uname(2)`. Public DMI identity fields
are included when present; serial numbers and UUIDs are deliberately omitted.
It never opens a DRM node,
PCI `config`, a `resourceN` mapping, `/dev/mem`, or debugfs.

Run it on the target machine as an unprivileged user when sysfs permissions
permit:

```sh
python3 sgx535_probe.py
python3 sgx535_probe.py --json >sgx535-probe.json
```

If more than one exact target is present, select it explicitly:

```sh
python3 sgx535_probe.py --pci-address 0000:00:02.0 --json
```

The process exits `0` only for an exact supported PCI identity. It exits `2`
when the target is absent, ambiguous, malformed, or explicitly selected with
an unsupported vendor/device ID. Revision and subsystem IDs are reported, not
used as proof of a particular SGX stepping.

The `state_modified_by_probe: false` result applies only to this process. A
bound `gma500` driver performs hardware initialization before publishing its
DRM device, so the report does not claim that the device has remained untouched
since boot.

Tests use only a synthetic directory tree:

```sh
python3 -m unittest -v test_probe.py
```

The probe is original project code under the MIT license. No historical DDK or
EMGD implementation was copied into it..
