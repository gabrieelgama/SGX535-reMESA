#!/usr/bin/env python3
"""Deterministic frozen-scene host dry run. No device access or submission.

All GPU addresses, handles, CPU pointers, and USE reservations here are
synthetic test inputs. The resulting command template must never be sent to a
driver: service-generated state, publication, source containment, and Gate B
remain unresolved.
"""
import argparse
import hashlib
import json
import struct

import frozen_triangle_bo as bo
import frozen_triangle_closure as closure
import frozen_triangle_contracts as contracts
import frozen_triangle_image as image


def build_report():
    """Build the complete known CPU image with synthetic offline inputs."""
    scene = image.build()
    image.validate(scene)
    plan = bo.build(scene)
    bo.check(scene, plan)
    contracts.check_target(scene, contracts.target())
    contracts.check_auxiliary(scene, contracts.auxiliary(scene))
    closure.check(scene, closure.build(scene))

    addresses = bo.test_addresses(plan)
    resolved = bo.resolve(scene, plan, addresses, {0: 0, 1: 1}, 0x80000000)
    objects = {}
    for name, item in scene['objects'].items():
        binding = plan['bindings'].get(name)
        data = None
        bytes_origin = 'UNKNOWN_OR_SERVICE_GENERATED'
        if binding is not None and binding['bo'] in resolved:
            start = binding['offset']
            length = item['size'] if item['size'] is not None else binding['size']
            data = bytes(resolved[binding['bo']][start:start + length])
            bytes_origin = 'SYNTHETIC_RELOCATED_CPU_IMAGE'
        elif binding is None and item['bytes'] is not None and all(
                type(value) is int for value in item['bytes']):
            data = bytes(item['bytes'])
            bytes_origin = 'CPU_LITERAL_IMAGE'
        objects[name] = {
            'size': item['size'] if item['size'] is not None and data is None
                    else len(data) if data is not None else None,
            'alignment': item['alignment'],
            'bo': binding['bo'] if binding else None,
            'offset': binding['offset'] if binding else None,
            'synthetic_gpu_address': (
                addresses[binding['bo']] + binding['offset']
                if binding and plan['bos'][binding['bo']]['domain'] != 'LOCAL'
                else None
            ),
            'bytes_hex': data.hex() if data is not None else None,
            'bytes_origin': bytes_origin,
            'producer': item['producer'],
            'status': item['status'],
            'blockers': item['blockers'],
            'field_provenance': item['fields'],
        }

    primary = bytes.fromhex(objects['primary_pds']['bytes_hex'])
    program = primary[0x30:0x38]
    if len(primary) != 56 or program != bytes.fromhex('45030007000000af'):
        raise ValueError('selected primary image is not the retained 56-byte program')
    state = bytes.fromhex(objects['triangle_state']['bytes_hex'])
    launch = list(struct.unpack_from('<3I', state, 8))
    handles = {row['bo']: row['index'] + 1 for row in plan['validation']}
    host_pointers = {'validation_nodes': 0x1000, 'scene_arg': 0x2000,
                     'fence_reply': 0x3000}
    submission = bo.submission_words(scene, plan, handles, host_pointers)
    objects['submit_command']['bytes_hex'] = struct.pack('<36I', *submission).hex()
    objects['submit_command']['bytes_origin'] = 'SYNTHETIC_HOST_TEMPLATE'
    try:
        bootstrap_model = contracts.bootstrap(
            0x00010201, 'QUALIFIED_SGX_REVISION_EVIDENCE')
    except ValueError as exc:
        bootstrap_model = None
        bootstrap_status = 'BLOCKED: ' + str(exc)
    else:
        bootstrap_status = 'CPU_BRANCH_MODEL_ONLY; hardware readiness UNKNOWN'

    return {
        'scope': 'OFFLINE_HOST_MODEL_ONLY',
        'address_source': 'SYNTHETIC_TEST_ONLY',
        'target': 'Dell Inspiron Mini 12 / SGX535 rev121',
        'bos': {
            name: {
                'size': spec['size'], 'alignment': spec['alignment'],
                'domain': spec['domain'], 'owner': spec['owner'],
                'synthetic_base': addresses[name] if spec['domain'] != 'LOCAL' else None,
                'cpu_image_sha256': hashlib.sha256(resolved[name]).hexdigest()
                if name in resolved else None,
            }
            for name, spec in plan['bos'].items()
        },
        'objects': objects,
        'relocations': plan['wire_relocations'],
        'primary_program_bytes_hex': program.hex(),
        'launch_words': launch,
        'publication_order': list(closure.ORDER),
        'publication_status': 'HOST_ORDER_MODEL_ONLY; DEVICE_VISIBILITY_UNPROVED',
        'bootstrap_status': bootstrap_status,
        'bootstrap_cpu_model': bootstrap_model,
        'service_requests': scene['service_requests'],
        'submission_template': {
            'status': 'SYNTHETIC_NON_EXECUTABLE',
            'synthetic_handles': handles,
            'synthetic_host_pointers': host_pointers,
            'cpu_words': submission,
            'limitation': 'No target provider, service context, or Gate B authorization',
        },
        'source_containment': 'UNPROVED',
        'target_provider': 'CONDITIONAL',
        'hardware_ready': False,
        'gate_b': 'BLOCKED',
        'whitelist': [],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    print(json.dumps(build_report(), sort_keys=True, indent=2))


if __name__ == '__main__':
    main()
