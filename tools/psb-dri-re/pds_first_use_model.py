#!/usr/bin/env python3
"""P7H-030: arithmetic model of a successful, fresh first-scene CPU path.

No ELF execution, ISA decoding, ioctl, or hardware access. Inputs are manually
recovered reservation/commit facts cited in pds-stack-pairing.md. Output offsets
are slot-relative, not GPU addresses. This model does not prove stack pairing.
"""
import argparse
import csv
import io
from pathlib import Path

DOC = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re'
FIELDS = ('branch', 'sequence', 'content', 'emitter', 'alignment', 'reservation',
          'start', 'commit', 'end', 'overlaps_selected', 'evidence')


def timeline(clear, capacity=0x20000):
    # (content, emitter ELF VA, alignment, reservation, commit)
    allocations = [
        ('event data', '0x392ab', 4, 0x10, 0x10),
        ('event PDS', '0x392ab', 32, 400, 0x94),
    ]
    if clear:
        allocations.extend([
            ('internal clear PDS', '0x39ac4', 32, 0x50, 0x38),
            ('clear secondary PDS', '0x3faa3', 32, 4, 4),
        ])
    allocations.extend([
        ('texture-replace PDS', '0x39c5c', 32, 0x50, 0x40),
        ('texture secondary PDS', '0x3faa3', 32, 4, 4),
        ('preterminate state data', '0x287ff', 4, 0xa0, 8),
        # 0x381a0 receives two dwords: one descriptor, hence 60-byte PDS.
        ('preterminate state PDS', '0x287ff', 32, 0xa0, 0x3c),
        ('selected primary PDS', '0x25970', 32, 400, 56),
        ('selected secondary PDS', '0x3faa3', 32, 4, 4),
    ])
    result = []
    cursor = 0
    for ordinal, (content, emitter, align, reserve, commit) in enumerate(allocations, 1):
        start = (cursor + align - 1) & -align
        if commit > reserve or start + reserve > capacity:
            raise ValueError('Trace would require rotation or exceed reservation')
        result.append(dict(branch='color_field_24_eq_1' if clear else 'color_field_24_ne_1',
                           sequence=str(ordinal), content=content, emitter=emitter,
                           alignment=hex(align), reservation=hex(reserve), start=hex(start),
                           commit=hex(commit), end=hex(start + commit),
                           overlaps_selected='NO', evidence='P7H-030'))
        cursor = start + commit
    selected = next(r for r in result if r['content'] == 'selected primary PDS')
    selected['overlaps_selected'] = 'SELF'
    lo, hi = int(selected['start'], 16), int(selected['end'], 16)
    for r in result:
        if r is not selected and int(r['start'], 16) < hi and lo < int(r['end'], 16):
            raise ValueError('Payload overlaps selected primary')
    return result


def render():
    out = io.StringIO(newline='')
    writer = csv.DictWriter(out, FIELDS, lineterminator='\n')
    writer.writeheader()
    for clear in (False, True):
        writer.writerows(timeline(clear))
    return out.getvalue()


def verify():
    for clear, expected in ((False, 0x160), (True, 0x1c0)):
        small = timeline(clear)
        if small != timeline(clear, 0x40000):
            raise ValueError('Slot-size branches diverged')
        selected = next(r for r in small if r['content'] == 'selected primary PDS')
        if int(selected['start'], 16) != expected:
            raise ValueError('Unexpected selected offset')
        # Prior reservations overlap later ranges; only actual payload stores
        # and their relocation destinations must be disjoint.
        for hole in (8, 12, 16, 20, 24, 28, 36, 40, 44):
            address = expected + hole
            for row in small[:int(selected['sequence']) - 1]:
                if int(row['start'], 16) <= address < int(row['end'], 16):
                    raise ValueError('Earlier committed payload covers a hole')
    try:
        timeline(False, 0x100)
    except ValueError:
        pass
    else:
        raise ValueError('Insufficient capacity was not rejected')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write', action='store_true', help='write the derived timeline CSV')
    args = parser.parse_args()
    verify()
    path = DOC / 'pds-first-use-timeline.csv'
    expected = render()
    if args.write:
        path.write_text(expected)
    elif path.read_text() != expected:
        raise SystemExit('Timeline CSV differs from arithmetic model')
    print('PASS: both branches, both slot sizes, 18 holes, capacity rejection, timeline CSV')


if __name__ == '__main__':
    main()
