#ifndef SGX535_FROZEN_SCENE_OWNER_H
#define SGX535_FROZEN_SCENE_OWNER_H

#include "frozen_kernel_contract.h"

/* These callbacks are internal to a future DRM owner. They are not an ioctl.
 * acquire/reserve must fail atomically. map may fail after partial insertion;
 * unmap must then remove every inserted PTE, including on a failed map.
 * Release callbacks return zero only after the resource is fully gone.
 */
struct sgx535_frozen_backend {
    int (*acquire)(void *context, sgx535_u32 role,
                   const struct sgx535_frozen_requirement *requirement,
                   sgx535_u64 *owner_token,
                   struct sgx535_frozen_cpu_view *view);
    int (*reserve_va)(void *context, sgx535_u32 role,
                      const struct sgx535_frozen_requirement *requirement,
                      sgx535_u64 *gpu_va, sgx535_u64 *reservation_token);
    int (*map)(void *context, sgx535_u32 role, sgx535_u64 owner_token,
               sgx535_u64 reservation_token, sgx535_u64 gpu_va);
    int (*unmap)(void *context, sgx535_u32 role, sgx535_u64 reservation_token);
    int (*release_va)(void *context, sgx535_u32 role,
                      sgx535_u64 reservation_token);
    int (*release)(void *context, sgx535_u32 role, sgx535_u64 owner_token);
};

struct sgx535_frozen_scene_owner {
    struct sgx535_frozen_bo bos[SGX535_BO_COUNT];
    struct sgx535_frozen_cpu_view views[SGX535_BO_COUNT];
    struct sgx535_frozen_command_pairs commands;
    sgx535_u64 reservation_tokens[SGX535_BO_COUNT];
    sgx535_u8 acquired[SGX535_BO_COUNT];
    sgx535_u8 reserved[SGX535_BO_COUNT];
    sgx535_u8 map_attempted[SGX535_BO_COUNT];
    struct sgx535_frozen_session session;
    sgx535_u32 initialized;
    sgx535_u32 cleanup_permitted_after_failure;
};

/* Every callback is required, even for a backend with no LOCAL SGX mapping. */
int sgx535_frozen_scene_create(struct sgx535_frozen_scene_owner *scene,
                               const struct sgx535_frozen_backend *backend,
                               void *context, sgx535_u64 mmu_end);
/* Caller must have constructed all canonical pre-relocation CPU bytes. This
 * performs the fixed patches once, captures the immutable TA/raster register
 * pairs, and advances to HOST_IMAGE_FINAL. */
int sgx535_frozen_scene_patch(struct sgx535_frozen_scene_owner *scene,
                              sgx535_u64 mmu_end,
                              const sgx535_u32 use_registers[2]);
/* Before submission or after proven completion only. A cleanup error holds
 * remaining resources and permits a later retry of cleanup, not submission. */
int sgx535_frozen_scene_destroy(struct sgx535_frozen_scene_owner *scene,
                                const struct sgx535_frozen_backend *backend,
                                void *context);

#endif
