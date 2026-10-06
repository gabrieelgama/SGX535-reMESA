/* PROSPECTIVE RESPONSE-EXPORT CLIENT; NOT SELECTED OR EXECUTION-AUTHORIZED.
 * Separate artifact from the exact approved client. Offline qualification and
 * applicable identity-bound selection are required before target use.
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

/* Exact post-call bytes only; no ioctl or retry of the frozen operation. */
_Static_assert(sizeof(struct sgx535_fixed_ioctl) == 4268, "raw response size");
static int preserve_response(const char *path, const void *bytes, size_t size)
{
    const unsigned char *raw = bytes;
    size_t offset = 0;
    int out = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (out < 0) {
        perror("open raw response");
        return -1;
    }
    while (offset < size) {
        ssize_t written = write(out, raw + offset, size - offset);
        if (written < 0 && errno == EINTR)
            continue; /* File I/O only. */
        if (written <= 0) {
            if (written == 0)
                errno = EIO;
            perror("write raw response");
            if (close(out))
                perror("close partial raw response");
            return -1; /* Retain partial evidence. */
        }
        offset += (size_t)written;
    }
    while (fsync(out) < 0) {
        if (errno == EINTR)
            continue;
        perror("fsync raw response");
        if (close(out))
            perror("close partial raw response");
        return -1;
    }
    if (close(out)) {
        perror("close raw response");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct sgx535_fixed_ioctl request = {0};
    int fd, out, rc, raw_failed;
    size_t row;
    ssize_t written;

    if (argc != 4 || strcmp(argv[1], "--one-shot-sgx535-rev121") ||
        strcmp(argv[2], argv[3]) == 0) {
        fprintf(stderr, "No action. Exact reviewed flag and distinct color/raw output paths required.\n");
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
        if (preserve_response(argv[3], &request, sizeof(request)) < 0)
            fprintf(stderr, "raw response preservation failed; execution=UNKNOWN; DO NOT RETRY\n");
        return 2;
    }
    close(fd);
    raw_failed = preserve_response(argv[3], &request, sizeof(request)) < 0;
    if (raw_failed)
        fprintf(stderr, "raw response preservation failed; ioctl_attempted=1 execution=UNKNOWN; DO NOT RETRY\n");
    printf("errno=%d outcome=%u phase=%u events=0x%08" PRIx32
           " color_observed=%u nonzero=%u fnv1a=0x%08" PRIx32 "\n",
           request.operation_errno, request.outcome, request.phase,
           request.observed_events, request.color_observed,
           request.color_nonzero_pixels, request.color_fnv1a);
    for (row = 0; row < 32; row++)
        printf("row[%zu]=%u\n", row, request.color_row_nonzero[row]);
    if (!request.color_observed || request.operation_errno ||
        request.outcome != 2)
        return raw_failed ? 2 : 1;
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
    return raw_failed ? 2 : 0;
}
