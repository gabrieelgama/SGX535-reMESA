"""Execute actual gma500 cleanup/install control flow at kernel boundaries."""
import pathlib
import subprocess
import tempfile
import unittest
import resource
import re

HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parents[1]
FIXTURE = ROOT / 'docs/phase8/artifacts/candidate-01-20260930/psb_drv.c'
PATCH = ROOT / 'kernel/sgx535_frozen/patches/antix-irq-lifecycle.patch'


def extract_function(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


class IRQLifecycleTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        directory = pathlib.Path(cls.tmp.name)
        target = directory / 'psb_drv.c'
        target.write_bytes(FIXTURE.read_bytes())
        irq_target = directory / 'psb_irq.c'
        irq_target.write_bytes((FIXTURE.parent / 'psb_irq.c').read_bytes())
        irq_patch = ROOT / 'kernel/sgx535_frozen/patches/antix-fixed-irq.patch'
        # The lifecycle patch consumes the retained fixed-IRQ result and exact
        # unmodified driver result, before the separate ioctl patch is applied.
        for patch in (irq_patch, PATCH):
            if not patch.exists():  # Absence exercises original code in RED.
                continue
            argv = ['patch', '--batch', '--fuzz=0', '-p5', '-i', str(patch)]
            for dry in (True, False):
                run = subprocess.run(argv + (['--dry-run'] if dry else []),
                                     cwd=directory, capture_output=True, text=True)
                if run.returncode or 'offset' in run.stdout or 'fuzz' in run.stdout:
                    raise AssertionError(run.stdout + run.stderr)
        contract = ROOT / 'docs/phase8/artifacts/candidate-01-build-20260930/source-contracts'
        definitions = []
        for header, names in (
            ('drivers/gpu/drm/gma500/psb_drv.h',
             ['PSB_HWSTAM', 'PSB_INT_IDENTITY_R', 'PSB_INT_MASK_R',
              'PSB_INT_ENABLE_R', '_PSB_IRQ_SGX_FLAG', '_PSB_IRQ_MSVDX_FLAG',
              '_LNC_IRQ_TOPAZ_FLAG']),
            ('drivers/gpu/drm/gma500/psb_intel_reg.h', ['PIPE_VBLANK_INTERRUPT_ENABLE'])):
            text = (contract / header).read_text()
            for name in names:
                definitions.append(re.search(r'^#define ' + name + r'\s+[^\n]+',
                                              text, re.MULTILINE).group(0))
        (directory / 'extracted-irq-definitions.inc').write_text('\n'.join(definitions) + '\n')
        callback = extract_function(irq_target.read_text(), 'void psb_irq_uninstall(')
        (directory / 'extracted-irq-callback.inc').write_text(callback)
        source = target.read_text()
        unload = extract_function(source, 'static void psb_driver_unload(')
        load = extract_function(source, 'static int psb_driver_load(')
        error_tail = load[load.rindex('\nout_err:'):-1]
        call = source.index('drm_irq_install(dev, dev->pdev->irq);')
        start = source.rfind('\n', 0, call) + 1
        end = source.rfind('\n', 0, source.index('dev->max_vblank_count', call))
        actual_statements = source[start:end]
        install = ('static int test_actual_install(struct drm_device *dev) {\n'
                   'int ret = 0;\n' + actual_statements +
                   '\nnext_phase_seen = 1; return 0;\n'
                   + error_tail + '\n}\n')
        late_end = source.index('psb_intel_opregion_enable_asle(dev);')
        late_start = source.rfind('\n\tif (ret)', 0, late_end)
        late = ('static int test_actual_late_init(struct drm_device *dev, int ret) {\n'
                + source[late_start:late_end] +
                'next_phase_seen = 1; return 0;\n'
                + error_tail + '\n}\n')
        (directory / 'extracted-lifecycle.inc').write_text(unload + '\n' + install + late)
        cls.binary = directory / 'lifecycle-test'
        build = subprocess.run([
            '/usr/bin/gcc-14', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
            '-Wno-unused-function', '-Wno-unused-parameter', '-Wno-unused-label',
            '-fsanitize=undefined', '-fno-sanitize-recover=all', '-I', str(directory),
            str(HERE / 'test_frozen_irq_lifecycle.c'), '-o', str(cls.binary)],
            capture_output=True, text=True)
        if build.returncode:
            raise AssertionError(build.stderr)

    def run_case(self, case):
        run = subprocess.run([str(self.binary), str(case)],
                             capture_output=True, text=True,
                             preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
        self.assertEqual(run.returncode, 0, run.stderr)

    def test_active_irq_quiesces_vblank_before_driver_resources_are_freed(self):
        self.run_case(0)

    def test_no_irq_and_no_private_state_do_not_uninstall_unowned_irq(self):
        self.run_case(1)
        self.run_case(3)

    def test_registered_irq_before_modeset_is_released_without_crtc_iteration(self):
        self.run_case(2)

    def test_install_error_returns_before_modeset_and_uses_existing_cleanup(self):
        self.run_case(4)
        self.run_case(5)

    def test_actual_uninstall_masks_all_sources_before_irq_handler_release(self):
        self.run_case(8)

    def test_direct_suspend_retains_the_existing_non_display_irq_mask(self):
        self.run_case(9)

    def test_late_probe_error_unwinds_registered_irq_and_private_state(self):
        self.run_case(6)
        self.run_case(7)


if __name__ == '__main__':
    unittest.main()
