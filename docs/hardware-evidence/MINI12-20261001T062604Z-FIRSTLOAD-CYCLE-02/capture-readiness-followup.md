# Offline follow-up — evidence capture readiness

The experimental userspace answer confirmed manual selection and userspace,not
local sudo validation. The sole root capture was nevertheless started and sudo
refused before script execution. This did not alter hardware,but it exhausted
the prescribed bounded capture and lost required evidence for this cycle.

For a separately authorized future cycle,the controller must obtain an explicit
`local sudo-v succeeded` witness before launching the root capture,not infer it
from desktop success or a reply about selection. This is not a guarantee that
a different process's noninteractive authorization works:the actual sudo-n check
still has to succeed,and failure remains STOP/no retry. No password transport.

The capture deadline must refer to completed capture,not merely userspace arrival.
Require the operator's explicit handoff-clock/capture-completion measurement.
Never fill missing timing with host UTC/uptime or pretend it was within120.

A future bounded capture should emit minimal unprivileged boot identity before
the privileged script in the SAME connection. This would preserve the boot ID
on privileged refusal;it cannot retrospectively repair cycle02's missing ID.
This mechanism must be tested/reviewed before future use;it was NOT deployed in
this cycle and is not authority for another experimental boot.

Current recovery may verify genuinely normalstock,but complete three-boot ledger
qualification remains missing without the experimental boot ID. No new boot or
hot operation may be used to manufacture that lost historical fact.

The proposed readiness/identity/timing transport has now been tested OFFLINE in
offline-capture-followup/ (11 actual-wrapper tests); it was not used for any
cycle02 connection. Four existing-file receipt tests also pass. New live use
remains contingent on separate authorization and fresh checks.
