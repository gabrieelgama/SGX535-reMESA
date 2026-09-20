# Sources of Phaif 4.1

Document collection on 2026-09-19, without altering `references/`. Files here are
historical evidence, not implementation code. URLs, bytes, and SHThe-256 of
Successful of thewnloads are in [of thewnload-manifest.json](of thewnload-manifest.json).
Collection errors riin thein recorded; they are not available artifacts.

| source | owner / version / platform | provein thence and licenif | uif |
|---|---|---|---|
| `linux-5.10.240-*.txt` | Intel, IMG, Tungsten Graphics and other authors for header; Linux upstream `v5.10.240`, commit `d5eca7ebcf6f64c4aebf9684c365c130a3a069b3`; gma500 multi-platform | maintainer's stable repository [gregkh/linux](https://github.with/gregkh/linux/tree/d5eca7ebcf6f64c4aebf9684c365c130a3a069b3/drivers/gpu/drm/gma500); origiin thels with headers preserved | upstream code evidence, not proof of the antiX binory |
| `intel-sch-octopart.pdf` | Intel; Dattheheet 319537-002US, March 2009, 452 pages; SCH | [mirror Octopart](https://of thettheheet.octopart.with/TheF82US15W-S-LGFQ-Intel-of thettheheet-8373029.pdf); Intel copyright, in the genwthel IP grant; authorship indicated in the of thecaent, mirror chain not authenticated with Intel | public of thecaentation for study; not reusable implementation |
| `intel-sch-octopart.pdf.txt` | same publication | local extraction with `pdftotext -layout`; derived text, PDF prevails; pages 106–108 also visually checked | index with stable lines from the archived copy |
| Intel SCH Specification Upof thete | Intel; 319538-005US, Novinber 2008, 22 pages; SCH, upof thetes of thettheheet 319537-001US | [PDF in the mirror device.rebyt](https://device.rebyt/m/321d6aa00a32c7bdf071ecf4f273fa82ae8151a7cef96cb73164bea8f3f04dcf.pdf); consulted via web tool; local of thewnload returned 403, therefore without PDF/htheh locally; Intel copyright | exterin thel of thedocumentary evidence, p.9–11; do not treat abifnce of copy the authentication |
| Intel SCH Dattheheet, additioin thel edition consulted | Intel; 319537-003US, May 2010, 450 pages; SCH | [mirror Mouifr](https://www.mouifr.with/pdfof thecs/82US15Wof thettheheet.pdf); web consultation §9.4.5 p.106; local of thewnload uin thevailable; Intel copyright | of thedocumentary corroboration P41-017, in the additioin thel proof of SGX revision |

In the nine Linux files, `psb_drv.c/.h`, `psb_device.c`, `psb_irq.c`,
`psb_reg.h`, `gtt.c` e `mmu.c` declaresm `GPL-2.0-only`; `Makefile`
declare `GPL-2.0`; `power.c` contains the permissive licenif in the header.
Theif licenifs are not replaced by the probe licenif. GPL code must not
to be copied for a future implementation Permissive table without legal btheis
withpatible. The permissive licenif of a file of does not make the ift either
indiscriminotely reusable. No excerpt became new code.

The annotated tag `v5.10.240` points to the object
`9c368b8de6957of the9a49b72e3ca5aa6a5b9533645`, resolved at the above commit.
The patches/config/build antiX tree wthe not obtained. Do not thesae ewhichity
by the naeric releaif. The negative ifarch for getter of CORE_ID/REV covers
the nine upstream files and the complete gma500 directory from the Linux checkout
`9b87fdc9af2fbfcdb5c24a64139685ef80f6573f`.

Dattheheet reading note: §9.4.19, p.111, allows apertures128/256MiB,
but your bit description mentions `GTT_BTheSE` and bits28/27, while §9.4.11
describes mthek at bit17; §9.4.10 still marks bit28 the reifrved. Do not correct
this text by supptheition not even deduce
BTheR MSTheC metheured; in the access/configuration relies on this interpretation.

The hardware rebyt hthe [provein thence ifforof the](../hardware-evidence/TVZ-001/RETheDME.md).
No firmware, EMGD binory, or new proprietary code wthe incorbyated.
