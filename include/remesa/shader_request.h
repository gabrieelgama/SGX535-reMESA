/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SHADER_REQUEST_H
#define REMESA_SHADER_REQUEST_H

#include "remesa/shader_uapi.h"

enum remesa_shader_request_status {
    REMESA_SHADER_REQUEST_OK,
    REMESA_SHADER_REQUEST_INVALID,
    REMESA_SHADER_REQUEST_UNSUPPORTED,
    REMESA_SHADER_REQUEST_NOMEM
};

/* Shared kernel/userspace validation. Input is an already copied, immutable
 * snapshot, not a userspace pointer. This establishes bounded CPU storage only.
 * EXECUTE is rejected regardless of otherwise valid resource contents. */
enum remesa_shader_request_status remesa_shader_request_validate(
    const struct remesa_shader_request *request, __u64 owner, __u64 generation);

/* Original-program backing contract, separate from the byte staging ABI.
 * These are per-provider resource IDs, not PRIME/KMS GEM handles or GPU PCs.
 * The integrating provider must authenticate and retain each allocation.
 * Validation proves matching CPU byte ranges only, never GPU execution. */
#define REMESA_SHADER_BINDING_VERSION 1u
#define REMESA_SHADER_BINDINGS_BYTES 160u
struct remesa_shader_binding {
    __u32 kind, state, handle, offset;
    __u32 length, alignment, allocation_bytes, reserved;
};
struct remesa_shader_bindings {
    __u32 version, struct_bytes, resource_mask, reserved;
    __u64 owner, generation;
    struct remesa_shader_binding resources[REMESA_SHADER_RESOURCE_COUNT];
};
/* Same-handle ranges must be disjoint and describe one allocation extent.
 * This does not authenticate a handle, compare backing contents, pin memory,
 * assign execution addresses or authorize an otherwise rejected EXECUTE. */
enum remesa_shader_request_status remesa_shader_bindings_validate(
    const struct remesa_shader_request *, const struct remesa_shader_bindings *,
    __u64 owner, __u64 generation);

#ifndef __KERNEL__
struct remesa_shader_blob {
    const void *bytes;
    __u32 length;
    __u32 alignment;
    __u32 state;
};

/* Copies original bytes into a zero-padded staging request in fixed slot order.
 * Absent slots must be entirely zero. Output is unchanged on failure; overlapping
 * input/output storage is rejected. Descriptors and source bytes must remain
 * immutable for the call. This does not call ioctl or upload to GPU. */
enum remesa_shader_request_status remesa_shader_request_build(
    struct remesa_shader_request *out, __u64 owner, __u64 generation,
    const struct remesa_shader_blob blobs[REMESA_SHADER_RESOURCE_COUNT]);
/* Bind all request byte ranges to one already owned CPU backing, preserving
 * request.data offsets. Output is unchanged on error, and may not overlap the
 * request. Caller establishes actual allocation identity/lifetime separately. */
enum remesa_shader_request_status remesa_shader_bindings_build(
    struct remesa_shader_bindings *, const struct remesa_shader_request *,
    __u64 owner, __u64 generation, __u32 handle, __u32 allocation_bytes);
#endif
#endif
