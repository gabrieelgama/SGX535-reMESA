"""Apply the fixed integration to verified source fixtures, without fuzz."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
FIXTURE = ROOT / 'docs/phase8/artifacts/candidate-01-20260930'
PATCHES = ROOT / 'kernel/sgx535_frozen/patches'


class FixedIntegrationTests(unittest.TestCase):
    def apply_patch(self, filename, patch_name, source):
        with tempfile.TemporaryDirectory() as directory:
            target = pathlib.Path(directory) / filename
            target.write_bytes(source)
            argv = ['patch', '--batch', '--fuzz=0', '-p5', '-i',
                    str(PATCHES / patch_name)]
            dry = subprocess.run(argv + ['--dry-run'], cwd=directory,
                                 capture_output=True, text=True)
            self.assertEqual(dry.returncode, 0, dry.stdout + dry.stderr)
            actual = subprocess.run(argv, cwd=directory,
                                    capture_output=True, text=True)
            self.assertEqual(actual.returncode, 0, actual.stdout + actual.stderr)
            for result in (dry, actual):
                self.assertNotIn('offset', result.stdout)
                self.assertNotIn('fuzz', result.stdout)
            self.assertFalse(list(pathlib.Path(directory).glob('*.rej')))
            return target.read_bytes()

    def test_makefile_appends_only_the_fixed_object_list_at_real_eof(self):
        source = (FIXTURE / 'original-Makefile').read_bytes()
        expected = source + (
            b'\n# Exact fixed-scene path; the separately reviewed ioctl patch '
            b'installs its only caller.\n'
            b'gma500_gfx-y += gma500_bo_owner.o frozen_kernel_contract.o \\\n'
            b'                 frozen_fixed_service.o frozen_fixed_io.o \\\n'
            b'                 gma500_fixed_backend.o gma500_fixed_entry.o\n')
        self.assertEqual(self.apply_patch('Makefile', 'antix-fixed-makefile.patch',
                                         source), expected)

    def test_ioctl_integrates_only_fixed_headers_and_root_only_entry(self):
        source = (FIXTURE / 'psb_drv.c').read_bytes()
        expected = source.replace(
            b'#include "psb_drv.h"\n',
            b'#include "psb_drv.h"\n#include "gma500_fixed_entry.h"\n'
            b'#include "gma500_fixed_uapi.h"\n', 1).replace(
            b'static const struct drm_ioctl_desc psb_ioctls[] = {\n};',
            b'static const struct drm_ioctl_desc psb_ioctls[] = {\n'
            b'\tDRM_IOCTL_DEF_DRV(PSB_FIXED_TRIANGLE, sgx535_gma500_fixed_ioctl,\n'
            b'\t\t\t   DRM_ROOT_ONLY),\n};', 1)
        self.assertEqual(self.apply_patch('psb_drv.c', 'antix-fixed-ioctl.patch',
                                         source), expected)

    def test_irq_retains_only_the_reviewed_capture_and_lock_integration(self):
        source = (FIXTURE / 'psb_irq.c').read_bytes()
        expected = source.replace(
            b'#include "psb_reg.h"\n', b'#include "psb_reg.h"\n'
            b'#include "gma500_fixed_backend.h"\n', 1).replace(
            b'\tu32 sgx_stat_1, sgx_stat_2;\n',
            b'\tu32 sgx_stat_1, sgx_stat_2;\n\tunsigned long fixed_flags;\n', 1
        ).replace(b'\tif (sgx_int) {\n', b'\tif (sgx_int) {\n'
                  b'\t\tfixed_flags = sgx535_gma500_fixed_irq_lock();\n', 1
        ).replace(b'\t\tpsb_sgx_interrupt(dev, sgx_stat_1, sgx_stat_2);\n',
                  b'\t\tsgx535_gma500_fixed_irq_capture_locked(dev, sgx_stat_1,\n'
                  b'\t\t\t\t\t\t   sgx_stat_2);\n'
                  b'\t\tpsb_sgx_interrupt(dev, sgx_stat_1, sgx_stat_2);\n'
                  b'\t\tsgx535_gma500_fixed_irq_unlock(fixed_flags);\n', 1)
        self.assertTrue(expected.endswith(b'\n\n'))
        expected = expected[:-1]  # Retained IRQ patch's already reviewed EOF.
        self.assertEqual(self.apply_patch('psb_irq.c', 'antix-fixed-irq.patch',
                                         source), expected)

    def test_changed_ioctl_source_is_rejected_without_fuzz(self):
        source = (FIXTURE / 'psb_drv.c').read_bytes().replace(
            b'#include "psb_drv.h"', b'#include "different_drv.h"', 1)
        with tempfile.TemporaryDirectory() as directory:
            (pathlib.Path(directory) / 'psb_drv.c').write_bytes(source)
            result = subprocess.run([
                'patch', '--batch', '--fuzz=0', '-p5', '--dry-run', '-i',
                str(PATCHES / 'antix-fixed-ioctl.patch')], cwd=directory,
                capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)


if __name__ == '__main__':
    unittest.main()
