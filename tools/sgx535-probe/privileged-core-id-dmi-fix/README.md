# CORE_ID helper DMI correction — review only

This is a **new, uninstalled source revision**. It preserves the original reviewed [helper](../privileged-core-id/core_id_once.c), its installed binary, H0, P1, FIRST-MMIO-01 and the build/install evidence. It grants no execution or SGX MMIO authorization.

The operator reported `REFUSE: DMI identity changed`. A bounded pinned-SSH read of only four DMI fields found that `/sys/class/dmi/id/product_name` returns hex `496e737069726f6e20313231302020200a`, or `Inspiron 1210` followed by **three ASCII spaces** and newline. The original helper removes the newline only, so its expected `Inspiron 1210` does not match. The other three DMI values match. See [the refusal evidence](../../../docs/hardware-evidence/MINI12-20260928-COREID-DMI-REFUSAL-01/results.md).

The sole source change is the exact `product_name` expected literal, now `"Inspiron 1210   "`, plus an explanatory comment. [The assembly](load32_once.S) and [sudoers draft](sudoers.draft) are byte-identical to the installed revision. This remains strict matching: a different product name or different trailing bytes still refuses. Host syntax and a comparison against the captured raw bytes passed. **No target build, replacement installation or helper execution was performed for this revision.** A new review and explicit build/install authorization are needed before replacing the installed binary; a separate authorization would still be required for an actual MMIO read.

Current state: MMIO attempt count `0`; CORE_ID UNKNOWN; CORE_REVISION untouched; Gate B BLOCKED; whitelist `[]`.
