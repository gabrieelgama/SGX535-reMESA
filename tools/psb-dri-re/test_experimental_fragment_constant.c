/* CPU construction tests only. Never opens a device or submits commands. */
#include "experimental_fragment_constant.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "failed at line %d\n", __LINE__); return 1; } } while (0)

struct seed { sgx535_u32 role, offset, value; };
static const struct seed historical_seeds[] = {
#include "frozen_kernel_initial.inc"
};

static void put32(sgx535_u8 *p, sgx535_u32 v)
{
    unsigned int i;
    for (i = 0; i < 4; i++) p[i] = (sgx535_u8)(v >> (8U * i));
}

int main(void)
{
    struct sgx535_frozen_cpu_view views[SGX535_BO_COUNT] = {{0}};
    sgx535_u8 *expected[6] = {0};
    sgx535_u32 w[2], v = 0;
    size_t i, j, changed = 0;
    const sgx535_u32 colors[] = {
        0, 0xffffffffU, 0xffff00ffU, 0xff00ff00U,
        0x001fffffU, 0x00200000U, 0x04000000U, 0x80000000U
    };
    sgx535_experimental_fragment_constant(0xffff00ffU, w);
    CHECK(w[0] == 0x001f00ffU && w[1] == 0xfca7f1f1U);
    for (i = 0; i < 1032; i++) {
        v = i < 8 ? colors[i] : v * 1664525U + 1013904223U;
        sgx535_experimental_fragment_constant(v, w);
        CHECK((w[0] | (((w[1] >> 4) & 31U) << 21)
                         | (((w[1] >> 12) & 63U) << 26)) == v);
        CHECK((w[1] & ~0x0003f1f0U) == 0xfca40001U);
    }
    for (i = 0; i < 6; i++) {
        struct sgx535_frozen_requirement r;
        CHECK(sgx535_frozen_get_requirement((sgx535_u32)i, &r) == 0);
        views[i].length = r.size;
        views[i].bytes = malloc((size_t)r.size);
        expected[i] = calloc(1, (size_t)r.size);
        CHECK(views[i].bytes && expected[i]);
        memset(views[i].bytes, 0xa5, (size_t)r.size);
    }
    for (i = 0; i < sizeof(historical_seeds)/sizeof(historical_seeds[0]); i++) {
        const struct seed *s = &historical_seeds[i];
        put32(expected[s->role]+s->offset, s->value);
    }
#ifdef SGX535_EXPERIMENTAL_CONSTANT_FRAGMENT
    sgx535_experimental_fragment_constant(0xffff00ffU, w);
    put32(expected[SGX535_BO_USE], w[0]);
    put32(expected[SGX535_BO_USE]+4, w[1]);
#endif
    CHECK(sgx535_frozen_initialize_user_images(views) == 0);
    for (i = 0; i < 6; i++)
        CHECK(memcmp(views[i].bytes, expected[i], (size_t)views[i].length) == 0);
    /* A bad view must fail before modifying any backing. */
    views[SGX535_BO_USE].length--;
    CHECK(sgx535_frozen_initialize_user_images(views) != 0);
    views[SGX535_BO_USE].length++;
    for (i = 0; i < 6; i++)
        CHECK(memcmp(views[i].bytes, expected[i], (size_t)views[i].length) == 0);
    /* Confirm the complete backing, including zero color, remains stable. */
    CHECK(sgx535_frozen_initialize_user_images(views) == 0);
    for (i = 0; i < 6; i++) {
        for (j = 0; j < (size_t)views[i].length; j++)
            if (views[i].bytes[j] != expected[i][j]) changed++;
        free(views[i].bytes); free(expected[i]);
    }
    CHECK(changed == 0);
    puts("1032 immediate roundtrips; full six-backing construction; invalid-view no-write; repeat construction PASS (CPU ONLY)");
    return 0;
}
