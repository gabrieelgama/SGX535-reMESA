"""CPU-only fault injection of an unqualified prospective client's output block.

No device open, ioctl, invocation flag, selected artifact, or GPU evidence is used.
"""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
SOURCE = HERE / 'frozen_triangle_one_shot_prospective.c'
CPU_BYTES = bytes(i % 251 for i in range(4096))
HARNESS_HEAD = r"""#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct synthetic_request { unsigned char color_bytes[4096]; };
static const char *mode, *allowed_path;
static unsigned write_calls, fsync_calls, close_calls, open_calls, synced_before_close;
static int controlled_open(const char *path, int flags, ...)
{
    va_list args;
    int permissions;
    open_calls++;
    /* Reject every path except the temporary output; never open a device. */
    if (strcmp(path, allowed_path) || strncmp(path, "/dev/", 5) == 0) {
        errno = EACCES;
        return -1;
    }
    va_start(args, flags);
    permissions = va_arg(args, int);
    va_end(args);
    return open(path, flags, permissions);
}
static ssize_t controlled_write(int fd, const void *data, size_t count)
{
    write_calls++;
    if (!strcmp(mode, "zero")) { errno = 0; return 0; }
    if (!strcmp(mode, "permanent") ||
        (!strcmp(mode, "partial_permanent") && write_calls > 1)) {
        errno = EIO;
        return -1;
    }
    if (write_calls == 1) {
        if (!strcmp(mode, "short") || !strcmp(mode, "partial_permanent"))
            return write(fd, data, 1024);
        if (!strcmp(mode, "eintr")) { errno = EINTR; return -1; }
    }
    return write(fd, data, count);
}
static int controlled_fsync(int fd)
{
    fsync_calls++;
    if (!strcmp(mode, "fsync_failure")) { errno = EIO; return -1; }
    return fsync(fd);
}
static int controlled_close(int fd)
{
    int result;
    close_calls++;
    synced_before_close = fsync_calls > 0;
    result = close(fd);
    if (!strcmp(mode, "close_failure")) { errno = EIO; return -1; }
    return result;
}
#define open controlled_open
#define write controlled_write
#define fsync controlled_fsync
#define close controlled_close
static int preserve_output(struct synthetic_request request, const char *path)
{
    int out;
    ssize_t written;
    const char *argv[] = {"cpu-only-output-harness", "unused", path};
"""
HARNESS_TAIL = r"""}
#undef open
#undef write
#undef fsync
#undef close
int main(int argc, char **argv)
{
    struct synthetic_request request;
    size_t i;
    int result;
    if (argc != 3) return 99;
    mode = argv[1];
    allowed_path = argv[2];
    for (i = 0; i < sizeof(request.color_bytes); i++)
        request.color_bytes[i] = (unsigned char)(i % 251);
    errno = 0;
    result = preserve_output(request, allowed_path);
    printf("{\"result\":%d,\"write_calls\":%u,\"fsync_calls\":%u,"
           "\"close_calls\":%u,\"open_calls\":%u,\"synced_before_close\":%u,"
           "\"ioctl_calls\":0}\n",
           result, write_calls, fsync_calls, close_calls, open_calls, synced_before_close);
    return 0;
}
"""


class ProspectivePreservationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix='sgx535-cpu-preservation-')
        cls.addClassCleanup(cls.directory.cleanup)
        cls.root = Path(cls.directory.name)
        # Compile the actual prospective output statements, never a copied fix.
        source = SOURCE.read_text()
        start = source.index('    out = open(argv[2],')
        end = source.rindex('    return 0;') + len('    return 0;\n')
        block = source[start:end]
        cls.harness = cls.root / 'output-harness.c'
        cls.harness.write_text(HARNESS_HEAD + block + HARNESS_TAIL)
        cls.binary = cls.root / 'output-harness'
        build = subprocess.run(
            ['cc', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-pedantic',
             # The initial unmodified source does not yet call fsync.
             '-Wno-unused-function', str(cls.harness), '-o', str(cls.binary)],
            capture_output=True, text=True)
        if build.returncode:
            raise AssertionError(build.stderr)

    def run_output(self, mode, path):
        run = subprocess.run([str(self.binary), mode, str(path)],
                             capture_output=True, text=True, timeout=2)
        self.assertEqual(run.returncode, 0, run.stderr)
        report = json.loads(run.stdout)
        self.assertEqual(report['ioctl_calls'], 0)
        self.assertEqual(report['open_calls'], 1)
        return report, run.stderr

    def test_full_write_preserves_all_original_bytes(self):
        path = self.root / 'full.bin'
        report, error = self.run_output('full', path)
        self.assertEqual(path.read_bytes(), CPU_BYTES)
        self.assertEqual(report['result'], 0)
        self.assertEqual(report['write_calls'], 1)
        self.assertEqual(report['close_calls'], 1)
        self.assertEqual(error, '')

    def test_recoverable_short_write_preserves_all_original_bytes(self):
        path = self.root / 'short.bin'
        report, error = self.run_output('short', path)
        self.assertEqual(path.read_bytes(), CPU_BYTES)
        self.assertEqual(report['result'], 0)
        self.assertEqual(report['write_calls'], 2)
        self.assertEqual(error, '')

    def test_interrupted_write_preserves_all_original_bytes(self):
        path = self.root / 'eintr.bin'
        report, error = self.run_output('eintr', path)
        self.assertEqual(path.read_bytes(), CPU_BYTES)
        self.assertEqual(report['result'], 0)
        self.assertEqual(report['write_calls'], 2)
        self.assertEqual(error, '')

    def test_zero_write_fails_without_false_success_or_loop(self):
        path = self.root / 'zero.bin'
        report, error = self.run_output('zero', path)
        self.assertEqual(report['result'], 2)
        self.assertEqual(report['write_calls'], 1)
        self.assertEqual(report['close_calls'], 1)
        self.assertEqual(path.read_bytes(), b'')
        self.assertIn('write color output', error)
        self.assertNotIn('Success', error)

    def test_permanent_write_error_reports_failure(self):
        path = self.root / 'permanent.bin'
        report, error = self.run_output('permanent', path)
        self.assertEqual(report['result'], 2)
        self.assertEqual(report['write_calls'], 1)
        self.assertEqual(report['close_calls'], 1)
        self.assertEqual(path.read_bytes(), b'')
        self.assertIn('write color output', error)
        self.assertNotIn('Success', error)

    def test_write_error_preserves_partial_original_bytes(self):
        path = self.root / 'partial.bin'
        report, error = self.run_output('partial_permanent', path)
        self.assertEqual(report['result'], 2)
        self.assertEqual(report['write_calls'], 2)
        self.assertEqual(report['close_calls'], 1)
        self.assertEqual(path.read_bytes(), CPU_BYTES[:1024])
        self.assertIn('write color output', error)
        self.assertNotIn('Success', error)

    def test_fsync_failure_reports_failure_and_preserves_bytes(self):
        path = self.root / 'fsync.bin'
        report, error = self.run_output('fsync_failure', path)
        self.assertEqual(path.read_bytes(), CPU_BYTES)
        self.assertEqual(report['result'], 2)
        self.assertEqual(report['fsync_calls'], 1)
        self.assertEqual(report['close_calls'], 1)
        self.assertIn('fsync color output', error)

    def test_close_failure_reports_failure_and_preserves_bytes(self):
        path = self.root / 'close.bin'
        report, error = self.run_output('close_failure', path)
        self.assertEqual(path.read_bytes(), CPU_BYTES)
        self.assertEqual(report['result'], 2)
        self.assertEqual(report['fsync_calls'], 1)
        self.assertEqual(report['close_calls'], 1)
        self.assertIn('close color output', error)

    def test_complete_write_is_synced_before_close(self):
        path = self.root / 'synced.bin'
        report, error = self.run_output('full', path)
        self.assertEqual(report['result'], 0)
        self.assertEqual(report['fsync_calls'], 1)
        self.assertEqual(report['close_calls'], 1)
        self.assertEqual(report['synced_before_close'], 1)
        self.assertEqual(error, '')

    def test_exclusive_creation_preserves_existing_file_and_link_targets(self):
        for kind in ('file', 'hardlink', 'symlink'):
            with self.subTest(kind=kind):
                original = self.root / (kind + '-original.bin')
                original.write_bytes(b'PRESERVED ORIGINAL EVIDENCE')
                path = original
                if kind == 'hardlink':
                    path = self.root / 'output-hardlink.bin'
                    os.link(original, path)
                elif kind == 'symlink':
                    path = self.root / 'output-symlink.bin'
                    path.symlink_to(original)
                report, error = self.run_output('full', path)
                self.assertEqual(report['result'], 2)
                self.assertEqual(report['write_calls'], 0)
                self.assertEqual(report['fsync_calls'], 0)
                self.assertEqual(report['close_calls'], 0)
                self.assertEqual(original.read_bytes(), b'PRESERVED ORIGINAL EVIDENCE')
                self.assertEqual(path.read_bytes(), b'PRESERVED ORIGINAL EVIDENCE')
                self.assertIn('open color output', error)
                if kind == 'symlink':
                    self.assertTrue(path.is_symlink())


if __name__ == '__main__':
    unittest.main()
