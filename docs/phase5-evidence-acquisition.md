# Phase 5 evidence acquisition

Phase 5 began from the English-audited Phase 4.7 handoff at commit `f6421c7`. Gate B was `BLOCKED`, neither candidate was a `SAFE-CANDIDATE`, and the whitelist was empty. This selected the evidence-acquisition branch: no active bring-up was authorized.

## Search scope

I rechecked the local SGX535/Poulsbo sources and searched public Intel and Imagination material for:

- `CORE_ID`, `CORE_REVISION`, `EUR_CR_CORE_ID`, and `EUR_CR_CORE_REVISION`;
- SGX535 and Series5 register references, TRMs, access attributes, and errata;
- Poulsbo, US15W/US15WP/US15WPT, EMGD, and document `364236`;
- PCI revision/stepping mappings and rev116/rev121/rev126 references;
- power, clock, reset, MMIO failure, and recovery requirements.

The search recovered bibliographic and access-control evidence, but no applicable public hardware contract for the first read. The public sources are the [Intel EMGD specification update](https://www.intel.de/content/dam/www/public/us/en/documents/specification-updates/emgd-v1-14-winxp-spec-update.pdf), [Intel's support response about 364236](https://community.intel.com/t5/Embedded-Intel-Atom-Processors/Intel-SCH-US15WPT/m-p/1248506), and the Imagination forum discussions about [PDS programming](https://forums.imgtec.com/t/pds-programming/333) and [SGX535/SGX545 differences](https://forums.imgtec.com/t/sgx545-blitter/2000).

## New evidence

1. Intel EMGD specification update `445348-016US` (April 2012), page 6, Table 1 identifies document `364236` as *Intel System Controller HUB External Design Specification* (P5-001).
2. An Intel moderator states that document `364236` is available after signing into a privileged Resource and Documentation Center account and supplies the same content URL (P5-002).
3. On an Imagination forum, a user names `Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf`; Imagination staff redirects the technical discussion to private developer support (P5-003). This proves only what the public posts say. It does not authenticate the file, establish its license, or establish any register semantics.
4. Imagination staff directs platform-specific SGX535/SGX545 questions involving Intel hardware to the platform provider and confirms that feature differences exist (P5-004). This supports the existing rule against extrapolating between cores; it does not define SGX535 read safety.
5. An anonymous request to Intel's content URL for `364236` on 2026-09-20 redirected to the public 404 page (P5-005). This does not establish authenticated availability.

## What the search did not establish

No public source found in this phase explicitly states that SGX535/Poulsbo `CORE_ID` or `CORE_REVISION` is read-only or side-effect-free. None defines the minimum power, clock, or reset state; covers every plausible Poulsbo SGX535 revision; maps graphics-function PCI revision `0x06` to an SGX core revision; selects applicable BRNs; or defines CPU behavior and recovery when the SGX aperture is unavailable.

Historical reads remain evidence of historical software behavior. They are not a hardware guarantee. An unsuccessful search is not negative proof, so all affected requirements remain `UNKNOWN` and Gate B remains `BLOCKED`.

## Result

**INFERRED:** the public trail suggests that the decisive contracts were distributed through Intel or Imagination partner channels. Their existence is documented; their relevant contents are unavailable. No restricted material was downloaded, copied, or added to the repository.
