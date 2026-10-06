# Authoritative Phase 8 FIRE #3 handoff — 2026-10-06

**Triangle ESTABLISHED in FIRE3. CONSTANT_FRAGMENT_HYPOTHESIS_SUPPORTED.**

Boot: `f1ab6606-0561-445f-a397-28a028516cd7` (reused; no reboot after controller correction).
Driver Build ID: `85ec06b428c99fac7f9127919b7a488d204f4a77`.
Observer Build ID: `f11d3abb072caa4e1d32836ef92ce201e9c9d126`.
Image SHA-256: `3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e`.

Structural controller correction passed28/28 CPU tests. Existing first-owner82/82 retained with current continuity47/47. Protected preparation56/56, final204/204, immediate precheck42/42 PASS. PRE07 LIVE PASS before exactly ONE qualified client/ioctl invocation. Client exit0, ioctl0, operation errno0, phase9 RETIRED, accepted ledger0x7. Source/capsule CLOSED, reasons/invalidity0. Accepted TA completion, end-render, 3D-memory-free and retirement CONFIRMED. Full4268 response and4096 image agree with the immutable capsule, with current-operation provenance CONFIRMED from actual boot/producer/source interval bindings.

Exactly120 opaque-magenta pixels (`0xffff00ff`),904 zero pixels. Complete-image match to the known frozen triangle pixel-center mask with excluded hypotenuse ties. Coordinates `y=8..22`, `x=8..(30-y)` inclusive; bbox `(8,8)..(22,22)`. Sole experimental rendering variable: qualified fragment words `00000000 f8040140` → `001f00ff fca7f1f1`. No additional rendering change. This establishes FIRE3's diagnostic triangle and supports the fragment hypothesis; FIRE2 coverage and internal primitive grammar remain unobserved. Direct command acceptance/instruction-level export/PBE transaction traces are not claimed.

All18 originals sealed BEFORE interpretation. See [result](fire3-one-authorized-call/outcome-report.md), [machine-readable findings](fire3-one-authorized-call/outcome-report.json), [original seal](fire3-one-authorized-call/originals-seal.json), [all pixel coordinates](fire3-one-authorized-call/spatial-analysis.json), [lossless preview](fire3-one-authorized-call/readback-nearest512.derived.png), and [final integrity verification](fire3-one-authorized-call/final-seal-verification.json).

FIRE3 invocation count1; total historical authorized client invocations3. Authorization consumed YES. **Further SGX execution authorized NO; sgx_execution_authorized=false. No retry. STOP.** Current capsule is consumed/CLOSED; do not treat this boot as UNUSED for another operation. No further hardware action is needed to establish this triangle. Any future operation needs new explicit authorization and applicable fresh preparation.

Previous STOP/inspection/failed-transfer evidence, main81-file seal, late operator report and all68 FIRE2 evidence files remain unchanged. Candidate/client/UAPI bytes and repository/index state remain unchanged. Only new controller, tests and evidence/documentation artifacts were created outside the repository.
