from pathlib import Path
import shutil,json,hashlib,importlib.util
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');e=r/'docs/phase8/artifacts/candidate-01-build-20260930'
def h(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for path,sha in json.loads((w/'inputs.json').read_text()).items():assert h(Path(path))==sha,path
for row in json.loads((w/'tool-identity-check.json').read_text()):assert h(Path(row['path']))==row['sha256'],row['path']
for candidate in ('candidate-01','candidate-02-lifecycle'):
 d=json.loads((w/candidate/'identity.json').read_text());assert d['full_qualification']=='ABI QUALIFICATION: PASS';assert d['repeat_build']=='BIT IDENTICAL';assert d['missing_imports']==d['crc_mismatches']==0
s=importlib.util.spec_from_file_location('fv',r/'tools/psb-dri-re/frozen_module_versions.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m);original,_=m.read_versions(r/'docs/hardware-evidence/MINI12-20260927-H0/raw/installed-gma500_gfx.ko');assert 'drm_irq_uninstall' not in original and 'free_irq' in original
review=json.loads((w/'original-irq-callsite-review.json').read_text());assert review['no_direct_irq_release_in_selected_teardown']
(w/'qualification-decision.json').write_text(json.dumps({'candidate_01':'ABI QUALIFICATION: PASS','candidate_02_lifecycle':'ABI QUALIFICATION: PASS','layout':'0xb84efb99','config_generated_hashes_unchanged':7426,'new_config_generated_entries':0,'all_authoritative_input_tool_hashes_unchanged':True,'original_has_free_irq_import_only_other_chip_calls':True,'original_selected_teardown_has_no_irq_release':'confirmed by source + binary callsites','current_original_hot_transition':'BLOCKED','gate_b':'BLOCKED','whitelist':[],'target_contact':'NONE','sgx_fire':'NONE','completion_observed':False,'color_readback':False,'triangle_demonstrated':False,'next_step_classification':'READ-ONLY TARGET OBSERVATION','next_step':'separately authorized normal boot module/initramfs/fallback inventory; no execution in this task'},indent=2)+'\n')
for p in sorted(w.iterdir()):
 if p.is_file() and p.suffix in ('.json','.txt','.stdout','.stderr','.py','.md','.patch'):
  shutil.copyfile(p,e/p.name)
for dirname in ('candidate-01','candidate-02-lifecycle','documentation-before'):
 shutil.copytree(w/dirname,e/dirname)
for dest,tree in (('candidate-01',w/'build-2-candidate-01-first'),('candidate-02-lifecycle',w/'build-6-candidate02-first')):
 shutil.copyfile(tree/'gma500_gfx.mod.c',e/dest/'gma500_gfx.mod.c')
 shutil.copyfile(tree/'psb_drv.c',e/dest/'integrated-psb_drv.c')
 shutil.copyfile(tree/'psb_irq.c',e/dest/'integrated-psb_irq.c')
 shutil.copyfile(tree/'Makefile',e/dest/'integrated-Makefile')
(e/'fixed-inputs').mkdir()
for row in json.loads((w/'fixed-inputs-lifecycle.json').read_text()):
 p=r/row['repository_path'];shutil.copyfile(p,e/'fixed-inputs'/row['module_filename'])
for name in ('antix-fixed-makefile.patch','antix-fixed-irq.patch','antix-irq-lifecycle.patch','antix-fixed-ioctl.patch'):
 shutil.copyfile(r/'kernel/sgx535_frozen/patches'/name,e/('final-'+name))
print('Artifact evidence copied with distinct candidate1/lifecycle modules and Kbuild-generated mod.c files')
