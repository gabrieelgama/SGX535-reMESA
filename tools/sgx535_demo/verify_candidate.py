#!/usr/bin/env python3
"""Read-only offline candidate/card/document checks; no device access."""
import gzip
import hashlib
import json
from pathlib import Path
import re
import two_triangle as q

def main():
    repo=q.REPO; doc=repo/'docs/phase8'
    mp=doc/'two-triangle-experimental-candidate-20261006.json'
    m=json.loads(mp.read_text());c=json.loads((doc/'next-3d-execution-card-20261006.json').read_text())
    def verify(path,meta):
        raw=path.read_bytes()
        assert len(raw)==meta['bytes'],path
        assert hashlib.sha256(raw).hexdigest()==meta['sha256'],path
    for k,v in m['archived_files'].items():verify(repo/k,v)
    base=json.loads((doc/'FIRE3-TRIANGLE-REPRODUCTION.json').read_text())
    for n in ('observer','client','uapi'):
        assert m[n]['sha256']==base['repository_artifacts'][n]['sha256'],n
    assert c['candidate_manifest_sha256']==hashlib.sha256(mp.read_bytes()).hexdigest()
    for n in ('driver','observer','image','client','uapi'):assert c[n]==m[n],n
    assert c['boot_uuid'] is None and c['source_witness'] is None
    assert c['maximum_ioctl_attempts']==c['maximum_client_launches']==1
    assert c['sgx_execution_authorized'] is False and c['display_publication_follows'] is False
    assert m['delta']['shader_words_unchanged']==['001f00ff','fca7f1f1']
    a=doc/'artifacts/two-triangle-experiment-20261006'
    info=json.loads((a/'image-identity.json').read_text())
    import frozen_first_load_image as image
    blob=(repo/m['image']['path']).read_bytes()
    rows,end=image.scan(gzip.decompress(blob[info['early_prefix_bytes']:]))
    members={r['name']:r['data'] for r in rows}
    assert image.verify_init_hook(members['init'],members[image.HOOK])==info['packaged_hook_sha256']
    assert members[image.PAYLOAD]==(a/'gma500_gfx.ko').read_bytes()
    assert hashlib.sha256(members['usr/lib/sgx535-first-load/sgx535_provenance.ko']).hexdigest()==m['observer']['sha256']
    oracle=(a/'expected-color.bin').read_bytes()
    assert oracle==q.expected_image()
    stats=q.inspect_readback(oracle);assert stats['magenta_pixels']==256
    qual=json.loads((doc/'first-3d-offline-qualification-20261006.json').read_text())
    assert qual['CPU']==m['qualification']
    links=0
    for name in ('FIRST-REAL-3D-ROADMAP.md','REUSABLE-RENDER-FRAMES.md','VISIBLE-TRIANGLE-REPRODUCTION.md'):
        p=doc/name;text=p.read_text()
        for link in re.findall(r'\]\(([^)]+)\)',text):
            if '://' in link:continue
            assert (p.parent/link.split('#')[0]).exists(),(name,link)
            links+=1
    print(json.dumps({'candidate_archive_files':len(m['archived_files']),'local_links':links,
        'image_packaged_driver_observer_hook':'PASS','card_manifest':'PASS',
        'oracle':'256 magenta /768 zero (CPU only)','hardware_interactions':0,'SGX_invocations':0},indent=2))

if __name__=='__main__':main()
