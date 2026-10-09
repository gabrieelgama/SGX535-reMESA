/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SEEDED_COLOR_H
#define REMESA_SEEDED_COLOR_H
#include <stddef.h>
#include <stdint.h>

/* Host-side input for a future bounded observation, NOT SGX load/store state.
 * No hardware admission, ownership, mapping or cache visibility is implied.
 * Caller must own a writable CPU mapping of the entire target and separately
 * establish GPU visibility before submitting through a qualified endpoint.
 * Fixed 32x32 little-endian ARGB8888, pitch 128, sentinel 0xff102030.
 * Neither function accepts a larger target or padded/partial mapping.
 * Initialization failure leaves the target unchanged. */
#define REMESA_SEEDED_COLOR_BYTES 4096u
#define REMESA_SEEDED_COLOR_ARGB UINT32_C(0xff102030)
int remesa_seeded_color_initialize(void *target, size_t bytes,
                                  unsigned width, unsigned height, unsigned pitch);
int remesa_seeded_color_validate(const void *target, size_t bytes,
                                unsigned width, unsigned height, unsigned pitch);

/* CPU-side preparation bookkeeping, NOT a kernel ticket or visibility proof.
 * Identity values must come from the owning memory service, not framebuffer
 * pixels. Page ownership must be independently checked by the executor. A
 * capture does not prove that the GPU will observe those bytes.
 * Only one fixed-size, page-aligned mapped/pinned page is supported here. */
struct remesa_seeded_color_identity {
    uint64_t owner, resource, generation, gpu_va, mapping_generation;
    uint64_t content_epoch;
    uint32_t mapped_pages, verified_pages, pins, leases, held_jobs, quarantined;
};
enum remesa_seeded_color_state {
    REMESA_SEED_EMPTY = 0, REMESA_SEED_PREPARED, REMESA_SEED_CHECKED,
    REMESA_SEED_INVALID
};
struct remesa_seeded_color_binding {
    struct remesa_seeded_color_identity identity;
    uint64_t operation, operation_generation;
    enum remesa_seeded_color_state state;
};
/* Caller initializes binding to zero. Failure changes neither target nor
 * binding. Target must not overlap identity or binding bookkeeping.
 * PREPARED/CHECKED/INVALID bindings cannot be silently reused. */
int remesa_seeded_color_prepare(struct remesa_seeded_color_binding *binding,
                               const struct remesa_seeded_color_identity *identity,
                               uint64_t operation, uint64_t operation_generation,
                               void *target, size_t bytes);
/* Revalidate exact seed and every identity immediately before ending the CPU
 * lease. Success is single-use CHECKED, not permission to submit. GPU cache,
 * TLB visibility and qualified admission remain independent prerequisites.
 * Any failed check quarantines this bookkeeping binding as INVALID. */
int remesa_seeded_color_check(struct remesa_seeded_color_binding *binding,
                             const struct remesa_seeded_color_identity *identity,
                             uint64_t operation, uint64_t operation_generation,
                             const void *target, size_t bytes);
#endif
