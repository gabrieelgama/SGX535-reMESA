# Revision and stepping map

The table is deliberately sparse. It prevents a software configuration from being presented as a measurement of the Dell hardware.

| identity | evidence | established value | unsupported relationships |
|---|---|---|---|
| graphics PCI function | TVZ-001 | `8086:8108`, PCI revision `0x06` | no mapping to graphics/SCH stepping, SGX revision, or BRNs |
| Intel graphics stepping | no applicable source recovered | UNKNOWN | PCI `0x06` → any stepping |
| SCH/platform stepping | no applicable source recovered for this function | UNKNOWN | another PCI function's revision table → graphics function |
| physical SGX core revision | no board measurement or authoritative mapping | UNKNOWN | rev116, rev121, rev126, or any other core revision |
| DDK `SGXCORE_REV=121` | historical Poulsbo build configuration | 121 as software target | physical core identity |
| rev116 | historical Linux device-tree proposal naming Poulsbo SGX535-116 | software/proposal evidence | installed silicon identity |
| rev126 | historical build/configuration evidence | software target evidence | installed silicon identity |
| BRN applicability | DDK branches select BRNs by build target | configuration-specific source behavior | Dell physical applicability |

No source recovered in Phase 6 maps graphics PCI revision `0x06` to a physical SGX revision. The physical SGX revision and applicable BRN set remain `UNKNOWN`.
