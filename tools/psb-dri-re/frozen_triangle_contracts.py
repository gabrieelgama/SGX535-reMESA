#!/usr/bin/env python3
"""Selected static target, auxiliary input and bootstrap constraints. No device I/O.

Target closure is scoped to the retained renderer's shared surface contract,
not hardware functionality. Revision evaluation reproduces CPU branch arithmetic;
it neither authenticates target revision nor establishes register-write safety.
"""
import argparse
import csv
import json
import struct
from pathlib import Path

DOC=Path(__file__).resolve().parents[2]/'docs/phase7/psb-dri-re'

def target():
    return dict(width=32,height=32,cpp=4,pitch_pixels=32,stride_bytes=128,
        size=4096,layout='LINEAR',word_format='ARGB8888',memory_order='BGRA',
        base_alignment=4096,samples=1,depth=False,stencil=False,
        initial_value='ZERO_BY_CLEANROOM_POLICY',status='DERIVED_CONFIRMED',
        evidence='P7H-048; DRI0x4c300/0x4e638/0x4effa/0x4b818/0x392ab/0x4b710')

def pixel_offset(t,x,y,flip_y=False):
    if not 0<=x<t['width'] or not 0<=y<t['height']:raise ValueError('pixel out of bounds')
    row=t['height']-1-y if flip_y else y
    return 4*(row*t['pitch_pixels']+x)

def pack_rgba(r,g,b,a):
    if any(not isinstance(x,int) or not 0<=x<=255 for x in (r,g,b,a)):raise ValueError('channel range')
    return struct.pack('<I',a<<24|r<<16|g<<8|b)

def check_target(m,t):
    if t!=target():raise ValueError('unsupported target layout/stride')
    if t['stride_bytes']!=t['cpp']*t['pitch_pixels'] or t['size']<t['stride_bytes']*t['height']:raise ValueError('target extent')
    # No general encoding inference: scoped formulas from the retained cpp4 branch.
    want=[((t['pitch_pixels']//2)-1)<<16|0x8000,'GPU(color)',0,(t['height']-1)<<12|t['width']-1]
    if [f['value'] for f in m['objects']['target_data']['fields']]!=want:raise ValueError('target descriptor')
    if m['objects']['color']['bytes']!=[0]*t['size']:raise ValueError('target initialization')
    return True

def auxiliary(m):
    names=['vertex_pds','bounds_vertex_pds','bounds_state_pds','triangle_state_pds','terminate_state_pds','event_pds','background_pds','secondary_pds','vertex_secondary','bounds_vertex_secondary','background_secondary']
    rows=[]
    for n in names:
        o=m['objects'][n]
        if n=='event_pds':
            role='end-of-render/event path; CPU target descriptor and two USE addresses'
            missing='Selected event-entry/control source domain before first write; CPU branches do not establish hardware event inputs'
        elif n=='background_pds':
            role='background color copy; CPU color descriptor and USE address'
            missing='Source-domain completeness of selected four-word background program'
        elif n.endswith('secondary') or n=='secondary_pds':
            role='single af000000 record; zero data-prefix dwords emitted'
            missing='No-data CPU grammar does not establish absence of architectural/implicit inputs'
        elif 'state' in n:
            role='state upload; descriptor count derived from serialized state length'
            missing='Selected upload program source-domain completeness beyond initialized descriptor'
        else:
            role='vertex fetch; deterministic address/stride/width plus selected index stream'
            missing='Selected vertex fetch source-domain completeness including launch-supplied selection'
        rows.append(dict(program=n,size=o['size'],producer=o['producer'],role=role,
          data_prefix_bytes=64 if n=='event_pds' else (0 if 'secondary' in n else 48),
          cpu_image='DEFINED_WITH_SYMBOLIC_RELOCATIONS',dependencies=';'.join(o['dependencies']),
          coverage='UNKNOWN',missing_rule=missing,
          evidence=o['evidence']+';P7H-049',hardware_ready=False))
    return rows

def check_auxiliary(m,rows):
    if rows!=auxiliary(m):raise ValueError('missing/changed auxiliary coverage or provenance')
    known_sites={(r['owner'],r['offset']) for r in m['relocations']}
    for row in rows:
        o=m['objects'][row['program']]
        if o['bytes'] is None:raise ValueError('auxiliary initialization absent')
        for i,b in enumerate(o['bytes']):
            if b is None and (row['program'],i//4*4) not in known_sites:raise ValueError('uninitialized auxiliary byte')
    # Scoped CPU descriptor arithmetic; not an ISA/DMA decode.
    for name,n in [('vertex',8),('bounds_vertex',4),('bounds_state',11),('triangle_state',14),('terminate_state',2)]:
        o=m['objects'][name+'_pds']
        if o['fields'][1]['value']!=0x80000000|((n-1)<<21)|(n-1):
            raise ValueError('auxiliary source descriptor length mismatch')
        source='vertices' if name=='vertex' else 'bounds_vertices' if name=='bounds_vertex' else name
        if m['objects'][source]['size']<n*4:raise ValueError('auxiliary source too short')
    return True

def ta_cookie(pages,pt_gpu,param_gpu):
    """CPU cookie for selected allocation; NOT contents of hardware TA tables."""
    if pages<=0x61f or pages>0xffff:raise ValueError('TA pages outside scoped CPU range')
    if pt_gpu%4096 or param_gpu%0x100000:raise ValueError('TA backing alignment')
    start=(param_gpu&0xfffffff)>>12
    if start+pages-1>0xffff:raise ValueError('TA packed endpoint overflow')
    c=[0]*13;c[0]=pt_gpu;c[1]=param_gpu&0xfffffff;c[2]=pages
    c[3:6]=[pages-0x600]*3;c[6]=pages-0x640 if pages-0x600>=0x41 else 0
    c[10]=start;c[11]=start+pages-1;c[12]=c[11]-1
    return c

def bootstrap(raw_revision,provenance='SYNTHETIC_MODEL_INPUT'):
    if not isinstance(raw_revision,int) or not 0<=raw_revision<2**32:raise ValueError('revision word at SGX mapping+0x14 required')
    if provenance not in ('SYNTHETIC_MODEL_INPUT','QUALIFIED_SGX_REVISION_EVIDENCE'):raise ValueError('PCI revision is not SGX revision')
    code=100+(raw_revision&255)+10*((raw_revision>>8)&255)
    listed=(107,108,109,111,113)
    # Xpsb0x4af0 returns without option stores for code 121; XpsbInit
    # allocates the option state with Xcalloc before that call. Admit only
    # the separately observed raw target revision, not arbitrary defaults.
    observed_default=(raw_revision==0x00010201 and
                      provenance=='QUALIFIED_SGX_REVISION_EVIDENCE')
    if code not in listed and not observed_default:
        raise ValueError('unqualified revision branch: no guessed default')
    options=set()
    if code==107:options.add(7)
    if code in (107,108):options.update((6,13))
    if code in (107,108,109):options.update((5,8,9,10,11,12,14,20,15,16,17,19,21,22,23,24,25,26,27,28,30,31,43))
    if code in listed:
        options.update((18,29,32,33,34,35,37,38,40,41,42,44,45,46))
    if code in (111,113):options.add(36)
    values={0xca0:0xc07c,0x13c:0,0xa7c:0,0xa80:0,0xa74:0x5021900 if 13 in options else 0x5188200,
            0xa78:0,0xaac:0x44,0xabc:0xc,0x804:0x100ffff if 43 in options else 0xffff,
            0xa00:0x7c000,0x630:0,0xa58:0}
    return dict(cpu_branch_code=code,raw_revision=raw_revision,provenance=provenance,
        option_dword_indices=sorted(options),register_values=values,
        branch_selection='ZEROED_DEFAULT_OPTIONS' if observed_default else 'LISTED_OPTION_CASE',
        evidence='P7H-050; XpsbInit0x28c0 Xcalloc; Xpsb0x4af0 default0x4c79;0x3820',
        ready=False,hardware_ready=False,gate_b='BLOCKED',
        remaining='Target-qualified service ready-state contract for these revision-conditioned register actions; CPU arithmetic is not applicability proof')

def publication_trace(cache_control):
    """Non-executable CPU transaction inventory; named registers are correlations."""
    if cache_control not in (0xffff,0x100ffff):raise ValueError('unqualified cache-control branch')
    return [dict(cache_control_input=cache_control,write_offset=w,write_value=v,status_offset=s,status_mask=mask,
        status_expected=mask,clear_offset=clear,clear_value=mask,clear_expected=0,
        cpu_source='Xpsb0x4f10/0x5040 ->0x4e10',
        register_correlation=name,postcondition='UNKNOWN for target; no publication closure',
        failure_policy='CLEANROOM_POLICY abort before consumer on either poll failure',
        evidence='P7H-051') for w,v,s,mask,clear,name in [
          (0xad4,1,0x138,0x44,0x140,'SGX535 header PDS_INV1; status0x138 fields unavailable'),
          (0xae0,1,0x138,1,0x140,'SGX535 header PDS_INV_CSC; status0x138 fields unavailable'),
          (0x804,cache_control|0x10000000,0x12c,0x4000000,0x134,'SGX535 header MADD_CACHE_INVALCOMPLETE status bit')]]

def check_poll_results(results):
    # This checks completion observations only, not their unknown architectural scope.
    if len(results)!=3 or any(pair!=(True,True) for pair in results):
        raise ValueError('publication transaction incomplete; do not kick consumer')
    return True

def closure(m):
    check_target(m,target());check_auxiliary(m,auxiliary(m))
    return dict(B1='OPEN',B2='CLOSED',B3='CONDITIONAL',B4='OPEN',L12='OPEN',
        remaining=['L12','FT-AUX','FT-BO','FT-SERVICE'],complete=False,hardware_ready=False,
        obligations={
          'required_objects':'PARTIAL: selected payload inventory present; service-generated contents conditional',
          'consumed_bytes':'UNKNOWN: L12 and auxiliary source domains',
          'address_fields':'CONFIRMED for emitted CPU objects; 38 sites including 7 aliases',
          'relocation_targets':'CONFIRMED: every emitted site has an existing target',
          'bo_constraints':'CONDITIONAL: explicit sizes/domains; dynamic manager limit and publication required',
          'target_layout':'CONFIRMED: selected linear ARGB8888 surface contract',
          'auxiliary_inputs':'UNKNOWN: CPU images do not certify all launch inputs',
          'bootstrap':'UNKNOWN: target-qualified ready-state postcondition',
          'primary_coverage':'UNKNOWN: L12 frozen OPEN',
          'no_required_unknowns':'FALSE'})

def main():
    import frozen_triangle_image as image
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--write-tables',action='store_true');ap.add_argument('--revision',type=lambda x:int(x,0));ap.add_argument('--complete',action='store_true');a=ap.parse_args()
    m=image.build();result=closure(m)
    if a.complete:ap.exit(1,'PARTIAL: '+', '.join(result['remaining'])+'\n')
    if a.write_tables:
        for name,rows in [('frozen-triangle-target.csv',[target()]),('frozen-triangle-auxiliary.csv',auxiliary(m)),('frozen-triangle-publication.csv',publication_trace(0xffff)+publication_trace(0x100ffff))]:
            with (DOC/name).open('w',newline='') as f:
                w=csv.DictWriter(f,fieldnames=list(rows[0]),lineterminator='\n');w.writeheader();w.writerows(rows)
    print(json.dumps(bootstrap(a.revision) if a.revision is not None else result,indent=2))
if __name__=='__main__':main()
