/* SPDX-License-Identifier: MIT */
#include "remesa/seeded_color.h"
#include <errno.h>

static int layout(const void *target, size_t bytes,
                  unsigned width, unsigned height, unsigned pitch)
{
    if (!target) return EINVAL;
    if (width != 32 || height != 32 || pitch != 128) return ENOTSUP;
    if (bytes != REMESA_SEEDED_COLOR_BYTES) return EINVAL;
    return 0;
}

int remesa_seeded_color_initialize(void *target, size_t bytes,
                                  unsigned width, unsigned height, unsigned pitch)
{
    int error = layout(target, bytes, width, height, pitch);
    unsigned char *p = target;
    if (error) return error;
    /* Explicit wire bytes, independent of host endian and mapping alignment. */
    for (size_t i = 0; i < bytes; i += 4) {
        p[i] = 0x30; p[i + 1] = 0x20; p[i + 2] = 0x10; p[i + 3] = 0xff;
    }
    return 0;
}

int remesa_seeded_color_validate(const void *target, size_t bytes,
                                unsigned width, unsigned height, unsigned pitch)
{
    int error = layout(target, bytes, width, height, pitch);
    const unsigned char *p = target;
    if (error) return error;
    for (size_t i = 0; i < bytes; i += 4)
        if (p[i] != 0x30 || p[i + 1] != 0x20 || p[i + 2] != 0x10 || p[i + 3] != 0xff)
            return EILSEQ;
    return 0;
}

static int identity_valid(const struct remesa_seeded_color_identity *id)
{
    if (!id || !id->owner || !id->resource || !id->generation ||
        !id->mapping_generation || !id->content_epoch || !id->gpu_va ||
        (id->gpu_va & 4095u) || id->gpu_va > UINT64_MAX - 4095u)
        return EINVAL;
    if (id->mapped_pages != 1 || id->verified_pages != 1 ||
        id->pins != 1 || id->leases != 1 || id->held_jobs || id->quarantined)
        return EBUSY;
    return 0;
}

static int identity_equal(const struct remesa_seeded_color_identity *a,
                          const struct remesa_seeded_color_identity *b)
{
    /* Compare members rather than struct padding, including on i386. */
    return a->owner == b->owner && a->resource == b->resource &&
        a->generation == b->generation && a->gpu_va == b->gpu_va &&
        a->mapping_generation == b->mapping_generation &&
        a->content_epoch == b->content_epoch &&
        a->mapped_pages == b->mapped_pages &&
        a->verified_pages == b->verified_pages && a->pins == b->pins &&
        a->leases == b->leases && a->held_jobs == b->held_jobs &&
        a->quarantined == b->quarantined;
}

static int overlaps(const void *a, size_t an, const void *b, size_t bn)
{
    uintptr_t aa = (uintptr_t)a, bb = (uintptr_t)b;
    if (!a || !b || !an || !bn) return 0;
    /* Difference form avoids wrapping one-past-end addresses. */
    return aa <= bb ? bb - aa < an : aa - bb < bn;
}

int remesa_seeded_color_prepare(struct remesa_seeded_color_binding *binding,
                               const struct remesa_seeded_color_identity *identity,
                               uint64_t operation, uint64_t operation_generation,
                               void *target, size_t bytes)
{
    int error;
    if (!binding || !operation || !operation_generation) return EINVAL;
    if (binding->state != REMESA_SEED_EMPTY) return EALREADY;
    if (overlaps(target, bytes, binding, sizeof(*binding)) ||
        overlaps(target, bytes, identity, sizeof(*identity))) return EINVAL;
    error = identity_valid(identity);
    if (error) return error;
    error = remesa_seeded_color_initialize(target, bytes, 32, 32, 128);
    if (error) return error;
    binding->identity = *identity;
    binding->operation = operation;
    binding->operation_generation = operation_generation;
    binding->state = REMESA_SEED_PREPARED;
    return 0;
}

int remesa_seeded_color_check(struct remesa_seeded_color_binding *binding,
                             const struct remesa_seeded_color_identity *identity,
                             uint64_t operation, uint64_t operation_generation,
                             const void *target, size_t bytes)
{
    int error;
    if (!binding) return EINVAL;
    if (binding->state != REMESA_SEED_PREPARED) return EALREADY;
    error = overlaps(target, bytes, binding, sizeof(*binding)) ||
        overlaps(target, bytes, identity, sizeof(*identity)) ? EINVAL :
        identity_valid(identity);
    if (!error && (!identity_equal(&binding->identity, identity) ||
        binding->operation != operation ||
        binding->operation_generation != operation_generation)) error = ESTALE;
    if (!error) error = remesa_seeded_color_validate(target, bytes, 32, 32, 128);
    binding->state = error ? REMESA_SEED_INVALID : REMESA_SEED_CHECKED;
    return error;
}
