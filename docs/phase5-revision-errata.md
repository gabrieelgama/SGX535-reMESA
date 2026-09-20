# Phase 5 revision and errata review

## Identities kept separate

| identity | target status |
|---|---|
| graphics PCI revision ID | `0x06`, recorded by TVZ-001 |
| Intel SCH/platform stepping | `UNKNOWN` |
| graphics-function stepping | `UNKNOWN` |
| physical SGX core revision | `UNKNOWN` |
| DDK `SGXCORE_REV` | historical build configuration only |
| applicable BRNs/errata | `UNKNOWN` |

No source recovered in Phase 5 maps graphics function `8086:8108` revision `0x06` to D1 or to SGX535 rev116, rev121, rev126, or any BRN. A stepping table for another SCH function remains out of scope.

## rev116 and rev121

- The rev116 evidence establishes the text and intended compatibility of a Linux device-tree proposal. It does not identify this machine's silicon.
- The rev121 evidence establishes the default `SGXCORE_REV` of a historical Poulsbo DDK build target. It does not identify this machine's silicon.

These sources describe different software contexts and are not measurements. They do not establish a hardware conflict that can be resolved by choosing one. The physical revision remains `UNKNOWN`.

## Errata

No public Intel or Imagination source found in this phase ties a register-read, power, clock, reset, aperture, or MMIO-failure erratum to the target's graphics PCI revision. `BRN22693` remains only a possible historical clock-gating concern; applicability to this machine is `UNKNOWN`.

Revision coverage and errata coverage therefore remain blockers for both candidate reads.
