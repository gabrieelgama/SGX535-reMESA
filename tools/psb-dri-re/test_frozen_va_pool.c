#include "frozen_va_pool.h"

#include <stdio.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; \
} } while (0)

int main(void)
{
    struct sgx535_frozen_va_pool pool = {0};
    struct sgx535_frozen_bo bos[SGX535_BO_COUNT] = {{0}};
    sgx535_u64 tokens[SGX535_BO_COUNT] = {0};
    sgx535_u64 first, token;
    size_t i;

    CHECK(sgx535_frozen_va_pool_init(&pool, 0x80000000) == 0);
    CHECK(sgx535_frozen_va_claim_external(&pool, SGX535_DOMAIN_MMU,
                                          0x40000000, 0x10000) == 0);
    CHECK(sgx535_frozen_va_claim_external(&pool, SGX535_DOMAIN_MMU,
                                          0x40008000, 0x1000) == SGX535_FROZEN_ALIAS);
    CHECK(sgx535_frozen_va_claim_external(&pool, SGX535_DOMAIN_MMU,
                                          0x40000001, 0x1000) == SGX535_FROZEN_BAD_ALIGNMENT);
    for (i = 0; i < SGX535_BO_COUNT; i++) {
        struct sgx535_frozen_requirement req;
        CHECK(sgx535_frozen_get_requirement(i, &req) == 0);
        bos[i].role = i;
        bos[i].domain = req.domain;
        bos[i].size = req.size;
        bos[i].owner_token = i + 1;
        if (req.domain == SGX535_DOMAIN_LOCAL)
            continue;
        CHECK(sgx535_frozen_va_reserve(&pool, req.domain, req.size,
                                       req.alignment, &bos[i].gpu_va,
                                       &tokens[i]) == 0);
    }
    CHECK(bos[SGX535_BO_VERTEX_TA].gpu_va == 0x40010000);
    CHECK(sgx535_frozen_validate_bos(bos, SGX535_BO_COUNT,
                                      0x80000000) == 0);
    CHECK(sgx535_frozen_va_reserve(&pool, SGX535_DOMAIN_LOCAL, 0x1000,
                                   0x1000, &first, &token) != 0);
    CHECK(sgx535_frozen_va_reserve(&pool, SGX535_DOMAIN_PDS, 0x1000,
                                   3, &first, &token) ==
          SGX535_FROZEN_BAD_ALIGNMENT);
    for (i = SGX535_BO_COUNT; i > 0; i--) {
        size_t role = i - 1;
        if (tokens[role])
            CHECK(sgx535_frozen_va_release(&pool, tokens[role]) == 0);
    }
    CHECK(sgx535_frozen_va_release(&pool, tokens[SGX535_BO_PDS]) ==
          SGX535_FROZEN_BAD_OWNER);
    CHECK(sgx535_frozen_va_reserve(&pool, SGX535_DOMAIN_MMU, 0x1000,
                                   0x1000, &first, &token) == 0);
    CHECK(first == 0x40010000); /* The external GTT claim still excludes it. */
    CHECK(sgx535_frozen_va_release(&pool, token) == 0);
    return 0;
}
