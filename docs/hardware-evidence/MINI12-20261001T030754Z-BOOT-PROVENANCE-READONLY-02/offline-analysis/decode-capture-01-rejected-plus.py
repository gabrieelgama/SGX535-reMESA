"""Decode only hash-checked framed ordinary files; never execute target text."""
import base64
import hashlib
import json
from pathlib import Path
import re
import sys


def decode_records(raw):
    records = []
    used = set()
    for match in re.finditer(rb'^FILE_BEGIN ([^\n]+)\n(.*?)\nFILE_END\n', raw, re.M | re.S):
        header, body = match.groups()
        label, source = header.decode().split(' ', 1)
        if not re.fullmatch(r'[A-Za-z0-9_-]+', label) or label in used:
            raise ValueError('unsafe or duplicate artifact label')
        used.add(label)
        lines = body.split(b'\n')
        stat = re.fullmatch(rb'size=([0-9]+) mtime=(.*?) mode=([0-7]+)', lines[0])
        checksum = re.fullmatch(rb'([0-9a-f]{64}) [ *](.*)', lines[1])
        if not stat or not checksum or checksum[2].decode() != source:
            raise ValueError('malformed file metadata')
        record = {'label': label, 'target_source': source,
                  'target_bytes': int(stat[1]), 'mtime': stat[2].decode(),
                  'mode': stat[3].decode(), 'target_sha256': checksum[1].decode()}
        if len(lines) == 3 and lines[2].startswith(b'FILE_LIMIT '):
            record['omitted'] = 'target capture size limit'
            record['data'] = None
        else:
            if len(lines) != 5 or lines[2] != b'BASE64_BEGIN' or lines[4] != b'BASE64_END':
                raise ValueError('malformed base64 frame')
            data = base64.b64decode(lines[3], validate=True)
            record.update(data=data, bytes=len(data),
                          sha256=hashlib.sha256(data).hexdigest())
            record['target_hash_verified'] = record['sha256'] == record['target_sha256']
            record['target_size_verified'] = record['bytes'] == record['target_bytes']
        records.append(record)
    return records


def main(directory):
    meta = json.loads((directory/'capture.json').read_text())
    raw = (directory/'stdout.txt').read_bytes()
    error = (directory/'stderr.txt').read_bytes()
    if meta['exit_status'] != 0 or error or not raw.endswith(b'BOOT_PROVENANCE_READONLY_PASS\n'):
        raise ValueError('capture did not pass')
    for label, data in [('stdout', raw), ('stderr', error), ('script', (directory/'capture.sh').read_bytes())]:
        if hashlib.sha256(data).hexdigest() != meta[label+'_sha256']:
            raise ValueError(label+' transport hash mismatch')
    output = directory/'artifacts'
    if output.exists():
        raise ValueError('do not overwrite artifacts')
    output.mkdir()
    records = decode_records(raw)
    for record in records:
        data = record.pop('data')
        if data is not None:
            (output/record['label']).write_bytes(data)
    (directory/'artifact-manifest.json').write_text(json.dumps(records, indent=2)+'\n')
    # Text-only target transcript excludes framed base64 without changing raw evidence.
    text = re.sub(rb'\nBASE64_BEGIN\n[A-Za-z0-9+/=]*\nBASE64_END',
                  b'\n[BASE64 FILE BYTES PRESERVED IN ARTIFACTS]', raw)
    (directory/'transcript-without-base64.txt').write_bytes(text)
    print(json.dumps({'records': len(records), 'decoded': sum('bytes' in p for p in records),
                      'unstable': [p['label'] for p in records if p.get('target_hash_verified') is False
                                   or p.get('target_size_verified') is False]}, indent=2))


if __name__ == '__main__':
    main(Path(sys.argv[1]))
