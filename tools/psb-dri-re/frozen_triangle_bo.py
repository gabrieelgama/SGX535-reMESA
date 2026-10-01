#!/usr/bin/env python3
"""Static clean-room BO plan and reference-family wire packing. NEVER submits.

CLEANROOM_POLICY grouping/offsets; not historical allocation replay. GPU addresses
and USE register assignments are explicit caller inputs, never production defaults.
The existing primary image is retained at +0x160; a separate overlap check covers
this new plan. Architectural coverage and device publication remain open.
"""
import argparse
import csv
import json
from pathlib import Path
import re
import struct

DOC=Path(__file__).resolve().parents[2]/'docs/phase7/psb-dri-re'
PAGE=4096
DOMAINS={'PDS':(0x20000000,0x30000000),'RASTGEOM':(0x30000000,0x40000000),
         'MMU':(0x40000000,0x100000000),'LOCAL':None}

def align(n,a):return (n+a-1)//a*a

def build(m):
    def spec(size,alignment,domain,flags,owner,role):
        return dict(size=size,alignment=alignment,domain=domain,create_flags=flags,
                    owner=owner,role=role,buffer_start=0,bo_type='drm_bo_type_dc; not userptr',initialization='ZERO entire allocation before object stores',
                    lifetime='retain until applicable TA/raster/scene fence completes',
                    map='CPU read/write; CACHED only where flag0x80 set; otherwise TTM UC/WC',
                    classification='CLEANROOM_POLICY constrained by P7G-015/P7H-047')
    p={'schema':1,'bos':{
       'pds':spec(0x20000,PAGE,'PDS',0x20000001,'user','PDS; constants; bounds geometry; indices'),
       'use':spec(0x8000,0x8000,'PDS',0x8000020000001,'user','USE code; USSE fence tag; two data masters'),
       'vertex_ta':spec(PAGE,PAGE,'MMU',0x1000010000001,'user','vertex input and TA stream; TA fence tag'),
       'background':spec(PAGE,PAGE,'RASTGEOM',0x80000001,'user','background object'),
       'control':spec(PAGE,PAGE,'LOCAL',0x01000084,'user','CPU-consumed TA/raster lists and relocation table'),
       'color':spec(PAGE,PAGE,'MMU',0x10000003,'user','render target; read and write; excludes VRAM'),
       'scene_hw':spec(0x2000,PAGE,'MMU',0x2000010000083,'kernel','scene requested0x1420 rounded two pages'),
       'ta_page_table':spec(0x2000000,PAGE,'MMU',0x2000010000003,'kernel','8192-page policy matches candidate default'),
       'ta_parameter':spec(0x2000000,0x100000,'RASTGEOM',0x2000080000003,'kernel','TA parameters; 1MiB alignment'),
       'xhw_comm':spec(PAGE,PAGE,'LOCAL',0x01000093,'service','132-byte XHW communication; privileged legacy NO_EVICT flag')},
       'bindings':{},'host_objects':{},'validation':[],'wire_relocations':[],
       'publication':{'gpu_visibility':'CONDITIONAL','missing_rule':'Device-side publication contract: selected cache/TLB maintenance and completion must make the initialized/relocated backing visible to the first consumer',
          'ordered_steps':['fresh allocate; reject fixed/stolen placement','map and zero complete payload','write exact CPU objects','finish CPU writes before provider handoff','validate every BO without presumed hint','apply relocations to validated backing','complete CPU store barrier and selected device cache/TLB maintenance','publish register lists then first service consumer','retain backing and USE reservations through fences'],
          'same_backing_source':'drm_bo_handle_move_mem nonfixed branch -> drm_bo_move_ttm -> same ttm; psb_buffer bind passes same pages to MMU',
          'scope':'implementation requirement; not a running provider or hardware proof'},
       'static_triangle_complete':False}
    cursors={n:0 for n in p['bos']};cursors['pds']=0x200
    p['bindings']['primary_pds']={'bo':'pds','offset':0x160,'size':56,'reserved':56,'alias':False}
    # Explicit non-payload interfaces; not spurious GPU allocations.
    hosts={'submit_command','validation_nodes','fence_reply','scene_arg','scene_cookie','xhw_service'}
    special={'scene_hw','ta_page_table','ta_parameter'}
    for n,o in m['objects'].items():
        if n=='primary_pds' or 'VIEW ta_stream' in o['notes']:continue
        if n in hosts:
            p['host_objects'][n]={'size':o['size'],'kind':'CPU interface/state; not GPU BO payload'};continue
        if n in special:
            p['bindings'][n]={'bo':n,'offset':0,'size':p['bos'][n]['size'],'reserved':p['bos'][n]['size'],'alias':False};continue
        if n=='relocation_wire':continue # size becomes known from canonical sites
        if n.endswith('_use'):group='use'
        elif n in ('vertices','ta_stream'):group='vertex_ta'
        elif n=='background_object':group='background'
        elif n in ('ta_registers','raster_registers'):group='control'
        elif n=='color':group='color'
        else:group='pds'
        if o['bytes'] is None:raise ValueError('unclassified non-payload object: '+n)
        off=align(cursors[group],o['alignment']);size=o['size']
        reserved=0x4000 if n=='indices' else size # original minimum index capacity; all initialized in new policy
        p['bindings'][n]={'bo':group,'offset':off,'size':size,'reserved':reserved,'alias':False}
        cursors[group]=off+reserved
    for n,o in m['objects'].items():
        found=re.search(r'VIEW ta_stream\+(0x[0-9a-f]+)',o['notes'])
        if found:
            parent=p['bindings']['ta_stream']
            p['bindings'][n]={'bo':parent['bo'],'offset':parent['offset']+int(found[1],16),'size':o['size'],'reserved':o['size'],'alias':True}
    users=[n for n,b in p['bos'].items() if b['owner']=='user']
    for n in users:
        b=p['bos'][n]
        # Full explicit mask; policy does not rely on old presumed offsets or defaults.
        p['validation'].append({'bo':n,'index':len(p['validation']),
          'flags':b['create_flags'],'mask':0x000b0000ff000087,'hint':0,
          'presumed':False,'handle':f'HANDLE({n})','next':'next CPU validation node or zero',
          'scope':'user set; kernel scene/TA objects separately validated by scene path'})
    vindex={r['bo']:r['index'] for r in p['validation']}
    for r in m['relocations']:
        dst=p['bindings'][r['owner']];src=p['bindings'][r['target']]
        if dst['alias']:continue
        common={'site':r['id'],'owner_bo':dst['bo'],'target_bo':src['bo'],
           'where':(dst['offset']+r['offset'])//4,'buffer':vindex[src['bo']],
           'pre_add':src['offset'],'dst_buffer':vindex[dst['bo']],
           'target_alignment':r['target_alignment']}
        if r['kind']=='USE_THREE_RECORDS':
            dm=int(re.search(r'dm=(\d)',r['formula'])[1])
            for op,left,right,mask,bg in [(5,0,0,15,int(r['background'],16)),(4,4,15,0xf0,0),(4,8,4,0x7ff00,0)]:
                p['wire_relocations'].append(dict(common,op=op,mask=mask,shift=right<<16|left,
                    background=bg,arg0=m['objects'][r['target']]['size'],arg1=dm))
        else:
            p['wire_relocations'].append(dict(common,op=0,mask=int(r['mask'],16),
              shift=r['right_shift']<<16|r['left_shift'],background=int(r['background'],16),arg0=0,arg1=0))
    off=align(cursors['control'],8);size=len(p['wire_relocations'])*40
    p['bindings']['relocation_wire']={'bo':'control','offset':off,'size':size,'reserved':size,'alias':False}
    for n,b in p['bos'].items():
        b['payload_end']=max((v['offset']+v['reserved'] for v in p['bindings'].values() if v['bo']==n),default=132 if n=='xhw_comm' else 0)
        b['gpu_access']='CPU interface only' if b['domain']=='LOCAL' else ('READ_WRITE' if b['create_flags']&2 else 'READ')
    return p

def check(m,p):
    """Evidence model integrity AND independent range/relocation completeness checks."""
    want=build(m)
    if p['bos'].keys()!=want['bos'].keys():raise ValueError('absent BO')
    if p['bindings']!=want['bindings']:raise ValueError('missing/changed binding or alignment')
    if p['validation']!=want['validation']:raise ValueError('missing/changed validation entry')
    if p['wire_relocations']!=want['wire_relocations']:raise ValueError('missing/changed relocation or control')
    if p['publication']!=want['publication']:raise ValueError('unsupported publication assertion')
    if p.get('static_triangle_complete') is not False:raise ValueError('unproved closure')
    ranges={n:[] for n in p['bos']}
    for n,b in p['bos'].items():
        if b!=want['bos'][n]:raise ValueError('altered BO requirement: '+n)
        if b['size']%PAGE or b['alignment']<PAGE or b['payload_end']>b['size']:raise ValueError('BO size/alignment')
    for n,v in p['bindings'].items():
        b=p['bos'][v['bo']]
        if v['offset']<0 or v['offset']+v['reserved']>b['size']:raise ValueError('undersized BO')
        alignment=m['objects'][n]['alignment']
        if v['offset']%alignment:raise ValueError('misaligned object')
        if not v['alias']:
            for lo,hi in ranges[v['bo']]:
                if v['offset']<hi and lo<v['offset']+v['reserved']:raise ValueError('incompatible overlap')
            ranges[v['bo']].append((v['offset'],v['offset']+v['reserved']))
    sites={(r['owner'],r['offset']) for r in m['relocations']}
    for n,o in m['objects'].items():
        if n not in p['bindings'] or p['bindings'][n]['alias'] or o['bytes'] is None:continue
        for off in range(0,len(o['bytes']),4):
            if None in o['bytes'][off:off+4] and (n,off) not in sites:raise ValueError('address field missing relocation: '+n)
    for r in p['wire_relocations']:
        if r['where']*4+4>p['bos'][r['owner_bo']]['size']:raise ValueError('relocation destination outside BO')
        if r['pre_add']%r['target_alignment']:raise ValueError('relocation target alignment')
        if r['background'] & r['mask']:raise ValueError('control bits overlap insertion mask')
    return True

def validation_words(p):
    out=[]
    for i,v in enumerate(p['validation']):
        w=[0]*34
        w[0]=f'CPU(validation[{i+1}])' if i+1<len(p['validation']) else 0
        w[4]=v['mask']&0xffffffff;w[5]=v['mask']>>32
        w[6]=v['flags']&0xffffffff;w[7]=v['flags']>>32
        w[8]=v['handle'];w[9]=0
        out.append(w)
    return out

def submission_words(m,p,handles,host_pointers):
    """Pack CPU command parameters; handles/pointers are explicit ABI inputs."""
    check(m,p)
    required={r['bo'] for r in p['validation']}
    if (set(handles)!=required
            or any(type(v) is not int or not 0<v<2**32 for v in handles.values())
            or len(set(handles.values()))!=len(handles)):
        raise ValueError('all user BO handles required')
    if set(host_pointers)!={'validation_nodes','scene_arg','fence_reply'} or any(not isinstance(v,int) or not 0<v<2**32 or v%4 for v in host_pointers.values()):
        raise ValueError('aligned i386 host pointers required')
    words=[f['value'] for f in m['objects']['submit_command']['fields']]
    for off,name in [(0,'validation_nodes'),(16,'scene_arg'),(24,'fence_reply')]:words[off//4]=host_pointers[name]
    for handleoff,offsetoff,name in [(36,40,'ta_registers'),(48,52,'raster_registers'),(60,64,'raster_registers'),(72,76,'relocation_wire')]:
        b=p['bindings'][name];words[handleoff//4]=handles[b['bo']];words[offsetoff//4]=b['offset']
    words[80//4]=len(p['wire_relocations'])
    if any(not isinstance(w,int) for w in words):raise ValueError('unresolved command field')
    return words

def relocation_bytes(p):
    return b''.join(struct.pack('<10I',*(r[k] for k in ['op','where','buffer','mask','shift','pre_add','background','dst_buffer','arg0','arg1'])) for r in p['wire_relocations'])

def merge(value,right,left,mask,background):
    return ((background&~mask)|(((value>>right)<<left)&mask))&0xffffffff

def test_addresses(p):
    """SYNTHETIC unit-test assignments ONLY; never a suggested device layout."""
    return dict(pds=0x20000000,use=0x20080000,vertex_ta=0x40000000,background=0x30000000,
                control=0,color=0x40001000,scene_hw=0x40002000,ta_page_table=0x42000000,
                ta_parameter=0x31000000,xhw_comm=0)

def zero_user_backings(p, backings=None):
    """Clean-room CPU policy: clear every byte of every user BO before stores.

    Supplied dirty backings model reused storage. This says nothing about device
    coherence, kernel-owned BOs, or on-chip PDS store initialization.
    """
    sizes={n:b['size'] for n,b in p['bos'].items() if b['owner']=='user'}
    if backings is None:
        backings={n:bytearray(size) for n,size in sizes.items()}
    if set(backings)!=set(sizes):raise ValueError('missing or extra user backing')
    for n,size in sizes.items():
        b=backings[n]
        if type(b) is not bytearray or len(b)!=size:
            raise ValueError('partial, immutable or wrong-size user backing: '+n)
    for n,size in sizes.items():
        backings[n][:]=bytes(size)
    return backings


def require_zeroed_user_backings(p, backings):
    sizes={n:b['size'] for n,b in p['bos'].items() if b['owner']=='user'}
    if set(backings)!=set(sizes):raise ValueError('missing user backing')
    for n,size in sizes.items():
        b=backings[n]
        if type(b) is not bytearray or len(b)!=size or any(b):
            raise ValueError('dirty or partial user backing: '+n)
    return True


def resolve(m,p,addresses,use_registers,mmu_limit,backings=None):
    check(m,p)
    if addresses.keys()!=p['bos'].keys():raise ValueError('incomplete address assignment')
    if not isinstance(mmu_limit,int) or not 0x40000000<mmu_limit<=0x100000000 or mmu_limit%PAGE:
        raise ValueError('provider MMU manager end required; not an invented aperture')
    seen=[]
    for n,b in p['bos'].items():
        a=addresses[n];region=DOMAINS[b['domain']]
        if type(a) is not int or not 0<=a<2**32:
            raise ValueError('32-bit GPU address or LOCAL zero placeholder required')
        if b['domain']=='LOCAL' and a!=0:
            raise ValueError('LOCAL CPU interface has no GPU address')
        if a%b['alignment']:raise ValueError('GPU address alignment')
        if b['domain']=='MMU':region=(region[0],min(region[1],mmu_limit))
        if region:
            if not region[0]<=a or a+b['size']>region[1]:raise ValueError('GPU placement outside domain')
            for lo,hi in seen:
                if a<hi and lo<a+b['size']:raise ValueError('overlapping GPU BOs')
            seen.append((a,a+b['size']))
    if (set(use_registers)!={0,1}
            or any(type(x) is not int or not 0<=x<16 for x in use_registers.values())
            or len(set(use_registers.values()))!=2):
        raise ValueError('USE reservations')
    # CPU output only for user objects; avoids inventing kernel-generated payload.
    out=zero_user_backings(p,backings)
    for n,v in p['bindings'].items():
        if v['bo'] not in out or v['alias'] or n=='relocation_wire':continue
        data=m['objects'][n]['bytes']
        if data is None:raise ValueError('UNKNOWN required CPU image')
        out[v['bo']][v['offset']:v['offset']+len(data)]=bytes(x if x is not None else 0 for x in data)
    for r in p['wire_relocations']:
        addr=addresses[r['target_bo']]+r['pre_add']
        if addr%r['target_alignment']:raise ValueError('resolved alignment')
        old=struct.unpack_from('<I',out[r['owner_bo']],r['where']*4)[0]
        bg=r['background'];value=addr
        if r['op'] in (4,5):
            base=addr&~0x7ffff
            if addr+r['arg0']>=base+0x80000:raise ValueError('USE range exceeds base window')
            value=use_registers[r['arg1']] if r['op']==5 else addr-base
            if r['op']==4:bg=old # required: preserve earlier USE pieces
        word=merge(value,r['shift']>>16,r['shift']&0xffff,r['mask'],bg)
        struct.pack_into('<I',out[r['owner_bo']],r['where']*4,word)
    b=p['bindings']['relocation_wire'];raw=relocation_bytes(p)
    out[b['bo']][b['offset']:b['offset']+len(raw)]=raw
    return out

def closure(m,p):
    check(m,p)
    return {'complete':False,'cpu_bo_plan':'DEFINED','wire_relocations':len(p['wire_relocations']),
            'B3':'CONDITIONAL','remaining_rule':p['publication']['missing_rule'],
            'L12':m['l12'],'hardware_ready':False}

def write_tables(p):
    tables={'frozen-triangle-bos.csv':[dict(bo=n,**v) for n,v in p['bos'].items()],
            'frozen-triangle-bindings.csv':[dict(object=n,**v) for n,v in p['bindings'].items()],
            'frozen-triangle-validation.csv':p['validation'],
            'frozen-triangle-wire-relocations.csv':p['wire_relocations']}
    for name,rows in tables.items():
        with (DOC/name).open('w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=list(rows[0]),lineterminator='\n');w.writeheader();w.writerows(rows)

def main():
    import frozen_triangle_image as image
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--json',action='store_true');ap.add_argument('--write-tables',action='store_true');ap.add_argument('--complete',action='store_true');args=ap.parse_args()
    m=image.build();p=build(m);check(m,p)
    if args.complete:ap.exit(1,'PARTIAL: B1 B3-publication B4 L12; hardware-ready NO\n')
    if args.write_tables:write_tables(p)
    if args.json:print(json.dumps(dict(plan=p,validation_words=validation_words(p)),indent=2))
    else:print(json.dumps(closure(m,p),indent=2))
if __name__=='__main__':main()
