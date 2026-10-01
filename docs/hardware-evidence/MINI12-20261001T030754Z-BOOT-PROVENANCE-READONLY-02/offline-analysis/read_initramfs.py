"""Read newc/gzip archive bytes offline; never execute/extract links or devices."""
import hashlib
import json
from pathlib import Path, PurePosixPath
import stat
import sys
import zlib


def read_cpio(data, start=0):
    pos=start
    records=[]
    while True:
        if len(data)-pos<110 or data[pos:pos+6] not in (b'070701',b'070702'):
            raise ValueError('invalid/truncated newc header')
        crc=data[pos:pos+6]==b'070702'
        fields=[int(data[pos+6+i*8:pos+14+i*8],16) for i in range(13)]
        inode,mode,uid,gid,nlink,mtime,size,major,minor,rmajor,rminor,namesize,check=fields
        ns=pos+110
        if namesize<1 or ns+namesize>len(data) or data[ns+namesize-1]!=0:
            raise ValueError('invalid newc name')
        name=data[ns:ns+namesize-1].decode('utf-8')
        if '\x00' in name or name.startswith('/') or '..' in PurePosixPath(name).parts:
            raise ValueError('unsafe newc path')
        ds=(ns+namesize+3)&~3
        end=ds+size
        if end>len(data): raise ValueError('truncated newc payload')
        payload=data[ds:end]
        if crc and sum(payload)&0xffffffff != check:
            raise ValueError('newc checksum mismatch')
        pos=(end+3)&~3
        if name=='TRAILER!!!': return records,pos
        records.append({'name':name,'mode':mode,'uid':uid,'gid':gid,'nlink':nlink,
                        'inode':inode,'device':[major,minor],'mtime':mtime,
                        'bytes':size,'sha256':hashlib.sha256(payload).hexdigest(),
                        'data':payload})


def read_image(data):
    pos=0; records=[]; containers=[]
    while pos<len(data):
        while pos<len(data) and data[pos]==0: pos+=1
        if pos==len(data): break
        if data[pos:pos+6] in (b'070701',b'070702'):
            part,end=read_cpio(data,pos)
            containers.append({'format':'newc','offset':pos,'bytes':end-pos,'members':len(part)})
            records.extend(part); pos=end
        elif data[pos:pos+2]==b'\x1f\x8b':
            stream=zlib.decompressobj(31)
            expanded=stream.decompress(data[pos:])+stream.flush()
            if not stream.eof: raise ValueError('truncated gzip stream')
            used=len(data)-pos-len(stream.unused_data)
            part,nested=read_image(expanded)
            containers.append({'format':'gzip','offset':pos,'bytes':used,
                               'expanded_bytes':len(expanded),'sha256':hashlib.sha256(expanded).hexdigest(),
                               'containers':nested})
            records.extend(part); pos+=used
        else:
            raise ValueError('unsupported archive/compression at offset '+str(pos))
    return records,containers


def main(image,out):
    if out.exists(): raise ValueError('do not overwrite derived analysis')
    records,containers=read_image(image.read_bytes())
    out.mkdir(); selected=out/'selected-files'; selected.mkdir()
    catalog=[]
    for index,row in enumerate(records):
        data=row.pop('data'); name=row['name']
        wanted=(name=='init' or name.startswith(('conf/','scripts/','etc/modprobe.d/',
                'etc/udev/','lib/udev/rules.d/','usr/lib/udev/rules.d/')) or
                name.endswith(('/modules.alias','/modules.dep','/modules.softdep','/modules.builtin')) or
                Path(name).name in ('gma500_gfx.ko','drm.ko','drm_kms_helper.ko','video.ko','i2c-algo-bit.ko'))
        if wanted and stat.S_ISREG(row['mode']):
            target=selected/name
            if target.exists(): raise ValueError('duplicate selected regular path '+name)
            target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
            row['preserved_regular_file']=str(target.relative_to(out))
        catalog.append(row)
    (out/'catalog.json').write_text(json.dumps({'image':str(image),'image_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),'containers':containers,'members':catalog},indent=2)+'\n')
    print(json.dumps({'members':len(catalog),'containers':containers,
          'gma500':[r['name'] for r in catalog if 'gma500' in r['name']],
          'selected_regular_files':sum('preserved_regular_file' in r for r in catalog)},indent=2))


if __name__=='__main__': main(Path(sys.argv[1]),Path(sys.argv[2]))
