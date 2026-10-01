#!/usr/bin/env python3
"""Check recorded launch-model integrity, NOT architectural completeness.

L12 has no architectural proof certificate in the current evidence. CPU layout
and raw launch-word arithmetic cannot supply one. The default successful exit
means the inventory faithfully records that OPEN result, not closure.
"""
import argparse
import csv
import json
from pathlib import Path

from cleanroom_pds_image import DOCS, build_image, u32, verify_image, verify_table

CPU_IDS = {f'cpu_{i:02x}' for i in range(0, 56, 4)}
PARAMETER_IDS = {'packed_controls', 'use_target', 'secondary'}
UNKNOWN_IDS = {'ds0', 'ds1', 'temp0', 'temp1', 'entry', 'implicit', 'dout'}
REQUIRED_IDS = CPU_IDS | PARAMETER_IDS | UNKNOWN_IDS
FIELDS = ('state_id', 'state_class', 'location', 'source', 'producer',
          'initialization', 'first_possible_use', 'consumer', 'determinism',
          'confidence', 'coverage_rule', 'evidence', 'notes', 'role',
          'selected_reference', 'read_before_write', 'range_status',
          'first_read_defined', 'persistence', 'availability_evidence',
          'exclusion_evidence')
SOURCE_IDS = {'ds0', 'ds1', 'temp0', 'temp1', 'implicit'}
CONTROL_IDS = {'entry', 'packed_controls', 'use_target', 'dout'}
# Evidence ledger, not a switch to enable closure. A future change needs an
# independently reviewed architectural proof, not a different CSV label.
LAUNCH_DOMAIN_EVIDENCE = {'status': 'UNKNOWN', 'evidence': 'P7H-039;P7H-041'}


def assess_availability(rows):
    """Conditional coverage of SUPPLIED rows, not proof they cover the ISA.

    SOURCE includes unresolved eligibility, not demonstrated source access.
    Synthetic axioms may exercise model logic in tests, never certify hardware.
    """
    sources, controls = [], []
    for row in rows:
        if not row.get('availability_evidence', '').strip():
            raise ValueError('missing availability provenance')
        role = row['role']
        if role not in {'SOURCE', 'CONTROL', 'CPU_IMAGE', 'SEPARATE_LAUNCH'}:
            raise ValueError('invalid role')
        if role == 'SOURCE':
            if row['selected_reference'] == 'EXCLUDED':
                if row.get('exclusion_evidence', 'UNKNOWN') in ('', 'UNKNOWN'):
                    raise ValueError('source exclusion requires evidence')
                continue
            if (row['range_status'] != 'BOUNDED' or
                    row['first_read_defined'] != 'YES' or
                    row['availability_evidence'] == 'UNKNOWN'):
                sources.append(row['state_id'])
        elif role == 'CONTROL' and row['first_read_defined'] != 'YES':
            controls.append(row['state_id'])
    envelope = LAUNCH_DOMAIN_EVIDENCE['status'] == 'CONFIRMED'
    return dict(source_blockers=sorted(sources), control_obligations=sorted(controls),
                listed_sources_covered=not sources, envelope_proven=envelope,
                coverage_closed=not sources and not controls and envelope)


def cpu_slot(bank, index):
    """CPU placement arithmetic only; not an architectural DS address decoder."""
    if type(bank) is not int or bank not in (0, 1) or type(index) is not int or index < 0:
        raise ValueError('invalid CPU layout label')
    return 4 * ((index & 7) + 16 * (index >> 3) + 8 * bank)


def cpu_extent(n0, n1):
    """Xpsb 0x7470 raw extent, followed by selected DRI four-dword rounding."""
    if any(type(n) is not int or n < 0 for n in (n0, n1)):
        raise ValueError('invalid CPU count')
    raw = max(cpu_slot(b, n-1)//4+1 if n else 0 for b, n in enumerate((n0, n1)))
    return raw, (raw+3) & ~3


def selected_control_words(primary_address, secondary_address):
    """0x287ff present&0x40 nonnull-primary branch, selected descriptor only.

    Addresses are supplied resolved CPU-relocation inputs. No mapping, hardware
    address validity, global-base selection or entry-point decoding is asserted.
    """
    for address in (primary_address, secondary_address):
        u32(address)
        if address & 0xf:
            raise ValueError('selected reference loses nonzero low address bits')
    return ((secondary_address >> 4) & 0xffffff, 0x00030000,
            ((primary_address >> 4) & 0xffffff) | 0x0c000000)


def read_inventory(path=DOCS / 'pds-launch-state.csv'):
    with Path(path).open(newline='') as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames != list(FIELDS):
            raise ValueError('inventory schema mismatch')
        return list(reader)


def check_inventory(rows, image, relocated_use_word):
    verify_image(image, relocated_use_word)
    verify_table(DOCS / 'cleanroom-pds-image.csv')
    ids = [row.get('state_id') for row in rows]
    if len(ids) != len(set(ids)) or set(ids) != REQUIRED_IDS:
        raise ValueError('inventory missing/duplicate/new input: evidence review required')
    for row in rows:
        if set(row) != set(FIELDS) or any(not isinstance(v, str) or not v.strip() for v in row.values()):
            raise ValueError('inventory missing provenance or field')
        sid = row['state_id']
        role = ('SOURCE' if sid in SOURCE_IDS else 'CONTROL' if sid in CONTROL_IDS
                else 'CPU_IMAGE' if sid in CPU_IDS else 'SEPARATE_LAUNCH')
        if row['role'] != role:
            raise ValueError('unsupported role change: source/control evidence review required')
        # Pin current evidence scope. Editing labels must not manufacture closure.
        expected = ('CPU_DEFINED' if sid in CPU_IDS else
                    'PARAMETERIZED_CPU' if sid in PARAMETER_IDS else 'UNKNOWN')
        if row['determinism'] != expected:
            raise ValueError('unsupported determinism promotion/change')
        if sid in CPU_IDS:
            offset = int(sid[4:], 16)
            if (row['location'] != f'+0x{offset:02x}' or
                    row['initialization'] != 'before_publish' or
                    row['confidence'] != 'CONFIRMED' or row['coverage_rule'] != 'CPU'):
                raise ValueError('CPU location/initialization/proof-scope mismatch')
        elif row['coverage_rule'] != 'L12':
            raise ValueError('launch rule must remain explicit')
    availability = assess_availability(rows)
    # A complete CPU manifest is not an architectural source-domain certificate.
    return dict(inventory_valid=True, cpu_bytes_defined=56,
                cpu_data_dwords=12, cpu_instruction_dwords=2,
                **availability, missing_launch_rule='L12',
                possible_inputs='PARTIALLY BOUNDED',
                unknown_state_rows=sorted(UNKNOWN_IDS),
                gpu_contents_preserved='CONDITIONAL',
                implementation_dependency_removed=False)


def require_closed(result):
    if not result['coverage_closed']:
        raise ValueError('L12: selected launch initialized-state domain is unproved')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--require-closed', action='store_true')
    args = parser.parse_args()
    # Explicit arbitrary fixture U, not a target USE address.
    result = check_inventory(read_inventory(), build_image(0x12345678), 0x12345678)
    print(json.dumps(result, indent=2))
    if args.require_closed:
        try:
            require_closed(result)
        except ValueError as error:
            parser.exit(1, f'OPEN: {error}\n')


if __name__ == '__main__':
    main()
