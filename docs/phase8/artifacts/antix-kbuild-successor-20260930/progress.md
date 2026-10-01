# Successor kernel preparation ledger — 2026-09-30

Scope: fresh output olddefconfig/modules_prepare and complete equivalence only. No candidate, implementation change or target operation.

- Inputs/source/tool identities verified against authoritative evidence.
- Initial repository status and file hashes recorded.
- Failed output snapshotted with file hashes, links, modes, inode and modification times; previous evidence manifest verified.
- Fresh successor output contains only exact captured configuration before Kbuild.
- Source reused read-only as authorized; failed output is never a Kbuild input.

- olddefconfig exit 0 (29.599s): only documented compiler banner changes.
- Fresh modules_prepare exit 0 (68.515s), empty stderr; expected syscall generator invoked once.
- Complete independent comparison PASS: 33/35 captured headers exact; compiler-banner and phase-specific identity metadata explicitly classified; no unexplained drift.
- 193 tests PASS (20.653s), three UBSan harnesses PASS, generator PASS, 222/222 reference CRC PASS, both dry hashes unchanged, expected --complete refusal.
- All 10569 failed-output entries unchanged by before/after snapshot.
- Candidate build/table installation intentionally deferred to separately scoped next task; no target contact.
