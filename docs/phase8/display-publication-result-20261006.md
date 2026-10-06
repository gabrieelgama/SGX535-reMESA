# Single authorized display publication — 2026-10-06

**DISPLAY_VISIBILITY_NOT_ESTABLISHED. Authorization consumed; no retry.**

Exactly one display-only attempt used the unchanged
[execution card](display-publication-execution-card-20261006.json) on boot
`f1ab6606-0561-445f-a397-28a028516cd7`. Current boot/module/Xorg/CRTC34/FB77,
mode, format and pitch matched the card before publication. The saved FIRE #3
source was copied through the existing software Xorg owner at `(64,64)` and held
for the configured 15 seconds. Total SSH/controller duration was 18.059893794939853
seconds; helper exit was 0. There was no SGX submission or rendering-state change.

## Independently established observations

- Published XGetImage RGB24 matches the sealed source: 120 pixels `0x00ff00ff`
  and 904 black pixels. Inclusive magenta bounds `(72,72)..(86,86)`;
  `y=72..86`, `x=72..158-y`. All coordinates are retained in the interpretation.
- Original 4096-byte display-region backup was preserved before publication.
  Restored capture matches both RGB24 and all 4096 saved bytes.
- Maintainer's physical observation: **“Triangle not seen; display normal.”**
- These facts establish the software drawable copy and restoration, but do not
  establish a visibly displayed triangle on the physical panel. The evidence does
  not distinguish a missed observation from a physical publication-path gap.
  Neither explanation is asserted as fact.

## Evidence and final boundary

[Attempt evidence](artifacts/display-one-publication-20261006/) contains the exact
card, authorization, deployed tools, precheck, launch/transport receipts, original
source, before/published/restored captures, operator observation and interpretation.
The original remote 9-file seal and the local 36-file attempt seal verify.
Original evidence was preserved before interpretation. All 95 sealed successful
FIRE #3 files and 135 pre-existing implementation files remain unchanged.

| Milestone | State |
| --- | --- |
| TRIANGLE_ESTABLISHED / KNOWN_GOOD_TRIANGLE_READBACK | YES — unchanged FIRE #3 |
| DISPLAY_PUBLICATION_ESTABLISHED | NO — physical visibility not established |
| GPU_RENDER_CPU_PUBLICATION_ESTABLISHED | NO — visible publication not established |
| DIRECT_SGX_SCANOUT_ESTABLISHED | NO — not attempted |

Display publication attempts: **1**. Task SGX invocations: **0**.
Display authorization consumed: **YES**. Further display publication authorized:
**NO**. Further SGX execution authorized: **NO**.

STOP. The smallest next justified work is separately requested passive analysis
of the drawable-to-panel visibility gap using these preserved captures and owner
bindings. No repeated display attempt follows from this result.
