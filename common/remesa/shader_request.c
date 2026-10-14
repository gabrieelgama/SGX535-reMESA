/* SPDX-License-Identifier: MIT */
#include "remesa/shader_request.h"

#ifdef __KERNEL__
#include <linux/stddef.h>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

/* Explicit offsets make i386/host agreement independent of pointer sizes. */
typedef char remesa_shader_resource_size[
    sizeof(struct remesa_shader_resource) == 32 ? 1 : -1];
typedef char remesa_shader_request_size[
    sizeof(struct remesa_shader_request) == REMESA_SHADER_REQUEST_BYTES ? 1 : -1];
typedef char remesa_shader_request_resources_offset[
    offsetof(struct remesa_shader_request, resources) == 64 ? 1 : -1];
typedef char remesa_shader_request_data_offset[
    offsetof(struct remesa_shader_request, data) == 192 ? 1 : -1];
typedef char remesa_shader_binding_size[
    sizeof(struct remesa_shader_binding) == 32 ? 1 : -1];
typedef char remesa_shader_bindings_size[
    sizeof(struct remesa_shader_bindings) == REMESA_SHADER_BINDINGS_BYTES ? 1 : -1];
typedef char remesa_shader_bindings_resources_offset[
    offsetof(struct remesa_shader_bindings, resources) == 32 ? 1 : -1];

static int valid_alignment(__u32 alignment)
{
    return alignment && alignment <= REMESA_SHADER_MAX_ALIGNMENT &&
           !(alignment & (alignment - 1));
}

static int zero_bytes(const void *bytes, __u32 size)
{
    const __u8 *p = bytes;
    __u32 i;
    for (i = 0; i < size; ++i)
        if (p[i]) return 0;
    return 1;
}

enum remesa_shader_request_status remesa_shader_request_validate(
    const struct remesa_shader_request *r, __u64 owner, __u64 generation)
{
    __u32 i, j, previous_end = 0, mask = 0;
    if (!r || !owner || !generation || r->owner != owner ||
        r->generation != generation || r->version != REMESA_SHADER_ABI_VERSION ||
        r->struct_bytes != REMESA_SHADER_REQUEST_BYTES || r->flags ||
        !zero_bytes(r->reserved, sizeof(r->reserved)) ||
        !r->byte_count || r->byte_count > REMESA_SHADER_DATA_BYTES ||
        !r->resource_mask || r->resource_mask >> REMESA_SHADER_RESOURCE_COUNT)
        return REMESA_SHADER_REQUEST_INVALID;
    if (r->action != REMESA_SHADER_STAGE && r->action != REMESA_SHADER_EXECUTE)
        return REMESA_SHADER_REQUEST_UNSUPPORTED;
    for (i = 0; i < REMESA_SHADER_RESOURCE_COUNT; ++i) {
        const struct remesa_shader_resource *s = &r->resources[i];
        if (!(r->resource_mask & (1u << i))) {
            if (!zero_bytes(s, sizeof(*s))) return REMESA_SHADER_REQUEST_INVALID;
            continue;
        }
        if (s->kind != i + 1 || !s->length ||
            s->length > REMESA_SHADER_RESOURCE_BYTES ||
            !valid_alignment(s->alignment) || s->offset % s->alignment ||
            s->offset < previous_end || s->offset > r->byte_count ||
            s->length > r->byte_count - s->offset ||
            !zero_bytes(s->reserved, sizeof(s->reserved)))
            return REMESA_SHADER_REQUEST_INVALID;
        if (s->state != REMESA_SHADER_CPU_CANDIDATE &&
            s->state != REMESA_SHADER_ENCODED_UNQUALIFIED)
            return REMESA_SHADER_REQUEST_UNSUPPORTED;
        /* Fixed slot ordering plus nonoverlap allows a bounded padding check. */
        for (j = previous_end; j < s->offset; ++j)
            if (r->data[j]) return REMESA_SHADER_REQUEST_INVALID;
        previous_end = s->offset + s->length;
        mask |= 1u << i;
    }
    if (mask != r->resource_mask || previous_end != r->byte_count ||
        !zero_bytes(r->data + previous_end,
                    REMESA_SHADER_DATA_BYTES - previous_end))
        return REMESA_SHADER_REQUEST_INVALID;
    /* No state in this ABI can establish register mapping, termination, PDS
     * fetch or device ownership. Staging is never an execution admission. */
    return r->action == REMESA_SHADER_EXECUTE ? REMESA_SHADER_REQUEST_UNSUPPORTED :
                                              REMESA_SHADER_REQUEST_OK;
}

enum remesa_shader_request_status remesa_shader_bindings_validate(
    const struct remesa_shader_request *r, const struct remesa_shader_bindings *b,
    __u64 owner, __u64 generation)
{
    __u32 i, j;
    enum remesa_shader_request_status status =
        remesa_shader_request_validate(r, owner, generation);
    if (status != REMESA_SHADER_REQUEST_OK) return status;
    if (!b || b->version != REMESA_SHADER_BINDING_VERSION ||
        b->struct_bytes != REMESA_SHADER_BINDINGS_BYTES || b->reserved ||
        b->owner != owner || b->generation != generation ||
        b->resource_mask != r->resource_mask)
        return REMESA_SHADER_REQUEST_INVALID;
    for (i = 0; i < REMESA_SHADER_RESOURCE_COUNT; ++i) {
        const struct remesa_shader_binding *s = &b->resources[i];
        const struct remesa_shader_resource *source = &r->resources[i];
        if (!(r->resource_mask & (1u << i))) {
            if (!zero_bytes(s, sizeof(*s))) return REMESA_SHADER_REQUEST_INVALID;
            continue;
        }
        if (s->kind != source->kind || s->state != source->state ||
            s->length != source->length || s->alignment != source->alignment ||
            !s->handle || s->reserved || s->offset % s->alignment ||
            s->offset > s->allocation_bytes ||
            s->length > s->allocation_bytes - s->offset)
            return REMESA_SHADER_REQUEST_INVALID;
        for (j = 0; j < i; ++j) {
            const struct remesa_shader_binding *other = &b->resources[j];
            if (!(r->resource_mask & (1u << j)) || other->handle != s->handle)
                continue;
            /* Bounds above justify both additions without 32-bit wrap. */
            if (other->allocation_bytes != s->allocation_bytes ||
                (s->offset < other->offset + other->length &&
                 other->offset < s->offset + s->length))
                return REMESA_SHADER_REQUEST_INVALID;
        }
    }
    return REMESA_SHADER_REQUEST_OK;
}

#ifndef __KERNEL__
static int overlaps(const void *a, size_t as, const void *b, size_t bs)
{
    uintptr_t x = (uintptr_t)a, y = (uintptr_t)b;
    if (as > UINTPTR_MAX - x || bs > UINTPTR_MAX - y) return 1;
    return x < y + bs && y < x + as;
}

enum remesa_shader_request_status remesa_shader_bindings_build(
    struct remesa_shader_bindings *out, const struct remesa_shader_request *r,
    __u64 owner, __u64 generation, __u32 handle, __u32 allocation_bytes)
{
    struct remesa_shader_bindings b = {0};
    enum remesa_shader_request_status status;
    __u32 i;
    if (!out || !r || !handle || !allocation_bytes ||
        overlaps(out, sizeof(*out), r, sizeof(*r)))
        return REMESA_SHADER_REQUEST_INVALID;
    status = remesa_shader_request_validate(r, owner, generation);
    if (status != REMESA_SHADER_REQUEST_OK) return status;
    b.version = REMESA_SHADER_BINDING_VERSION;
    b.struct_bytes = REMESA_SHADER_BINDINGS_BYTES;
    b.owner = owner;
    b.generation = generation;
    b.resource_mask = r->resource_mask;
    for (i = 0; i < REMESA_SHADER_RESOURCE_COUNT; ++i) {
        const struct remesa_shader_resource *source = &r->resources[i];
        struct remesa_shader_binding *s = &b.resources[i];
        if (!(r->resource_mask & (1u << i))) continue;
        s->kind = source->kind;
        s->state = source->state;
        s->handle = handle;
        s->offset = source->offset;
        s->length = source->length;
        s->alignment = source->alignment;
        s->allocation_bytes = allocation_bytes;
    }
    status = remesa_shader_bindings_validate(r, &b, owner, generation);
    if (status == REMESA_SHADER_REQUEST_OK) *out = b;
    return status;
}

enum remesa_shader_request_status remesa_shader_request_build(
    struct remesa_shader_request *out, __u64 owner, __u64 generation,
    const struct remesa_shader_blob blobs[REMESA_SHADER_RESOURCE_COUNT])
{
    struct remesa_shader_request *r;
    enum remesa_shader_request_status status;
    __u32 i, cursor = 0;
    if (!out || !blobs || !owner || !generation ||
        overlaps(out, sizeof(*out), blobs,
                 sizeof(*blobs) * REMESA_SHADER_RESOURCE_COUNT))
        return REMESA_SHADER_REQUEST_INVALID;
    /* Validate all source bounds before reading any caller bytes. */
    for (i = 0; i < REMESA_SHADER_RESOURCE_COUNT; ++i) {
        const struct remesa_shader_blob *b = &blobs[i];
        __u32 offset;
        if (!b->length) {
            if (b->bytes || b->alignment || b->state)
                return REMESA_SHADER_REQUEST_INVALID;
            continue;
        }
        if (!b->bytes || b->length > REMESA_SHADER_RESOURCE_BYTES ||
            !valid_alignment(b->alignment) ||
            overlaps(out, sizeof(*out), b->bytes, b->length))
            return REMESA_SHADER_REQUEST_INVALID;
        if (b->state != REMESA_SHADER_CPU_CANDIDATE &&
            b->state != REMESA_SHADER_ENCODED_UNQUALIFIED)
            return REMESA_SHADER_REQUEST_UNSUPPORTED;
        offset = (cursor + b->alignment - 1) & ~(b->alignment - 1);
        if (offset > REMESA_SHADER_DATA_BYTES ||
            b->length > REMESA_SHADER_DATA_BYTES - offset)
            return REMESA_SHADER_REQUEST_INVALID;
        cursor = offset + b->length;
    }
    if (!cursor) return REMESA_SHADER_REQUEST_INVALID;
    r = calloc(1, sizeof(*r));
    if (!r) return REMESA_SHADER_REQUEST_NOMEM;
    r->version = REMESA_SHADER_ABI_VERSION;
    r->struct_bytes = REMESA_SHADER_REQUEST_BYTES;
    r->action = REMESA_SHADER_STAGE;
    r->owner = owner;
    r->generation = generation;
    cursor = 0;
    for (i = 0; i < REMESA_SHADER_RESOURCE_COUNT; ++i) {
        const struct remesa_shader_blob *b = &blobs[i];
        struct remesa_shader_resource *s = &r->resources[i];
        if (!b->length) continue;
        cursor = (cursor + b->alignment - 1) & ~(b->alignment - 1);
        s->kind = i + 1;
        s->state = b->state;
        s->offset = cursor;
        s->length = b->length;
        s->alignment = b->alignment;
        memcpy(r->data + cursor, b->bytes, b->length);
        cursor += b->length;
        r->resource_mask |= 1u << i;
    }
    r->byte_count = cursor;
    status = remesa_shader_request_validate(r, owner, generation);
    if (status == REMESA_SHADER_REQUEST_OK) memcpy(out, r, sizeof(*r));
    free(r);
    return status;
}
#endif
