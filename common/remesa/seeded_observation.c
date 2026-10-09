/* SPDX-License-Identifier: MIT */
#include "remesa/seeded_observation.h"
#include <errno.h>

/* Independently observed FIRE3 pixel coverage only, row-major bit i%8.
 * Source: phase9-visible-proven-20261009T052000Z/originals/triangle-evidence/
 * reference.bin SHA-256 52b2aeb7316820f955d1b9a46cb55a888efb8f74de916acd7ec7ddff6d99ebb5.
 * This identifies a captured output; it encodes no commands or raster rules. */
static const unsigned char fire3_coverage[128] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0xff,0x7f,0x00,0x00,0xff,0x3f,0x00,0x00,0xff,0x1f,0x00,0x00,0xff,0x0f,0x00,
    0x00,0xff,0x07,0x00,0x00,0xff,0x03,0x00,0x00,0xff,0x01,0x00,0x00,0xff,0x00,0x00,
    0x00,0x7f,0x00,0x00,0x00,0x3f,0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x0f,0x00,0x00,
    0x00,0x07,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static uint32_t pixel(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 |
        (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}
static int overlaps(const void *a, size_t an, const void *b, size_t bn)
{
    uintptr_t aa = (uintptr_t)a, bb = (uintptr_t)b;
    return aa <= bb ? bb-aa < an : aa-bb < bn;
}
int remesa_seeded_triangle_observe(
    const struct remesa_framebuffer_observation_identity *expected,
    const struct remesa_framebuffer_observation_identity *observed,
    const void *before, size_t before_bytes,
    const void *reference, size_t reference_bytes,
    const void *after, size_t after_bytes,
    const void *independent_after, size_t independent_after_bytes,
    struct remesa_seeded_observation_report *report)
{
    const unsigned char *b = before, *d = reference, *a = after;
    const unsigned char *independent = independent_after;
    unsigned char coverage[1024];
    unsigned magenta = 0;
    struct remesa_seeded_observation_report r = {0};
    const struct remesa_framebuffer_observation_layout layout = {
        32,32,128,32,32,4096,4096};
    int error;
    if (!expected || !observed || !before || !reference || !after ||
        !independent_after || !report || before_bytes != 4096 ||
        reference_bytes != 4096 || after_bytes != 4096 ||
        independent_after_bytes != 4096) return EINVAL;
    /* A pointer alias cannot represent a separately captured corroboration.
     * Distinct pointers are necessary, but do not authenticate its source. */
    if (overlaps(after,4096,independent_after,4096) ||
        overlaps(before,4096,after,4096) ||
        overlaps(before,4096,independent_after,4096) ||
        overlaps(reference,4096,before,4096) ||
        overlaps(reference,4096,after,4096) ||
        overlaps(reference,4096,independent_after,4096)) return EINVAL;
    if (overlaps(report,sizeof(*report),before,4096) ||
        overlaps(report,sizeof(*report),reference,4096) ||
        overlaps(report,sizeof(*report),after,4096) ||
        overlaps(report,sizeof(*report),independent_after,4096) ||
        overlaps(report,sizeof(*report),expected,sizeof(*expected)) ||
        overlaps(report,sizeof(*report),observed,sizeof(*observed))) return EINVAL;
    error = remesa_seeded_color_validate(before,before_bytes,32,32,128);
    if (error) return error;
    for (size_t i = 0; i < 1024; ++i) {
        uint32_t color = pixel(d + i*4);
        coverage[i] = (fire3_coverage[i/8] >> (i%8)) & 1u;
        if (color != (coverage[i] ? UINT32_C(0xffff00ff) : 0)) return EILSEQ;
        magenta += coverage[i];
    }
    if (magenta != 120) return EINVAL;
    error = remesa_framebuffer_observation_compare(&layout,expected,observed,
        before,after,reference,coverage,sizeof(coverage),0,&r.pixels);
    if (error) return error;
    for (size_t i = 0; i < 4096; ++i) {
        unsigned char preserved = coverage[i/4] ? d[i] : b[i];
        r.preserve_byte_mismatches += a[i] != preserved;
        r.discard_byte_mismatches += a[i] != d[i];
        r.corroboration_byte_mismatches += a[i] != independent[i];
    }
    *report = r;
    return r.corroboration_byte_mismatches ? EILSEQ : 0;
}
