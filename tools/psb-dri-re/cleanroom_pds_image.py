#!/usr/bin/env python3
"""Route C CPU byte-image proof aid; NOT a decoder, BO provider or GPU test.

U is an explicit resolved relocation-word parameter, not a validated address.
Snapshot comparison proves only equality of the supplied ordinary-file/host
bytes. No result here proves cache visibility, SGX binding or read containment.
"""
import argparse
import csv
from dataclasses import dataclass
import json
from pathlib import Path

SIZE = 56
HOLES = (8, 12, 16, 20, 24, 28, 36, 40, 44)
KNOWN = {4: 0, 32: 0x20, 48: 0x07000345, 52: 0xaf000000}
DOCS = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re'


@dataclass(frozen=True)
class Image:
    data: bytes
    defined: tuple
    origin: tuple


def u32(value):
    if type(value) is not int or not 0 <= value <= 0xffffffff:
        raise ValueError('resolved relocation word must be an explicit unsigned u32')
    return value


def initialize_payload(payload):
    """Explicitly clear a caller-owned host payload before ALL scene writers.

    Caller must establish ownership/bounds; this is not a DMA mapping operation.
    """
    if not isinstance(payload, bytearray):
        raise ValueError('payload must be an exclusively owned host bytearray')
    payload[:] = bytes(len(payload))


def build_image(relocated_use_word, *, initial=None, initialize=True):
    """Construct only the selected CPU image; initialize=False is a fault fixture."""
    u32(relocated_use_word)
    if initial is None:
        initial = bytes(SIZE)
    if len(initial) != SIZE:
        raise ValueError('initial image must have exactly 56 bytes')
    data = bytearray(initial)
    defined = [False] * SIZE
    origin = ['UNDEFINED'] * SIZE
    if initialize:
        initialize_payload(data)
        defined[:] = [True] * SIZE
        origin[:] = ['ZERO_INIT'] * SIZE
    for offset, value in {0: relocated_use_word, **KNOWN}.items():
        data[offset:offset+4] = value.to_bytes(4, 'little')
        defined[offset:offset+4] = [True] * 4
        origin[offset:offset+4] = [
            'RESOLVED_USE_WORD_INPUT' if offset == 0 else 'BUILDER_FIELD'] * 4
    return Image(bytes(data), tuple(defined), tuple(origin))


def verify_image(image, relocated_use_word):
    u32(relocated_use_word)
    if any(len(v) != SIZE for v in (image.data, image.defined, image.origin)):
        raise ValueError('image/definedness/provenance must each cover exactly 56 bytes')
    if not all(v is True for v in image.defined):
        raise ValueError('undefined CPU bytes: initialization omitted')
    for offset in range(0, SIZE, 4):
        if offset == 0:
            value, provenance = relocated_use_word, 'RESOLVED_USE_WORD_INPUT'
        elif offset in KNOWN:
            value, provenance = KNOWN[offset], 'BUILDER_FIELD'
        else:
            value, provenance = 0, 'ZERO_INIT'
        if image.data[offset:offset+4] != value.to_bytes(4, 'little'):
            raise ValueError(f'wrong word at +0x{offset:02x}')
        if image.origin[offset:offset+4] != (provenance,) * 4:
            raise ValueError(f'wrong provenance at +0x{offset:02x}')


def verify_preserved_snapshot(before, after):
    """Whole supplied payload equality, not a synchronization implementation."""
    if len(before) != len(after) or before != after:
        raise ValueError('payload contents or extent not preserved')


def require_contained(read_ranges, initialized_size):
    """Check a SUPPLIED superset. Never infer one from the CPU allocation size."""
    if type(initialized_size) is not int or initialized_size < 0:
        raise ValueError('invalid initialized extent')
    if read_ranges is None:
        raise ValueError('UNKNOWN architectural read superset: containment unproved')
    for start, size in read_ranges:
        if (type(start) is not int or type(size) is not int or
                start < 0 or size <= 0 or start + size > initialized_size):
            raise ValueError('read range not contained in initialized extent')


def verify_table(path):
    with Path(path).open(newline='') as stream:
        rows = list(csv.DictReader(stream))
    if [r['offset'] for r in rows] != [f'0x{i:02x}' for i in range(0, SIZE, 4)]:
        raise ValueError('table must cover each of the fourteen dwords exactly once')
    for row in rows:
        offset = int(row['offset'], 16)
        expected = 'U' if offset == 0 else f'0x{KNOWN.get(offset, 0):08x}'
        provenance = ('RESOLVED_USE_WORD_INPUT' if offset == 0 else
                      'ZERO_INIT' if offset in HOLES else 'BUILDER_FIELD')
        if row['value'] != expected or row['cpu_origin'] != provenance:
            raise ValueError(f'table formula/provenance mismatch at {row["offset"]}')
        if row['width_bytes'] != '4' or row['gpu_first_use_status'] != 'CONDITIONAL':
            raise ValueError('table width or GPU-status mismatch')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--relocated-use-word', type=lambda s: int(s, 0))
    parser.add_argument('--check-table', action='store_true')
    args = parser.parse_args()
    if args.relocated_use_word is None and not args.check_table:
        parser.error('supply --relocated-use-word or --check-table')
    if args.check_table:
        verify_table(DOCS / 'cleanroom-pds-image.csv')
        print('PASS: 14 dwords; formulas/provenance; GPU status remains CONDITIONAL')
    if args.relocated_use_word is not None:
        image = build_image(args.relocated_use_word)
        verify_image(image, args.relocated_use_word)
        print(json.dumps(dict(cpu_image_hex=image.data.hex(), size=SIZE,
                              all_cpu_bytes_defined=True,
                              relocation='explicit input; validity not checked',
                              gpu_contents_preserved='CONDITIONAL',
                              architectural_read_superset='UNKNOWN',
                              implementation_dependency_removed=False), indent=2))


if __name__ == '__main__':
    main()
