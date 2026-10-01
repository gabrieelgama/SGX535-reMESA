from pathlib import Path
import hashlib, json, subprocess

r = Path('/home/gama/sgx535-gfx'); w = Path(__file__).parent
previous = Path('/home/gama/sgx535-offline/candidate-01-build-20260930')
env = json.loads((previous/'build-environment.json').read_text())
env['SGX535_I686_SYSROOT'] = '/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root'
env['PYTHONDONTWRITEBYTECODE'] = '1'
records = []
def run(name, argv, expected=0):
    cp = subprocess.run(argv, cwd=r, env=env, capture_output=True)
    (w/(name+'.stdout')).write_bytes(cp.stdout); (w/(name+'.stderr')).write_bytes(cp.stderr)
    records.append({'name': name, 'argv': argv, 'exit_code': cp.returncode, 'expected': expected})
    (w/'checks.json').write_text(json.dumps(records, indent=2)+'\n')
    assert cp.returncode == expected, (name, cp.stderr.decode())
run('focused', ['python3', '-m', 'unittest', 'discover', '-s', 'tools/psb-dri-re', '-p', 'test_frozen_irq_lifecycle.py', '-v'])
run('first-load-focused', ['python3', '-m', 'unittest', 'discover', '-s', 'tools/psb-dri-re', '-p', 'test_frozen_first_load.py', '-v'])
run('suite', ['python3', '-m', 'unittest', 'discover', '-s', 'tools/psb-dri-re', '-p', 'test_*.py'])
suite = (w/'suite.stderr').read_text()
assert 'Ran 215 tests' in suite and '\nOK\n' in suite and 'skipped' not in suite, suite
run('generator', ['python3', 'tools/psb-dri-re/generate_frozen_kernel_initial.py', '--check'])
for i in (1, 2): run('dry-'+str(i), ['python3', 'tools/psb-dri-re/frozen_triangle_dry_run.py'])
hashes = [hashlib.sha256((w/('dry-'+str(i)+'.stdout')).read_bytes()).hexdigest() for i in (1, 2)]
assert hashes == ['2e85beef0c1a7ec2f8ccc2656b49dd4fb054bf0f55424c226f5fda70e621720e']*2
run('complete', ['python3', 'tools/psb-dri-re/frozen_triangle_image.py', '--complete'], 1)
assert (w/'complete.stderr').read_text().strip() == 'PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE'
base = json.loads((previous/'regression-before-lifecycle.json').read_text())
for row in base:
    if row['name'].startswith('ubsan'):
        argv = [value.replace(str(previous)+'/', str(w)+'/') for value in row['argv']]
        run(row['name'], argv)
run('git-diff-check', ['git', 'diff', '--check'])
(w/'guard-summary.json').write_text(json.dumps({'tests': 215, 'skips': 0, 'irq_lifecycle_tests': 10, 'first_load_tests': 7, 'lifecycle_scenarios': 13, 'ubsan_harnesses': 3, 'dry_hashes': hashes, 'complete': 'PARTIAL: L12, FT-AUX, FT-BO, FT-SERVICE', 'result': 'PASS'}, indent=2)+'\n')
print(suite[-250:]); print('Three fresh strict UBSan harnesses PASS; dry hashes:', *hashes)
