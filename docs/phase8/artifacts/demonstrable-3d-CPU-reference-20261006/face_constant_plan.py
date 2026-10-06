"""Symbolic CPU plan for future per-draw constant colors, not executable state.

Uses the preserved constant constructor and historical state-cache serializer.
Actual per-draw fragment rebind remains unproven until a separately authorized run.
No GPU addresses, module changes or rendering invocation are produced here.
"""
from pathlib import Path
import struct
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools/psb-dri-re'))
import frozen_triangle_image as historical

def constant(argb):
    if type(argb) is not int or not 0<=argb<=0xffffffff:raise ValueError('packed ARGB')
    return (argb&0x001fffff,0xfca40001|(((argb>>21)&31)<<4)|((argb>>26)<<12))

def rebind(previous_symbol,next_symbol):
    if previous_symbol not in ('magenta','green','yellow') or next_symbol not in ('magenta','green','yellow'):
        raise ValueError('explicit palette symbols required')
    previous=historical.triangle_state();current=historical.triangle_state()
    previous[9]='R(primary_pds_'+previous_symbol+')|0x0c000000'
    current[9]='R(primary_pds_'+next_symbol+')|0x0c000000'
    mask=historical.diff_state(current,previous)
    if not mask:return {'mask':0,'emission':'NONE'}
    words=historical.pack_state(current,mask)
    assert mask==0x40 and len(words)==4
    return {'mask':mask,'words':words,'copy_dwords':len(words),
            'state_USE_words':historical.state_use(len(words)),
            'new_contract':'per-draw fragment primary-PDS selection; not established on hardware'}

def plan():
    palette={n:{'ARGB':f'0x{c:08x}','USE_words':[f'0x{x:08x}' for x in constant(c)],
                'raw_LE_hex':struct.pack('<2I',*constant(c)).hex(),'alignment':32,
                'GPU_output':'ESTABLISHED for magenta only' if n=='magenta' else 'NOT YET OBSERVED'}
             for n,c in [('magenta',0xffff00ff),('green',0xff00ff00),('yellow',0xffffff00)]}
    return {'scope':'symbolic CPU construction plan; NO candidate or executable stream',
            'preferred_face_distinction':'per-face constants before interpolation/lighting',
            'palette':palette,'first_probe_after_square':'TWO_COLOR_TWO_DRAW_QUAD',
            'geometry_unchanged_from_square':True,'indices_by_draw':[[0,1,2],[1,3,2]],
            'constant_programs':'distinct immutable USE objects; clone known primary PDS template with corresponding USE relocations',
            'state_switch':rebind('magenta','green'),
            'target_unchanged':{'width':32,'height':32,'stride':128,'format':'ARGB8888'},
            'expected_oracle':{'upper_magenta':120,'lower_green':136,'zero':768,
                               'upper_predicate':'8<=x,y<=23 and x+y<=30','lower_predicate':'8<=x,y<=23 and x+y>=31',
                               'condition':'only after square live fill result matches its oracle'},
            'unchanged':['geometry','vertex PDS/USE','ISP tests','PBE','surface','UAPI/client','source/capsule/lifecycle guards'],
            'required_before_candidate':['square evidence preserved','exact independent object placement/relocations and TA stream built/bounds-checked',
                                          'compiled relocation whitelist updated without weakening validation','finished candidate identities and focused qualification'],
            'claims':{'per_draw_binding_ESTABLISHED':False,'interpolation_ESTABLISHED':False,'lighting_ESTABLISHED':False},
            'provenance':['tools/psb-dri-re/experimental_fragment_constant.h: historical0x39ac4→0x30ffd→0x30d15→0x30fb7',
                          'tools/psb-dri-re/frozen_triangle_image.py: triangle_state,diff_state,pack_state,state_use,ref',
                          'docs/phase8/FIRE3-TRIANGLE-REPRODUCTION.json: magenta output only confirmed']}
