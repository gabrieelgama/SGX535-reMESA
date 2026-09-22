# Minimum technical question set

These questions are deliberately narrow. They request a scoped technical confirmation, not publication of an EDS or TRM. The recipient should state document/version, platform and core-revision scope, and whether each answer is a hardware guarantee or a software recommendation.

1. For Poulsbo-integrated SGX535, are `CORE_ID` at SGX-relative `0x0010` and `CORE_REVISION` at `0x0014` 32-bit read-only identification registers?
2. Can either read clear, acknowledge, latch, pop, or otherwise alter hardware state?
3. What SGX and graphics power state must hold before either register is accessed, and how is that state established or verified?
4. Which clock or clock-gate conditions are required for the register interface?
5. Must SGX reset be released? Are firmware, BIF, MMU, or microkernel initialization prerequisites?
6. Which SGX535 revisions and Poulsbo variants does the answer cover, and which BRNs/errata affect either read?
7. What does the CPU/interconnect observe if the aperture is read while the block is unavailable, unpowered, clock-gated, reset, or in transition?
8. What recovery is required or supported after such an access fails?
9. Does the graphics function `8086:8108`, PCI revision `0x06`, map to any Intel graphics stepping or SGX core revision? If so, what document defines that mapping?

For vendor support, questions 1–8 should be answered only for the recipient’s stated scope. Intel is the likely source for Poulsbo integration and PCI identity; Imagination is the likely source for generic SGX535 semantics and BRNs. Neither division of responsibility is assumed to be complete.
