# FIRE #3 preparation STOP — candidate boot 2026-10-06

New transfer COMPLETE_VERIFIED (50,816,631 bytes; qualified image SHA-256), staged and independently verified 78/78. Successful FIRSTLOAD boot `f1ab6606-0561-445f-a397-28a028516cd7` passed 82/82 passive guards, including exact loaded identities, ordered hook receipt, valid prepared source witness, clean UNUSED capsule and kernel health.

Protected preparation encountered an **offline controller assertion before dispatch**. The current source witness legitimately has the same hash as the historical deterministic prepared witness. `controller.late()` inserts the new CONTEXT, then globally counts/replaces historical-hash literals; it finds two (fresh CONTEXT plus existing passive predicate), but expects one. The 14 synthetic audit tests used a different hash and missed this real-input case. This is a controller bug, not evidence of candidate hardware failure.

No remote protected destination or client copy was attempted. No client launch, ioctl or SGX invocation occurred. PRE07 NOT READY. FIRE #3 authorization unconsumed. Experiment NOT RUN. Triangle NOT ESTABLISHED. STOP; no automatic continuation/retry.

See [first-owner result](first-owner/result.json), [timing validation](first-owner/validation.json), [STOP/root-cause record](preparation-stop.json), [complete transfer verification](transfer-complete-verified.json), [staging](stage/result.json), and [independent poststage result](independent-poststage/result.json).

The minimum correction is to bind the specific source-witness predicate structurally, preserving all guard semantics, and test the same-hash case in every late phase. No correction or new live attempt was performed under this STOP. Old partial file, previous STOP/inspection seals, 67-file pre-boot seal and all 68 FIRE #2 files remain preserved. Candidate artifacts and Git state remain unchanged.
