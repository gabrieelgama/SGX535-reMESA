from pathlib import Path
import hashlib,json,re,subprocess
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/boot-provenance-readonly-20261001T025947Z');e=r/'docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02'
initial=json.loads((w/'initial-repository-files.json').read_text())
allowed={f'docs/phase8/{n}' for n in ['CHATGPT-HANDOFF-POST-ATTEMPT03.md','post-attempt03-checkpoint-audit.md','fixed-one-shot-gate-b-review.md','active-gma500-transition-review.md','first-load-boot-qualification.md']}
changed=[]
for name,digest in initial.items():
 p=r/name;assert p.is_file(),('preexisting file lost',name)
 if hashlib.sha256(p.read_bytes()).hexdigest()!=digest:
  assert name in allowed,('unrelated file modified',name)
  text=p.read_text();start=text.index('**Newest read-only boot observation (2026-10-01):**');end=text.index('Earlier no-contact/missing-boot-file statements describe their historical turns.',start)+len('Earlier no-contact/missing-boot-file statements describe their historical turns.\n\n')
  restored=text[:start-1]+text[end:]
  assert hashlib.sha256(restored.encode()).hexdigest()==digest,('historical content overwritten',name)
  changed.append(name)
assert set(changed)==allowed
newdocs=[r/'docs/phase8/first-load-stock-boot-observation.md',e/'RESULT.md',e/'README.md']+[r/x for x in changed]
links=[]
for p in newdocs:
 # Only new portions of preexisting docs: old historical links are outside this turn.
 text=p.read_text()
 if str(p.relative_to(r)) in allowed:
  text=text.split('**Newest read-only boot observation (2026-10-01):**',1)[1].split('Earlier no-contact/missing-boot-file statements describe their historical turns.',1)[0]
 for match in re.finditer(r'\[[^\]]*\]\(([^)]+)\)',text):
  dest=match[1]
  if '://' in dest or dest.startswith('#'):continue
  target=(p.parent/dest.split('#',1)[0]).resolve();assert target.exists(),('broken new link',p,dest)
  links.append({'source':str(p.relative_to(r)),'target':dest})
status=subprocess.run(['git','status','--short'],cwd=r,capture_output=True);assert status.returncode==0
(e/'offline-analysis/final-checks/final-git-status.stdout').write_bytes(status.stdout)
cp=subprocess.run(['git','diff','--check'],cwd=r,capture_output=True);assert cp.returncode==0,cp.stdout.decode()+cp.stderr.decode()
(e/'offline-analysis/final-checks/final-diff-check.stdout').write_bytes(cp.stdout);(e/'offline-analysis/final-checks/final-diff-check.stderr').write_bytes(cp.stderr)
# All new payloads reside only within the two captures and the new report.
current=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=r).decode().split('\0')
new=[n for n in current if n and n not in initial and (r/n).is_file()]
newprefix=['docs/hardware-evidence/MINI12-20261001T025947Z-BOOT-PROVENANCE-READONLY-01/','docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02/']
assert all(n=='docs/phase8/first-load-stock-boot-observation.md' or any(n.startswith(q) for q in newprefix) for n in new),[n for n in new if not any(n.startswith(q) for q in newprefix) and n!='docs/phase8/first-load-stock-boot-observation.md']
result={'initial_preserved_files':len(initial),'modified_preexisting_files':sorted(changed),'new_paths_so_far':sorted(new),'new_file_count_so_far':len(new),'historical_content_preserved':True,'no_unrelated_preexisting_changes':True,'new_relative_links_checked':links,'git_diff_check_exit':cp.returncode,'target_contact_after_capture_02':False}
(e/'offline-analysis/final-checks/repository-delta.json').write_text(json.dumps(result,indent=2)+'\n')
print('Preserved',len(initial),'preexisting files; only five additive checkpoint notices; new capture directories/new report only; new relative links',len(links),'PASS; final diff check PASS.')
