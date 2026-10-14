/* SPDX-License-Identifier: MIT; CPU preparation only, NO GPU execution. */
#include "remesa/seeded_color.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error "Seeded COLOR qualification requires enabled assertions"
#endif
static struct remesa_seeded_color_identity identity(void)
{
    struct remesa_seeded_color_identity id = {
        .owner = 11, .resource = 22, .generation = 3, .gpu_va = 0x100000,
        .mapping_generation = 4, .content_epoch = 5, .mapped_pages = 1,
        .verified_pages = 1, .pins = 1, .leases = 1
    };
    return id;
}

static void binding_cases(void)
{
    unsigned char target[4096], original[4096];
    struct remesa_seeded_color_binding binding = {0}, saved;
    struct remesa_seeded_color_identity id = identity(), changed;
    memset(target, 0xa5, sizeof(target));
    memcpy(original, target, sizeof(target));
    saved = binding;
    changed = id; changed.pins = 0;
    assert(remesa_seeded_color_prepare(&binding, &changed, 1, 1, target, 4096) == EBUSY);
    assert(!memcmp(target, original, sizeof(target)));
    assert(!memcmp(&binding, &saved, sizeof(binding)));
    assert(remesa_seeded_color_prepare(&binding, &id, 0, 1, target, 4096) == EINVAL);
    assert(remesa_seeded_color_prepare(&binding, &id, 1, 1, &binding, 4096) == EINVAL);
    assert(remesa_seeded_color_prepare(&binding, &id, 1, 1, &id, 4096) == EINVAL);
    assert(binding.state == REMESA_SEED_EMPTY && id.owner == 11);
    assert(remesa_seeded_color_prepare(&binding, &id, 1, 1, target, 4095) == EINVAL);
    assert(binding.state == REMESA_SEED_EMPTY);
    assert(!remesa_seeded_color_prepare(&binding, &id, 1, 1, target, 4096));
    assert(binding.state == REMESA_SEED_PREPARED);
    assert(remesa_seeded_color_prepare(&binding, &id, 2, 2, target, 4096) == EALREADY);
    assert(!remesa_seeded_color_check(&binding, &id, 1, 1, target, 4096));
    assert(binding.state == REMESA_SEED_CHECKED);
    assert(remesa_seeded_color_check(&binding, &id, 1, 1, target, 4096) == EALREADY);

    /* Independently mutate each ownership / mapping / operation prerequisite. */
    for (unsigned field = 0; field < 16; field++) {
        binding = (struct remesa_seeded_color_binding){0};
        id = identity();
        assert(!remesa_seeded_color_prepare(&binding, &id, 8, 9, target, 4096));
        changed = id;
        uint64_t operation = 8, generation = 9;
        switch (field) {
        case 0: changed.owner++; break;
        case 1: changed.resource++; break;
        case 2: changed.generation++; break;
        case 3: changed.gpu_va += 4096; break;
        case 4: changed.mapping_generation++; break;
        case 5: changed.content_epoch++; break;
        case 6: changed.mapped_pages = 0; break;
        case 7: changed.verified_pages = 0; break;
        case 8: changed.pins = 0; break;
        case 9: changed.leases = 0; break;
        case 10: changed.held_jobs = 1; break;
        case 11: changed.quarantined = 1; break;
        case 12: operation++; break;
        case 13: generation++; break;
        case 14: target[0] ^= 1; break;
        case 15: changed.gpu_va++; break;
        }
        assert(remesa_seeded_color_check(&binding, &changed, operation, generation,
                                        target, 4096) != 0);
        assert(binding.state == REMESA_SEED_INVALID);
        assert(remesa_seeded_color_check(&binding, &id, 8, 9, target, 4096) == EALREADY);
        assert(remesa_seeded_color_prepare(&binding, &id, 10, 10, target, 4096) == EALREADY);
    }
    binding = (struct remesa_seeded_color_binding){0};
    id = identity(); id.gpu_va = UINT64_MAX;
    assert(remesa_seeded_color_prepare(&binding, &id, 1, 1, target, 4096) == EINVAL);
    id = identity(); id.leases = 2;
    assert(remesa_seeded_color_prepare(&binding, &id, 1, 1, target, 4096) == EBUSY);
    id = identity(); id.owner = 0;
    assert(remesa_seeded_color_prepare(&binding, &id, 1, 1, target, 4096) == EINVAL);
}
int main(void)
{
    unsigned char backing[4098], original[4098];
    /* Unaligned target and adjacent canaries must work on native and i386. */
    memset(backing, 0xa5, sizeof(backing));
    assert(!remesa_seeded_color_initialize(backing + 1, 4096, 32, 32, 128));
    assert(backing[0] == 0xa5 && backing[4097] == 0xa5);
    assert(!remesa_seeded_color_validate(backing + 1, 4096, 32, 32, 128));
    for (unsigned i = 1; i < 4097; i += 4) {
        assert(backing[i] == 0x30 && backing[i + 1] == 0x20);
        assert(backing[i + 2] == 0x10 && backing[i + 3] == 0xff);
    }
    memcpy(original, backing, sizeof(backing));
    assert(remesa_seeded_color_initialize(backing + 1, 4095, 32, 32, 128) == EINVAL);
    assert(remesa_seeded_color_initialize(backing + 1, 4097, 32, 32, 128) == EINVAL);
    assert(remesa_seeded_color_initialize(backing + 1, 4096, 320, 240, 1280) == ENOTSUP);
    assert(remesa_seeded_color_initialize(backing + 1, 4096, 32, 32, 132) == ENOTSUP);
    assert(remesa_seeded_color_initialize(NULL, 4096, 32, 32, 128) == EINVAL);
    assert(!memcmp(original, backing, sizeof(backing)));
    backing[4096] ^= 1;
    assert(remesa_seeded_color_validate(backing + 1, 4096, 32, 32, 128) == EILSEQ);
    assert(remesa_seeded_color_validate(backing + 1, 4095, 32, 32, 128) == EINVAL);
    binding_cases();
    puts("PASS seeded COLOR CPU initializer and binding: exact bytes, ownership, mapping epochs, operation generations, single-use, quarantine, bounds; NO GPU visibility proof");
    return 0;
}
