/* SPDX-License-Identifier: MIT */
#ifndef REMESA_LINUX_H
#define REMESA_LINUX_H
#include "remesa_device.h"
/* Single-attempt DRM hardware transport; no CPU rasterization or retries. */
int remesa_linux_open(struct remesa_device *, const char *render_node);
/* Duplicates a writable loader fd with CLOEXEC; never closes the caller fd.
 * DRM name validation is discovery, not proof of native-ioctl support. */
int remesa_linux_open_fd(struct remesa_device *, int borrowed_fd);
void remesa_linux_close(struct remesa_device *);
struct remesa_native_draw;
/* Pure wire construction; no ioctl or device access. */
int remesa_linux_request_build(const struct remesa_draw_packet *,struct remesa_native_draw *);
struct remesa_linux_receipt {
 uint64_t operation;
 int ioctl_errno, operation_errno;
 uint32_t phase, events, issued, cpu_published, attempted;
};
/* Full original ioctl buffer is retained even on failed/ambiguous issuance. */
int remesa_linux_response(const struct remesa_device *,struct remesa_native_draw *);
int remesa_linux_last_receipt(const struct remesa_device *,struct remesa_linux_receipt *);
#endif
