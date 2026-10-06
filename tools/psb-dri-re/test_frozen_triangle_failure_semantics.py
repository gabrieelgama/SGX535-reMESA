"""CPU-only failure semantics of the unqualified prospective one-shot client.

No existing qualified artifact is built, hashed, or executed. Device open,
ioctl, and close are replaced in the included prospective source.
"""
import json
import errno
import pathlib
import subprocess
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent
PROSPECTIVE = HERE / "frozen_triangle_one_shot_prospective.c"

HARNESS = r"""
#define main failure_client_main
#define open failure_mock_open
#define ioctl failure_mock_ioctl
#define close failure_mock_close
#include "PROSPECTIVE_SOURCE"
#undef main
#undef open
#undef ioctl
#undef close
#include <stdarg.h>

static const char *mode;
static int card_opens, output_opens, ioctl_calls, closes, final_raw_unchanged;
static struct sgx535_fixed_ioctl *raw_request;
static struct sgx535_fixed_ioctl expected_request;

int failure_mock_open(const char *path, int flags, ...)
{
    (void)flags;
    if (strcmp(path, "/dev/dri/card0") == 0) {
        card_opens++;
        if (strcmp(mode, "open-failure") == 0) {
            errno = EACCES;
            return -1;
        }
        return 123;
    }
    output_opens++;
    errno = EACCES;
    return -1;
}

int failure_mock_ioctl(int fd, unsigned long code, ...)
{
    va_list args;
    struct sgx535_fixed_ioctl *request;
    (void)fd;
    (void)code;
    va_start(args, code);
    request = va_arg(args, struct sgx535_fixed_ioctl *);
    va_end(args);
    ioctl_calls++;
    if (strcmp(mode, "untouched-eintr") != 0) {
        request->operation_errno = -77;
        request->outcome = 3;
        request->phase = 11;
        request->observed_events = 0x13572468;
        request->color_observed = 0;
        request->color_fnv1a = 0x24681357;
        request->color_nonzero_pixels = 19;
        request->color_row_nonzero[0] = 7;
        request->color_row_nonzero[31] = 12;
        memset(request->color_bytes, 0xa5, sizeof(request->color_bytes));
    }
    expected_request = *request;
    raw_request = request;
    if (strcmp(mode, "service-failure") == 0)
        return 0;
    errno = strcmp(mode, "efault") == 0 ? EFAULT : EINTR;
    return -1;
}

int failure_mock_close(int fd)
{
    (void)fd;
    closes++;
    if (raw_request)
        final_raw_unchanged = memcmp(raw_request, &expected_request,
                                    sizeof(expected_request)) == 0;
    /* A client must preserve the ioctl errno before cleanup. */
    errno = EBADF;
    return 0;
}

int main(int argc, char **argv)
{
    int result;
    char *call_argv[] = {"mock-client", "--one-shot-sgx535-rev121",
                         "/NEVER-OPEN-MOCK-OUTPUT", NULL};
    if (argc != 2)
        return 99;
    mode = argv[1];
    if (strcmp(mode, "precall") == 0)
        call_argv[1] = "--invalid";
    result = failure_client_main(3, call_argv);
    fprintf(stderr, "HARNESS {\"return\":%d,\"card_opens\":%d,"
            "\"ioctl_calls\":%d,\"output_opens\":%d,\"closes\":%d,"
            "\"raw_unchanged_at_close\":%d,\"request_size\":%zu",
            result, card_opens, ioctl_calls, output_opens, closes,
            final_raw_unchanged, sizeof(expected_request));
    fprintf(stderr, ",\"expected_raw_hex\":\"");
    for (size_t i = 0; i < sizeof(expected_request); i++)
        fprintf(stderr, "%02x", ((unsigned char *)&expected_request)[i]);
    fprintf(stderr, "\"}\n");
    return result;
}
"""


class FailureSemanticsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = tempfile.TemporaryDirectory(prefix="sgx535-failure-cpu-")
        cls.addClassCleanup(cls.directory.cleanup)
        base = pathlib.Path(cls.directory.name)
        harness = base / "failure-harness.c"
        harness.write_text(HARNESS.replace("PROSPECTIVE_SOURCE", str(PROSPECTIVE)))
        cls.binary = base / "failure-mocked-client"
        built = subprocess.run([
            "cc", "-std=c11", "-O0", "-Wall", "-Wextra", "-Werror",
            "-pedantic", str(harness), "-o", str(cls.binary),
        ], capture_output=True, text=True)
        if built.returncode:
            raise AssertionError(built.stderr)

    def run_mode(self, mode):
        run = subprocess.run([str(self.binary), mode], capture_output=True,
                             text=True)
        client_stderr, audit_text = run.stderr.rsplit("HARNESS ", 1)
        return run, client_stderr, json.loads(audit_text)

    def assert_stopped_after_one_ioctl(self, run, audit):
        self.assertNotEqual(run.returncode, 0)
        self.assertEqual(audit["ioctl_calls"], 1)
        self.assertEqual(audit["card_opens"], 1)
        self.assertEqual(audit["output_opens"], 0)
        self.assertEqual(audit["closes"], 1)
        self.assertEqual(audit["raw_unchanged_at_close"], 1)

    def assert_raw_bytes_preserved_in_report(self, report, audit):
        raw_lines = [line.removeprefix("raw_request_hex=")
                     for line in report.splitlines()
                     if line.startswith("raw_request_hex=")]
        self.assertEqual(len(raw_lines), 1)
        reported = bytes.fromhex(raw_lines[0])
        self.assertEqual(len(reported), audit["request_size"])
        self.assertEqual(reported, bytes.fromhex(audit["expected_raw_hex"]))

    def test_negative_ioctl_reports_unknown_no_retry_and_raw_unauthoritative_tuple(self):
        for mode, syscall_message, syscall_errno in (
                ("eintr", "Interrupted system call", errno.EINTR),
                ("efault", "Bad address", errno.EFAULT)):
            with self.subTest(mode=mode):
                run, report, audit = self.run_mode(mode)
                self.assert_stopped_after_one_ioctl(run, audit)
                expected = ("ioctl_attempted=1", "execution=UNKNOWN",
                            "DO NOT RETRY", "UNAUTHORITATIVE",
                            "operation_errno=-77", "outcome=3", "phase=11",
                            "events=0x13572468", syscall_message,
                            "syscall_errno=" + str(syscall_errno))
                missing = [token for token in expected if token not in report]
                self.assertEqual(missing, [], "missing failure report: " + repr(missing))
                self.assert_raw_bytes_preserved_in_report(report, audit)

    def test_untouched_request_still_requires_unknown_and_no_retry(self):
        run, report, audit = self.run_mode("untouched-eintr")
        self.assert_stopped_after_one_ioctl(run, audit)
        expected = ("ioctl_attempted=1", "execution=UNKNOWN", "DO NOT RETRY",
                    "UNAUTHORITATIVE", "operation_errno=0", "outcome=0",
                    "phase=0", "events=0x00000000")
        missing = [token for token in expected if token not in report]
        self.assertEqual(missing, [], "missing untouched failure report: " + repr(missing))
        self.assert_raw_bytes_preserved_in_report(report, audit)

    def test_pre_ioctl_failures_have_zero_calls(self):
        for mode, card_opens in (("precall", 0), ("open-failure", 1)):
            with self.subTest(mode=mode):
                run, report, audit = self.run_mode(mode)
                self.assertNotEqual(run.returncode, 0)
                self.assertEqual(audit["card_opens"], card_opens)
                self.assertEqual(audit["ioctl_calls"], 0)
                self.assertEqual(audit["output_opens"], 0)
                self.assertEqual(audit["closes"], 0)
                self.assertNotIn("ioctl_attempted=1", report)

    def test_positive_ioctl_service_failure_stops_with_returned_tuple(self):
        run, report, audit = self.run_mode("service-failure")
        self.assert_stopped_after_one_ioctl(run, audit)
        for token in ("errno=-77", "outcome=3", "phase=11", "events=0x13572468"):
            self.assertIn(token, run.stdout)
        self.assertNotIn("execution=UNKNOWN", report)

    def test_exit_status_alone_does_not_classify_attempt(self):
        before, _, before_audit = self.run_mode("precall")
        after, _, after_audit = self.run_mode("eintr")
        self.assertEqual(before.returncode, after.returncode)
        self.assertEqual(before_audit["ioctl_calls"], 0)
        self.assertEqual(after_audit["ioctl_calls"], 1)


if __name__ == "__main__":
    unittest.main()
