/* PROSPECTIVE, UNQUALIFIED CLIENT SOURCE.
 * Not the selected frozen client; do not stage or invoke this source.
 * Native CPU-only fault-injection tests are not hardware qualification.
 */
/* SPDX-License-Identifier: GPL-2.0-only */
/* Exact one-shot diagnostic client. Build only; Gate B is not authorization. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../../kernel/sgx535_frozen/gma500_fixed_uapi.h"

int main(int argc, char **argv)
{
    struct sgx535_fixed_ioctl request = {0};
    int fd, out, rc;
    size_t row;
    ssize_t written;

    if (argc != 3 || strcmp(argv[1], "--one-shot-sgx535-rev121")) {
        fprintf(stderr, "No action. Exact reviewed flag and color output path required.\n");
        return 2;
    }
    fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        perror("open card0");
        return 2;
    }
    request.abi_version = 1;
    request.operation = 1;
    rc = ioctl(fd, DRM_IOCTL_PSB_FIXED_TRIANGLE, &request);
    if (rc < 0) {
        const int syscall_errno = errno;
        const unsigned char *raw = (const unsigned char *)&request;
        fprintf(stderr, "fixed triangle ioctl: %s\n", strerror(syscall_errno));
        fprintf(stderr, "ioctl_attempted=1 syscall_result=%d syscall_errno=%d "
                "execution=UNKNOWN; DO NOT RETRY\n", rc, syscall_errno);
        /* A failed syscall does not authenticate a returned service response. */
        fprintf(stderr, "UNAUTHORITATIVE raw userspace request: "
                "operation_errno=%d outcome=%u phase=%u events=0x%08" PRIx32
                " color_observed=%u nonzero=%u fnv1a=0x%08" PRIx32 "\n",
                request.operation_errno, request.outcome, request.phase,
                request.observed_events, request.color_observed,
                request.color_nonzero_pixels, request.color_fnv1a);
        fprintf(stderr, "raw_request_hex=");
        for (row = 0; row < sizeof(request); row++)
            fprintf(stderr, "%02x", raw[row]);
        fprintf(stderr, "\n");
        close(fd);
        return 2;
    }
    close(fd);
    printf("errno=%d outcome=%u phase=%u events=0x%08" PRIx32
           " color_observed=%u nonzero=%u fnv1a=0x%08" PRIx32 "\n",
           request.operation_errno, request.outcome, request.phase,
           request.observed_events, request.color_observed,
           request.color_nonzero_pixels, request.color_fnv1a);
    for (row = 0; row < 32; row++)
        printf("row[%zu]=%u\n", row, request.color_row_nonzero[row]);
    if (!request.color_observed || request.operation_errno ||
        request.outcome != 2)
        return 1;
    out = open(argv[2], O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (out < 0) {
        perror("open color output");
        return 2;
    }
    size_t offset = 0;
    while (offset < sizeof(request.color_bytes)) {
        written = write(out, request.color_bytes + offset,
                        sizeof(request.color_bytes) - offset);
        /* Retry only file I/O, never the ioctl or the frozen operation. */
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0) {
            if (written == 0)
                errno = EIO;
            perror("write color output");
            close(out);
            return 2;
        }
        offset += (size_t)written;
    }
    while (fsync(out) < 0) {
        if (errno == EINTR)
            continue;
        perror("fsync color output");
        close(out);
        return 2;
    }
    if (close(out)) {
        perror("close color output");
        return 2;
    }
    return 0;
}
