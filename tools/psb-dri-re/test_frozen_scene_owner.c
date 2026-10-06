#include "frozen_scene_owner.h"
#include "frozen_va_pool.h"

#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; \
} } while (0)

struct mock {
    int step;
    int fail_step;
    int fail_unmap;
    int duplicate_va;
    int duplicate_owner;
    int omit_view;
    int partial_cpu_alias;
    int overflowing_cpu_view;
    int adjacent_cpu_view;
    int map_calls;
    unsigned acquired;
    unsigned reserved;
    unsigned mapped;
};

/* Extra fixture capacity lets USE occupy the disjoint range just after PDS. */
static sgx535_u8 pds[0x28000], use[0x8000], vertex[0x1000];
static sgx535_u8 background[0x1000], control[0x1000], color[0x1000];
static sgx535_u8 *const bytes[6] = {
    pds, use, vertex, background, control, color
};
static const sgx535_u64 address[SGX535_BO_COUNT] = {
    0x20000000, 0x20080000, 0x40000000, 0x30000000, 0,
    0x40001000, 0x40002000, 0x42000000, 0x31000000, 0
};

static int fail(struct mock *m)
{
    m->step++;
    return m->step == m->fail_step;
}

static int acquire(void *context, sgx535_u32 role,
                   const struct sgx535_frozen_requirement *requirement,
                   sgx535_u64 *token, struct sgx535_frozen_cpu_view *view)
{
    struct mock *m = context;
    if (fail(m)) return -1;
    m->acquired |= 1U << role;
    *token = m->duplicate_owner && role == SGX535_BO_COLOR
        ? SGX535_BO_VERTEX_TA + 1 : role + 1;
    if (role < 6 && !(m->omit_view && role == SGX535_BO_COLOR)) {
        view->bytes = bytes[role];
        view->length = requirement->size;
        if (m->partial_cpu_alias && role == SGX535_BO_USE)
            view->bytes = pds + 0x1000;
        if (m->overflowing_cpu_view && role == SGX535_BO_PDS)
            view->bytes = (sgx535_u8 *)(~0UL - 0xfffUL);
        if (m->adjacent_cpu_view && role == SGX535_BO_USE)
            view->bytes = pds + 0x20000;
    }
    return 0;
}

static int reserve_va(void *context, sgx535_u32 role,
                      const struct sgx535_frozen_requirement *requirement,
                      sgx535_u64 *gpu_va, sgx535_u64 *reservation)
{
    struct mock *m = context;
    (void)requirement;
    if (fail(m)) return -1;
    m->reserved |= 1U << role;
    *gpu_va = m->duplicate_va && role == SGX535_BO_COLOR
        ? address[SGX535_BO_VERTEX_TA] : address[role];
    *reservation = role + 100;
    return 0;
}

static int map(void *context, sgx535_u32 role, sgx535_u64 owner,
               sgx535_u64 reservation, sgx535_u64 gpu_va)
{
    struct mock *m = context;
    m->map_calls++;
    if (owner != role + 1 || reservation != role + 100 ||
        gpu_va != address[role]) return -1;
    m->mapped |= 1U << role; /* Model even a failed map as partially inserted. */
    if (fail(m)) return -1;
    return 0;
}

static int unmap(void *context, sgx535_u32 role, sgx535_u64 reservation)
{
    struct mock *m = context;
    if (m->fail_unmap || reservation != role + 100) return -1;
    m->mapped &= ~(1U << role);
    return 0;
}

static int release_va(void *context, sgx535_u32 role,
                      sgx535_u64 reservation)
{
    struct mock *m = context;
    if (reservation != role + 100 || (m->mapped & (1U << role))) return -1;
    m->reserved &= ~(1U << role);
    return 0;
}

static int release(void *context, sgx535_u32 role, sgx535_u64 owner)
{
    struct mock *m = context;
    sgx535_u64 expected = m->duplicate_owner && role == SGX535_BO_COLOR
        ? SGX535_BO_VERTEX_TA + 1 : role + 1;
    if (owner != expected || (m->reserved & (1U << role))) return -1;
    m->acquired &= ~(1U << role);
    return 0;
}

static const struct sgx535_frozen_backend backend = {
    acquire, reserve_va, map, unmap, release_va, release
};

struct pool_mock {
    struct sgx535_frozen_va_pool pool;
    unsigned acquired;
    unsigned mapped;
};

static int pool_acquire(void *context, sgx535_u32 role,
                        const struct sgx535_frozen_requirement *req,
                        sgx535_u64 *owner,
                        struct sgx535_frozen_cpu_view *view)
{
    struct pool_mock *m = context;
    m->acquired |= 1U << role;
    *owner = role + 1;
    if (role < 6) {
        view->bytes = bytes[role];
        view->length = req->size;
    }
    return 0;
}

static int pool_reserve(void *context, sgx535_u32 role,
                        const struct sgx535_frozen_requirement *req,
                        sgx535_u64 *gpu_va, sgx535_u64 *reservation)
{
    struct pool_mock *m = context;
    (void)role;
    return sgx535_frozen_va_reserve(&m->pool, req->domain, req->size,
                                    req->alignment, gpu_va, reservation);
}

static int pool_map(void *context, sgx535_u32 role, sgx535_u64 owner,
                    sgx535_u64 reservation, sgx535_u64 gpu_va)
{
    struct pool_mock *m = context;
    if (owner != role + 1 || !reservation || !gpu_va) return -1;
    m->mapped |= 1U << role;
    return 0;
}

static int pool_unmap(void *context, sgx535_u32 role,
                      sgx535_u64 reservation)
{
    struct pool_mock *m = context;
    if (!reservation) return -1;
    m->mapped &= ~(1U << role);
    return 0;
}

static int pool_release_va(void *context, sgx535_u32 role,
                           sgx535_u64 reservation)
{
    struct pool_mock *m = context;
    if (m->mapped & (1U << role)) return -1;
    return sgx535_frozen_va_release(&m->pool, reservation);
}

static int pool_release(void *context, sgx535_u32 role,
                        sgx535_u64 owner)
{
    struct pool_mock *m = context;
    if (owner != role + 1) return -1;
    m->acquired &= ~(1U << role);
    return 0;
}

static const struct sgx535_frozen_backend pool_backend = {
    pool_acquire, pool_reserve, pool_map, pool_unmap,
    pool_release_va, pool_release
};

static int advance_to_submit(struct sgx535_frozen_scene_owner *scene)
{
    struct sgx535_frozen_bootstrap boot = {0};
    int next;
    const enum sgx535_frozen_boot_event events[] = {
        SGX535_BOOT_INIT_WRITES_RETURN, SGX535_BOOT_XHW_INIT_REPLY,
        SGX535_BOOT_TA_INFO_REPLY, SGX535_BOOT_SCENE_INFO_REPLY,
        SGX535_BOOT_LOAD_KICKS, SGX535_BOOT_LOAD_STATUS2,
        SGX535_BOOT_INITEND_STATUS, SGX535_BOOT_TA_LOAD_REPLY,
        SGX535_BOOT_SCENE_VALIDATED
    };
    const sgx535_u32 values[] = {0, 0, 0, 0, 0x1f, 15, 0x400000, 0, 0};
    size_t i;

    if (sgx535_frozen_bootstrap_begin(&boot, 0x00010201))
        return -1;
    for (i = 0; i < sizeof(events) / sizeof(events[0]); i++)
        if (sgx535_frozen_bootstrap_observe(&boot, events[i], values[i]))
            return -1;
    for (next = SGX535_PHASE_CPU_PUBLISHED;
         next <= SGX535_PHASE_DEVICE_MAINTAINED; next++) {
        if (sgx535_frozen_session_advance(&scene->session, next) != 0)
            return -1;
    }
    if (sgx535_frozen_session_service_ready(&scene->session, &boot, 1))
        return -1;
    return sgx535_frozen_session_enter_fire(&scene->session, 1, 1);
}

int main(void)
{
    struct sgx535_frozen_scene_owner scene;
    struct mock m;
    struct pool_mock pm;
    const sgx535_u32 use_registers[2] = {0, 1};
    int failed, steps;

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    pds[0] = 0xa5;
    use[0x7fff] = 0xa5;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m, 0x80000000) == 0);
    CHECK(pds[0] == 0 && use[0x7fff] == 0);
    CHECK(scene.session.phase == SGX535_PHASE_BOS_VALIDATED);
    CHECK(m.acquired == (1U << SGX535_BO_COUNT) - 1U);
    CHECK(sgx535_frozen_initialize_user_images(scene.views) == 0);
    CHECK(sgx535_frozen_scene_patch(&scene, 0x80000000,
                                    use_registers) == 0);
    CHECK(scene.session.phase == SGX535_PHASE_HOST_IMAGE_FINAL);
    CHECK(scene.commands.raster[0] == 0x400);
    CHECK(scene.commands.raster[50] == 0xa64);
    CHECK(scene.commands.ta[0] == 0x204);
    CHECK(sgx535_frozen_scene_patch(&scene, 0x80000000,
                                    use_registers) != 0);
    CHECK(sgx535_frozen_scene_destroy(&scene, &backend, &m) == 0);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    memset(&scene, 0, sizeof(scene));
    memset(&pm, 0, sizeof(pm));
    CHECK(sgx535_frozen_va_pool_init(&pm.pool, 0x80000000) == 0);
    CHECK(sgx535_frozen_va_claim_external(&pm.pool, SGX535_DOMAIN_MMU,
                                          0x40000000, 0x10000) == 0);
    CHECK(sgx535_frozen_scene_create(&scene, &pool_backend, &pm,
                                     0x80000000) == 0);
    CHECK(scene.bos[SGX535_BO_VERTEX_TA].gpu_va == 0x40010000);
    CHECK(pm.acquired == (1U << SGX535_BO_COUNT) - 1U);
    CHECK(sgx535_frozen_scene_destroy(&scene, &pool_backend, &pm) == 0);
    CHECK(pm.acquired == 0 && pm.mapped == 0);
    steps = m.step;
    CHECK(steps == 26); /* 10 acquisitions, eight VA reserves and maps. */

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    m.duplicate_owner = 1;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m,
                                     0x80000000) == SGX535_FROZEN_BAD_OWNER);
    CHECK(m.map_calls == 0);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    /* Distinct pointers can still describe overlapping CPU backings. Reject
     * before zeroing either view or inserting any GPU mapping. */
    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    m.partial_cpu_alias = 1;
    pds[0] = pds[0x1000] = 0xa5;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m,
                                     0x80000000) == SGX535_FROZEN_ALIAS);
    CHECK(pds[0] == 0xa5 && pds[0x1000] == 0xa5);
    CHECK(m.map_calls == 0 && !scene.initialized);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    m.overflowing_cpu_view = 1;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m,
                                     0x80000000) == SGX535_FROZEN_BAD_SIZE);
    CHECK(m.map_calls == 0 && !scene.initialized);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    m.adjacent_cpu_view = 1;
    pds[0x27fff] = 0xa5;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m, 0x80000000) == 0);
    CHECK(pds[0x27fff] == 0);
    CHECK(sgx535_frozen_scene_destroy(&scene, &backend, &m) == 0);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    for (failed = 1; failed <= steps; failed++) {
        memset(&scene, 0, sizeof(scene));
        memset(&m, 0, sizeof(m));
        m.fail_step = failed;
        CHECK(sgx535_frozen_scene_create(&scene, &backend, &m,
                                         0x80000000) != 0);
        CHECK(!scene.initialized);
        CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);
    }

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    m.duplicate_va = 1;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m,
                                     0x80000000) == SGX535_FROZEN_ALIAS);
    CHECK(m.map_calls == 0);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    m.omit_view = 1;
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m,
                                     0x80000000) == SGX535_FROZEN_BAD_SIZE);
    CHECK(m.map_calls == 0);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m, 0x80000000) == 0);
    m.fail_unmap = 1;
    CHECK(sgx535_frozen_scene_destroy(&scene, &backend, &m) != 0);
    CHECK(scene.initialized && m.mapped);
    m.fail_unmap = 0;
    CHECK(sgx535_frozen_scene_destroy(&scene, &backend, &m) == 0);
    CHECK(m.acquired == 0 && m.reserved == 0 && m.mapped == 0);

    memset(&scene, 0, sizeof(scene));
    memset(&m, 0, sizeof(m));
    CHECK(sgx535_frozen_scene_create(&scene, &backend, &m, 0x80000000) == 0);
    CHECK(sgx535_frozen_initialize_user_images(scene.views) == 0);
    CHECK(sgx535_frozen_scene_patch(&scene, 0x80000000,
                                    use_registers) == 0);
    CHECK(advance_to_submit(&scene) == 0);
    CHECK(sgx535_frozen_scene_destroy(&scene, &backend, &m) != 0);
    CHECK(m.acquired && m.mapped);
    CHECK(sgx535_frozen_session_abort(&scene.session) == 0);
    CHECK(sgx535_frozen_scene_destroy(&scene, &backend, &m) != 0);
    CHECK(m.acquired && m.mapped);
    CHECK(sgx535_frozen_session_advance(&scene.session,
                                        SGX535_PHASE_SCENE_COMPLETED) != 0);
    /* State stays held; tests must not manufacture completion to free it. */
    CHECK(m.acquired && m.reserved && m.mapped);
    return 0;
}
