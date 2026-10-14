/* SPDX-License-Identifier: MIT */
#include "remesa/framebuffer_observation.h"
#include <errno.h>

static int identity_valid(const struct remesa_framebuffer_observation_identity *i)
{
    return i && i->owner && i->resource && i->resource_generation &&
        i->mapping_generation && i->operation && i->operation_generation &&
        i->content_epoch;
}
static int identity_equal(const struct remesa_framebuffer_observation_identity *a,
                          const struct remesa_framebuffer_observation_identity *b)
{
    return a->owner == b->owner && a->resource == b->resource &&
        a->resource_generation == b->resource_generation &&
        a->mapping_generation == b->mapping_generation &&
        a->operation == b->operation &&
        a->operation_generation == b->operation_generation &&
        a->content_epoch == b->content_epoch;
}
static uint32_t pixel(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
        (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static int overlap(const void *a, size_t an, const void *b, size_t bn)
{
    uintptr_t aa = (uintptr_t)a, bb = (uintptr_t)b;
    return aa <= bb ? bb - aa < an : aa - bb < bn;
}
int remesa_framebuffer_observation_compare(
    const struct remesa_framebuffer_observation_layout *l,
    const struct remesa_framebuffer_observation_identity *expected,
    const struct remesa_framebuffer_observation_identity *observed,
    const void *before, const void *after, const void *draw,
    const unsigned char *coverage, size_t coverage_bytes, uint32_t discard,
    struct remesa_framebuffer_observation_report *report)
{
    const unsigned char *b = before, *a = after, *d = draw;
    struct remesa_framebuffer_observation_report r = {0};
    size_t pixels, row_bytes, required, distinct = 0, covered_changes = 0;
    if (!l || !before || !after || !draw || !coverage || !report ||
        !identity_valid(expected) || !identity_valid(observed)) return EINVAL;
    if (!identity_equal(expected, observed)) return ESTALE;
    if (!l->width || !l->height || !l->max_width || !l->max_height ||
        l->width > l->max_width || l->height > l->max_height ||
        !l->max_bytes) return EINVAL;
#if SIZE_MAX <= UINT32_MAX
    if (l->width > SIZE_MAX / 4) return EOVERFLOW;
#endif
    row_bytes = (size_t)l->width * 4;
    if (l->pitch < row_bytes || l->pitch % 4 ||
        l->height > SIZE_MAX / l->pitch || l->height > SIZE_MAX / l->width)
        return EOVERFLOW;
    required = (size_t)l->pitch * l->height;
    pixels = (size_t)l->width * l->height;
    if (required != l->bytes || required > l->max_bytes ||
        coverage_bytes != pixels) return EINVAL;
    if (overlap(report, sizeof(*report), before, required) ||
        overlap(report, sizeof(*report), after, required) ||
        overlap(report, sizeof(*report), draw, required) ||
        overlap(report, sizeof(*report), coverage, pixels) ||
        overlap(report, sizeof(*report), l, sizeof(*l)) ||
        overlap(report, sizeof(*report), expected, sizeof(*expected)) ||
        overlap(report, sizeof(*report), observed, sizeof(*observed))) return EINVAL;
    for (size_t y = 0; y < l->height; y++) {
        size_t row = y * l->pitch;
        for (size_t x = 0; x < l->width; x++) {
            size_t i = y * l->width + x, offset = row + x * 4;
            uint32_t old = pixel(b + offset), actual = pixel(a + offset);
            uint32_t preserve, replace;
            if (coverage[i] > 1) return EINVAL;
            preserve = coverage[i] ? pixel(d + offset) : old;
            replace = coverage[i] ? pixel(d + offset) : discard;
            distinct += preserve != replace;
            covered_changes += coverage[i] && preserve != old;
            r.preserve_pixel_mismatches += actual != preserve;
            r.discard_pixel_mismatches += actual != replace;
            r.changed_pixels += actual != old;
        }
        for (size_t x = row_bytes; x < l->pitch; x++)
            r.padding_byte_mismatches += a[row + x] != b[row + x];
    }
    if (!distinct || !covered_changes) return EINVAL;
    r.result = r.padding_byte_mismatches ? REMESA_FB_MISMATCH :
        !r.changed_pixels ? REMESA_FB_UNCHANGED :
        !r.preserve_pixel_mismatches ? REMESA_FB_PRESERVE_MATCH :
        !r.discard_pixel_mismatches ? REMESA_FB_DISCARD_MATCH : REMESA_FB_MISMATCH;
    *report = r;
    return 0;
}
