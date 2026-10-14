/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SEEDED_OBSERVATION_H
#define REMESA_SEEDED_OBSERVATION_H
#include "remesa/framebuffer_observation.h"
#include "remesa/seeded_color.h"

/* Comparison of captured CPU bytes only. This API authenticates neither GPU
 * execution nor visibility. The caller must independently authenticate both
 * readbacks and the exact resource/operation identities. The reference is
 * validated against every sealed FIRE3 pixel, not merely color counts.
 * This experiment deliberately retains the qualified 32x32 profile; it is not
 * a hardware dimension limit or a generalized SGX surface encoding. */
struct remesa_seeded_observation_report {
    struct remesa_framebuffer_observation_report pixels;
    size_t preserve_byte_mismatches, discard_byte_mismatches;
    size_t corroboration_byte_mismatches;
};
/* All four byte spans must be exactly 4096 bytes. Before must be the qualified
 * uniform seed; reference must exactly match the independently observed FIRE3 pixels.
 * Coverage comes solely from the independently sealed reference, never after.
 * UNCHANGED is a failed negative control, not evidence of preservation.
 * All four byte spans must not alias; disjointness is not proof of
 * independent capture. A contradictory readback returns EILSEQ with metrics; all other
 * input failures leave report unchanged. No input buffer is modified. */
int remesa_seeded_triangle_observe(
    const struct remesa_framebuffer_observation_identity *expected_identity,
    const struct remesa_framebuffer_observation_identity *observed_identity,
    const void *before, size_t before_bytes,
    const void *reference, size_t reference_bytes,
    const void *after, size_t after_bytes,
    const void *independent_after, size_t independent_after_bytes,
    struct remesa_seeded_observation_report *report);
#endif
