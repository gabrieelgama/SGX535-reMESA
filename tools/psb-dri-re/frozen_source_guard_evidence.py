"""Offline consistency of a retained source witness, never live provenance."""
import argparse
import json
from pathlib import Path

FIELDS = {'state', 'reasons', 'producers_active', 'driver_attached', 'capsule_state',
          'boot', 'prepared', 'asserted_reset', 'released_reset', 'startup_pending',
          'startup_reads', 'startup_fault', 'startup_autonomous', 'delayed_exclusion'}


def evaluate(raw, *, closed=False):
    errors = []
    values = {}
    try:
        if len(raw) > 4096 or not raw.endswith(b'\n'):
            raise ValueError('incomplete or oversized record')
        lines = raw.decode('ascii').splitlines()
        if not lines or lines.pop(0) != 'SGXSOURCE2':
            raise ValueError('unsupported source record')
        for line in lines:
            key, value = line.split('=', 1)
            if key not in FIELDS or key in values:
                raise ValueError('unexpected or duplicate field')
            if key == 'delayed_exclusion':
                values[key] = value
            else:
                if not value.isdecimal() or int(value) > 0xffffffff:
                    raise ValueError('invalid integer')
                values[key] = int(value)
        if set(values) != FIELDS:
            raise ValueError('missing field')
        required = dict(state=3 if closed else 0, reasons=0, producers_active=0,
                        # frozen_capsule.h exports UNUSED=0, CLOSED=4.
                        driver_attached=1, capsule_state=4 if closed else 0,
                        boot=3, prepared=1, asserted_reset=127, released_reset=0,
                        startup_pending=0, startup_reads=0, startup_fault=0,
                        startup_autonomous=0,
                        delayed_exclusion='STARTUP_LIFECYCLE')
        errors += [key + ': witness condition not satisfied'
                   for key, value in required.items() if values[key] != value]
    except (ValueError, UnicodeError) as error:
        errors.append(str(error))
    return {'supplied_record_consistency': 'CONFIRMED' if not errors else 'UNKNOWN',
            'interval': 'CLOSED' if closed else 'PREPARED, MUST REMAIN CONTINUOUS',
            'hardware_attribution': 'UNKNOWN WITHOUT VERIFIED LIVE BOOT/PRODUCER PROVENANCE',
            'triangle': 'NOT ESTABLISHED BY SOURCE WITNESS',
            'errors': errors, 'values': values}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('record', type=Path)
    parser.add_argument('--closed', action='store_true')
    args = parser.parse_args()
    result = evaluate(args.record.read_bytes(), closed=args.closed)
    print(json.dumps(result, indent=2))
    raise SystemExit(bool(result['errors']))
