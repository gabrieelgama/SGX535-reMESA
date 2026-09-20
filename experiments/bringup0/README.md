# bringup0 — passive harness design

The first bring-up experiment is a read-only inventory through normal Linux interfaces. Test Vector Zero established the target identity without MMIO; its provenance is recorded in `docs/hardware-evidence/TVZ-001/`.

A future active harness remains conditional on the formal gates. It must be owned by gma500, use a fixed whitelist, reject unknown PCI identity, log every operation, and fail closed. It must not expose arbitrary MMIO, create a second PCI owner, load firmware, submit commands, or execute PDS/USSE.

Phase 4.6 skipped Test Vector One because Gate B is BLOCKED and the whitelist is empty.
