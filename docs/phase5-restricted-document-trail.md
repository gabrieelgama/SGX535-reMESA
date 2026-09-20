# Restricted and missing document trail

## Intel document 364236

**CONFIRMED:** Intel document [`445348-016US`](https://www.intel.de/content/dam/www/public/us/en/documents/specification-updates/emgd-v1-14-winxp-spec-update.pdf), *Intel Embedded Media and Graphics Driver, EFI Video Driver, and Video BIOS v1.14 for Windows XP and Linux Specification Update*, lists `364236` as *Intel System Controller HUB External Design Specification* in its reference table (P5-001).

**CONFIRMED:** an [Intel Community moderator](https://community.intel.com/t5/Embedded-Intel-Atom-Processors/Intel-SCH-US15WPT/m-p/1248506) directs a requester to `https://cdrdv2.intel.com/v1/dl/getContent/364236` after logging into an Intel Resource and Documentation Center privileged account (P5-002).

An anonymous request on 2026-09-20 redirected to Intel's public 404 page (P5-005). This current retrieval result does not establish whether authenticated users can still obtain the document.

**UNKNOWN:** the document revision, classification, complete scope, and contents. The public references do not show that it contains SGX535 register access attributes, power/clock/reset requirements, stepping mappings, MMIO failure behavior, or recovery rules. The document's existence cannot be used as evidence for those facts.

## Imagination documentation trail

A [2009 public forum post](https://forums.imgtec.com/t/pds-programming/333) names `Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf`. Imagination staff declines to continue the technical discussion in public and redirects it to developer support (P5-003). The filename comes from a forum user, not from a published document index, so authenticity and access classification remain `UNKNOWN`.

Current public Imagination architecture material describes Series5 at a high level. It does not provide the SGX535/Poulsbo register contract needed by this project.

## Legal and provenance boundary

The repository records public URLs, titles, document numbers, dates, and the scope of public statements. It does not archive restricted documents or infer their contents. Any future use of partner material must be authorized and compatible with the intended open-source implementation. A restricted manual could guide factual research where permitted; its code or proprietary text must not be copied into Mesa or a new driver.

## Consequence for Gate B

The missing primary contract is still the limiting evidence. The strongest next artifact would be an authorized SGX535/Poulsbo register and errata reference that explicitly covers access attributes, side effects, power, clock, reset, failure behavior, and all applicable revisions. The relevant sections of `364236` would qualify only if they actually provide that contract.
