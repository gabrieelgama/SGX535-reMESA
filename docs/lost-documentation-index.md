# Lost documentation index

This index records references found during the search; it does not claim that the manuals were obtained. No proprietary PDF was added to the repository.

| item | evidence | status |
|---|---|---|
| IMG SGX535/Series5 TRM | IMG forum references state that TRMs are restricted to licensees | not obtained; read semantics and hardware contract remain UNKNOWN |
| Intel SCH External Design Specification | document **364236**, cited by EMGD and Intel support material | endpoint retrieval was unavailable; revision and content UNKNOWN |
| Intel SCH datasheet | `319537-002US` and later edition | useful for PCI/PM context; does not define the SGX ID read contract |
| IMG SGX Architecture Guide | public developer documentation reference | distinct from a hardware TRM; not used for register safety |
| Historical TI/IMG DDK manuals and manifests | local README, INSTALL, and manifest references | no SGX535/Poulsbo register contract recovered |

The most valuable next artifact is EDS 364236 with revision and access to its contents, followed by an authorized IMG SGX535/Poulsbo register, power, clock, reset, and errata contract.
