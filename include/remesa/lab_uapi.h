/* SPDX-License-Identifier: MIT
 * Laboratory interface only. NOT the planned DRM/GEM reMESA ABI.
 * This provider does not identify, map or submit to SGX hardware.
 */
#ifndef REMESA_LAB_UAPI_H
#define REMESA_LAB_UAPI_H
#include <linux/ioctl.h>
#include <linux/types.h>
#define REMESA_LAB_VERSION 1
#define REMESA_LAB_FEATURE_CPU_BUFFERS (1u << 0)
struct remesa_lab_info { __u32 version, features; __u64 max_buffer_bytes; };
struct remesa_lab_alloc { __u64 bytes; __u32 flags; __s32 fd; };
#define REMESA_LAB_INFO _IOR('R', 0xe0, struct remesa_lab_info)
#define REMESA_LAB_ALLOC _IOWR('R', 0xe1, struct remesa_lab_alloc)
#define REMESA_LAB_SUBMIT _IO('R', 0xe2)
#endif
