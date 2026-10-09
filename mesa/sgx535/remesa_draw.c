/* SPDX-License-Identifier: MIT */
#include "remesa_draw.h"
#include "remesa_fixed_state.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static uint32_t read_index(const unsigned char *p, unsigned width)
{
    uint32_t value = 0;
    /* memcpy respects native Gallium index storage and avoids unaligned loads.
     * Widths smaller than uint32_t need their matching native integer type. */
    if (width == 1) return *p;
    if (width == 2) { uint16_t v; memcpy(&v, p, sizeof(v)); return v; }
    memcpy(&value, p, sizeof(value));
    return value;
}

enum remesa_draw_status remesa_prepare_constant_draw(
    const struct pipe_draw_info *info, const struct pipe_draw_start_count_bias *draw,
    const struct pipe_vertex_buffer *vertices, const struct pipe_vertex_element *position,
    const void *vertex_snapshot, size_t vertex_bytes,
    const void *index_snapshot, size_t index_bytes, uint32_t argb,
    const struct pipe_viewport_state *viewport, uint32_t width, uint32_t height,
    struct remesa_prepared_triangles *out)
{
    struct remesa_draw_plan plan = {0};
    struct remesa_prepared_triangles *result;
    enum remesa_draw_status status;
    uint64_t base;
    if (!info || !draw || !vertices || !position || !viewport || !out || !width || !height)
        return REMESA_DRAW_INVALID;
    if (info->mode != MESA_PRIM_TRIANGLES || info->primitive_restart ||
        info->has_user_indices || info->take_index_buffer_ownership ||
        info->instance_count > 1 || info->start_instance || vertices->is_user_buffer ||
        position->instance_divisor || position->dual_slot || position->vertex_buffer_index ||
        position->src_format != PIPE_FORMAT_R32G32B32A32_FLOAT || !position->src_stride ||
        draw->count % 3 || draw->count > REMESA_DRAW_MAX_INDICES)
        return REMESA_DRAW_UNSUPPORTED;
    if (!vertices->buffer.resource || vertices->buffer.resource->target != PIPE_BUFFER ||
        !vertex_snapshot || vertex_bytes < vertices->buffer.resource->width0)
        return REMESA_DRAW_INVALID;
    if (info->index_size) {
        status = remesa_validate_indexed_triangles(info, draw, vertices, position,
                                                  index_snapshot, index_bytes, &plan);
        if (status != REMESA_DRAW_VALIDATED && status != REMESA_DRAW_EMPTY) return status;
    } else if ((uint64_t)draw->start + draw->count > (uint64_t)UINT32_MAX + 1) {
        return REMESA_DRAW_INVALID;
    }
    result = calloc(1, sizeof(*result));
    if (!result) return REMESA_DRAW_NOMEM;
    result->width = width; result->height = height; result->argb = argb;
    base = (uint64_t)vertices->buffer_offset + position->src_offset;
    status = REMESA_DRAW_VALIDATED;
    for (unsigned triangle=0; info->instance_count && triangle < draw->count; triangle+=3) {
        float clip[3][4];
        struct remesa_clipped_polygon polygon;
        for (unsigned vertex=0; vertex<3; ++vertex) {
            unsigned index = triangle + vertex;
            uint32_t source = info->index_size ?
                (uint32_t)((int64_t)read_index((const unsigned char *)index_snapshot +
                     (size_t)plan.index_first_byte + (size_t)index * info->index_size,
                     info->index_size) + draw->index_bias) : draw->start + index;
            uint64_t available = vertices->buffer.resource->width0;
            /* Subtraction validates the fetch without overflowing addition or
             * source*stride, even for malformed UINT32_MAX-sized strides. */
            if (base > available || available-base < 16 ||
                source > (available-base-16) / position->src_stride) {
                status = REMESA_DRAW_INVALID; goto done;
            }
            memcpy(clip[vertex], (const unsigned char *)vertex_snapshot +
                   (size_t)(base + (uint64_t)source * position->src_stride), 16);
        }
        int error = remesa_prepare_constant_triangle((const float (*)[4])clip,
                                                     viewport,width,height,&polygon);
        if (error) {
            status = error == EINVAL ? REMESA_DRAW_INVALID : REMESA_DRAW_UNSUPPORTED;
            goto done;
        }
        for (unsigned vertex=2; vertex < polygon.count; ++vertex) {
            if (result->vertex_count > REMESA_DRAW_MAX_INDICES-3) {
                status = REMESA_DRAW_UNSUPPORTED; goto done;
            }
            memcpy(result->positions[result->vertex_count++],polygon.window[0],16);
            memcpy(result->positions[result->vertex_count++],polygon.window[vertex-1],16);
            memcpy(result->positions[result->vertex_count++],polygon.window[vertex],16);
        }
    }
    *out = *result;
    if (!result->vertex_count) status = REMESA_DRAW_EMPTY;
done:
    free(result);
    return status;
}

enum remesa_draw_status remesa_validate_indexed_triangles(
    const struct pipe_draw_info *info,
    const struct pipe_draw_start_count_bias *draw,
    const struct pipe_vertex_buffer *vertices,
    const struct pipe_vertex_element *position,
    const void *index_snapshot, size_t snapshot_bytes,
    struct remesa_draw_plan *plan)
{
    struct remesa_draw_plan p = {0};
    const struct pipe_resource *ib, *vb;
    uint64_t first, end, vertex_end;
    uint32_t min = UINT32_MAX, max = 0;
    unsigned i;
    if (plan) memset(plan, 0, sizeof(*plan));
    if (!info || !draw || !vertices || !position || !plan)
        return REMESA_DRAW_INVALID;
    if (info->mode != MESA_PRIM_TRIANGLES ||
        info->primitive_restart || info->has_user_indices ||
        info->take_index_buffer_ownership || vertices->is_user_buffer ||
        info->instance_count > 1 || info->start_instance ||
        position->instance_divisor || position->dual_slot ||
        position->vertex_buffer_index ||
        position->src_format != PIPE_FORMAT_R32G32B32A32_FLOAT)
        return REMESA_DRAW_UNSUPPORTED;
    if (!draw->count || !info->instance_count) return REMESA_DRAW_EMPTY;
    /* Legal Gallium inputs outside this first experimental packet subset. */
    if (draw->count % 3 || !position->src_stride || !info->index_size)
        return REMESA_DRAW_UNSUPPORTED;
    if (info->index_size != 1 && info->index_size != 2 && info->index_size != 4)
        return REMESA_DRAW_INVALID;
    ib = info->index.resource;
    vb = vertices->buffer.resource;
    if (!ib || !vb || ib->target != PIPE_BUFFER || vb->target != PIPE_BUFFER ||
        !index_snapshot) return REMESA_DRAW_INVALID;
    first = (uint64_t)draw->start * info->index_size;
    end = first + (uint64_t)draw->count * info->index_size;
    if (end > ib->width0 || end > snapshot_bytes) return REMESA_DRAW_INVALID;
    for (i = 0; i < draw->count; ++i) {
        uint32_t raw = read_index((const unsigned char *)index_snapshot +
                                  (size_t)first + (size_t)i * info->index_size,
                                  info->index_size);
        int64_t biased = (int64_t)raw + draw->index_bias;
        if (info->index_bounds_valid &&
            (raw < info->min_index || raw > info->max_index))
            return REMESA_DRAW_INVALID;
        if (biased < 0 || biased > UINT32_MAX) return REMESA_DRAW_INVALID;
        if ((uint32_t)biased < min) min = (uint32_t)biased;
        if ((uint32_t)biased > max) max = (uint32_t)biased;
    }
    /* Four float components. This validates only a position fetch, not a
     * complete vertex layout, shader input contract or command encoding. */
    uint64_t base = (uint64_t)vertices->buffer_offset + position->src_offset;
    if (base > vb->width0 || vb->width0-base < 16 ||
        max > (vb->width0-base-16) / position->src_stride)
        return REMESA_DRAW_INVALID;
    vertex_end = base + (uint64_t)max * position->src_stride + 16;
    p.primitive_count = draw->count / 3;
    p.min_vertex = min; p.max_vertex = max;
    p.index_first_byte = first; p.index_end_byte = end;
    p.vertex_end_byte = vertex_end;
    *plan = p;
    return REMESA_DRAW_VALIDATED;
}

enum remesa_draw_status remesa_pack_triangle_draw_vertex(
    const struct pipe_draw_info *info,
    const struct pipe_draw_start_count_bias *draw,
    const struct pipe_vertex_buffer *vertices,
    const struct pipe_vertex_element *position,
    const void *vertex_snapshot, size_t vertex_bytes,
    const void *index_snapshot, size_t index_bytes,
    uint32_t argb,const struct pipe_viewport_state *viewport, struct remesa_draw_packet *packet)
{
    struct remesa_draw_plan plan;
    float (*positions)[4] = NULL;
    uint32_t *source_ids = NULL, unique = 0, capacity;
    uint16_t *indices = NULL;
    enum remesa_draw_status status;
    enum remesa_packet_status built;
    uint64_t end;
    if (!info || !draw || !vertices || !position || !packet)
        return REMESA_DRAW_INVALID;
    if (viewport) {
        if (viewport->scale[0]!=16 || viewport->scale[1]!=16 || viewport->scale[2]!=.5f ||
            viewport->translate[0]!=16 || viewport->translate[1]!=16 || viewport->translate[2]!=.5f ||
            viewport->swizzle_x || viewport->swizzle_y || viewport->swizzle_z || viewport->swizzle_w)
            return REMESA_DRAW_UNSUPPORTED;
    }
    if (info->mode != MESA_PRIM_TRIANGLES || info->primitive_restart ||
        info->instance_count > 1 || info->start_instance || vertices->is_user_buffer ||
        position->instance_divisor || position->dual_slot || position->vertex_buffer_index ||
        position->src_format != PIPE_FORMAT_R32G32B32A32_FLOAT || !position->src_stride)
        return REMESA_DRAW_UNSUPPORTED;
    if (!draw->count || !info->instance_count) return REMESA_DRAW_EMPTY;
    if (draw->count % 3 || draw->count > REMESA_DRAW_MAX_INDICES)
        return REMESA_DRAW_UNSUPPORTED;
    if (!vertices->buffer.resource || vertices->buffer.resource->target != PIPE_BUFFER ||
        !vertex_snapshot || vertex_bytes < vertices->buffer.resource->width0)
        return REMESA_DRAW_INVALID;
    if (info->index_size) {
        status = remesa_validate_indexed_triangles(info, draw, vertices, position,
                                                  index_snapshot, index_bytes, &plan);
        if (status != REMESA_DRAW_VALIDATED) return status;
    } else {
        /* The index union and index_bias are unspecified for this draw kind. */
        end = (uint64_t)draw->start + draw->count;
        if (end > (uint64_t)UINT32_MAX + 1) return REMESA_DRAW_INVALID;
        uint64_t base = (uint64_t)vertices->buffer_offset + position->src_offset;
        uint64_t available = vertices->buffer.resource->width0;
        if (base > available || available-base < 16 ||
            end-1 > (available-base-16) / position->src_stride)
            return REMESA_DRAW_INVALID;
    }
    capacity = draw->count < REMESA_DRAW_MAX_VERTICES ? draw->count : REMESA_DRAW_MAX_VERTICES;
    positions = calloc(capacity, sizeof(*positions));
    source_ids = malloc(capacity * sizeof(*source_ids));
    indices = malloc(draw->count * sizeof(*indices));
    status = REMESA_DRAW_NOMEM;
    if (!positions || !source_ids || !indices) goto done;
    for (unsigned i = 0; i < draw->count; ++i) {
        uint32_t source = info->index_size ?
            (uint32_t)((int64_t)read_index((const unsigned char *)index_snapshot +
                (size_t)plan.index_first_byte + (size_t)i * info->index_size,
                info->index_size) + draw->index_bias) : draw->start + i;
        unsigned compact = 0;
        while (compact < unique && source_ids[compact] != source) ++compact;
        if (compact == unique) {
            if (unique == capacity) { status = REMESA_DRAW_UNSUPPORTED; goto done; }
            source_ids[unique] = source;
            memcpy(positions[unique], (const unsigned char *)vertex_snapshot +
                   (size_t)((uint64_t)vertices->buffer_offset + position->src_offset +
                            (uint64_t)source * position->src_stride), 16);
            if (viewport) {
                /* Identity VS semantics are host preparation, never CPU raster.
                 * All vertices lie inside canonical clip volume: no edges need
                 * clipping; w=1 avoids unqualified perspective interpolation. */
                for(unsigned axis=0;axis<4;axis++)
                    if(!isfinite(positions[unique][axis])) { status=REMESA_DRAW_INVALID;goto done; }
                if(positions[unique][3]!=1) { status=REMESA_DRAW_UNSUPPORTED;goto done; }
                for(unsigned axis=0;axis<3;axis++) {
                    if(positions[unique][axis]<-1 || positions[unique][axis]>1) { status=REMESA_DRAW_UNSUPPORTED;goto done; }
                    positions[unique][axis]=positions[unique][axis]*viewport->scale[axis]+viewport->translate[axis];
                }
            }
            ++unique;
        }
        indices[i] = (uint16_t)compact;
    }
    built = remesa_draw_packet_build(packet, (const float (*)[4])positions,
                                    unique, indices, draw->count, argb);
    status = built == REMESA_PACKET_OK ? REMESA_DRAW_VALIDATED :
             built == REMESA_PACKET_UNSUPPORTED ? REMESA_DRAW_UNSUPPORTED : REMESA_DRAW_INVALID;
done:
    free(positions); free(source_ids); free(indices);
    return status;
}

enum remesa_draw_status remesa_pack_triangle_draw(
 const struct pipe_draw_info *info,const struct pipe_draw_start_count_bias *draw,
 const struct pipe_vertex_buffer *vertices,const struct pipe_vertex_element *position,
 const void *vertex_snapshot,size_t vertex_bytes,const void *index_snapshot,size_t index_bytes,
 uint32_t argb,struct remesa_draw_packet *packet)
{
 return remesa_pack_triangle_draw_vertex(info,draw,vertices,position,vertex_snapshot,vertex_bytes,
                                        index_snapshot,index_bytes,argb,NULL,packet);
}
