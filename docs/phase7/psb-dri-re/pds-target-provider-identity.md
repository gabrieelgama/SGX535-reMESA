# Target-to-provider identity: restart discriminator check

**Superseding project decision: Route C selected (2026-09-27).** The historical
question below remains unresolved, but is no longer a prerequisite for the
restricted clean-room implementation. Continue at
[the clean-room contract](cleanroom-backing-contract.md), not another provider
selection request. Its CPU image is deterministic; complete launch-state coverage
and concrete GPU-provider conformance remain unproved. This report preserves the
earlier historical discriminator result.

2026-09-27. Continuation of **P7H-035**, not a new provider survey or an ISA
result. The [backing-provider audit](pds-backing-provider.md) and
[first-use proof](pds-stack-pairing.md) remain authoritative within their scopes.

**Result: CONDITIONAL.** No retained deployment artifact identifies the
implementation intended to service the frozen construction's legacy
`drmBOCreate(mask=0x20000001, buffer_start=0)` request. No provider is selected
by this report. FG-02 remains OPEN; Gate B BLOCKED; whitelist `[]`; hardware
UNVERIFIED.

## What the available artifacts discriminate

The [discriminator table](pds-target-provider-discriminators.csv) separates
physical-machine evidence, source acquisition, and implementation selection.
The relevant retained sets were inspected by filename inventory and their
provenance records:

- `docs/hardware-evidence/` contains only the TVZ-001 README, operator report,
  and manifest. It contains no original probe output, installed-module artifact,
  root filesystem, package-install database, or boot-selection record.
- `references/home:lkundrak:poulsbo/xpsb-glx/` contains the acquired archive,
  spec, file inventory and extracted userspace files. These are acquisition
  artifacts, not a target filesystem snapshot.
- The project filename inventory, excluding the large reference source trees
  and archived kernel-source excerpts, found no `rootfs`, `initramfs`, `dkms`,
  `vermagic`, `deployment`, `installed-packages`, `dpkg/status`, `rpmdb`, or
  kernel-module filename candidate. This is a bounded inventory, **not proof
  that an artifact cannot exist elsewhere or under another name**.
- The [kernel audit](../../kernel-5.10.240-antix-audit.md),
  [Phase 7 provenance](../source-provenance.md), and
  [version-pairing report](../xpsb-re/version-pairing.md) explicitly preserve
  the missing installed-build correspondence. They do not supply an overlooked
  historical deployment binding.

Thus the immediate missing input is more specific than a kernel revision:
**which concrete provider is intended to supply this request on the target?**
The recorded modern installation and the historical research stack are not
one authenticated installation. Neither their source compatibility nor the
physical GPU's identity selects a historical runtime.

No new external package search was performed. Another publicly available PSB
source package cannot establish an absent private deployment or future
implementation choice. No live host or target device information was used as
a substitute for attributable target records.

## Target report integrity: original versus annotated file

The [TVZ manifest](../../hardware-evidence/TVZ-001/manifest.json) records the
original operator-report excerpt as 733 bytes, SHA-256
`3aceb9ed6daa68afa49ceaa5e8afc0b8a2c32b18f787fd007a71991b2954386b`.
The current [report](../../hardware-evidence/TVZ-001/operator-report.txt) is
764 bytes, SHA-256
`45c167c1980f05dc40817f972bc9841998a1e6182a0bc59a3f85999b6e10f4d5`.

Git commit `61d41b1291ff78e73db086f1c893078e4a4f71e1` adds exactly the line
`ATTENTION: PORTUGUESE DOCUMENT` and its newline, 31 bytes. Its parent file
matches the manifest's length and digest exactly. Removing that one added
line from the current bytes reproduces the parent exactly. The recorded target
observations are unchanged. This explains the integrity discrepancy; it does
not authenticate the original measurement or provide a module identity.
The old manifest and report were preserved, not silently rehashed or rewritten.

Reproduce from the repository root without accessing hardware:

```sh
python3 - <<'PY'
from pathlib import Path
import hashlib, json, subprocess
p = Path('docs/hardware-evidence/TVZ-001/operator-report.txt')
m = json.loads(p.with_name('manifest.json').read_text())
old = subprocess.check_output([
    'git', 'show', '61d41b1291ff78e73db086f1c893078e4a4f71e1^:' + str(p)])
now = p.read_bytes()
assert len(old) == m['bytes'] == 733
assert hashlib.sha256(old).hexdigest() == m['sha256']
assert now.replace(b'ATTENTION: PORTUGUESE DOCUMENT\n', b'', 1) == old
assert len(now) == 764
assert hashlib.sha256(now).hexdigest() == (
    '45c167c1980f05dc40817f972bc9841998a1e6182a0bc59a3f85999b6e10f4d5')
print('TVZ original manifest and annotated report reconciled')
PY
```

## One missing selection, with two admissible evidence forms

For a **historical deployment**, the next useful input is an attributable
offline target artifact selecting the supplying legacy module, with a path to
its source/build/patch configuration. A package database can identify installed
candidates; a module digest can identify bytes; a boot-selection record can
establish which candidate supplied the interface. None alone necessarily proves
all three. Review only the newly selected implementation differences against
the existing audited contract. Do not re-audit the entire recovered family.

For a **future clean-room provider**, state that implementation choice
explicitly and supply or construct its reviewable allocation-to-SGX-mapping
contract. This is route C in the [handoff](CODEX-HANDOFF.md), not evidence that
the historical machine used that provider. Closure still requires actual
allocation/zeroing, payload bounds, CPU/GPU contents preservation, placement,
lifetime/synchronization, and failure behavior. A proposed contract, an empty
interface, or modern GEM allocation alone is not implementation-grade proof.

The clarification requested on restart is which of these is intended, and
the offline artifact/path or implementation identity if already available.
No answer has yet been incorporated into this report. No new probe, ioctl,
module load, or hardware work is requested or authorized.

## Decision and next action

| Item | Status |
| --- | --- |
| Target backing provider | CONDITIONAL |
| Fresh backing zero | CONDITIONAL for target |
| Contents preserved to first GPU use | CONDITIONAL for target |
| First-use no-overlap | CONFIRMED, existing P7H-030 |
| Nine restricted first-use holes | CONDITIONAL |
| `0x07000345` read set | UNKNOWN |
| FG-02 | OPEN |
| Gate B / whitelist / hardware | BLOCKED / `[]` / UNVERIFIED |

**Next action:** resolve the intended supplying-provider selection using one
attributable offline deployment/build record or a concrete replacement-provider
choice. Do not substitute another package-lineage correlation. If that selection
and its deterministic backing contract close the premise, immediately continue
[FG-02](frozen-fragment-link-progress.md); do not reopen PDS decoding.

## Restart validation

Both retained ELF SHA-256 values matched. The existing backing audit verified
195 acquisition/derived-file hashes and 30 Debian source size/digest checks;
signatures remain unauthenticated. Its placement-mask and altered-source negative
regressions passed. All 36 Phase 7/evidence CSV header/width checks, the seven
new discriminator rows, P7H-001–035 defining-ID uniqueness, and 574 local
Markdown file links passed. The report-reconciliation command parsed and ran.
`git diff --check` passed; `references/` has no Git changes and the index is
empty. No historical program or device operation was executed. No new evidence
ID or confidence promotion was needed for this continuation of P7H-035.
