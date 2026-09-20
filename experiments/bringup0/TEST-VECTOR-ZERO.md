# TEST VECTOR ZERO

## Objetivo formal

Collect a reproducible photograph of identity and binding using only
passive Linux interfaces. This is not a functional test of SGX.

## Pre-conditions

- kernel em estado normal e sysfs/procfs montados;
- exact copy of the probe of the recorded review;
- user without privileges whenever permissions allow;
- no other diagnostic command that opens BAR/DRM mixed with the capture.

## Allowed operations, in order

1. enumerar `/sys/bus/pci/devices`;
2. read `vendor`/`device` and select only `8086:8108` or `8086:8109`;
3. read revision, subsystem, class, IRQ and the text file `resource`;
4. resolver o symlink `driver`;
5. correlate nodes in `/sys/class/drm` through the symlink `device`;
6. ler runtime status/contadores se presentes;
7. read kernel version, get architecture with `uname` and public DMI fields
from system/placa/BIOS, without serial or UUID;
8. declare GTT/stolen unavailable when not exposed;
9. produce human or JSON report and exit.

Future execution:

```sh
python3 tools/sgx535-probe/sgx535_probe.py --json >sgx535-probe.json
```

## Prohibited operations

Do not open PCI `config`, `resourceN`, ROM, `/dev/mem`, DRM node or debugfs. Do not
write sysfs. Do not reset, initialize, change clocks/PM/MMU, load firmware,
enviar CCB/`EVENT_KICK`, nem executar PDS/USSE/shader.

## Output and criteria

Success requires exact ID, all mandatory fields readable, valid report
and exit `0`. Unknown ID, absence, ambiguity, or parsing error returns
exit `2`; no alternative attempt is made.

Estado final esperado:

```text
hardware state modified by probe: NO
MMIO reads: NO
MMIO writes: NO
command submission: NO
firmware loading: NO
```

O primeiro campo tem escopo deliberado. Se `gma500` estiver associado, o driver
has already performed active initialization before publishing the DRM node (P4-006). Therefore
the vector does not certify 'hardware untouched since boot'.

## Phase 4 Design Result

**GO for future execution.** The functional tests used sysfs/procfs
synthetics. An invocation of sanity in the development environment failed
closed, by permission, before listing any PCI function. No results
of Inspiron/Poulsbo is alleged and no device attribute was read.
