#!/usr/bin/env python3
"""Bounded arithmetic model of DRI 0x287ff; not an ISA/launch-ABI decoder.

Inputs outside the deliberately small non-overflow audit domain are rejected.
Acceptance does not prove a shader/descriptor is hardware-valid or was executed.
Q has no confirmed architectural unit. Data size is a different word/field.
"""
import csv
from pathlib import Path

TABLE = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re/pds-launch-count-corpus.csv'
NUMERIC = ('a','t','b','dp','budget','q','q_encoded','m','middle','data_field')


def model(a,t,b,dp):
    # These bounds define this proof aid, NOT architectural register capacities.
    if any(type(v) is not int or not 0 <= v <= 256 for v in (a,t,b)):
        raise ValueError('outside bounded CPU arithmetic audit domain')
    if type(dp) is not int or not 0 <= dp <= 252 or dp % 4:
        raise ValueError('data count must be representable and CPU-aligned')
    a4=(a+3)&~3
    budget=((0x4df-((b+31)&~31))&0xffffffa0)//3
    q=4 if t==0 else min(4,192//((4*t+15)&~15))
    if a4:q=min(q,budget//(4*a4))
    q=max(1,q)
    m=32 if a4==0 else max(1,min(32,budget//(4*a4)))
    if 8*q < min(m+3,32):m=8*q-3
    middle=(((4*b+127)&0x3f80)<<11)|a|((q-1)<<16)|((m&31)<<25)|(0x8000 if b else 0)
    return dict(a=a,t=t,b=b,dp=dp,budget=budget,q=q,q_encoded=q-1,m=m,
                middle=middle,data_field=(dp&0xfc)<<24)


def verify_case(row):
    expected=model(*(row[k] for k in ('a','t','b','dp')))
    for key in NUMERIC:
        if type(row[key]) is not int or row[key]!=expected[key]:
            raise ValueError(f'contradictory {key}: count/layout/formula mismatch')


def check_table():
    with TABLE.open(newline='') as f:rows=list(csv.DictReader(f))
    ids=[r['case_id'] for r in rows]
    if len(ids)!=len(set(ids)) or not rows:raise ValueError('case IDs')
    for row in rows:
        if row['kind'] not in ('SELECTED_CPU_RESULT','SERIALIZER_FORMULA_CASE'):
            raise ValueError('synthetic cases must not be labeled hardware observations')
        if row['hardware_unit']!='UNKNOWN' or row['source']!='DRI ELF 0x287ff':
            raise ValueError('unsupported source/architectural promotion')
        verify_case({k:int(row[k],0) for k in NUMERIC})
        xor=int(row['middle'],0)^0x30000
        bits=';'.join(str(i) for i in range(32) if xor>>i&1) or 'NONE'
        if (int(row['middle_xor_selected'],0)!=xor or
                row['middle_changed_bits']!=bits or
                int(row['data_xor_selected'],0)!=(int(row['data_field'],0)^0x0c000000)):
            raise ValueError('differential mismatch')
    print(f'PASS: {len(rows)} scoped CPU cases; Q unit UNKNOWN; L12 OPEN')


if __name__=='__main__':
    check_table()
