/* SPDX-License-Identifier: MIT
 * Experimental native draw bridge. Not a permanent DRM ABI. No userspace
 * addresses, raw commands or register values. One supported asset-matched draw.
 */
#ifndef REMESA_NATIVE_UAPI_H
#define REMESA_NATIVE_UAPI_H
#include <linux/types.h>
#include <drm/drm.h>
#define REMESA_NATIVE_ABI_VERSION 1u
#define REMESA_NATIVE_MAX_VERTICES 4u
#define REMESA_NATIVE_MAX_INDICES 6u
#define REMESA_NATIVE_COLOR_BYTES 4096u
#define REMESA_NATIVE_STATUS_REJECTED 0u
#define REMESA_NATIVE_STATUS_COMPLETED 1u
#define REMESA_NATIVE_STATUS_HELD 2u
struct remesa_native_draw {
    __u32 version, flags, width, height;
    __u32 vertex_count, index_count, argb, reserved;
    __u32 positions[REMESA_NATIVE_MAX_VERTICES][4]; /* IEEE754 bits */
    __u16 indices[REMESA_NATIVE_MAX_INDICES];
    __u32 reserved2;
    __u64 operation;
    __s32 operation_errno;
    __u32 status, phase, events, issued, published;
    __u8 color[REMESA_NATIVE_COLOR_BYTES];
};
/* Private driver index 1; original fixed experiment occupies index 0. */
#define DRM_REMESA_NATIVE_DRAW 1
#define DRM_IOCTL_REMESA_NATIVE_DRAW DRM_IOWR(DRM_COMMAND_BASE + DRM_REMESA_NATIVE_DRAW, struct remesa_native_draw)
#endif
