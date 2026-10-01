from pathlib import Path
import json,hashlib
r=Path('/home/gama/sgx535-gfx');w=Path(__file__).parent
e=r/'docs/phase8/artifacts/experimental-first-load-01-20261001'
notice='''\n**Newest offline image qualification (2026-10-01):**
[Experimental first-load image](experimental-first-load-image-qualification.md)
and its [evidence](artifacts/experimental-first-load-01-20261001/README.md) now
establish construction/guards/pre-udev ordering, exact dependency CRC coverage,
stock preservation and non-saving GRUB text **PASS OFFLINE**. The final image is
50,804,481 bytes, SHA-256
`4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71`;
two corrected builds are byte-identical. **249 tests, zero skips; 34 new image
checks; 14 boot-analysis tests; three UBSan harnesses PASS.** No module rebuilt,
no target contact/mutation, no image/entry installed, no SGX fire. Actual early
first ownership, display/SSH and reset/fallback remain UNKNOWN. The next step is
OFFLINE exact staging/experimental-boot/reset-to-stock recovery review, followed
by separate authorization if justified. **Gate B BLOCKED; whitelist `[]`.**
Earlier “no image constructed”/next-step statements below are historical.
\n'''
paths=['docs/phase8/CHATGPT-HANDOFF-POST-ATTEMPT03.md','docs/phase8/post-attempt03-checkpoint-audit.md','docs/phase8/fixed-one-shot-gate-b-review.md','docs/phase8/active-gma500-transition-review.md','docs/phase8/first-load-boot-qualification.md','docs/phase8/first-load-stock-boot-observation.md']
initial=json.loads((w/'initial-files.json').read_text())
records=[]
for p in paths:
 f=r/p;old=f.read_bytes();assert hashlib.sha256(old).hexdigest()==initial[p],p
 head,tail=old.split(b'\n',1)
 new=head+b'\n'+notice.encode()+tail;f.write_bytes(new)
 assert new.replace(notice.encode(),b'',1)==old
 records.append({'path':p,'original_sha256':initial[p],'current_sha256':hashlib.sha256(new).hexdigest(),'original_bytes_preserved_by_removing_one_new_notice':True})
(e/'checkpoint-notice.txt').write_text(notice)
(e/'allowed-checkpoint-updates.json').write_text(json.dumps(records,indent=2)+'\n')
p=r/'docs/phase8/experimental-first-load-build-plan.md'
p.write_text(p.read_text().replace('- [ ]','- [x]'))
