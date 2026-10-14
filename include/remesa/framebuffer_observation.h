/* SPDX-License-Identifier: MIT */
#ifndef REMESA_FRAMEBUFFER_OBSERVATION_H
#define REMESA_FRAMEBUFFER_OBSERVATION_H
#include <stddef.h>
#include <stdint.h>
/* CPU observation comparison only. Coverage is independently supplied expected
 * coverage, never inferred from the observed image. No SGX commands, rasterizer,
 * admission or GPU visibility proof is implemented here. */
struct remesa_framebuffer_observation_layout {
    uint32_t width, height, pitch;
    uint32_t max_width, max_height;
    size_t bytes, max_bytes;
};
struct remesa_framebuffer_observation_identity {
    uint64_t owner, resource, resource_generation, mapping_generation;
    uint64_t operation, operation_generation, content_epoch;
};
enum remesa_framebuffer_observation_result {
    REMESA_FB_PRESERVE_MATCH, REMESA_FB_DISCARD_MATCH,
    REMESA_FB_UNCHANGED, REMESA_FB_MISMATCH
};
struct remesa_framebuffer_observation_report {
    enum remesa_framebuffer_observation_result result;
    size_t preserve_pixel_mismatches, discard_pixel_mismatches;
    size_t changed_pixels, padding_byte_mismatches;
};
/* Exact identity must come from separately authenticated owning-service records.
 * Comparison success does not authenticate the caller's identity or GPU events.
 * report remains unchanged on malformed input or identity mismatch.
 * coverage contains width*height bytes, each 0 or 1. Covered pixels must equal
 * expected_draw; uncovered PRESERVE pixels must equal before; DISCARD pixels
 * must equal discard_argb. Row padding must remain unchanged in both modes.
 * Both predictions must differ, and at least one covered pixel must change.
 * All input buffers are read-only. */
int remesa_framebuffer_observation_compare(
    const struct remesa_framebuffer_observation_layout *layout,
    const struct remesa_framebuffer_observation_identity *expected_identity,
    const struct remesa_framebuffer_observation_identity *observed_identity,
    const void *before, const void *after, const void *expected_draw,
    const unsigned char *coverage, size_t coverage_bytes, uint32_t discard_argb,
    struct remesa_framebuffer_observation_report *report);
#endif
