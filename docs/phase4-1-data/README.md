# Phase 4.1 sources

These files were collected on 2026-09-19 without changing `references/`. They are historical evidence, not implementation code. [download-manifest.json](download-manifest.json) records the URL, size, and SHA-256 digest of each successful download, plus failed retrievals.

| Source | Owner, version, and platform | Provenance and license | Use |
|---|---|---|---|
| `linux-5.10.240-*.txt` | Linux contributors; upstream `v5.10.240`, commit `d5eca7ebcf6f64c4aebf9684c365c130a3a069b3`; gma500 supports several platforms | [gregkh/linux](https://github.com/gregkh/linux/tree/d5eca7ebcf6f64c4aebf9684c365c130a3a069b3/drivers/gpu/drm/gma500); original headers retained | Upstream code evidence; not proof of the installed antiX binary |
| `intel-sch-octopart.pdf` | Intel SCH Datasheet 319537-002US, March 2009, 452 pages | [Octopart mirror](https://datasheet.octopart.com/AF82US15W-S-LGFQ-Intel-datasheet-8373029.pdf); Intel copyright; no general IP license; the document identifies Intel as author, but the mirror chain was not authenticated by Intel | Public documentation for study; not reusable implementation |
| `intel-sch-octopart.pdf.txt` | Derived from the preceding publication | Local `pdftotext -layout` extraction; the PDF controls; pages 106–108 were also checked visually | Searchable index with stable local line numbers |
| Intel SCH Specification Update | Intel 319538-005US, November 2008, 22 pages; updates datasheet 319537-001US | [device.report mirror](https://device.report/m/321d6aa00a32c7bdf071ecf4f273fa82ae8151a7cef96cb73164bea8f3f04dcf.pdf); consulted through the web tool; direct local retrieval returned HTTP 403 | External documentary evidence, pp. 9–11; absence of a local copy does not authenticate the mirror |
| Additional Intel SCH Datasheet edition | Intel 319537-003US, May 2010, 450 pages | [Mouser mirror](https://www.mouser.com/pdfDocs/82US15Wdatasheet.pdf) consulted through the web tool, §9.4.5 p.106; local download unavailable; Intel copyright | Corroborates P41-017; does not identify the SGX core revision |

Among the nine Linux snapshots, `psb_drv.c`, `psb_drv.h`, `psb_device.c`, `psb_irq.c`, `psb_reg.h`, `gtt.c`, and `mmu.c` declare `GPL-2.0-only`; the `Makefile` declares `GPL-2.0`; `power.c` carries a permissive notice. These licenses are independent of the probe license. GPL code must not be copied into a permissively licensed implementation without a compatible licensing basis. A permissive notice on one file does not make the whole set reusable. No excerpt was turned into new driver code.

Annotated tag `v5.10.240` is object `9c368b8de6957da9a49b72e3ca5aa6a5b9533645`, resolving to the commit above. At this stage, the antiX patches, configuration, and build tree had not been retrieved. The generic release name alone did not establish equivalence. The negative search for a CORE_ID/CORE_REVISION getter covered these nine upstream files and the complete gma500 directory at Linux commit `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f`.

Datasheet note: §9.4.19 p.111 permits 128 or 256 MiB apertures, but its bit description mentions `GTT_BASE` and bits 28/27, while §9.4.11 describes a mask at bit 17 and §9.4.10 still marks bit 28 reserved. I preserved this inconsistency. It does not justify correcting the source by assumption or deriving the measured BAR mask or access configuration from it.

The measured hardware report has separate [TVZ-001 provenance](../hardware-evidence/TVZ-001/README.md). No firmware, EMGD binary, or new proprietary code was incorporated.
