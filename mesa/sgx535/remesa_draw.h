/* SPDX-License-Identifier: MIT */
#ifndef REMESA_DRAW_H
#define REMESA_DRAW_H
#include <stddef.h>
#include <stdint.h>
#include "pipe/p_state.h"
#include "remesa/draw_packet.h"
/* Supported host position fetch widths. Missing FLOAT2 z/w are 0/1;
 * FLOAT3 w is 1. Unsupported formats return zero; no GPU ISA is generated. */
unsigned remesa_position_format_bytes(enum pipe_format);
/* Resolution-independent CPU primitive preparation, not an SGX packet.
 * Triangle vertices are duplicated in primitive order after clipping; only
 * constant fragment output is admitted (no varying interpolation). */
struct remesa_prepared_triangles {
    uint32_t width, height, argb, vertex_count;
    float positions[REMESA_DRAW_MAX_INDICES][4];
};

/* CPU-only validation, not a Gallium draw callback or an SGX submission.
 * Borrowed resources and the explicit index snapshot must remain immutable
 * throughout the call. index_snapshot starts at index-resource byte zero,
 * not at draw->start. No reference is adopted, retained or released.
 * VALIDATED means position/index ranges pass this diagnostic subset only;
 * it does not establish Gallium feature support or hardware correctness. */
enum remesa_draw_status {
    REMESA_DRAW_VALIDATED, REMESA_DRAW_EMPTY,
    REMESA_DRAW_INVALID, REMESA_DRAW_UNSUPPORTED, REMESA_DRAW_NOMEM
};
/* Output unchanged on rejection or capacity exhaustion. Empty success sets
 * vertex_count=0. Hardware capabilities and submission must be admitted by a
 * separate backend, which may reject all but its historical 32x32 profile. */
enum remesa_draw_status remesa_prepare_constant_draw(
    const struct pipe_draw_info *, const struct pipe_draw_start_count_bias *,
    const struct pipe_vertex_buffer *, const struct pipe_vertex_element *,
    const void *, size_t, const void *, size_t, uint32_t,
    const struct pipe_viewport_state *, uint32_t, uint32_t,
    struct remesa_prepared_triangles *);
struct remesa_draw_plan {
    uint32_t primitive_count, min_vertex, max_vertex;
    uint64_t index_first_byte, index_end_byte, vertex_end_byte;
};
enum remesa_draw_status remesa_validate_indexed_triangles(
    const struct pipe_draw_info *info,
    const struct pipe_draw_start_count_bias *draw,
    const struct pipe_vertex_buffer *vertices,
    const struct pipe_vertex_element *position,
    const void *index_snapshot, size_t snapshot_bytes,
    struct remesa_draw_plan *plan);
/* Gather referenced positions from strided/offset buffer snapshots. Indexed
 * draws accept native 8/16/32-bit indices and signed index bias. Nonindexed
 * draws never read the index union or index bias. Sparse source indices are
 * remapped by first occurrence; primitive order and duplicate references are
 * preserved. Only active positions are inspected. Output is unchanged on
 * failure. This translates host data, not shaders or SGX command encodings.
 * The backend must still enforce its independently qualified hardware subset. */
enum remesa_draw_status remesa_pack_triangle_draw(
    const struct pipe_draw_info *info,
    const struct pipe_draw_start_count_bias *draw,
    const struct pipe_vertex_buffer *vertices,
    const struct pipe_vertex_element *position,
    const void *vertex_snapshot, size_t vertex_bytes,
    const void *index_snapshot, size_t index_bytes,
    uint32_t argb, struct remesa_draw_packet *packet);
/* Optional restricted host clip-to-window preparation for an admitted identity
 * position VS. Canonical 32x32 viewport only; w=1, finite xyz within [-1,1].
 * No clipping, perspective interpolation or CPU rasterization is supplied. */
enum remesa_draw_status remesa_pack_triangle_draw_vertex(
    const struct pipe_draw_info *,const struct pipe_draw_start_count_bias *,
    const struct pipe_vertex_buffer *,const struct pipe_vertex_element *,
    const void *,size_t,const void *,size_t,uint32_t,
    const struct pipe_viewport_state *,struct remesa_draw_packet *);
#endif
