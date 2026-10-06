#!/usr/bin/env python3
"""Read-only manifest/tutorial agreement; no X connection or hardware access."""
import hashlib
import json
from pathlib import Path
import re
from triangle_pixels import validate_source

def main():
    repo=Path(__file__).resolve().parents[2]
    doc=repo/'docs/phase8/VISIBLE-TRIANGLE-REPRODUCTION.md'
    m=json.loads(doc.with_suffix('.json').read_text())
    def verify(name,meta):
        p=repo/name
        assert p.stat().st_size==meta['bytes'],name
        assert hashlib.sha256(p.read_bytes()).hexdigest()==meta['sha256'],name
    for k,v in m['archive_files'].items():verify(k,v)
    for k,v in m['implementation'].items():verify(k,v)
    verify(m['source']['path'],m['source'])
    validate_source((repo/m['source']['path']).read_bytes(),m['source']['sha256'])
    render=json.loads((repo/m['render_manifest']['path']).read_text())
    assert hashlib.sha256((repo/m['render_manifest']['path']).read_bytes()).hexdigest()==m['render_manifest']['sha256']
    assert m['render_candidate']==render['candidate']
    a=m['actual_first_successful_display']; card=json.loads((repo/a['card_path']).read_text())
    assert hashlib.sha256((repo/a['card_path']).read_bytes()).hexdigest()==a['card_sha256']
    assert a['target']==card['target']
    assert a['operator']['physical_triangle_seen'] and a['restoration_all_bytes_equal']
    assert (a['rectangle']['x'],a['rectangle']['y'],a['presentation_scale'])==(560,320,5)
    assert m['earlier_64_64_attempt']['physical_visibility_established'] is False
    assert m['milestones']['DIRECT_SGX_SCANOUT_ESTABLISHED'] is False
    human=m['historical_human_observation_time']
    assert human['local']=='2026-10-06 04:11 BRT' and human['UTC_minute']=='2026-10-06 07:11 UTC'
    assert human['wall_clock_onset_independently_instrumented'] is False
    text=doc.read_text()
    for t in ('HISTORICAL SUCCESSFUL PROCEDURE','CURRENT RECOMMENDED REPRODUCTION PROCEDURE',
              'KNOWN_GOOD_TRIANGLE_RENDER','KNOWN_GOOD_TRIANGLE_DISPLAY_PUBLICATION',
              '560,320','64,64','2026-10-06 04:11 BRT',m['source']['sha256']):assert t in text,t
    links=0
    for link in re.findall(r'\]\(([^)]+)\)',text):
        if '://' in link:continue
        assert (doc.parent/link.split('#')[0]).exists(),link
        links+=1
    print(json.dumps({'archive_files':len(m['archive_files']),'implementation_files':len(m['implementation']),
                      'local_links':links,'tutorial_manifest':'PASS','hardware_interactions':0},indent=2))

if __name__=='__main__':main()
