/* SPDX-License-Identifier: MIT */
#ifndef REMESA_DEVICE_H
#define REMESA_DEVICE_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "remesa/draw_packet.h"
/* Internal serialized userspace seam, not a kernel ABI. Submit snapshots the
 * packet before returning and retains backing through retirement. Allocation
 * destruction must defer release of pending or quarantined backing. All map calls must be nonblocking; reads
 * must fail until the backend establishes CPU publication. The Linux backend performs one synchronous native DRM draw; recording tests
 * must opt in explicitly and cannot establish hardware rendering. */
struct remesa_device_ops {
 /* Any returned BO transfers ownership, even when allocation reports error. */
 int (*allocate)(void *, size_t, void **);
 void (*destroy_bo)(void *, void *);
 /* Zero means one acquired CPU-map lease. A non-NULL returned pointer also
  * transfers that lease on error. Frontend releases it exactly once. Zero with
  * NULL is a protocol error and still requires lease cleanup, never access. */
 int (*map)(void *, void *, bool write, void **);
 void (*unmap)(void *, void *, bool write);
 /* Any non-NULL returned fence transfers ownership, including on error.
  * An error after possible issuance must quarantine backing in the backend;
  * destroying that fence must not release uncertain device resources. */
 int (*submit)(void *, void *target, const struct remesa_draw_packet *, void **fence);
 /* timeout_ns=0 polls; UINT64_MAX waits indefinitely. Success with
  * cpu_visible=true means all relevant writes are published and backing is
  * reusable. A fence may outlive its submitting context. This interface and
  * resources shared between contexts require external serialization. */
 int (*wait)(void *, void *, uint64_t timeout_ns, bool cpu_visible);
 void (*destroy_fence)(void *, void *);
};
/* Explicit target load contract. Current physical executor is REPLACE only.
 * PRESERVE is permitted solely for opt-in offline fixtures until an actual
 * preserving kernel path has been independently qualified. */
enum remesa_framebuffer_mode { REMESA_FRAMEBUFFER_REPLACE=0, REMESA_FRAMEBUFFER_PRESERVE=1 };
/* Transfer storage limits, not SGX render/sampler capabilities. Zero fields
 * select conservative frontend defaults. The backing allocator may still
 * reject allocations based on its actual GEM/address-space budget. */
struct remesa_resource_limits {
 uint32_t max_width,max_height,row_alignment;
 uint64_t max_resource_bytes;
};
struct remesa_device {
 const struct remesa_device_ops *ops; void *priv;
 bool offline_test_backend, qualified_hardware_backend;
 bool resource_only_backend; /* allocation/map only; submit must refuse */
 enum remesa_framebuffer_mode framebuffer_mode;
 struct remesa_resource_limits resource_limits;
};
struct pipe_context;
/* Diagnostic fixed program, not NIR/OpenGL support. Positions are projected
 * window XY, Z in [0,1], W=1; no vertex transformation is claimed. */
int remesa_bind_diagnostic_pipeline(struct pipe_context *, uint32_t argb);
int remesa_context_error(const struct pipe_context *);
#endif
