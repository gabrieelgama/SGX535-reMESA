/* SPDX-License-Identifier: MIT */
#ifndef REMESA_PERSISTENT_H
#define REMESA_PERSISTENT_H
#include "remesa_device.h"
#include "remesa/provider.h"
#define REMESA_PERSISTENT_BUFFERS 32
/* Internal provider seam, NOT ioctl ABI. All calls/receipts require external
 * serialization and an authenticated provider. GEM ownership transfers even
 * when create returns an error with a handle. Other operations must be atomic
 * on error; EOWNERDEAD explicitly means uncertain side effects: retain backing.
 * This seam does not define PTEs, cache operations or GPU instruction bytes. */
struct remesa_gpu_submit {
 uint64_t context,operation,gpu_va,bytes;
 uint32_t gem_handle;
 const struct remesa_draw_packet *packet; /* retained until release succeeds */
};
struct remesa_gpu_receipt {
 uint64_t context,operation;
 bool retired,published,failed; /* publication is independent of retirement */
};
struct remesa_persistent_ops {
 int (*create)(void *,size_t,uint32_t *);
 int (*pin)(void *,uint32_t);
 int (*gpu_map)(void *,uint32_t,uint64_t,size_t);
 int (*gpu_unmap)(void *,uint32_t,uint64_t,size_t);
 int (*unpin)(void *,uint32_t);
 int (*destroy)(void *,uint32_t);
 int (*cpu_map)(void *,uint32_t,bool,void **);
 int (*cpu_unmap)(void *,uint32_t,bool);
 int (*sync_for_device)(void *,uint32_t);
 enum remesa_dispatch (*dispatch)(void *,const struct remesa_gpu_submit *);
 int (*poll)(void *,uint64_t,uint64_t,uint64_t,struct remesa_gpu_receipt *);
 /* Authenticated retirement plus any required visibility/acknowledgement.
  * Success guarantees commands are no longer read and backing is reusable.
  * The persistent SGX mapping remains installed. EAGAIN is allowed only when
  * no acknowledgement/release side effect occurred; retrying release never
  * resubmits hardware. Other errors retain quarantine semantics. */
 int (*release)(void *,uint64_t,uint64_t);
};
struct remesa_persistent_config {
 const struct remesa_persistent_ops *ops;void *priv;
 uint64_t context,va_start,va_end;size_t byte_budget;
 bool offline_test_backend,qualified_hardware_backend;
 bool resource_only_backend; /* real allocation transport, dispatch unavailable */
 /* Transfer-storage limits, not GPU rendering/sampling capabilities. Zero
  * dimensions/alignment use layout defaults; bytes are clamped to budget/VA. */
 struct remesa_resource_limits resource_limits;
};
/* Zero-initialize device before open. Caller retains the endpoint until all
 * Gallium screens/contexts and fences referring to it have been destroyed.
 * context must be unique among surviving endpoints and authenticated by the
 * integrating kernel backend; it is not a caller-controlled hardware ID.
 * Fresh address-space identity/configuration only; no device discovery or
 * qualification is performed here. The real GEM/MMU binding is not supplied
 * by this library. No mode silently falls back to host rasterization. */
int remesa_persistent_open(struct remesa_device *,const struct remesa_persistent_config *);
/* Completion receipts may arrive out of order; visibility and backing release
 * drain in operation order with nonblocking dependency polls. This is host
 * sequencing, not evidence of reusable SGX execution.
 * Poll detached jobs. Failed/ambiguous jobs and their backing remain retained.
 * Close returns EBUSY while resources/jobs survive; it never force-frees them. */
int remesa_persistent_collect(struct remesa_device *);
int remesa_persistent_close(struct remesa_device *);
#endif
