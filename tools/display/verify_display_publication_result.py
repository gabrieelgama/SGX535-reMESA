#!/usr/bin/env python3
"""CPU-only consistency check of the sealed visible-display milestone.

Verifies retained bytes and the operator observation record, not the panel itself.
No device access, X connection, hardware qualification or publication occurs.
"""
import hashlib
import json
from pathlib import Path

from triangle_display_centered import enlarge
from triangle_pixels import validate_source


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def check_seal(root, name):
    seal = json.loads((root/name).read_text())
    for name, meta in seal['files'].items():
        path = root/name
        assert path.stat().st_size == meta['bytes'], name
        assert digest(path) == meta['sha256'], name
    return len(seal['files'])


def main():
    repo = Path(__file__).resolve().parents[2]
    doc = repo/'docs/phase8'
    result = json.loads((doc/'display-publication-centered-result-20261006.json').read_text())
    root = doc/result['archive']
    assert digest(root/'attempt-seal.json') == result['attempt_seal_sha256']
    assert check_seal(root, 'attempt-seal.json') == result['sealed_attempt_files'] == 38
    originals = root/'originals'
    assert digest(originals/'seal.json') == result['originals_seal_sha256']
    assert check_seal(originals, 'seal.json') == 9
    assert digest(originals/'card.original.json') == result['card_sha256']
    card = json.loads((originals/'card.original.json').read_text())
    assert (doc/result['card']).read_bytes() == (originals/'card.original.json').read_bytes()
    assert result['boot_id'] == card['target']['boot_id']
    assert json.loads((root/'precheck.stdout.json').read_text())['target'] == card['target']
    for name, meta in card['tools'].items():
        archived = root/Path(name).name
        assert archived.stat().st_size == meta['bytes']
        assert digest(archived) == meta['sha256']
    source = (originals/'source.original.bin').read_bytes()
    validate_source(source, result['source_sha256'])
    assert source == (repo/card['source']['path']).read_bytes()
    published = (originals/'published.original.bin').read_bytes()
    expected = enlarge(source)
    assert len(published) == len(expected) == 102400
    assert all(published[n:n+3] == expected[n:n+3] for n in range(0, 102400, 4))
    assert (originals/'screen-before.original.bin').read_bytes() == (originals/'restored.original.bin').read_bytes()
    values = [int.from_bytes(published[n:n+3], 'little') for n in range(0, 102400, 4)]
    assert values.count(0xff00ff) == result['magenta_display_pixels'] == 3000
    assert values.count(0) == result['black_display_pixels'] == 22600
    op = json.loads((root/'operator-observation.json').read_text())
    assert op == result['operator_observation']
    assert op['physical_triangle_seen'] is True and op['display_restoration_seen'] is True
    assert result['display_attempts_under_this_authorization'] == 1
    assert result['sgx_invocations'] == 0
    assert result['DISPLAY_PUBLICATION_ESTABLISHED'] is True
    assert result['GPU_RENDER_CPU_PUBLICATION_ESTABLISHED'] is True
    assert result['DIRECT_SGX_SCANOUT_ESTABLISHED'] is False
    assert result['further_display_authorized'] is False
    assert result['further_sgx_authorized'] is False
    print(json.dumps({'attempt_seal': '38/38 PASS', 'original_seal': '9/9 PASS',
                      'source_enlargement_match': True, 'restoration_all_bytes_equal': True,
                      'archived_operator_confirmation': True, 'consistency': 'PASS',
                      'this_check_hardware_interactions': 0}, indent=2))


if __name__ == '__main__': main()
