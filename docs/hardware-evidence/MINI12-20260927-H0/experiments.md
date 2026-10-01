# Conditional R1/R2/R3 experiment designs

These are discriminator designs only. **None is executable while Gate B is BLOCKED, whitelist `[]`, target access and recovery are unproved, and the required interface is unidentified.** They do not authorize Test Vector One, an MMIO read, a service ioctl, or a command submission. Each design needs a separate gate review with exact operations, bounds and recovery before execution.

## R1 — payload and translation publication

- **Models:** A: successful selected maintenance plus host publication exposes final payload and translation to its first selected consumer. B: maintenance can complete while one consumer sees stale bytes or translation.
- **Discriminator:** the same minimal, bounded consumer observes a known final payload/address after a fresh completed publication, with a controlled pre-publication version distinguishable in its output. A single matching result does not prove universal coverage of all eight publication domains.
- **Prerequisites:** exact target identity and stack; an approved minimal consumer and output readback; proven BO isolation, host writeback/domain transition, binding, maintenance-bit semantics, submission ordering, finite timeout and recovery; Gate B pass for every operation. No arbitrary page-table corruption.
- **Operations/observations:** prepare isolated known backing, make one controlled update, complete documented publication, submit only the approved consumer, compare its result and status/log deltas. All operations remain **UNSPECIFIED/NOT PERMITTED** until the prerequisites define them.
- **Timeout/abort/recovery:** abort before submission on missing or stale completion; stop after one unexplained mismatch or timeout; capture logs/state through already approved passive interfaces; invoke only a separately validated recovery procedure. Recovery is currently absent.
- **Expected mutation:** BO contents, translations, maintenance state and one bounded consumer execution; therefore not a first-session action.

## R2 — LOAD3, INITEND and TA readiness

- **Models:** A: fresh required `0xf` load completion, INITEND and table/service observations make selected DHOST/TA state consumable. B: the same bits can appear before the first selected TA consumer is ready.
- **Discriminator:** a separately defined minimal TA consumer completes correctly only after the target-qualified full readiness predicate; status `0x8` alone is insufficient evidence. Observing a bit without a consumer leaves R2 open.
- **Prerequisites:** applicable revision/feature qualification, register access safety and ownership, documented four-load start and status semantics, fresh-baseline method, bounded poll, first-consumer contract, recovery and Gate B pass. Historical `0x7` polling is not the clean-room predicate.
- **Operations/observations:** start only an approved initialization path; record a pre-request baseline and bounded completion trace; require fresh `0xf`, INITEND and service/table success; then run only an approved minimal consumer. Exact register operations are **NOT SPECIFIED OR PERMITTED** yet.
- **Timeout/abort/recovery:** no completion, stale status, unsupported revision or inconsistent table state aborts; one unexplained failure stops the series. Recovery is currently unproved.
- **Expected mutation:** service/TA initialization and possibly GPU state; not allowed in this session.

## R3 — selected PDS pre-definition source eligibility

- **Models:** A: all source state eligible before definition for each selected program is deterministic. B: a selected instruction can observe on-chip DS/temp/implicit state outside that domain.
- **Discriminator:** an output that can isolate one pre-definition state class while holding every CPU-visible input fixed. A non-crash or repeatable output alone cannot exclude hidden nondeterministic sources.
- **Prerequisites:** a proven output interpretation for an existing selected instruction stream, controlled launch-state variation without undefined state or random instructions, exact first-use containment, R1/R2 or an independently approved launch path, finite timeout, recovery and Gate B pass. No opcode sweep or speculative encoding.
- **Operations/observations:** use only an already recovered selected stream in isolated contexts; vary one *known* launch/input condition; compare a specifically predicted output. No such safe discriminating condition has yet been established, so **NO R3 hardware test is currently runnable**.
- **Timeout/abort/recovery:** any unexpected output, fault or timeout stops; preserve passive logs and do not retry until explained. Recovery is not established.
- **Expected mutation:** PDS/USE task execution and output buffer writes; prohibited while the current gate blocks submission.

## Least invasive next action

The read-only Mini 12 path, one passive probe run and scoped baseline are now [captured](results.md). The next action is to close the remaining passive evidence gaps and establish a recovery/exclusion protocol through the existing Gate B process. No R1/R2/R3 experiment above may be converted into an executable protocol until all applicable gate requirements pass; the current whitelist is empty.
