#ifndef SGX535_FROZEN_VA_POOL_H
#define SGX535_FROZEN_VA_POOL_H

#include "frozen_kernel_contract.h"

#define SGX535_FROZEN_VA_SLOTS 32

struct sgx535_frozen_va_slot {
    sgx535_u64 start;
    sgx535_u64 end;
    sgx535_u64 token;
    sgx535_u32 domain;
    sgx535_u32 in_use;
    sgx535_u32 external;
};

/* Pure reservation ledger. A kernel backend must hold a device-wide lock and
 * register ALL other PD mappings before use. This object alone cannot prove
 * that the actual default SGX page directory has no conflicting mappings. */
struct sgx535_frozen_va_pool {
    struct sgx535_frozen_va_slot slots[SGX535_FROZEN_VA_SLOTS];
    sgx535_u64 mmu_end;
    sgx535_u64 next_token;
};

int sgx535_frozen_va_pool_init(struct sgx535_frozen_va_pool *pool,
                               sgx535_u64 mmu_end);
int sgx535_frozen_va_claim_external(struct sgx535_frozen_va_pool *pool,
                                    sgx535_u32 domain, sgx535_u64 start,
                                    sgx535_u64 size);
int sgx535_frozen_va_reserve(struct sgx535_frozen_va_pool *pool,
                             sgx535_u32 domain, sgx535_u64 size,
                             sgx535_u32 alignment, sgx535_u64 *start,
                             sgx535_u64 *token);
int sgx535_frozen_va_release(struct sgx535_frozen_va_pool *pool,
                             sgx535_u64 token);

#endif
