"""Recheck observation-derived facts locally; no commands/contact/mutation on target."""
from pathlib import Path
import hashlib,json,re
from read_initramfs import read_image
p=Path(__file__).resolve().parent;e=p.parent;r=e.parents[2]
raw=(e/'stdout.txt').read_bytes();cap=json.loads((e/'capture.json').read_text())
assert cap['exit_status']==0 and not (e/'stderr.txt').read_bytes()
assert len(raw)==cap['stdout_bytes'] and hashlib.sha256(raw).hexdigest()==cap['stdout_sha256']
assert raw.endswith(b'BOOT_PROVENANCE_READONLY_PASS\n')
files=e/'decoded-files'
rows=json.loads((e/'artifact-manifest.json').read_text())
assert len(rows)==55
for row in rows:
 data=(files/row['label']).read_bytes()
 assert len(data)==row['target_bytes'] and hashlib.sha256(data).hexdigest()==row['target_sha256']
 assert row['target_hash_verified'] and row['target_size_verified']
image=files/'release_initrd'
assert hashlib.sha256(image.read_bytes()).hexdigest()=='f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'
items,containers=read_image(image.read_bytes());assert len(items)==2138
lookup={x['name']:x for x in items}
assert not any('gma500' in x['name'] for x in items)
assert 'conf/modules' not in lookup
init=lookup['init']['data'].decode();order=lookup['scripts/init-top/ORDER']['data'].decode()
assert init.index('run_scripts /scripts/init-top')<init.index('\nload_modules\n')
assert '/scripts/init-top/udev' in order
assert lookup['init']['data']==(files/next(row['label'] for row in rows if row['target_source']=='/usr/share/initramfs-tools/init')).read_bytes()
cfg=(files/'_boot_grub_grub_cfg').read_text();grubenv=(files/'_boot_grub_grubenv').read_bytes()
assert 'set default="${saved_entry}"' in cfg and 'source ${config_directory}/custom.cfg' in cfg
saved='gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1'
assert ('saved_entry='+saved).encode() in grubenv and b'next_entry=' not in grubenv
assert cfg.count('linux\t/boot/vmlinuz-5.10.240-antix.1-486-smp')==6
assert cfg.count('initrd\t/boot/initrd.img-5.10.240-antix.1-486-smp')==6
coverage=json.loads((p/'dependency-coverage.json').read_text())
assert coverage['count']==9
for row in coverage['modules']:
 item=lookup[row['initramfs_member']]
 assert item['sha256']==row['sha256'] and row['missing']==0 and row['crc_mismatches']==0
 assert row['checker_exit_code']==0 and row['module_layout']=='0xb84efb99'
# Existing pinned artifacts are observations, not a new build/requalification.
base=r/'docs/phase8/artifacts/candidate-01-build-20260930'
identities=[]
for relative,size,digest in [('candidate-01/gma500_gfx.ko',242364,'13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f'),('candidate-02-lifecycle/gma500_gfx.ko',242724,'91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74')]:
 artifact=base/relative;data=artifact.read_bytes();assert len(data)==size and hashlib.sha256(data).hexdigest()==digest
 identities.append({'path':str(artifact.relative_to(r)),'size':size,'sha256':digest})
(p/'pinned-artifact-identities.json').write_text(json.dumps(identities,indent=2)+'\n')
print('55 captured files exact; stock image2138 members/no gma; init-top before conf modules; GRUB saved stock/six same-image entries; nine dependency records; both pinned modules unchanged: PASS')
