from pathlib import Path
import hashlib, importlib.util, json, re, struct, subprocess

r = Path('/home/gama/sgx535-gfx')
w = Path(__file__).parent
b = r / 'docs/phase8/artifacts/candidate-01-build-20260930'
previous = Path('/home/gama/sgx535-offline/candidate-01-build-20260930')
env = json.loads((previous / 'build-environment.json').read_text())
env['PYTHONDONTWRITEBYTECODE'] = '1'
table = r / 'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers'
h = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
assert h(table) == 'faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
spec = importlib.util.spec_from_file_location('versions', r / 'tools/psb-dri-re/frozen_module_versions.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
original, _ = m.read_versions(r / 'docs/hardware-evidence/MINI12-20260927-H0/raw/installed-gma500_gfx.ko')
reports = []
for label, size, digest, bid, count, repeat in [
    ('candidate-01', 242364, '13591674ef9fa82f185f075185d1fa09d94606ccce7253ed231627f2649c600f', 'd72f19c97c5fd1feffddad07634e12333562ae60', 230, previous / 'build-3-candidate01-repeat/gma500_gfx.ko'),
    ('candidate-02-lifecycle', 242724, '91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74', '074c650d48ccb463e37e90428b486dccdb23eb80', 232, previous / 'module/gma500_gfx.ko')]:
    ko = b / label / 'gma500_gfx.ko'; data = ko.read_bytes()
    assert len(data) == size and h(ko) == digest and data == repeat.read_bytes()
    assert data[:7] == b'\x7fELF\x01\x01\x01' and struct.unpack_from('<HH', data, 16) == (1, 3)
    outputs = {}
    commands = {}
    for name, argv in [
        ('readelf', [env['CROSS_COMPILE']+'readelf', '-hnSW', str(ko)]),
        ('nm', [env['CROSS_COMPILE']+'nm', '-u', str(ko)]),
        ('modinfo', ['/usr/sbin/modinfo', str(ko)]),
        ('vermagic', ['/usr/sbin/modinfo', '-F', 'vermagic', str(ko)]),
        ('versions', ['python3', str(r/'tools/psb-dri-re/frozen_module_versions.py'), '--check-symvers', str(ko), str(table)]),
        ('objdump', [env['CROSS_COMPILE']+'objdump', '-dr', str(ko)])]:
        cp = subprocess.run(argv, env=env, capture_output=True, text=True)
        (w/(label+'-'+name+'.stdout')).write_text(cp.stdout)
        (w/(label+'-'+name+'.stderr')).write_text(cp.stderr)
        commands[name] = {'argv': argv, 'exit_code': cp.returncode}
        assert cp.returncode == 0, (label, name, cp.stderr)
        outputs[name] = cp.stdout
    assert re.search('Build ID: '+bid, outputs['readelf'])
    assert outputs['vermagic'].rstrip('\n') == '5.10.240-antix.1-486-smp SMP mod_unload modversions 486 '
    check = json.loads(outputs['versions'])
    assert check['mismatched_count'] == 0 and not check['missing_imports']
    versions, _ = m.read_versions(ko)
    assert len(versions) == count and versions['module_layout'] == 0xb84efb99
    undefined = {line.split()[-1] for line in outputs['nm'].splitlines()}
    assert undefined == set(versions)-{'module_layout'}
    reports.append({'label': label, 'path': str(ko), 'sha256': digest, 'size': size,
                    'build_id': bid, 'module_layout': '0xb84efb99', 'imports': count,
                    'missing': 0, 'mismatched': 0, 'actual_undefined': len(undefined),
                    'extra_imports_vs_original': sorted(set(versions)-set(original)),
                    'removed_imports': sorted(set(original)-set(versions)),
                    'repeat_path': str(repeat), 'repeat': 'BIT IDENTICAL', 'commands': commands})
assert set(reports[1]['extra_imports_vs_original'])-set(reports[0]['extra_imports_vs_original']) == {'drm_irq_uninstall', 'drm_kms_helper_poll_disable'}
(w/'artifact-verification.json').write_text(json.dumps(reports, indent=2)+'\n')
print('Frozen hashes, ELF/vermagic/build IDs, complete imports, CRCs, repeats: PASS for both artifacts')
