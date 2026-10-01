#include "frozen_va_pool.h"

static int bounds(const struct sgx535_frozen_va_pool *pool,
                  sgx535_u32 domain, sgx535_u64 *lo, sgx535_u64 *hi)
{
    if (pool == NULL || pool->mmu_end <= 0x40000000ULL ||
        pool->mmu_end > 0x100000000ULL)
        return SGX535_FROZEN_BAD_MMU_END;
    if (domain == SGX535_DOMAIN_PDS) {
        *lo = 0x20000000ULL;
        *hi = 0x30000000ULL;
    } else if (domain == SGX535_DOMAIN_RASTGEOM) {
        *lo = 0x30000000ULL;
        *hi = 0x40000000ULL;
    } else if (domain == SGX535_DOMAIN_MMU) {
        *lo = 0x40000000ULL;
        *hi = pool->mmu_end;
    } else {
        return SGX535_FROZEN_BAD_DOMAIN;
    }
    return SGX535_FROZEN_OK;
}

static int vacant_slot(struct sgx535_frozen_va_pool *pool)
{
    int i;
    for (i = 0; i < SGX535_FROZEN_VA_SLOTS; i++) {
        if (!pool->slots[i].in_use)
            return i;
    }
    return -1;
}

static int insert(struct sgx535_frozen_va_pool *pool, sgx535_u32 domain,
                  sgx535_u64 start, sgx535_u64 size, int external,
                  sgx535_u64 *token)
{
    sgx535_u64 lo, hi;
    int i, slot;
    if (bounds(pool, domain, &lo, &hi) != SGX535_FROZEN_OK ||
        size == 0 || start < lo || start >= hi || size > hi - start)
        return SGX535_FROZEN_BAD_ADDRESS;
    for (i = 0; i < SGX535_FROZEN_VA_SLOTS; i++) {
        const struct sgx535_frozen_va_slot *other = &pool->slots[i];
        if (other->in_use && start < other->end && other->start < start + size)
            return SGX535_FROZEN_ALIAS;
    }
    slot = vacant_slot(pool);
    if (slot < 0 || pool->next_token == 0 ||
        pool->next_token == 0xffffffffffffffffULL)
        return SGX535_FROZEN_BAD_OWNER;
    pool->slots[slot] = (struct sgx535_frozen_va_slot){
        start, start + size, pool->next_token, domain, 1, (sgx535_u32)external
    };
    *token = pool->next_token++;
    return SGX535_FROZEN_OK;
}

int sgx535_frozen_va_pool_init(struct sgx535_frozen_va_pool *pool,
                               sgx535_u64 mmu_end)
{
    if (pool == NULL || mmu_end <= 0x40000000ULL ||
        mmu_end > 0x100000000ULL || (mmu_end & 0xfffU))
        return SGX535_FROZEN_BAD_MMU_END;
    *pool = (struct sgx535_frozen_va_pool){0};
    pool->mmu_end = mmu_end;
    pool->next_token = 1;
    return SGX535_FROZEN_OK;
}

int sgx535_frozen_va_claim_external(struct sgx535_frozen_va_pool *pool,
                                    sgx535_u32 domain, sgx535_u64 start,
                                    sgx535_u64 size)
{
    sgx535_u64 ignored;
    if ((start & 0xfffU) || (size & 0xfffU))
        return SGX535_FROZEN_BAD_ALIGNMENT;
    return insert(pool, domain, start, size, 1, &ignored);
}

int sgx535_frozen_va_reserve(struct sgx535_frozen_va_pool *pool,
                             sgx535_u32 domain, sgx535_u64 size,
                             sgx535_u32 alignment, sgx535_u64 *start,
                             sgx535_u64 *token)
{
    sgx535_u64 lo, hi, candidate;
    int i, result;
    if (start == NULL || token == NULL || alignment < 0x1000 ||
        (alignment & (alignment - 1U)) != 0 || size == 0 ||
        (size & 0xfffU))
        return SGX535_FROZEN_BAD_ALIGNMENT;
    result = bounds(pool, domain, &lo, &hi);
    if (result != SGX535_FROZEN_OK)
        return result;
    candidate = (lo + alignment - 1U) & ~(sgx535_u64)(alignment - 1U);
    while (candidate < hi && size <= hi - candidate) {
        sgx535_u64 conflicting_end = 0;
        for (i = 0; i < SGX535_FROZEN_VA_SLOTS; i++) {
            const struct sgx535_frozen_va_slot *other = &pool->slots[i];
            if (other->in_use && candidate < other->end &&
                other->start < candidate + size &&
                other->end > conflicting_end)
                conflicting_end = other->end;
        }
        if (conflicting_end == 0) {
            result = insert(pool, domain, candidate, size, 0, token);
            if (result == SGX535_FROZEN_OK)
                *start = candidate;
            return result;
        }
        if (conflicting_end > hi || alignment - 1U > hi - conflicting_end)
            break;
        candidate = (conflicting_end + alignment - 1U) &
                    ~(sgx535_u64)(alignment - 1U);
    }
    return SGX535_FROZEN_BAD_ADDRESS;
}

int sgx535_frozen_va_release(struct sgx535_frozen_va_pool *pool,
                             sgx535_u64 token)
{
    int i;
    if (pool == NULL || token == 0)
        return SGX535_FROZEN_BAD_OWNER;
    for (i = 0; i < SGX535_FROZEN_VA_SLOTS; i++) {
        if (pool->slots[i].in_use && pool->slots[i].token == token &&
            !pool->slots[i].external) {
            pool->slots[i].in_use = 0;
            return SGX535_FROZEN_OK;
        }
    }
    return SGX535_FROZEN_BAD_OWNER;
}
