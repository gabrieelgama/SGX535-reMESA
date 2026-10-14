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
struct remesa_shader_request;
struct remesa_shader_bindings;
struct remesa_shader_draw_programs;
struct remesa_usse_vertex_mapping;
struct remesa_draw_plan;
/* Explicit numeric VS CPU candidate plus the original FS candidate. Returns
 * ENOTSUP with an owned token after successful CPU-GEM staging; destroy through
 * device.ops->destroy_shader_programs. No SGX mapping or execution. A nonempty
 * output slot is rejected unchanged. This does not alter Gallium callbacks. */
int remesa_persistent_prepare_vertex_candidate(struct remesa_device *,
 const struct remesa_shader_draw_programs *,const struct remesa_usse_vertex_mapping *,void **);
/* CPU-only original-program staging in an owned GEM object. The allocation is
 * pinned for CPU access but has NO SGX mapping, execution address or job hold.
 * A staged CPU candidate is never passed to the diagnostic executor. Tokens
 * are owned by their creating endpoint and must be destroyed before close.
 * Identity cookies are never reused within that endpoint lifetime. At most
 * REMESA_SHADER_COOKIE_LIMIT preparations can publish a token before close;
 * this bounds host metadata, not hardware resources. Retired cookies remain
 * protected from query output aliases and are released only on successful close.
 * Uncertain lease/release failures retain backing and make close return EBUSY.
 */
struct remesa_persistent_shader_backing {
 uint64_t owner,generation;
 uint32_t handle;
 size_t allocation_bytes;
 bool pinned,staged,quarantined;
};
/* Alias-only validation against retained host identities, program images and
 * raw draw snapshots, including quarantined ones. Does not require storage
 * readiness, verify GEM contents or qualify execution. Used before copying
 * endpoint diagnostics so failed staging cannot hide the diagnostic record. */
int remesa_persistent_shader_output_check(const struct remesa_device *,
 const void *output,size_t bytes);
/* Copy the immutable request and storage identity for an owned staged token.
 * This is CPU readback verification, not instruction visibility or execution.
 * Outputs unchanged on failure; external endpoint serialization is required. */
int remesa_persistent_shader_snapshot(const struct remesa_device *,void *token,
 struct remesa_shader_request *,struct remesa_persistent_shader_backing *);
/* Token-specific program-ID/range snapshot derived from the owned allocation.
 * No GPU address or execution entry is supplied. Both outputs are copied only
 * on success, must not alias each other or owned storage, and are valid as
 * bindings only while the token lives under external endpoint serialization. */
int remesa_persistent_shader_bindings(const struct remesa_device *,void *token,
 struct remesa_shader_request *,struct remesa_shader_bindings *);
/* Explicit CPU lease/readback against the immutable body and zero-filled tail.
 * It never maps SGX VA, syncs for execution or submits. A changed body/tail or
 * uncertain lease invalidates admission and retains backing in quarantine. */
int remesa_persistent_shader_verify(struct remesa_device *,void *token);
/* Immutable original pre-VS VB/IB preparation under an owned program token.
 * CPU-GEM allocation only: no SGX mapping, cache sync, submission or ISA.
 * One successful geometry per token; replacement returns EBUSY. Ordinary
 * failures release only tentative geometry; uncertain provider failures keep
 * backing quarantined and close blocked. Source leases remain held for this
 * call. The target framebuffer is described, never owned by this snapshot. */
int remesa_persistent_shader_stage_draw(struct remesa_device *,void *token,
 const struct remesa_shader_draw_inputs *);
/* Borrow the resource owner's single immutable raw input/state snapshot.
 * No provider call or GPU operation. Bytes remain valid until token destruction;
 * both outputs must be disjoint and outside every live payload/retained cookie
 * on this endpoint. Unchanged on failure; serialize against destruction. */
int remesa_persistent_shader_draw_snapshot(const struct remesa_device *,void *token,
 struct remesa_shader_draw_inputs *,struct remesa_draw_plan *);
/* Provider-local handle and byte ranges, not GPU addresses or authentication.
 * VB occupies offset zero. IB is padded to a four-byte host serialization
 * boundary; that alignment is not an independently qualified SGX requirement.
 * Nonindexed geometry has zero index_offset/index_bytes. These ranges preserve
 * the original offsets/start/bias; they contain no CPU-executed vertex output.
 * Output unchanged on error; externally serialized with token destruction. */
struct remesa_persistent_draw_backing {
 uint64_t owner,generation;
 uint32_t handle;
 size_t allocation_bytes,vertex_bytes,index_offset,index_bytes;
 bool pinned,staged;
};
int remesa_persistent_shader_draw_backing(const struct remesa_device *,void *token,
 struct remesa_persistent_draw_backing *);
struct remesa_pds_host_data_view;
/* Snapshot and CPU-stage two original HOST DATA windows in one owned pinned
 * backing under the existing shader token. VS then FS, at most1024bytes each.
 * Internal sixteen-byte packing is host policy, not DS layout or fetch rules.
 * No CODE, SGX VA/map, cache publication, task launch or execution is supplied.
 * One pair per token. Ordinary failures roll back; uncertain provider effects
 * quarantine backing and block close. External serialization is required. */
int remesa_persistent_shader_stage_pds_data(struct remesa_device *,void *token,
 const struct remesa_pds_host_data_view *);
struct remesa_persistent_pds_backing {
 uint64_t owner,generation;
 uint32_t handle;
 size_t allocation_bytes,data_offset[2],data_bytes[2];
 bool pinned,staged;
};
/* Fresh token-specific provider ID and exact HOST ranges, never execution
 * addresses. Both stages are published together after byte-exact CPU readback.
 * Output unchanged on failure; teardown invalidates the binding. */
int remesa_persistent_shader_pds_backing(const struct remesa_device *,void *token,
 struct remesa_persistent_pds_backing *);
#endif
