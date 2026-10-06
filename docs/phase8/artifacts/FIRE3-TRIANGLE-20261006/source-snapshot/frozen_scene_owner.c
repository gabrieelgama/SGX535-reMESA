#include "frozen_scene_owner.h"

static int backend_valid(const struct sgx535_frozen_backend *backend)
{
    return backend != NULL && backend->acquire != NULL &&
           backend->reserve_va != NULL && backend->map != NULL &&
           backend->unmap != NULL && backend->release_va != NULL &&
           backend->release != NULL;
}

static int release_owned(struct sgx535_frozen_scene_owner *scene,
                         const struct sgx535_frozen_backend *backend,
                         void *context)
{
    size_t i;
    for (i = SGX535_BO_COUNT; i > 0; i--) {
        size_t role = i - 1;
        if (scene->map_attempted[role]) {
            if (backend->unmap(context, role,
                               scene->reservation_tokens[role]) != 0)
                return SGX535_FROZEN_BAD_OWNER;
            scene->map_attempted[role] = 0;
        }
        if (scene->reserved[role]) {
            if (backend->release_va(context, role,
                                    scene->reservation_tokens[role]) != 0)
                return SGX535_FROZEN_BAD_OWNER;
            scene->reserved[role] = 0;
        }
        if (scene->acquired[role]) {
            if (backend->release(context, role,
                                 scene->bos[role].owner_token) != 0)
                return SGX535_FROZEN_BAD_OWNER;
            scene->acquired[role] = 0;
        }
    }
    scene->initialized = 0;
    scene->session.phase = SGX535_PHASE_EMPTY;
    return SGX535_FROZEN_OK;
}

int sgx535_frozen_scene_create(struct sgx535_frozen_scene_owner *scene,
                               const struct sgx535_frozen_backend *backend,
                               void *context, sgx535_u64 mmu_end)
{
    static const struct sgx535_frozen_request fixed = {1, 1, 0, 0};
    size_t i;
    int result;

    if (scene == NULL || !backend_valid(backend) || scene->initialized)
        return SGX535_FROZEN_BAD_REQUEST;
    *scene = (struct sgx535_frozen_scene_owner){0};
    scene->initialized = 1;
    for (i = 0; i < SGX535_BO_COUNT; i++) {
        struct sgx535_frozen_requirement requirement;
        struct sgx535_frozen_bo *bo = &scene->bos[i];
        result = sgx535_frozen_get_requirement(i, &requirement);
        if (result != SGX535_FROZEN_OK)
            goto rollback;
        bo->role = i;
        bo->domain = requirement.domain;
        bo->size = requirement.size;
        result = backend->acquire(context, i, &requirement,
                                  &bo->owner_token, &scene->views[i]);
        if (result != 0)
            goto rollback;
        scene->acquired[i] = 1;
    }
    for (i = 0; i < SGX535_BO_COUNT; i++) {
        struct sgx535_frozen_requirement requirement;
        struct sgx535_frozen_bo *bo = &scene->bos[i];
        result = sgx535_frozen_get_requirement(i, &requirement);
        if (result != SGX535_FROZEN_OK)
            goto rollback;
        if (requirement.domain == SGX535_DOMAIN_LOCAL)
            continue;
        result = backend->reserve_va(context, i, &requirement,
                                     &bo->gpu_va,
                                     &scene->reservation_tokens[i]);
        if (result != 0)
            goto rollback;
        scene->reserved[i] = 1;
    }
    result = sgx535_frozen_session_begin(&scene->session, &fixed,
                                         sizeof(fixed), scene->bos,
                                         SGX535_BO_COUNT, mmu_end);
    if (result != SGX535_FROZEN_OK)
        goto rollback;
    for (i = 0; i <= SGX535_BO_COLOR; i++) {
        size_t previous;
        unsigned long start, end;
        if (scene->views[i].bytes == NULL ||
            scene->views[i].length != scene->bos[i].size) {
            result = SGX535_FROZEN_BAD_SIZE;
            goto rollback;
        }
        start = (unsigned long)scene->views[i].bytes;
        if (scene->views[i].length > ~0UL - start) {
            result = SGX535_FROZEN_BAD_SIZE;
            goto rollback;
        }
        end = start + scene->views[i].length;
        for (previous = 0; previous < i; previous++) {
            unsigned long other =
                (unsigned long)scene->views[previous].bytes;
            if (start < other + scene->views[previous].length && other < end) {
                result = SGX535_FROZEN_ALIAS;
                goto rollback;
            }
        }
    }
    for (i = 0; i <= SGX535_BO_COLOR; i++) {
        size_t byte;
        for (byte = 0; byte < scene->views[i].length; byte++)
            scene->views[i].bytes[byte] = 0;
    }
    for (i = 0; i < SGX535_BO_COUNT; i++) {
        const struct sgx535_frozen_bo *bo = &scene->bos[i];
        if (bo->domain == SGX535_DOMAIN_LOCAL)
            continue;
        scene->map_attempted[i] = 1;
        result = backend->map(context, i, bo->owner_token,
                              scene->reservation_tokens[i], bo->gpu_va);
        if (result != 0)
            goto rollback;
    }
    return SGX535_FROZEN_OK;
rollback:
    if (release_owned(scene, backend, context) != SGX535_FROZEN_OK) {
        scene->session.phase = SGX535_PHASE_HELD_AFTER_FAILURE;
        scene->cleanup_permitted_after_failure = 1;
        return SGX535_FROZEN_BAD_OWNER;
    }
    return result;
}

int sgx535_frozen_scene_patch(struct sgx535_frozen_scene_owner *scene,
                              sgx535_u64 mmu_end,
                              const sgx535_u32 use_registers[2])
{
    int result;
    if (scene == NULL || !scene->initialized ||
        scene->session.phase != SGX535_PHASE_BOS_VALIDATED)
        return SGX535_FROZEN_BAD_REQUEST;
    result = sgx535_frozen_apply_relocations(scene->bos, SGX535_BO_COUNT,
                                              mmu_end, scene->views,
                                              use_registers);
    if (result != SGX535_FROZEN_OK)
        return result;
    result = sgx535_frozen_extract_commands(
        &scene->views[SGX535_BO_CONTROL], &scene->commands);
    if (result != SGX535_FROZEN_OK)
        return result;
    return sgx535_frozen_session_advance(&scene->session,
                                         SGX535_PHASE_HOST_IMAGE_FINAL);
}

int sgx535_frozen_scene_destroy(struct sgx535_frozen_scene_owner *scene,
                                const struct sgx535_frozen_backend *backend,
                                void *context)
{
    int result;
    if (scene == NULL || !backend_valid(backend) || !scene->initialized ||
        (scene->session.phase >= SGX535_PHASE_FIRE_POSSIBLE &&
         scene->session.phase != SGX535_PHASE_SCENE_COMPLETED &&
         scene->session.phase != SGX535_PHASE_RETIRED &&
         scene->session.phase != SGX535_PHASE_ABORTED_BEFORE_SUBMIT &&
         !scene->cleanup_permitted_after_failure))
        return SGX535_FROZEN_BAD_REQUEST;
    result = release_owned(scene, backend, context);
    if (result != SGX535_FROZEN_OK) {
        scene->session.phase = SGX535_PHASE_HELD_AFTER_FAILURE;
        scene->cleanup_permitted_after_failure = 1;
    }
    return result;
}
