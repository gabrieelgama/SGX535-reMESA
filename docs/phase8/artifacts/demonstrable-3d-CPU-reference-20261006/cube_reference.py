#!/usr/bin/env python3
"""CPU 3D oracle, NOT an SGX renderer or an experimental candidate.

Column vectors, right-handed coordinates, OpenGL-style clip cube. CPU performs
all transforms/clipping. Future flat-face SGX payload keeps the observed fourth
position field1; this does not establish GPU perspective/varying/depth behavior.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct

VERTICES=((-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),
          (-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1))
FACES=(('front',(4,5,6,7),(0,0,1),0xffff00ff),
       ('back',(1,0,3,2),(0,0,-1),0xff00ffff),
       ('right',(5,1,2,6),(1,0,0),0xffff0000),
       ('left',(0,4,7,3),(-1,0,0),0xffffff00),
       ('top',(7,6,2,3),(0,1,0),0xff00ff00),
       ('bottom',(0,1,5,4),(0,-1,0),0xff0000ff))

def finite(values):
    if not all(math.isfinite(v) for v in values):raise ValueError('nonfinite geometry')
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def transform(m,v):
    if len(m)!=4 or any(len(row)!=4 for row in m) or len(v)!=4:raise ValueError('matrix shape')
    finite(v);finite([v for row in m for v in row])
    result=tuple(dot(row,v) for row in m);finite(result);return result
def multiply(a,b):return tuple(tuple(sum(a[i][k]*b[k][j] for k in range(4)) for j in range(4)) for i in range(4))
def model(angle):
    finite((angle,));y=math.radians(30+angle);x=math.radians(20)
    cy,sy,cx,sx=math.cos(y),math.sin(y),math.cos(x),math.sin(x)
    ry=((cy,0,sy,0),(0,1,0,0),(-sy,0,cy,0),(0,0,0,1))
    rx=((1,0,0,0),(0,cx,-sx,0),(0,sx,cx,0),(0,0,0,1))
    return multiply(ry,rx)
VIEW=((1,0,0,0),(0,1,0,0),(0,0,1,-6),(0,0,0,1))
def projection(fov=45,aspect=1,near=1,far=12):
    finite((fov,aspect,near,far))
    if not 0<fov<180 or aspect<=0 or not 0<near<far:raise ValueError('projection domain')
    f=1/math.tan(math.radians(fov)/2)
    return ((f/aspect,0,0,0),(0,f,0,0),(0,0,-(far+near)/(far-near),-2*far*near/(far-near)),(0,0,-1,0))

PLANES=((1,0,0,1),(-1,0,0,1),(0,1,0,1),(0,-1,0,1),(0,0,1,1),(0,0,-1,1))
def clip_polygon(points):
    """Homogeneous convex clipping, before perspective division."""
    if len(points)<3:raise ValueError('polygon size')
    poly=[tuple(p) for p in points]
    for p in poly:
        if len(p)!=4:raise ValueError('clip point shape')
        finite(p)
    for plane in PLANES:
        if not poly:break
        result=[];prev=poly[-1];pd=dot(plane,prev)
        for cur in poly:
            cd=dot(plane,cur)
            if (pd>=0)!=(cd>=0):
                t=pd/(pd-cd)
                point=tuple(prev[k]+t*(cur[k]-prev[k]) for k in range(4))
                finite(point);result.append(point)
            if cd>=0:result.append(cur)
            prev,pd=cur,cd
        poly=result
    return poly

def viewport(p,width=32,height=32):
    finite(p)
    if len(p)!=4 or p[3]<=1e-12 or width<=0 or height<=0:raise ValueError('invalid division/viewport')
    x,y,z,w=p; ndc=(x/w,y/w,z/w)
    result=((ndc[0]+1)*width/2,(1-ndc[1])*height/2,(ndc[2]+1)/2,1.)
    finite(result);return result
def normal(m,n):return transform(m,(*n,0))[:3] # rotation-only model: inverse-transpose equals rotation
def intensity(n,light=(1,2,3)):
    finite((*n,*light));length=math.sqrt(dot(n,n));ll=math.sqrt(dot(light,light))
    if not length or not ll:raise ValueError('zero normal/light')
    return max(0.,min(1.,dot(n,light)/(length*ll)))

def frame(angle):
    m=model(angle);p=projection(); records=[]
    for i,v in enumerate(VERTICES):
        world=transform(m,(*v,1));view=transform(VIEW,world);clip=transform(p,view)
        records.append({'index':i,'object':list(v),'world':world,'view':view,'clip':clip,
                        'reciprocal_clip_w':1/clip[3],'screen':viewport(clip)})
    draws=[]
    for name,ids,n,color in FACES:
        wn=normal(m,n);center=tuple(sum(records[i]['world'][k] for i in ids)/4 for k in range(3))
        facing=dot(wn,(-center[0],-center[1],6-center[2]))
        if facing<=0:continue # CPU convex-object backface selection, not hardware culling proof
        clipped=clip_polygon([records[i]['clip'] for i in ids])
        if len(clipped)<3:continue
        screens=[viewport(c) for c in clipped]
        indices=[(0,i,i+1) for i in range(1,len(screens)-1)]
        draws.append({'face':name,'original_indices':ids,'world_normal':wn,
                      'mean_view_z':sum(records[i]['view'][2] for i in ids)/4,
                      'screen_vertices':screens,'triangle_indices':indices,'ARGB':f'0x{color:08x}',
                      'diffuse_intensity_reference_only':intensity(wn)})
    draws.sort(key=lambda d:(d['mean_view_z'],d['face'])) # more negative = further from camera
    return {'angle_degrees':angle,'model':m,'view':VIEW,'projection':p,'vertices':records,'draws':draws,
            'claims':{'CPU_transform':True,'CPU_projection':True,'CPU_clip':True,
                      'SGX_execution':False,'hardware_depth':False,'lighting_rendered':False}}

def pack_vertices(records):
    out=bytearray()
    for r in records:
        p=r['screen']
        if len(p)!=4 or p[3]!=1:raise ValueError('unqualified fourth field')
        finite(p)
        out.extend(struct.pack('<8f',*p,1.,1.,1.,1.))
    return bytes(out)

def reference_pixels(f):
    """Flat-face CPU painter oracle; exact SGX fill rules remain unqualified."""
    pixels=[0]*1024
    def edge(a,b,p):return (b[0]-a[0])*(p[1]-a[1])-(b[1]-a[1])*(p[0]-a[0])
    for d in f['draws']:
        for ids in d['triangle_indices']:
            a,b,c=(d['screen_vertices'][i] for i in ids)
            area=edge(a,b,c)
            if not area:continue
            if area<0:b,c=c,b
            for y in range(32):
                for x in range(32):
                    pos=(x+.5,y+.5);pairs=((a,b),(b,c),(c,a))
                    inside=True
                    for u,v in pairs:
                        e=edge(u,v,pos);dy=v[1]-u[1];dx=v[0]-u[0]
                        if e<0 or (e==0 and not (dy<0 or (dy==0 and dx>0))):inside=False;break
                    if inside:pixels[y*32+x]=int(d['ARGB'],16)
    return b''.join(struct.pack('<I',p) for p in pixels)

def generate(output):
    output.mkdir(parents=True,exist_ok=False)
    frames=[]
    for i,angle in enumerate((0,15,30,45,60)):
        f=frame(angle);raw=reference_pixels(f);vertices=pack_vertices(f['vertices'])
        (output/f'frame-{i}.CPU-REFERENCE.argb').write_bytes(raw)
        (output/f'frame-{i}.screen-vertices.bin').write_bytes(vertices)
        (output/f'frame-{i}.json').write_text(json.dumps(f,indent=2)+'\n')
        # Standard lossless PPM, explicitly named CPU reference, not hardware result.
        rgb=b''.join(bytes(((v>>16)&255,(v>>8)&255,v&255)) for v in struct.unpack('<1024I',raw))
        (output/f'frame-{i}.CPU-REFERENCE.ppm').write_bytes(b'P6\n32 32\n255\n'+rgb)
        frames.append({'index':i,'angle':angle,'ARGB_sha256':hashlib.sha256(raw).hexdigest(),
                       'face_order':[d['face'] for d in f['draws']]})
    manifest={'scope':'CPU reference only; no SGX candidate/render/publication','frames':frames,
              'convention':'right-handed,column vectors,clip -w..w,y-down viewport,CPU painter order',
              'SGX_payload_fourth_field':1,'perspective_correct_varyings':'NOT ESTABLISHED',
              'lighting_values':'reference calculations only; constant diagnostic face colors used',
              'files':{p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in output.iterdir()}}
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');return manifest

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--output',type=Path,required=True)
    print(json.dumps(generate(ap.parse_args().output),indent=2))
