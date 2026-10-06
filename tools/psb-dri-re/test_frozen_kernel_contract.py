"""Compile and exercise the offline fixed-request C port core."""
import pathlib
import ctypes
import struct
import subprocess
import tempfile
import unittest
import os
import shutil

import frozen_triangle_bo as bo
import frozen_triangle_image as image
import generate_frozen_kernel_initial as initial_gen


HERE = pathlib.Path(__file__).resolve().parent


class FrozenKernelContractTests(unittest.TestCase):
    def test_freestanding_i386_object_needs_no_division_runtime(self):
        sysroot = os.environ.get('SGX535_I686_SYSROOT')
        compiler = shutil.which('i686-linux-gnu-gcc-14')
        nm = shutil.which('i686-linux-gnu-nm')
        if not sysroot or not compiler or not nm:
            self.skipTest('requires the qualified i686 toolchain environment')
        with tempfile.TemporaryDirectory() as directory:
            obj = pathlib.Path(directory) / 'contract.o'
            build = subprocess.run([
                compiler, '--sysroot=' + sysroot, '-m32', '-march=i486',
                '-std=c11', '-O2', '-ffreestanding', '-c',
                str(HERE / 'frozen_kernel_contract.c'), '-o', str(obj)],
                capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            self.assertEqual(obj.read_bytes()[:7], b'\x7fELF\x01\x01\x01')
            undefined = subprocess.run([nm, '-u', str(obj)],
                                       capture_output=True, text=True)
            self.assertEqual(undefined.returncode, 0, undefined.stderr)
            self.assertNotIn('__umoddi3', undefined.stdout)
            self.assertNotIn('__udivdi3', undefined.stdout)

    def test_sparse_initial_image_matches_selected_cpu_producer(self):
        self.assertEqual((HERE / 'frozen_kernel_initial.inc').read_text(),
                         initial_gen.expected_text())
        scene = image.build()
        plan = bo.build(scene)
        names = list(plan['bos'])[:6]
        expected = bo.zero_user_backings(plan)
        for name, binding in plan['bindings'].items():
            if (binding['bo'] not in expected or binding['alias'] or
                    name == 'relocation_wire'):
                continue
            data = scene['objects'][name]['bytes']
            expected[binding['bo']][binding['offset']:
                    binding['offset'] + len(data)] = bytes(
                        value if value is not None else 0 for value in data)

        class View(ctypes.Structure):
            _fields_ = [('bytes', ctypes.POINTER(ctypes.c_uint8)),
                        ('length', ctypes.c_uint64)]
        with tempfile.TemporaryDirectory() as directory:
            library_path = pathlib.Path(directory) / 'contract.so'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', '-fPIC', '-shared', '-Wl,-z,defs',
                str(HERE / 'frozen_kernel_contract.c'), '-o', str(library_path),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            seed = ctypes.CDLL(str(library_path)).sgx535_frozen_initialize_user_images
            seed.argtypes = [ctypes.POINTER(View)]
            seed.restype = ctypes.c_int
            storage = {name: (ctypes.c_uint8 * len(expected[name]))(
                *([0xa5] * len(expected[name]))) for name in names}
            views = (View * 10)()
            for role, name in enumerate(names):
                views[role] = View(ctypes.cast(
                    storage[name], ctypes.POINTER(ctypes.c_uint8)),
                    len(storage[name]))
            views[0].length -= 1
            self.assertNotEqual(seed(views), 0)
            self.assertEqual(storage['pds'][0], 0xa5)
            views[0].length += 1
            views[1].bytes = views[0].bytes
            self.assertNotEqual(seed(views), 0)
            self.assertEqual(storage['pds'][0], 0xa5)
            views[1].bytes = ctypes.cast(
                storage['use'], ctypes.POINTER(ctypes.c_uint8))
            self.assertEqual(seed(views), 0)
            for name in names:
                self.assertEqual(bytes(storage[name]), bytes(expected[name]), name)

    def test_c_requirements_match_canonical_python_plan(self):
        plan = bo.build(image.build())
        domains = {'LOCAL': 0, 'PDS': 1, 'RASTGEOM': 2, 'MMU': 3}
        class Requirement(ctypes.Structure):
            _fields_ = [('size', ctypes.c_uint64),
                        ('alignment', ctypes.c_uint32),
                        ('domain', ctypes.c_uint32)]
        with tempfile.TemporaryDirectory() as directory:
            library_path = pathlib.Path(directory) / 'contract.so'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', '-fPIC', '-shared', '-Wl,-z,defs',
                str(HERE / 'frozen_kernel_contract.c'), '-o', str(library_path),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            get = ctypes.CDLL(str(library_path)).sgx535_frozen_get_requirement
            get.argtypes = [ctypes.c_uint32, ctypes.POINTER(Requirement)]
            get.restype = ctypes.c_int
            for role, spec in enumerate(plan['bos'].values()):
                out = Requirement()
                self.assertEqual(get(role, ctypes.byref(out)), 0)
                self.assertEqual((out.size, out.alignment, out.domain),
                                 (spec['size'], spec['alignment'],
                                  domains[spec['domain']]))
            self.assertEqual(get(10, ctypes.byref(Requirement())), -3)

    def test_fixed_request_and_bo_boundaries(self):
        with tempfile.TemporaryDirectory() as directory:
            binary = pathlib.Path(directory) / 'contract-test'
            command = [
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
                str(HERE / 'frozen_kernel_contract.c'),
                str(HERE / 'test_frozen_kernel_contract.c'),
                '-o', str(binary),
            ]
            build = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)

    def test_unaligned_wire_request_is_validated_without_undefined_behavior(self):
        with tempfile.TemporaryDirectory() as directory:
            source = pathlib.Path(directory) / 'unaligned-request.c'
            binary = pathlib.Path(directory) / 'unaligned-request'
            source.write_text('''#include "frozen_kernel_contract.h"
int main(void)
{
    sgx535_u32 storage[5] = {0};
    sgx535_u8 *wire = (sgx535_u8 *)storage + 1;
    wire[0] = wire[4] = 1;
    if (sgx535_frozen_validate_request(wire, 16) != SGX535_FROZEN_OK)
        return 1;
    wire[11] = 1; /* Nonzero flags, including their high byte, must reject. */
    if (sgx535_frozen_validate_request(wire, 16) != SGX535_FROZEN_BAD_REQUEST)
        return 2;
    wire[11] = 0;
    if (sgx535_frozen_validate_request(wire, 15) != SGX535_FROZEN_BAD_REQUEST)
        return 3;
    return 0;
}
''')
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', '-fsanitize=undefined', '-fno-sanitize-recover=all',
                '-I', str(HERE), str(HERE / 'frozen_kernel_contract.c'),
                str(source), '-o', str(binary),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            run = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run.returncode, 0, run.stderr)
            self.assertEqual(run.stderr, '')

    def test_exact_frozen_relocation_wire(self):
        plan = bo.build(image.build())
        raw = bo.relocation_bytes(plan)
        self.assertEqual(len(raw), 49 * 40)
        with tempfile.TemporaryDirectory() as directory:
            library_path = pathlib.Path(directory) / 'contract.so'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', '-fPIC', '-shared', '-Wl,-z,defs',
                str(HERE / 'frozen_kernel_contract.c'), '-o', str(library_path),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            library = ctypes.CDLL(str(library_path))
            check = library.sgx535_frozen_validate_relocations
            check.argtypes = [ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t]
            check.restype = ctypes.c_int
            words = (ctypes.c_uint32 * (49 * 10)).from_buffer_copy(raw)
            self.assertEqual(check(words, 49), 0)
            self.assertEqual(check(words, 48), -2)
            words[6] ^= 1
            self.assertEqual(check(words, 49), -11)
            words[6] ^= 1
            words[0], words[10] = words[10], words[0]
            self.assertEqual(check(words, 49), -11)
            self.assertEqual(check(None, 49), -11)

    def test_exact_ta_and_raster_register_offsets(self):
        scene = image.build()
        def offsets(name):
            raw = bytes(0 if value is None else value
                        for value in scene['objects'][name]['bytes'])
            return [struct.unpack_from('<I', raw, at)[0]
                    for at in range(0, len(raw), 8)]
        ta_values = offsets('ta_registers')
        raster_values = offsets('raster_registers')
        self.assertEqual((len(ta_values), len(raster_values)), (7, 26))
        with tempfile.TemporaryDirectory() as directory:
            library_path = pathlib.Path(directory) / 'contract.so'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', '-fPIC', '-shared', '-Wl,-z,defs',
                str(HERE / 'frozen_kernel_contract.c'), '-o', str(library_path),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            check = ctypes.CDLL(str(library_path)).sgx535_frozen_validate_register_offsets
            check.argtypes = [ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t,
                              ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t]
            check.restype = ctypes.c_int
            ta = (ctypes.c_uint32 * 7)(*ta_values)
            raster = (ctypes.c_uint32 * 26)(*raster_values)
            self.assertEqual(check(ta, 7, raster, 26), 0)
            self.assertEqual(check(ta, 6, raster, 26), -2)
            ta[0] ^= 4
            self.assertEqual(check(ta, 7, raster, 26), -12)
            ta[0] ^= 4
            raster[0], raster[1] = raster[1], raster[0]
            self.assertEqual(check(ta, 7, raster, 26), -12)
            self.assertEqual(check(None, 7, raster, 26), -12)

    def test_c_relocations_match_complete_python_user_bo_images(self):
        scene = image.build()
        plan = bo.build(scene)
        addresses = bo.test_addresses(plan)
        expected = bo.resolve(scene, plan, addresses, {0: 4, 1: 3},
                              0x80000000)
        names = list(plan['bos'])
        user_names = names[:6]
        initial = bo.zero_user_backings(plan)
        for name, binding in plan['bindings'].items():
            if (binding['bo'] not in initial or binding['alias'] or
                    name == 'relocation_wire'):
                continue
            data = scene['objects'][name]['bytes']
            self.assertIsNotNone(data)
            initial[binding['bo']][binding['offset']:
                    binding['offset'] + len(data)] = bytes(
                        value if value is not None else 0 for value in data)

        class Bo(ctypes.Structure):
            _fields_ = [('role', ctypes.c_uint32), ('domain', ctypes.c_uint32),
                        ('size', ctypes.c_uint64), ('gpu_va', ctypes.c_uint64),
                        ('owner_token', ctypes.c_uint64)]
        class View(ctypes.Structure):
            _fields_ = [('bytes', ctypes.POINTER(ctypes.c_uint8)),
                        ('length', ctypes.c_uint64)]
        class UseEntry(ctypes.Structure):
            _fields_ = [('reg', ctypes.c_uint32), ('base', ctypes.c_uint32),
                        ('register_offset', ctypes.c_uint32),
                        ('register_word', ctypes.c_uint32)]
        class UsePlan(ctypes.Structure):
            _fields_ = [('by_data_master', UseEntry * 2),
                        ('assigned_count', ctypes.c_uint32)]
        domains = {'LOCAL': 0, 'PDS': 1, 'RASTGEOM': 2, 'MMU': 3}
        bos = (Bo * 10)(*(Bo(i, domains[plan['bos'][name]['domain']],
                              plan['bos'][name]['size'], addresses[name], i + 1)
                          for i, name in enumerate(names)))
        storage = {name: (ctypes.c_uint8 * len(initial[name])).from_buffer_copy(
            initial[name]) for name in user_names}
        views = (View * 10)()
        for i, name in enumerate(user_names):
            views[i] = View(ctypes.cast(storage[name],
                                        ctypes.POINTER(ctypes.c_uint8)),
                            len(storage[name]))
        registers = (ctypes.c_uint32 * 2)(4, 3)
        with tempfile.TemporaryDirectory() as directory:
            library_path = pathlib.Path(directory) / 'contract.so'
            build = subprocess.run([
                'cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                '-pedantic', '-fPIC', '-shared', '-Wl,-z,defs',
                str(HERE / 'frozen_kernel_contract.c'), '-o', str(library_path),
            ], capture_output=True, text=True)
            self.assertEqual(build.returncode, 0, build.stderr)
            library = ctypes.CDLL(str(library_path))
            planner = library.sgx535_frozen_plan_use_bases
            planner.argtypes = [ctypes.POINTER(Bo), ctypes.c_size_t,
                                ctypes.c_uint64, ctypes.POINTER(UsePlan)]
            planner.restype = ctypes.c_int
            selected_use = UsePlan()
            self.assertEqual(planner(bos, 10, 0x80000000,
                                     ctypes.byref(selected_use)), 0)
            self.assertEqual(selected_use.assigned_count, 2)
            self.assertEqual(tuple(selected_use.by_data_master[i].reg
                                   for i in range(2)), (4, 3))
            apply = library.sgx535_frozen_apply_relocations
            apply.argtypes = [ctypes.POINTER(Bo), ctypes.c_size_t,
                              ctypes.c_uint64, ctypes.POINTER(View),
                              ctypes.POINTER(ctypes.c_uint32)]
            apply.restype = ctypes.c_int
            before = bytes(storage['pds'])
            registers[1] = 4
            self.assertEqual(apply(bos, 10, 0x80000000, views, registers), -11)
            self.assertEqual(bytes(storage['pds']), before)
            registers[1] = 3
            bos[0].gpu_va += 1
            self.assertEqual(apply(bos, 10, 0x80000000, views, registers), -7)
            self.assertEqual(bytes(storage['pds']), before)
            bos[0].gpu_va -= 1
            views[1].bytes = views[0].bytes
            self.assertEqual(apply(bos, 10, 0x80000000, views, registers), -8)
            self.assertEqual(bytes(storage['pds']), before)
            views[1].bytes = ctypes.cast(storage['use'],
                                         ctypes.POINTER(ctypes.c_uint8))
            self.assertEqual(apply(bos, 10, 0x80000000, views, registers), 0)
        for name in user_names:
            self.assertEqual(bytes(storage[name]), bytes(expected[name]), name)
        shifted = {name: value + 0x100000 if value else 0
                   for name, value in addresses.items()}
        shifted_expected = bo.resolve(scene, plan, shifted, {0: 4, 1: 3},
                                      0x80000000)
        for role, name in enumerate(names):
            bos[role].gpu_va = shifted[name]
        for name in user_names:
            ctypes.memmove(storage[name], bytes(initial[name]),
                           len(initial[name]))
        self.assertEqual(apply(bos, 10, 0x80000000, views, registers), 0)
        for name in user_names:
            self.assertEqual(bytes(storage[name]),
                             bytes(shifted_expected[name]), name)


if __name__ == '__main__':
    unittest.main()
