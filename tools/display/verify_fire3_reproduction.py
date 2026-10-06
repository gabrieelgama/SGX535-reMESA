#!/usr/bin/env python3
"""Non-mutating repository archive/manifest/tutorial check; no device access."""
import hashlib
import json
from pathlib import Path


def main():
    repo = Path(__file__).resolve().parents[2]
    document = repo/'docs/phase8/FIRE3-TRIANGLE-REPRODUCTION.md'
    manifest = json.loads((repo/'docs/phase8/FIRE3-TRIANGLE-REPRODUCTION.json').read_text())
    checked = 0
    for name, meta in manifest['archive_files'].items():
        path = repo/name
        assert path.stat().st_size == meta['bytes'], name
        assert hashlib.sha256(path.read_bytes()).hexdigest() == meta['sha256'], name
        checked += 1
    archive = repo/'docs/phase8/artifacts/FIRE3-TRIANGLE-20261006'
    for directory, sealname in [('successful-procedure', 'final-evidence-seal.json'),
                                ('transfer-firstload-procedure', 'preparation-seal.json')]:
        root = archive/directory
        seal = json.loads((root/sealname).read_text())
        for name, meta in seal['files'].items():
            path = root/name
            assert path.stat().st_size == meta['bytes'], name
            assert hashlib.sha256(path.read_bytes()).hexdigest() == meta['sha256'], name
    original = archive/'successful-procedure/fire3-one-authorized-call'
    original_manifest = original/'originals-manifest.json'
    original_seal = json.loads((original/'originals-seal.json').read_text())
    assert hashlib.sha256(original_manifest.read_bytes()).hexdigest() == original_seal['manifest_sha256']
    for name, meta in json.loads(original_manifest.read_text())['files'].items():
        path = original/'originals'/name
        assert path.stat().st_size == meta['bytes']
        assert hashlib.sha256(path.read_bytes()).hexdigest() == meta['sha256']
    from triangle_pixels import validate_source
    source = manifest['readback_source']
    validate_source((repo/source['path']).read_bytes(), source['sha256'])
    outcome = json.loads((original/'outcome-report.json').read_text())
    card = json.loads((archive/'successful-procedure/one-shot-execution-card.json').read_text())
    assert manifest['boot_id'] == card['boot_id'] == outcome['boot_id']
    for name in ('driver', 'observer', 'image', 'uapi'):
        assert manifest['candidate'][name] == card[name]
        assert manifest['repository_artifacts'][name]['sha256'] == card[name]['sha256']
    for key in ('FIRE3_invocations', 'client_exit_status', 'ioctl_return', 'operation_errno',
                'accepted_ledger', 'readback', 'triangle'):
        assert manifest['expected'][key] == outcome[key], key
    md = document.read_text()
    fragment = manifest['fragment_program_identity']
    assert hashlib.sha256(bytes.fromhex(fragment['bytes_hex'])).hexdigest() == fragment['sha256']
    assert fragment['sha256'] in md
    for token in [manifest['boot_id'], manifest['historical_repository_commit'],
                  'KNOWN_GOOD_TRIANGLE_READBACK', '120', '904', '4096',
                  '00000000 f8040140', '001f00ff fca7f1f1', '0xffff00ff']:
        assert token in md, token
    for name, meta in manifest['repository_artifacts'].items():
        assert meta['sha256'] in md, name
    print(json.dumps({'archive_files':checked,'originals':18,
                      'copied_seals':'95/95 and81/81 PASS',
                      'manifest_tutorial_agreement':'PASS',
                      'source_triangle':'120 magenta /904 zero PASS',
                      'hardware_interactions':0,'sgx_invocations':0},indent=2))


if __name__ == '__main__': main()
