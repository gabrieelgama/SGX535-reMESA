from pathlib import Path
import hashlib, importlib.util, json, os, resource, subprocess, tempfile

r = Path('/home/gama/sgx535-gfx'); w = Path(__file__).parent
bundle = r/'docs/phase8/artifacts/candidate-01-build-20260930'
previous = Path('/home/gama/sgx535-offline/candidate-01-build-20260930')
output = Path('/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output')
h = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
for row in json.loads((bundle/'fixed-inputs-lifecycle.json').read_text()):
    assert h(r/row['repository_path']) == row['sha256'], row['repository_path']
for filename, digest in json.loads((bundle/'integrated-source-manifest-lifecycle-final.json').read_text()).items():
    assert h(previous/'module'/filename) == digest, filename
state = json.loads((previous/'prepared-state-before.json').read_text())
for name, digest in state.items(): assert h(output/name) == digest, name
assert h(output/'Module.symvers') == 'faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
applications = []
with tempfile.TemporaryDirectory() as temporary:
    directory = Path(temporary)
    for name in ('Makefile', 'psb_drv.c', 'psb_irq.c'):
        fixture = 'original-Makefile' if name == 'Makefile' else name
        (directory/name).write_bytes((r/'docs/phase8/artifacts/candidate-01-20260930'/fixture).read_bytes())
    for name in ('antix-fixed-makefile.patch', 'antix-fixed-irq.patch', 'antix-irq-lifecycle.patch', 'antix-fixed-ioctl.patch'):
        argv = ['patch', '--batch', '--fuzz=0', '-p5', '-i', str(r/'kernel/sgx535_frozen/patches'/name)]
        for dry in (True, False):
            cp = subprocess.run(argv+(['--dry-run'] if dry else []), cwd=directory, capture_output=True, text=True)
            applications.append({'patch': name, 'dry': dry, 'exit': cp.returncode, 'stdout': cp.stdout, 'stderr': cp.stderr})
            assert cp.returncode == 0 and not cp.stderr and 'offset' not in cp.stdout and 'fuzz' not in cp.stdout and 'FAILED' not in cp.stdout
    for name in ('Makefile', 'psb_drv.c', 'psb_irq.c'):
        assert (directory/name).read_bytes() == (bundle/'candidate-02-lifecycle'/('integrated-'+name)).read_bytes()
(w/'strict-application-and-provenance.json').write_text(json.dumps({'patches': applications, 'fixed_inputs': 15, 'prepared_state_unchanged': len(state), 'result': 'PASS'}, indent=2)+'\n')

spec = importlib.util.spec_from_file_location('irqtest', r/'tools/psb-dri-re/test_frozen_irq_lifecycle.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
m.IRQLifecycleTests.setUpClass()
directory = m.IRQLifecycleTests.binary.parent
source = directory/'extracted-lifecycle.inc'; baseline = source.read_text()
# Only ephemeral extracted code is mutated, never a production patch/artifact.
mutations = {
    'omit-owned-irq-release': baseline.replace('\t\tif (dev->irq_enabled)\n\t\t\tdrm_irq_uninstall(dev);', '', 1),
    'omit-pci-remove-unload': baseline.replace('\tdrm_dev_unregister(dev);\n\tpsb_driver_unload(dev);', '\tdrm_dev_unregister(dev);', 1),
}
results = []
for name, text in mutations.items():
    assert text != baseline
    source.write_text(text)
    binary = directory/name
    argv = ['/usr/bin/gcc-14', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
            '-Wno-unused-function', '-Wno-unused-parameter', '-Wno-unused-label',
            '-fsanitize=undefined', '-fno-sanitize-recover=all', '-I', str(directory),
            str(r/'tools/psb-dri-re/test_frozen_irq_lifecycle.c'), '-o', str(binary)]
    cp = subprocess.run(argv, capture_output=True, text=True); assert cp.returncode == 0, cp.stderr
    cp = subprocess.run([str(binary), '10'], capture_output=True, text=True,
                        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
    assert cp.returncode == -6 and 'Assertion' in cp.stderr, (name, cp.returncode, cp.stderr)
    (w/(name+'.stderr')).write_text(cp.stderr)
    results.append({'mutation': name, 'compile_argv': argv, 'scenario': 10, 'exit': cp.returncode, 'result': 'REJECTED as required'})
source.write_text(baseline)
(w/'extracted-lifecycle.inc').write_text(baseline)
(w/'extracted-irq-callback.inc').write_text((directory/'extracted-irq-callback.inc').read_text())
m.IRQLifecycleTests.doClassCleanups()
(w/'lifecycle-mutation-check.json').write_text(json.dumps(results, indent=2)+'\n')
print('Strict integration and 7,426-state drift check PASS; both PCI-removal mutations rejected')
