# Phase 4 adversarial audit

## Scope and method

The audit reread all the documents in `docs/`, the 123 lines of the Phase 3 evidence matrix and the corresponding original sources. The counted unit is a unique audited statement identified by `P3-*`, not each line of the matrix: a statement can have multiple sources. 60 `CONFIRMED` statements were audited (`P3-001` to `P3-064`, with four unused numbers). Each one was
verified against repository, commit, file, lines, and platform.

The result does not turn historical use into a hardware contract. Code that reads a register proves that the driver performed that read. It does not prove the absence of side effects, clock prerequisites, or safety in another revision.

## Quantitative result

| item | count |
|---|---:|
| CONFIRMED audited statements | 60 |
| lowered to INFERRED | 0 |
| lowered to UNKNOWN | 0 |
| open disagreements/contradictions with impact on bring-up | 5 |

The zero downgrade count means that the statements `P3-*`, already limited
to the platform and the source, remain supported. Three operational conclusions
were **narrowed**, without changing the confidence in the underlying evidence:

1. `CORE_ID`/`CORE_REVISION` remain historically observed offsets and reads; read safety is **UNKNOWN**.
2. `power/runtime_status=active` describes the PCI device runtime-PM state; it does not prove that SGX internal clocks are enabled.
3. “hardware state modified: NO” in Test Vector Zero describes only the actions
   of the probe. The earlier `gma500` bind may modify SGX, GTT, MMU, and IRQ.

Previous documentation was also corrected where it still said, in the present tense,
that `sgx535defs.h` was missing: `registers.md` and `mmu-bif.md` now distinguish the master checkout without the file from the historical commits that contain it.

## Claims downgraded during Phase 4

No statement `P3-*` was downgraded. The three operational constraints above correct possible interpretations, not the source-scoped audited facts.

## New decisive evidence

- **CONFIRMED — P4-001:** current Linux associates exactly `8086:8108` e
`8086:8109` to Poulsbo/SGX535 and `psb_chip_ops`; the name of `lspci` is not used
as proof (`linux`, commit `9b87fdc...`, `psb_drv.c:43-59`).
- **CONFIRMED — P4-006/P4-007:** during the bind, before registering the DRM, the
  `gma500` maps VDC/SGX, initializes PM, GTT, GEM, and MMU, resets SGX blocks,
  programs contexts/BIF/PDS, and installs the IRQ (`psb_drv.c:250-385,450-479`).
- **CONFIRMED — P4-012:** the driver calls `pm_runtime_get()`
unconditionally because runtime PM is declared broken; the Poulsbo callbacks `power_up`/`power_down` are stubs (`power.c:46-70`;
  `psb_device.c:186-194`).
- **CONFIRMED — P4-014:** Historical PSB reads `CORE_ID` and `CORE_REVISION`, but does not provide electrical semantics, clock requirements, or side-effect documentation
  (`PSB_psb_drv_c.txt:325-340`).

## Five preserved divergences

1. **SGX aperture:** current Linux maps `0x8000`; the historical Poulsbo DDK declares
`0x4000`. The origin is explicit in both, but the revision/configuration that
explains the difference remains **UNKNOWN** (`P3-003`, `P3-005`).
2. **BIF Contexts:** Linux/PSB use the expression based on `BASE1 + context*4`;
TI SGX535/EMGD use a different arrangement. There is no basis for choosing a generic formula (`P3-026`, `P3-061`).
3. **GATT/MMU addresses:** Linux inserts stolen into `gatt_start`, calculates space
starting from `mmu_gatt_start`, and on unload removes the sequence starting from
`mmu_gatt_start`. The comment `mmu_gatt ??` confirms uncertainty in the code itself (`psb_drv.c:134-160,189-193,352-359`).
4. **Power ownership:** the current gma500 maintains a runtime-PM reference and has
empty Poulsbo callbacks, while the historic Poulsbo DDK calls a external OSPM layer whose implementation is not in the artifact. There is no complete, reconciled SGX power/clock sequence (`P3-017`, `P3-045`).
5. **SGX535 header coverage:** the historical TI and EMGD headers have 597
common equal scalar values, but different exclusive sets. This proves kinship of the artifacts, not equivalence of integration nor
security of the registers (`P3-063`).

## Extrapolations refused

- SGX540/544 does not fill in missing SGX535 fields.
- OMAP/TI only describes the TI integration.
- EMGD remains a historical artifact of a community mirror.
- Historical PSB ABI is not a proposal for a modern ABI.
- The gma500 KMS display does not prove 3D acceleration.
- Register names and `#define` do not prove read-safety.

## Provenance and licenses

| material | owner/origin | license/provenance | permitted use at this stage |
|---|---|---|---|
| Linux/gma500 and kernel documentation | Linux contributors; commit `9b87fdc...` | SPDX per file, predominantly GPL-2.0 | primary evidence; code only according to license |
| TI KM/UM and historical Poulsbo DDK | TI/IMG and respective authors; commits fixed in the main branch | licenses by file/tree; do not assume uniformity | documentation/evidence; reuse depends on file audit |
| PSB KMP history | public snapshot `gregkh/psb-kmp` commit `98b5307...` | historical code GPL according to headers/tree | evidence and possible GPL reference; do not copy to permissive project without analysis |
| EMGD mirror | Intel-origin material in community mirror `e6884ec...` | package license and provenance incomplete for authentication | historical study only; do not copy for new implementation |
| EMGD binaries | community mirror | historical binaries, package terms | catalogue and hash; never reusable code |
| `sgx535-probe` | original code SGX535-GFX, Phase 4 | MIT, file `tools/sgx535-probe/LICENSE` | reusable under MIT |

No historical excerpt was copied in the probe. PCI identification constants come from the Linux table and are recorded in the matrix.

## Audit conclusion

**CONFIRMED (P4-001–P4-006):** a passive inventory by sysfs/procfs can
identify the PCI function and the binding without touching BAR or opening DRM. **UNKNOWN:**
the audited sources contain no SGX535/Poulsbo register with an explicit, simultaneous contract for side-effect-free reads, valid power/clock state, stepping coverage, and locking. Therefore, the audit approves the Test Vector Zero and locks MMIO.
