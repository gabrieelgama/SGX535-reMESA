#!/usr/bin/env python3
"""CPU-only experimental scene derived from the frozen FIRE3 construction.

No device access. Does not modify frozen serializers or known-good artifacts.
The experiment tests one indexed foreground draw of two triangles, not 3D yet.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import struct
import sys

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO/'tools/psb-dri-re'))
import frozen_triangle_image as image
import frozen_triangle_bo as bo

POSITIONS = ((8.,8.,.5,1.), (24.,8.,.5,1.),
             (8.,24.,.5,1.), (24.,24.,.5,1.))
INDICES = (0,1,2,1,3,2)
FIELDS = ('op','where','buffer','mask','shift','pre_add','background','dst_buffer','arg0','arg1')


def scene():
    m = copy.deepcopy(image.build())
    vertices = b''.join(struct.pack('<8f', *v, 1.,1.,1.,1.) for v in POSITIONS)
    m['objects']['vertices'].update(bytes=list(vertices), size=len(vertices))
    indices = struct.pack('<6H', *INDICES)
    m['objects']['indices'].update(bytes=list(indices), size=len(indices))
    index = m['objects']['triangle_index']
    # Same historical0x3b890 indexed TRIANGLES packet/count field; count6 also
    # appears in the independently emitted bounds draw, not an invented opcode.
    assert index['fields'][0]['value'] == 0x81400003
    index['fields'][0]['value'] = 0x81400006
    index['bytes'][:4] = struct.pack('<I', 0x81400006)
    # The combined TA stream owns the actual emitted bytes; the subobject is a
    # view. Derive its offset from the original view, not a second upload.
    import re
    offset = int(re.search(r'VIEW ta_stream\+(0x[0-9a-f]+)', index['notes'])[1], 16)
    stream = m['objects']['ta_stream']
    assert stream['fields'][offset//4]['value'] == 0x81400003
    stream['fields'][offset//4]['value'] = 0x81400006
    stream['bytes'][offset:offset+4] = struct.pack('<I', 0x81400006)
    m['experiment'] = 'TWO_TRIANGLE_FOREGROUND_QUAD'
    m['scope'] = 'CPU construction only; not GPU qualification or authorization'
    return m


def backings(m, p):
    result = bo.zero_user_backings(p)
    for name, binding in p['bindings'].items():
        if binding['bo'] not in result or binding['alias'] or name == 'relocation_wire':
            continue
        raw = m['objects'][name]['bytes']
        if raw is None or len(raw) != binding['size']:
            raise ValueError('missing constructed object')
        start = binding['offset']
        result[binding['bo']][start:start+len(raw)] = bytes(v if v is not None else 0 for v in raw)
    return result


def expected_image():
    # Predeclared pixel-center/top-left rectangle oracle. FIRE3's measured
    # triangle is the strict upper half; the second triangle should fill the
    # complementary half/diagonal. Exact device fill behavior is a live premise.
    return b''.join((0xffff00ff if 8 <= x < 24 and 8 <= y < 24 else 0).to_bytes(4,'little')
                    for y in range(32) for x in range(32))


def inspect_readback(raw):
    """CPU pixel interpretation only; caller must separately prove provenance."""
    if len(raw) != 4096:
        raise ValueError('complete 4096-byte readback required')
    words = struct.unpack('<1024I', raw)
    coords = [(i % 32, i // 32) for i,v in enumerate(words) if v]
    bbox = ([min(x for x,y in coords), min(y for x,y in coords),
             max(x for x,y in coords), max(y for x,y in coords)] if coords else None)
    old = b''.join((0xffff00ff if 8 <= y <= 22 and 8 <= x <= 30-y else 0).to_bytes(4,'little')
                   for y in range(32) for x in range(32))
    classification = ('EXACT_CPU_QUAD_ORACLE' if raw == expected_image() else
                      'BASELINE_TRIANGLE_ONLY' if raw == old else
                      'ALL_ZERO' if not coords else 'UNEXPECTED_COVERAGE')
    return {'classification':classification,'nonzero_pixels':len(coords),
            'magenta_pixels':words.count(0xffff00ff),'bbox_inclusive':bbox,
            'unique_values':[f'0x{v:08x}' for v in sorted(set(words))],
            'nonzero_coordinates':coords,'provenance':'NOT ESTABLISHED BY THIS CPU ANALYSIS'}


def generate(output):
    if output.exists():
        raise ValueError('exclusive new output required')
    output.mkdir(parents=True)
    m = scene(); p = bo.build(m); bo.check(m,p)
    base = image.build(); old = bo.build(base)
    assert p['bindings']['vertices']['size'] == 128
    assert p['bindings']['indices']['offset'] == old['bindings']['indices']['offset'] == 512
    assert p['bindings']['indices']['reserved'] == 16384
    assert p['bindings']['ta_stream']['offset'] == 128
    assert p['bindings']['ta_stream']['size'] == old['bindings']['ta_stream']['size'] == 68
    assert len(p['wire_relocations']) == 49
    data = backings(m,p)
    lines = ['/* Isolated TWO_TRIANGLE_FOREGROUND_QUAD CPU seed image. No execution authority. */']
    for role,name in enumerate(list(p['bos'])[:6]):
        raw = data[name]
        for offset in range(0,len(raw),4):
            value = struct.unpack_from('<I',raw,offset)[0]
            if value: lines.append('    {%d, 0x%05x, 0x%08x},' % (role,offset,value))
    (output/'frozen_kernel_initial.inc').write_text('\n'.join(lines)+'\n')
    text = '/* Isolated quad relocation whitelist: same49 records, shifted TA stream only. */\n'
    text += ''.join('    {'+', '.join(hex(r[k])+'U' for k in FIELDS)+'},\n' for r in p['wire_relocations'])
    (output/'frozen_kernel_relocations.inc').write_text(text)
    # Actual initializer retains the same FIRE3 constant-fragment compile option.
    # Apply it only to the CPU expected bytes; generated seed remains pre-overlay.
    data['use'][:8] = bytes.fromhex('ff001f00f1f1a7fc')
    for name,raw in data.items(): (output/(name+'.expected.bin')).write_bytes(raw)
    (output/'expected-color.bin').write_bytes(expected_image())
    (output/'scene.json').write_text(json.dumps(m,indent=2)+'\n')
    (output/'address-plan.json').write_text(json.dumps(p,indent=2)+'\n')
    changed_relocs = [{'index':i,'before':a,'after':b} for i,(a,b) in enumerate(zip(old['wire_relocations'],p['wire_relocations'])) if a != b]
    assert len(changed_relocs) == 8
    delta = {'experiment':'TWO_TRIANGLE_FOREGROUND_QUAD','GPU_result':'NOT YET OBSERVED',
             'sgx_execution_authorized':False,'maximum_future_invocations':1,
             'geometry':{'before_vertices':3,'after_vertices':4,'positions':POSITIONS,
                         'indices_before':[0,1,2],'indices_after':INDICES},
             'packet_count':{'before':'0x81400003','after':'0x81400006'},
             'layout':{'TA_stream_before':96,'TA_stream_after':128,'TA_bytes':68,
                       'vertex_stride':32,'indices_offset':512,'indices_capacity':16384},
             'relocation_changes':changed_relocs,
             'shader_words_unchanged':['001f00ff','fca7f1f1'],
             'source_overlay_files':['frozen_kernel_initial.inc','frozen_kernel_relocations.inc'],
             'unchanged':['fragment/PDS/USE programs','triangle ISP state','TA/raster register values except stream address',
                          'PBE/surface/target','BO sizes/domains','UAPI/client','observer','lifecycle/source/capsule guards'],
             'expected_pixels':{'magenta':256,'zero':768,'bbox_inclusive':[8,8,23,23],
                                'scope':'predeclared CPU oracle, not an observed SGX fill contract'},
             'files':{p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
                      for p in output.iterdir() if p.is_file()}}
    (output/'delta.json').write_text(json.dumps(delta,indent=2)+'\n')
    return delta


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    print(json.dumps(generate(args.output),indent=2))
