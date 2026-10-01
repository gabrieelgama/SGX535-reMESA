#!/usr/bin/env python3
"""Partial, symbolic CPU serialization. No GPU API, allocator, or submission.

Evidence and conditions: docs/phase7/psb-dri-re/frozen-triangle-spec.md.
--complete checks the frozen architectural/static proof obligations, not
operator risk acceptance, module ABI compatibility, or deployment readiness.
Current execution status: docs/phase8/post-attempt03-checkpoint-audit.md.
None is an unresolved byte, never a zero-valued address. Strings in word lists
name relocation expressions or explicitly UNKNOWN values. No executable output.
"""
import argparse
import csv
import json
from pathlib import Path
import struct
from cleanroom_pds_image import build_image, HOLES

DOC = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re'
DRI = '74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8'
GROUPS = [(1,1,1),(2,2,1),(4,3,1),(8,4,1),(16,5,1),(32,6,1),
          (64,7,3),(128,19,2),(256,21,6),(512,27,1),(1024,28,1),
          (2048,29,1),(4096,30,1),(8192,31,1),(16384,32,1),(32768,33,1)]
BLOCKERS = {
 'L12': ('static-contract','Selected primary two-word pre-definition source domain','P7H-041; external dependency frozen for this pass'),
 'FT-AUX': ('static-contract','Complete consumed-state coverage of vertex/state-upload/event/background programs beyond their deterministic CPU images','0x40355;0x287ff;0x392ab;0x39c5c; primary L12 does not cover these programs'),
 'FT-BO': ('static-submission','Device publication must preserve initialized and relocated BO contents through selected cache/TLB completion before first consumption','P7H-047; concrete BO/wire model exists; psb_invalidate_caches is a no-op; wmb is not SGX cache completion'),
 'FT-SERVICE': ('static-submission','Target-qualified ready-state postcondition for revision-conditioned service initialization and TA-table setup','P7H-050; branch/cookie CPU model does not prove applicability or awaited hardware table initialization'),
 'BACKEND': ('execution-contract','Concrete CPU-to-GPU content-preserving provider and synchronization','Route C CONDITIONAL'),
 'GATE-B': ('hardware-gate','Existing ownership/recovery/revision and safety requirements','BLOCKED; whitelist []; hardware UNVERIFIED'),
}

def bounds_state():
    a=[0]*34
    a[0]=0x54c5; a[1]=0x07f00100; a[3]=0x02000000
    a[28]=0x04001000; a[30]=0x10000
    return a

def triangle_state():
    a=[0]*34
    a[0]=0x5fc1; a[1]=0x01d00000
    a[7:10]=['R(secondary_pds)',0x30000,'R(primary_pds)|0x0c000000']
    a[28]=0x08001800; a[29]=0x358637bd; a[30]=0x10000
    return a

def pack_state(a, mask=None):
    """CPU packing only; a[7:10] are already serialized launch words."""
    mask=a[0] if mask is None else mask
    out=[mask]
    for bit,start,count in GROUPS:
        if mask & bit: out += a[start:start+count]
    return out

def diff_state(a, old):
    mask=a[0]
    for bits,start,count in [(7,1,3),(56,4,3),(64,7,3),(128,19,2),
                             (256,21,6),(512,27,1),(1024,28,1),
                             (2048,29,1),(4096,30,1),(16384,32,1)]:
        if (a[0] & bits)==(a[0] & old[0] & bits) and a[start:start+count]==old[start:start+count]:
            mask &= ~bits
    return mask

def state_use(n):
    """Scoped 0x287ff path, one copy slot, 1..16 dwords; CPU formulas."""
    if not 1 <= n <= 16: raise ValueError('only one-copy-slot evidence modeled')
    return [0xa0000000,0x28a10001|((n-1)<<12),0xa0200000|(n<<7),0xfb274000]

def build():
    m={'schema':1,'scope':'PARTIAL CPU object specification; not executable',
       'hardware_ready':False,'fg01':'CLOSED','fg02':'OPEN','l12':'OPEN',
       'objects':{},'relocations':[],'blockers':dict(BLOCKERS)}
    def obj(name, words=None, *, raw=None, size=None, align=4, producer='',
            deps=(), status='READY', blockers=(), evidence='P7H-042', notes=''):
        if raw is not None: data=list(raw); fields=[]
        elif words is not None:
            data=[]; fields=[]
            for i,w in enumerate(words):
                historical_unwritten = w == 'UNKNOWN: producer-unwritten'
                primary_hole = name == 'primary_pds' and i*4 in HOLES
                origin = 'CPU_PRODUCER' if isinstance(w,int) else 'SYMBOLIC_INPUT'
                if historical_unwritten or primary_hole:
                    # Route C CPU policy only. Hardware launch coverage stays open.
                    w=0
                    origin='ROUTE_C_CPU_ZERO_POLICY'
                data.extend(struct.pack('<I',w) if isinstance(w,int) else [None]*4)
                fields.append({'offset':i*4,'value':w,'origin':origin,
                               'historical_producer_unwritten':historical_unwritten or primary_hole})
        else: data=None; fields=[]
        m['objects'][name]={'size':len(data) if data is not None else size,'alignment':align,
          'bytes':data,'fields':fields,'producer':producer,'dependencies':list(deps),
          'status':status,'blockers':list(blockers),'evidence':evidence,'notes':notes,
          'placement':None}
    def reloc(owner,off,target,formula,mask=0xffffffff,right=0,left=0,bg=0,kind='OFFSET',align=4):
        rid=f'{owner}+{off:02x}'
        m['relocations'].append({'id':rid,'owner':owner,'offset':off,'target':target,
          'formula':formula,'mask':f'0x{mask:08x}','right_shift':right,'left_shift':left,
          'background':f'0x{bg:08x}','kind':kind,'target_alignment':align,
          'evidence':'P7G-011;P7H-042;P7H-043'})
    def use(owner,off,target,bg,dm):
        reloc(owner,off,target,f'USE_BASE(dm={dm}) mask0xf; USE_OFFSET>>15<<4 mask0xf0; USE_OFFSET>>4<<8 mask0x7ff00; background={bg:#x}',bg=bg,kind='USE_THREE_RECORDS',align=32)
    def ref(owner,off,target,bg=0,mask=0xffffff):
        reloc(owner,off,target,f'((GPU({target})>>4)&{mask:#x})|{bg:#x}',mask,right=4,bg=bg,align=16)
    pos=[(8.,8.,.5,1.),(24.,8.,.5,1.),(8.,24.,.5,1.)]
    obj('vertices',raw=b''.join(struct.pack('<8f',*p,1.,1.,1.,1.) for p in pos),producer='chosen post-viewport inputs;0x2daad;0x3f610',notes='IMPLEMENTATION_POLICY input; Mesa mask2 remains independent of compiled attribute count0',evidence='P7G-006;P7H-042')
    obj('indices',raw=struct.pack('<3H',0,1,2),producer='0x27e08',notes='base vertex0; six used bytes in larger historical capacity',evidence='P7G-009')
    obj('fragment_use',[0,0xf8040140],align=32,producer='0x370d8;0x264b7',evidence='P7H-012')
    primary=list(struct.unpack('<14I',build_image(0).data)); primary[0]='USE(fragment_use)'
    obj('primary_pds',primary,align=32,producer='cleanroom_pds_image.py;0x25970',deps=['fragment_use'],blockers=['L12'],notes='all non-relocation bytes defined; zero holes are Route C CPU policy',evidence='P7H-036')
    use('primary_pds',0,'fragment_use',0,1)
    obj('unused_secondary_helper_use',[0,0xf8040140],align=32,producer='0x28559->0x26434',evidence='P7H-044',notes='allocated before secondary fast path; no selected GPU reference; retained for allocation completeness')
    obj('secondary_pds',[0xaf000000],align=32,producer='0x3faa3',evidence='P7H-013',notes='READY CPU word only; no architectural no-read claim')
    for prefix,width,target in [('vertex',8,'vertices'),('bounds_vertex',4,'bounds_vertices')]:
        obj(prefix+'_use',[0xa0000000,0x28a10001|((width-1)<<12),0xa0200000,0xfb275000],align=32,producer='0x40355;0x30e22;0x30a09',evidence='P7F-003;P7G-007')
        w=['UNKNOWN: producer-unwritten']*16
        for i,v in {0:'GPU('+target+')',1:0x80000000|((width-1)<<21)|(width-1),4:'USE('+prefix+'_use)',5:0,8:width*4,9:0,12:0x67800070,13:0x2f030343,14:0x030803e5,15:0xaf000000}.items(): w[i]=v
        obj(prefix+'_pds',w,align=32,producer='0x40355',deps=[target,prefix+'_use'],status='CONDITIONAL',blockers=['FT-AUX'],evidence='P7G-007')
        reloc(prefix+'_pds',0,target,'GPU('+target+')'); use(prefix+'_pds',16,prefix+'_use',0x200000,0)
        obj(prefix+'_secondary',[0xaf000000],align=32,producer='0x40355->0x3faa3')
    bv=[(0,0),(32,0),(0,32),(32,32)]*2
    obj('bounds_vertices',raw=b''.join(struct.pack('<4f',x,y,1,1) for x,y in bv),producer='0x29fbd',notes='full 128-byte allocation; second quad not indexed with scissor disabled',evidence='P7G-001;P7H-043')
    obj('bounds_indices',raw=struct.pack('<6H',0,1,2,3,2,1),producer='0x29fbd;0x37409',evidence='P7F-002')
    # Source-state uploads, including the unchanged-field elision before triangle.
    tri=triangle_state(); mask=diff_state(tri,bounds_state())
    for name,words in [('bounds_state',pack_state(bounds_state())),('triangle_state',pack_state(tri,mask)),('terminate_state',[0x2000,0x20002])]:
        obj(name,words,producer='0x287ff;0x28fff;0x29f38;0x2a57b',notes='selected initial cache empty; full bounds; no query; software vertex branch',evidence='P7H-043')
        n=len(words)
        obj(name+'_use',state_use(n),align=32,producer='0x287ff;0x30e22;0x30a09;0x30fb7',evidence='P7H-043')
        w=['UNKNOWN: producer-unwritten']*15
        for i,v in {0:'GPU('+name+')',1:0x80000000|((n-1)<<21)|(n-1),2:'USE('+name+'_use)',3:0,10:0,12:0x07030223,13:0x07042345,14:0xaf000000}.items(): w[i]=v
        obj(name+'_pds',w,align=32,producer='0x287ff;0x381a0',deps=[name,name+'_use'],status='CONDITIONAL',blockers=['FT-AUX'],evidence='P7H-043')
        reloc(name+'_pds',0,name,'GPU('+name+')');use(name+'_pds',8,name+'_use',0x200000,0)
        flags=0x60000000 if name=='terminate_state' else 0x40000000
        obj(name+'_ta',['R('+name+'_pds)',0x0c02a200|((n*4+15)>>4)],producer='0x3b73b',deps=[name+'_pds'],evidence='P7H-043')
        ref(name+'_ta',0,name+'_pds',flags,0xfffffff)
    ref('triangle_state',8,'secondary_pds');ref('triangle_state',16,'primary_pds',0x0c000000)
    for name,index,pds,count,width in [('triangle_index','indices','vertex_pds',3,8),('bounds_index','bounds_indices','bounds_vertex_pds',6,4)]:
        obj(name,[0x81400000|count,'GPU('+index+')',0,'R('+pds+')',3|((width+3)//4<<25)|(((width*4+15)&0x1ff0)<<4)],producer='0x3b890',deps=[index,pds],evidence='P7G-009;P7H-043')
        reloc(name,4,index,'GPU('+index+')');ref(name,12,pds,mask=0xfffffff)
    obj('ta_termination',[0xc0000000],producer='0x2a39a',evidence='P7G-002')
    # Target policy: cpp4, pitch32 pixels, width=height32, surface state0 (no pending clear).
    obj('color',raw=bytes(4096),align=4,producer='clean-room target policy',status='CONDITIONAL',blockers=['FT-TARGET','BACKEND'],notes='IMPLEMENTATION_POLICY: zero-initialized 4096-byte CPU payload; format/channel and GPU preservation still conditional')
    obj('target_data',[0x000f8000,'GPU(color)',0,0x0001f01f],producer='0x392ab;0x4b710',deps=['color'],status='CONDITIONAL',blockers=['FT-TARGET'],evidence='P7H-042')
    reloc('target_data',4,'color','GPU(color) with relocation placement 0x16000000/mask0xff000000; address mask0xfffffffc; right2 left2',0xfffffffc,2,2)
    obj('event_use',[0x84208180,0xfb200004,0x81200080,0xfb240044],align=32,producer='0x391e0; cpp4 passes code0 at0x39308',evidence='P7H-042')
    obj('event_helper_use',[0,0xf8348000],align=32,producer='0x30422;0x2fabd;0x30fb7',evidence='P7H-042')
    w=['UNKNOWN: producer-unwritten']*37
    vals={0:'USE(event_helper_use)',1:0,2:'GPU(target_data)',3:0x80000003,4:'USE(event_use)',5:0,8:8,9:0,10:2,11:0x10000000,12:0x7f800,13:0x07f00000,14:0x08000000,
          16:0xcf820030,17:0x87600000,18:0x90000005,19:0x070003e5,20:0xaf000000,21:0xcf820830,22:0x87600000,23:0x90000014,24:0x07042363,25:0x170086e0,26:0xf7800b31,27:0xcf621031,28:0xc762c070,29:0xf7800b32,30:0xcf641432,31:0xc764c070,32:0xf7811933,33:0xcf661833,34:0xc766c070,35:0x070b0345,36:0xaf000000}
    for i,v in vals.items():w[i]=v
    obj('event_pds',w,align=32,producer='0x392ab',deps=['target_data','event_use','event_helper_use'],status='CONDITIONAL',blockers=['FT-AUX'],evidence='P7H-042')
    use('event_pds',0,'event_helper_use',0,1);reloc('event_pds',8,'target_data','GPU(target_data)');use('event_pds',16,'event_use',0x200000,1)
    obj('background_use',[0xa0000000,0x28851001],align=32,producer='0x39c5c;0x30e22;0x30fb7',evidence='P7H-042',notes='repeat field initially1; final bit added; see disassembly check')
    w=['UNKNOWN: producer-unwritten']*16
    for i,v in {0:'USE(background_use)',1:0,2:0x001e0092,3:0x6c01f01f,8:0x20,9:0xf800,10:'GPU(color)',12:0x07000345,13:0x070418a2,14:0x07042364,15:0xaf000000}.items():w[i]=v
    obj('background_pds',w,align=32,producer='0x39c5c',deps=['color','background_use'],status='CONDITIONAL',blockers=['FT-AUX','FT-TARGET'],evidence='P7H-042')
    use('background_pds',0,'background_use',0x100000,1);reloc('background_pds',40,'color','GPU(color); placement0x16000000/mask0xff000000')
    obj('background_secondary',[0xaf000000],align=32,producer='0x39c5c->0x3faa3')
    obj('background_object',['R(background_secondary)',0x30001,'R(background_pds)|0x0c000000',0,0,0x3f800000,0,0,0x3f800000,2,0,0x40004000,0x3f800000,0x42004000,0x3f800000,0x40004200,0x3f800000],align=16,producer='0x2f317->0x2ee34',deps=['background_pds','background_secondary'],evidence='P7H-042')
    ref('background_object',0,'background_secondary');ref('background_object',8,'background_pds',0x0c000000)
    rw=[0x400,0xffff80,0x404,0,0x40c,0,0x410,0x20002,0x414,0x100,0x418,1,0x41c,0x0da24260,0x420,0,0x424,0xfbf4,0x42c,0,0x480,0xf0000,0xcb0,0,0x484,0,0x488,0,0x48c,0,0x490,0,0x494,0xff,0x4c4,'R(background_object)|0x02000000',0x4bc,0x200,0x4b8,0x3f800000,0x4c8,0x88,0x4dc,0,0x800,0,0xa5c,'GPU(event_pds) aligned16',0xa60,4,0xa64,0x4fff]
    obj('raster_registers',rw,producer='0x2ab70;0x2a57b',deps=['background_object','event_pds'],status='CONDITIONAL',blockers=['FT-TARGET'],evidence='P7H-042',notes='no depth attachment; clear depth1 and stencil0 explicitly chosen; candidate Mesa defaults corroborate; not a GL clear call')
    ref('raster_registers',140,'background_object',0x2000000);reloc('raster_registers',188,'event_pds','(GPU(event_pds)>>4)<<4',right=4,left=4,align=16)
    sequence=['bounds_state_ta','bounds_index','triangle_state_ta','triangle_index','terminate_state_ta','ta_termination']
    stream=[]
    for n in sequence:stream.extend(f['value'] for f in m['objects'][n]['fields'])
    obj('ta_stream',stream,producer='0x29251;0x29fbd;0x27fa0;0x2a39a',deps=sequence,evidence='P7H-043',notes='68-byte scoped normal path; subobjects above are views, not extra uploads')
    base=0
    for n in sequence:
        for r in list(m['relocations']):
            if r['owner']==n:
                q=dict(r);q.update(id=f'ta_stream+{base+r["offset"]:02x}',owner='ta_stream',offset=base+r['offset'])
                m['relocations'].append(q)
        m['objects'][n]['notes'] += f'; VIEW ta_stream+{base:#x}; not an extra allocation'
        base+=m['objects'][n]['size']
    obj('ta_registers',[0x204,0,0x218,0x400000,0x23c,0x82,0x240,0x358637bd,0x244,0x358637bd,0x250,0x88,0x238,'GPU(ta_stream)'],producer='0x2b258',deps=['ta_stream'],evidence='P7G-002')
    reloc('ta_registers',52,'ta_stream','GPU(ta_stream)')
    obj('scene_cookie',[0,0x10,0x01004004,0x01004004,0x10,0x1001,0x1f01f,0x1000,0,0x1300,0x1350,0x13e0,0,0,0,'UNKNOWN: word15 not consumed by selected initial handler'],producer='Xpsb0x3a40',status='CONDITIONAL',blockers=['FT-SERVICE'],evidence='P7H-008')
    for n,size,producer in [('scene_hw',0x1420,'candidate psb_scene.c:143 scene-info allocation'),('ta_page_table',None,'candidate psb_scene.c:460; pages*PAGE_SIZE'),('ta_parameter',None,'candidate psb_scene.c:476; pages*PAGE_SIZE; alignment256 pages'),('validation_nodes',None,'0x443d1;0x37b51'),('relocation_wire',None,'0x37854;0x3a82c'),('submit_command',144,'0x37b51'),('fence_reply',48,'candidate DRM fence ABI'),('xhw_service',None,'candidate psb_xhw;Xpsb handlers')]:
        obj(n,size=size,producer=producer,status='BLOCKED_BY_OTHER',blockers=['FT-SERVICE' if n in ('scene_hw','ta_page_table','ta_parameter','xhw_service') else 'FT-BO'],notes='required object/interface; unknown contents are not an empty allocation',evidence='P7G-011;P7H-008;P7H-029')
    # 32-bit userspace pointers/handles remain parameters, not GPU relocations.
    cmd=[0]*36
    for off,val in {0:'PARAM: CPU(validation_nodes)',16:'PARAM: CPU(scene_arg)',24:'PARAM: CPU(fence_reply)',32:3,
                    36:'PARAM: HANDLE(ta_registers BO)',40:'PARAM: OFFSET(ta_registers)',44:14,
                    48:'PARAM: HANDLE(raster_registers BO)',52:'PARAM: OFFSET(raster_registers)',56:0,
                    60:'PARAM: HANDLE(raster_registers BO)',64:'PARAM: OFFSET(raster_registers)',68:52,
                    72:'PARAM: HANDLE(relocation_wire)',76:'PARAM: OFFSET(relocation_wire)',80:'PARAM: complete relocation record count',
                    88:0,92:3}.items():cmd[off//4]=val
    obj('submit_command',cmd,producer='0x37b51; normal TA; no feedback; fence flags0',deps=['ta_registers','raster_registers','relocation_wire','validation_nodes','scene_arg','fence_reply'],status='CONDITIONAL',blockers=['FT-BO','FT-SERVICE'],evidence='P7G-011;P7H-044',notes='OOM count0 does not imply zero handle/offset: routine aliases raster descriptor; CPU pointer high halves0')
    obj('scene_arg',[0,0,32,32,4],producer='0x276a5; new zeroed owner; candidate scene-pool handle request',status='BLOCKED_BY_OTHER',blockers=['FT-BO'],evidence='P7G-011;P7H-044')
    for r in m['relocations']:
        deps=m['objects'][r['owner']]['dependencies']
        if r['target'] not in deps:deps.append(r['target'])
    m['known_relocation_records']=sum(3 if r['kind']=='USE_THREE_RECORDS' else 1 for r in m['relocations'] if 'VIEW ta_stream' not in m['objects'][r['owner']]['notes'])
    # Reference-family request producers, not an executable MMIO/service implementation.
    # Fresh scene; final-pass clears CLEARED before scheduling; no OOM or eviction.
    m['service_requests']=[]
    for engine,flags,reads in [(0,4,[2,3,4,5,6,7,8]),(1,15,[7,14])]:
        m['service_requests'].append({
          'stage':'TA' if engine==0 else 'RASTER', 'size':132, 'op':2,
          'fire_flags':1,'hw_context':'PARAM: reserved hardware scene context',
          'offset':'GPU(scene_hw)','engine':engine,'scene_flags':flags,
          'num_oom_cmds':0,'issue_irq':0,'irq_op':0,'copy_back':False,
          'cookie_reads':reads,'cookie15_consumed':False,
          'rca':'OUTPUT: 2 on selected raster path' if engine else 'UNUSED',
          'prerequisites':['initialized revision-compatible service context',
                           'scene_info32 cookie words0..14',
                           'TA memory load completed',
                           'register-list submitted before bind/fire',
                           'TA completed; same hardware scene retained' if engine else 'fresh scene and final_pass'],
          'evidence':'P7H-046; psb_schedule.c; psb_xhw.c; Xpsb0x4550/0x4030',
          'status':'CPU_FIELDS_CONFIRMED; execution CONDITIONAL'})
    m['closed_items']={'FT-ORDER':'P7H-043; scoped initial no-query/no-scissor state path has six TA fragments /68 bytes'}
    # P7H-048 closes the scoped shared-surface layout contract, not execution.
    for o in m['objects'].values():
        if 'FT-TARGET' in o['blockers']:
            o['blockers'].remove('FT-TARGET')
            if not o['blockers']:o['status']='READY'
    m['objects']['color']['notes']='IMPLEMENTATION_POLICY: zero-initialized 4096-byte linear ARGB8888 payload; P7H-048 shared surface contract; GPU publication conditional'
    m['closed_items']['FT-TARGET']='P7H-048; shared surface -> ARGB8888 linear span -> cpp4 target/background descriptor; selected 32x32 pitch32'
    m['blocker_status']={k:('CONDITIONAL' if k in ('FT-BO','BACKEND') else 'OPEN') for k in m['blockers']}
    return m

def word(m,name,off):
    return int.from_bytes(bytes(m['objects'][name]['bytes'][off:off+4]),'little')

def remaining(m,assume_l12_closed=False,static_only=False):
    return [k for k,v in m['blockers'].items() if not(assume_l12_closed and k=='L12') and not(static_only and v[0] in ('execution-contract','hardware-gate'))]

def validate(m,complete=False):
    expected=build()
    for key in ('schema','scope','hardware_ready','fg01','fg02','l12','closed_items','blocker_status'):
        if m.get(key)!=expected[key]: raise ValueError('unproved top-level status change: '+key)
    if m['objects'].keys()!=expected['objects'].keys():raise ValueError('required object inventory differs')
    if m['relocations']!=expected['relocations']:raise ValueError('missing/changed relocation formula')
    if m.get('service_requests')!=expected['service_requests']:raise ValueError('service request/provenance differs')
    if m['blockers']!=expected['blockers']:raise ValueError('evidence blockers cannot be silently removed')
    placed=[]
    for n,o in m['objects'].items():
        e=expected['objects'][n]
        for key in e.keys()-{'placement'}:
            if o.get(key)!=e[key]:raise ValueError(f'{n}: altered known layout/provenance {key}')
        for dep in o['dependencies']:
            if dep not in m['objects']:raise ValueError('missing dependency')
        p=o['placement']
        if p is not None:
            off=p['offset']; size=o['size']
            if size is None or off<0 or off%o['alignment']:raise ValueError('unknown size/misaligned placement')
            for arena,start,end in placed:
                if p['arena']==arena and off<end and start<off+size:raise ValueError('incompatible overlap')
            placed.append((p['arena'],off,off+size))
    for r in m['relocations']:
        o=m['objects'][r['owner']]
        if r['target'] not in m['objects'] or r['offset']%4 or r['offset']+4>o['size']:raise ValueError('bad relocation endpoint')
        if o['bytes'][r['offset']:r['offset']+4] != [None]*4:raise ValueError('relocation bytes must remain unresolved')
    if complete:raise ValueError('PARTIAL: '+', '.join(remaining(m,static_only=True)))
    return True

def write_tables(m):
    def table(name,cols,rows):
        with (DOC/name).open('w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=cols,lineterminator="\n");w.writeheader();w.writerows(rows)
    rows=[]
    for n,o in m['objects'].items():
        reloc_consumers=';'.join(r['id'] for r in m['relocations'] if r['target']==n)
        rows.append(dict(node_id=n,size=o['size'] if o['size'] is not None else 'UNKNOWN',alignment=o['alignment'],producer=o['producer'],consumer=';'.join(x for x,v in m['objects'].items() if n in v['dependencies']) or 'submission/service or relocation consumer',dependencies=';'.join(o['dependencies']),status=o['status'],blocker_id=';'.join(o['blockers']),evidence=o['evidence'],notes=o['notes'],cpu_initialization=('ALL_BYTES_DEFINED_EXCEPT_EXPLICIT_SYMBOLS' if o['bytes'] is not None and all(not isinstance(f['value'],str) or not f['value'].startswith('UNKNOWN') for f in o['fields']) else 'PARTIAL_OR_UNKNOWN'),relocation_consumers=reloc_consumers,gpu_access='see named producer/consumer; hardware source domain not inferred',lifetime='retain through completed use; concrete backend pending',first_use='named consumer; exact hardware timing not asserted'))
    table('frozen-triangle-objects.csv',list(rows[0]),rows)
    table('frozen-triangle-relocations.csv',list(m['relocations'][0]),m['relocations'])
    services=[{k:(';'.join(map(str,v)) if isinstance(v,list) else v) for k,v in r.items()} for r in m['service_requests']]
    table('frozen-triangle-service.csv',list(services[0]),services)
    rows=[dict(blocker_id=k,layer=v[0],missing_fact=v[1],narrow_evidence=v[2],if_L12_closed='REMOVED HYPOTHETICALLY' if k=='L12' else 'REMAINS',status=m['blocker_status'][k]) for k,v in BLOCKERS.items()]
    rows.append(dict(blocker_id='FT-ORDER',layer='static-serialization',missing_fact='Scoped TA order and cache-diff packing recovered',narrow_evidence='P7H-043; initial empty cache; full bounds; no queries; software vertex branch',if_L12_closed='ALREADY CLOSED WITH STATED INPUTS',status='CLOSED'))
    rows.append(dict(blocker_id='FT-TARGET',layer='static-contract',missing_fact='Selected linear ARGB8888 surface contract recovered',narrow_evidence='P7H-048',if_L12_closed='CLOSED',status='CLOSED'))
    table('frozen-triangle-blockers.csv',list(rows[0]),rows)

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--complete',action='store_true');ap.add_argument('--write-tables',action='store_true');ap.add_argument('--json',action='store_true');ap.add_argument('--assume-l12-closed',action='store_true');ap.add_argument('--bundle',action='store_true');args=ap.parse_args()
    m=build()
    try:validate(m,args.complete)
    except ValueError as e:ap.exit(1,str(e)+'\n')
    if args.write_tables:write_tables(m)
    if args.bundle:
        import frozen_triangle_bo as bo
        import frozen_triangle_contracts as contracts
        import frozen_triangle_closure as closure
        plan=bo.build(m);bo.check(m,plan)
        ledger=closure.build(m)
        print(json.dumps(dict(image=m,bo_plan=plan,validation_words=bo.validation_words(plan),target=contracts.target(),auxiliary=contracts.auxiliary(m),publication_branches={str(v):contracts.publication_trace(v) for v in (0xffff,0x100ffff)},closure=contracts.closure(m),closure_ledger=ledger,closure_decision=closure.check(m,ledger)),indent=2))
    elif args.json:print(json.dumps(m,indent=2))
    else:print(json.dumps({'objects':len(m['objects']),'relocation_sites':len(m['relocations']),'spec':'PARTIAL','hardware_ready':False,'hypothetical_l12_closed':args.assume_l12_closed,'remaining_static':remaining(m,args.assume_l12_closed,True)},indent=2))
if __name__=='__main__':main()
