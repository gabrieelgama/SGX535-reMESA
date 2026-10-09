/* SPDX-License-Identifier: MIT */
#ifndef REMESA_MEMORY_UAPI_H
#define REMESA_MEMORY_UAPI_H
#include <linux/types.h>
#include <drm/drm.h>
/* Laboratory per-file GEM-backed IDs; NOT GEM handles for PRIME/KMS.
 * No submission opcode exists. This ioctl is not installed in the live driver.
 * All unused request fields and padding must be zero. */
#define REMESA_MEMORY_VERSION 1u
#define REMESA_MEMORY_CHUNK 4096u
enum remesa_memory_action {
 REMESA_MEM_CREATE=1, REMESA_MEM_PIN, REMESA_MEM_MAP, REMESA_MEM_UNMAP,
 REMESA_MEM_UNPIN, REMESA_MEM_DESTROY, REMESA_MEM_LEASE,
 REMESA_MEM_READ, REMESA_MEM_WRITE, REMESA_MEM_UNLEASE, REMESA_MEM_SYNC,
 REMESA_MEM_OPEN, REMESA_MEM_CLOSE, REMESA_MEM_INFO
};
/* Snapshot of software ownership plus CPU page-table lookup verification.
 * These counts do not prove GPU TLB visibility or execution. */
struct remesa_memory_info {
 __u64 allocated_bytes,object_bytes,gpu_va;
 __u32 objects,pinned_objects,mapped_pages,leases,held_jobs,quarantined;
 __u32 object_pinned,object_mapped_pages,object_lease,verified_pages;
};
struct remesa_memory_request {
 __u32 version,action,id,flags;
 __u64 context,bytes,va,offset;
 __u32 length,reserved;
 __u8 data[REMESA_MEMORY_CHUNK];
};
#define DRM_REMESA_MEMORY 2
#define DRM_IOCTL_REMESA_MEMORY DRM_IOWR(DRM_COMMAND_BASE+DRM_REMESA_MEMORY,struct remesa_memory_request)
#endif
