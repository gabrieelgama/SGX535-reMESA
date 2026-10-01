#!/usr/bin/env python3
"""P7H-029: verify candidate BO/fence wire layouts without executing any ELF.

Requires clang and the two already catalogued public drm.h files. Compile a
freestanding i386 constants object, read .rodata, compare field metrics with
pds-abi-pairing.csv. Source declarations stay in a temporary directory.
This checks source ABI equality, not the identity of an installed library.
"""
import argparse
import csv
import hashlib
from pathlib import Path
import struct
import subprocess
import tempfile

HEADER_SHA = 'e4e0c5dce5558d4aa09f33fa236832e354275ccc1612a7c4a1a906758bb51f41'
DOC = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re'


def rodata(path):
    data = path.read_bytes()
    if data[:6] != b'\x7fELF\x01\x01' or struct.unpack_from('<H', data, 18)[0] != 3:
        raise ValueError('Expected ELF32 little-endian i386 constants object')
    offset = struct.unpack_from('<I', data, 32)[0]
    size, count, names_index = struct.unpack_from('<HHH', data, 46)
    sections = [struct.unpack_from('<10I', data, offset + i * size) for i in range(count)]
    names = sections[names_index]
    strings = data[names[4]:names[4] + names[5]]
    for section in sections:
        name = strings[section[0]:].split(b'\0', 1)[0]
        if name == b'.rodata':
            result = data[section[4]:section[4] + section[5]]
            return struct.unpack('<' + 'I' * (len(result) // 4), result)
    raise ValueError('No .rodata')


def check(left, right, table):
    raw = left.read_bytes()
    if raw != right.read_bytes() or hashlib.sha256(raw).hexdigest() != HEADER_SHA:
        raise ValueError('Candidate drm.h identity mismatch')
    with table.open(newline='') as stream:
        rows = [r for r in csv.DictReader(stream) if r['category'] == 'wire_layout']
    source = raw.decode()
    start = source.index('struct drm_fence_arg {')
    end = source.index('struct drm_bo_op_arg {', start)
    end = source.index('\n};', end) + 3
    expressions = []
    for row in rows:
        name, field, metric = row['item'], row['field'], row['metric']
        if not all(part.replace('_', '').isalnum() for part in (name, field)):
            raise ValueError('Invalid identifier')
        if metric == 'size':
            expr = f'sizeof(struct {name})'
        elif metric == 'offset':
            expr = f'__builtin_offsetof(struct {name},{field})'
        elif metric == 'width':
            expr = f'sizeof(((struct {name}*)0)->{field})'
        else:
            raise ValueError('Unknown metric')
        expressions.append(expr)
    probe = ('typedef unsigned long long uint64_t;\n' + source[start:end]
             + '\nconst unsigned values[] = {' + ','.join(expressions) + '};\n')
    with tempfile.TemporaryDirectory(prefix='pds-abi-') as tmp:
        cfile, obj = Path(tmp) / 'layout.c', Path(tmp) / 'layout.o'
        cfile.write_text(probe)
        subprocess.run(['clang', '--target=i386-unknown-linux-gnu', '-std=c11',
                        '-c', str(cfile), '-o', str(obj)], check=True)
        values = rodata(obj)
    if len(values) != len(rows):
        raise ValueError('Unexpected constants count')
    for row, value in zip(rows, values):
        if str(value) != row['libdrm_value'] or str(value) != row['kernel_value']:
            raise ValueError(f"Layout mismatch: {row['item']}.{row['field']} {row['metric']}")
    print(f'PASS: identical headers; {len(rows)} i386 source layout metrics; no ELF executed')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('libdrm_header', type=Path)
    parser.add_argument('kernel_header', type=Path)
    parser.add_argument('--table', type=Path, default=DOC / 'pds-abi-pairing.csv')
    args = parser.parse_args()
    check(args.libdrm_header, args.kernel_header, args.table)
