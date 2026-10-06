"""Native CPU-only post-call export tests; every device operation is mocked.

Real file I/O occurs only within TemporaryDirectory. The historical approved
client is an unchanged differential baseline, not an artifact being requalified.
"""
import importlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
NEW = HERE / 'frozen_triangle_one_shot_response.c'
BASE = HERE / 'frozen_triangle_one_shot_prospective.c'

HARNESS = r'''
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
static int controlled_open(const char *, int, ...);
static int controlled_ioctl(int, unsigned long, ...);
static int controlled_close(int);
static int controlled_fsync(int);
static ssize_t controlled_write(int, const void *, size_t);
#define main client_main
#define open controlled_open
#define ioctl controlled_ioctl
#define close controlled_close
#define write controlled_write
#define fsync controlled_fsync
#include "CLIENT_SOURCE"
#undef main
#undef open
#undef ioctl
#undef close
#undef write
#undef fsync
static const char *mode, *image_path, *response_path;
static int calls, card_opens, card_closes, card_flags, raw_opens, image_opens, device_order;
static int raw_fd = -1, raw_writes, raw_syncs, before_ok, after_unchanged;
static unsigned long ioctl_number;
static struct sgx535_fixed_ioctl before, after, *live;
static int controlled_open(const char *path, int flags, ...)
{
    int permissions, fd;
    va_list args;
    if (!strcmp(path, "/dev/dri/card0")) {
        card_opens++; card_flags = flags; device_order = device_order * 10 + 1; return 123456;
    }
    if (strcmp(path, image_path) && strcmp(path, response_path)) {
        errno = EACCES; return -1;
    }
    va_start(args, flags); permissions = va_arg(args, int); va_end(args);
    fd = open(path, flags, permissions);
    if (!strcmp(path, response_path)) { raw_opens++; raw_fd = fd; }
    else image_opens++;
    return fd;
}
static int controlled_ioctl(int fd, unsigned long code, ...)
{
    size_t i;
    va_list args;
    struct sgx535_fixed_ioctl *request, expected = {0};
    if (fd != 123456) return -1;
    calls++; ioctl_number = code; device_order = device_order * 10 + 2;
    va_start(args, code); request = va_arg(args, struct sgx535_fixed_ioctl *); va_end(args);
    before = *request; expected.abi_version = 1; expected.operation = 1;
    before_ok = memcmp(request, &expected, sizeof(expected)) == 0;
    /* Exercise every returned byte, including the first 172 bytes. */
    for (i = 0; i < sizeof(*request); i++) ((unsigned char *)request)[i] = (unsigned char)(i % 251);
    request->abi_version = 1; request->operation = 1;
    request->flags = 0; request->reserved = 0;
    request->operation_errno = 0; request->outcome = 2; request->phase = 9;
    request->observed_events = 7; request->color_observed = 1;
    if (!strcmp(mode, "hold") || !strcmp(mode, "ioctl-eintr")) {
        request->operation_errno = -77; request->outcome = 3; request->phase = 11;
        request->color_observed = 0;
    }
    after = *request; live = request;
    if (!strcmp(mode, "ioctl-eintr")) { errno = EINTR; return -1; }
    return 0;
}
static ssize_t controlled_write(int fd, const void *data, size_t count)
{
    if (fd == raw_fd) {
        raw_writes++;
        if ((!strcmp(mode, "partial") || !strcmp(mode, "partial-close")) && raw_writes > 1) { errno = EIO; return -1; }
        if (!strcmp(mode, "zero")) { errno = 0; return 0; }
        if (raw_writes == 1) {
            if (!strcmp(mode, "short") || !strcmp(mode, "partial") || !strcmp(mode, "partial-close")) return write(fd, data, 1024);
            if (!strcmp(mode, "write-eintr")) { errno = EINTR; return -1; }
        }
    }
    return write(fd, data, count);
}
static int controlled_fsync(int fd)
{
    if (fd == raw_fd) {
        raw_syncs++;
        if ((!strcmp(mode, "sync-failure") || !strcmp(mode, "sync-close"))) { errno = EIO; return -1; }
    }
    return fsync(fd);
}
static int controlled_close(int fd)
{
    int is_raw = fd == raw_fd, result;
    if (fd == 123456) {
        card_closes++; device_order = device_order * 10 + 3;
        after_unchanged = memcmp(live, &after, sizeof(after)) == 0; return 0;
    }
    if (is_raw) raw_fd = -1;
    result = close(fd);
    if (is_raw && (!strcmp(mode, "close-failure") || !strcmp(mode, "partial-close") || !strcmp(mode, "sync-close"))) { errno = EIO; return -1; }
    return result;
}
int main(int argc, char **argv)
{
    int result, client_argc = CLIENT_ARGC;
    FILE *file;
    char *client_argv[] = {"CPU-MOCK-CLIENT", "--one-shot-sgx535-rev121", NULL, NULL, NULL};
    if (argc != 5) return 99;
    mode = argv[1]; image_path = argv[2]; response_path = argv[3];
    client_argv[2] = argv[2]; client_argv[3] = argv[3];
    if (!strcmp(mode, "wrong-flag")) client_argv[1] = "--INVALID";
    if (!strcmp(mode, "missing-response")) client_argc--;
    if (!strcmp(mode, "same-path")) client_argv[3] = client_argv[2];
    result = client_main(client_argc, client_argv);
    file = fopen(argv[4], "wb"); if (!file) return 98;
    if (fwrite(&before, 1, sizeof(before), file) != sizeof(before) ||
        fwrite(&after, 1, sizeof(after), file) != sizeof(after)) return 97;
    if (fclose(file)) return 96;
    fprintf(stderr, "HARNESS {\"result\":%d,\"ioctl_calls\":%d,\"card_opens\":%d,"
            "\"card_closes\":%d,\"card_flags\":%d,\"ioctl\":%lu,\"before_ok\":%d,"
            "\"after_unchanged\":%d,\"raw_opens\":%d,\"raw_writes\":%d,\"raw_syncs\":%d,"
            "\"image_opens\":%d,\"device_order\":%d}\n", result,calls,card_opens,card_closes,card_flags,ioctl_number,
            before_ok,after_unchanged,raw_opens,raw_writes,raw_syncs,image_opens,device_order);
    return 0;
}
'''


class ResponseExportTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix='sgx535-response-CPU-')
        cls.addClassCleanup(cls.directory.cleanup)
        cls.root = Path(cls.directory.name)
        # Baseline override permits demonstrating the missing feature before implementation.
        source = Path(os.environ.get('SGX535_RESPONSE_TEST_SOURCE', str(NEW)))
        cls.new_source = source
        cls.binaries = {}
        for label, path, arity in [('new', source, 3 if source == BASE else 4), ('base', BASE, 3)]:
            harness = cls.root / (label + '.c')
            harness.write_text(HARNESS.replace('CLIENT_SOURCE', str(path)).replace('CLIENT_ARGC', str(arity)))
            binary = cls.root / label
            args = ['cc', '-std=c11', '-O1', '-Wall', '-Wextra', '-Werror', '-pedantic']
            if os.environ.get('SGX535_RESPONSE_UBSAN'):
                args += ['-fsanitize=undefined', '-fno-sanitize-recover=all']
            built = subprocess.run(args + [str(harness), '-o', str(binary)], capture_output=True, text=True)
            if built.returncode: raise AssertionError(built.stderr)
            cls.binaries[label] = binary

    def run_case(self, mode, label='new', raw_kind=None):
        directory = Path(tempfile.mkdtemp(dir=self.root))
        color, raw, expected = directory/'color', directory/'response', directory/'expected'
        if raw_kind:
            original = directory / 'original'; original.write_bytes(b'PREEXISTING EVIDENCE')
            if raw_kind == 'file': raw.write_bytes(b'PREEXISTING EVIDENCE')
            elif raw_kind == 'hardlink': os.link(original, raw)
            elif raw_kind == 'symlink': raw.symlink_to(original)
        run = subprocess.run([str(self.binaries[label]),mode,str(color),str(raw),str(expected)],
                             capture_output=True,timeout=3)
        self.assertEqual(run.returncode,0,run.stderr)
        text, audit = run.stderr.decode().rsplit('HARNESS ',1)
        return directory, color, raw, expected.read_bytes(), json.loads(audit), text, run.stdout

    def test_success_exports_complete_response_and_exact_original_color(self):
        directory,color,raw,expected,audit,_,_ = self.run_case('complete')
        self.assertTrue(raw.exists(), 'post-call raw response was not exported')
        self.assertEqual(raw.read_bytes(), expected[4268:])
        self.assertEqual(len(raw.read_bytes()),4268)
        self.assertEqual(color.read_bytes(), expected[4268+172:])
        self.assertEqual(audit['ioctl_calls'],1)
        self.assertEqual(audit['raw_syncs'],1)
        self.assertEqual(raw.stat().st_mode & 0o777,0o600)

    def test_hold_response_is_exported_without_claiming_image_success(self):
        _,color,raw,expected,audit,_,_ = self.run_case('hold')
        self.assertTrue(raw.exists(), 'nonnegative ioctl failure tuple was not preserved')
        self.assertEqual(raw.read_bytes(), expected[4268:])
        self.assertFalse(color.exists())
        self.assertNotEqual(audit['result'],0)
        self.assertEqual(audit['ioctl_calls'],1)

    def test_failed_ioctl_exports_unauthoritative_bytes_and_never_retries(self):
        _,color,raw,expected,audit,error,_ = self.run_case('ioctl-eintr')
        self.assertTrue(raw.exists(), 'failed ioctl raw bytes were not written')
        self.assertEqual(raw.read_bytes(), expected[4268:])
        self.assertFalse(color.exists())
        self.assertIn('UNAUTHORITATIVE',error)
        self.assertIn('DO NOT RETRY',error)
        self.assertEqual(audit['ioctl_calls'],1)

    def test_short_and_interrupted_file_writes_preserve_all_response_bytes(self):
        for mode in ('short','write-eintr'):
            with self.subTest(mode=mode):
                _,_,raw,expected,audit,_,_ = self.run_case(mode)
                self.assertTrue(raw.exists(), 'response file absent')
                self.assertEqual(raw.read_bytes(),expected[4268:])
                self.assertEqual(audit['ioctl_calls'],1)
                self.assertEqual(audit['raw_writes'],2)

    def test_file_failures_retain_raw_and_image_and_stop_without_retry(self):
        for mode,size in [('partial',1024),('zero',0),('sync-failure',4268),('close-failure',4268)]:
            with self.subTest(mode=mode):
                _,color,raw,expected,audit,error,_ = self.run_case(mode)
                self.assertTrue(raw.exists(), 'failed preservation must retain partial file')
                self.assertEqual(raw.read_bytes(),expected[4268:4268+size])
                self.assertEqual(color.read_bytes(),expected[4268+172:])
                self.assertNotEqual(audit['result'],0)
                self.assertIn('DO NOT RETRY',error)
                self.assertEqual(audit['ioctl_calls'],1)

    def test_cleanup_close_failure_is_reported_after_write_or_sync_failure(self):
        for mode,size,primary in [('partial-close',1024,'write raw response'),
                                  ('sync-close',4268,'fsync raw response')]:
            with self.subTest(mode=mode):
                _,color,raw,expected,audit,error,_ = self.run_case(mode)
                self.assertIn(primary,error)
                self.assertIn('close partial raw response',error)
                self.assertEqual(raw.read_bytes(),expected[4268:4268+size])
                self.assertEqual(color.read_bytes(),expected[4268+172:])
                self.assertEqual(audit['ioctl_calls'],1)
                self.assertNotEqual(audit['result'],0)

    def test_existing_response_paths_and_link_targets_never_overwritten(self):
        for kind in ('file','hardlink','symlink'):
            with self.subTest(kind=kind):
                directory,_,raw,_,audit,_,_ = self.run_case('complete',raw_kind=kind)
                self.assertEqual(raw.read_bytes(),b'PREEXISTING EVIDENCE')
                self.assertEqual((directory/'original').read_bytes(),b'PREEXISTING EVIDENCE')
                self.assertNotEqual(audit['result'],0)
                self.assertEqual(audit['ioctl_calls'],1)

    def test_invalid_or_missing_response_arguments_are_inert(self):
        for mode in ('wrong-flag','missing-response','same-path'):
            with self.subTest(mode=mode):
                _,color,raw,_,audit,_,_ = self.run_case(mode)
                self.assertEqual(audit['ioctl_calls'],0)
                self.assertEqual(audit['card_opens'],0)
                self.assertFalse(raw.exists());self.assertFalse(color.exists())

    def test_full_pre_ioctl_bytes_device_flags_number_and_call_count_match_approved_base(self):
        for mode in ('complete','hold','ioctl-eintr'):
            with self.subTest(mode=mode):
                old=self.run_case(mode,'base');new=self.run_case(mode)
                self.assertEqual(old[3][:4268],new[3][:4268])
                self.assertEqual(new[4]['before_ok'],1)
                self.assertEqual(new[4]['after_unchanged'],1)
                self.assertEqual(new[4]['device_order'],123)
                for key in ('card_flags','ioctl','ioctl_calls','card_opens','card_closes','device_order'):
                    self.assertEqual(old[4][key],new[4][key],key)


if __name__ == '__main__':
    unittest.main()
