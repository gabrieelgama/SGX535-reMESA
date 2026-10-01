#include "frozen_fixed_service.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; \
} } while (0)

static const sgx535_u32 ta_offsets[7] = {
    0x204, 0x218, 0x23c, 0x240, 0x244, 0x250, 0x238
};
static const sgx535_u32 raster_offsets[26] = {
    0x400, 0x404, 0x40c, 0x410, 0x414, 0x418, 0x41c, 0x420,
    0x424, 0x42c, 0x480, 0xcb0, 0x484, 0x488, 0x48c, 0x490,
    0x494, 0x4c4, 0x4bc, 0x4b8, 0x4c8, 0x4dc, 0x800, 0xa5c,
    0xa60, 0xa64
};

struct mock {
    unsigned calls;
    unsigned fail_at;
    unsigned seen;
    unsigned bad_status;
    unsigned bad_baseline;
    unsigned samples;
    unsigned sample_mode;
};

static int sample_and_ack(void *context, sgx535_u32 sequence,
                          sgx535_u32 *status1, sgx535_u32 *status2,
                          int *exclusive_owned)
{
    struct mock *m = context;
    if (!sequence || !status1 || !status2 || !exclusive_owned)
        return -1;
    m->samples++;
    if (m->sample_mode == 3)
        return -1;
    *status1 = 0;
    *status2 = 0;
    *exclusive_owned = 1;
    if (m->sample_mode == 1)
        *status1 = m->samples == 1 ? (1U << 13) :
                   ((1U << 18) | 1U);
    if (m->sample_mode == 4)
        *status1 = 1U << 13;
    if (m->sample_mode == 5)
        *status1 = 1U << 23;
    if (m->sample_mode == 6)
        *status1 = m->samples == 1 ? (1U << 24) :
                   (m->samples == 2 ? (1U << 13) :
                    ((1U << 18) | 1U));
    if (m->sample_mode == 7)
        *status1 = 1U << 24;
    return 0;
}

static int run(void *context, enum sgx535_fixed_stage stage,
               const void *payload, size_t count,
               struct sgx535_fixed_observation *observation)
{
    struct mock *m = context;
    m->calls++;
    m->seen |= 1U << stage;
    if (m->calls == m->fail_at)
        return -1;
    if (stage == SGX535_FIXED_INIT_WRITES && count != 12)
        return -1;
    if (stage == SGX535_FIXED_TA_LOAD) {
        const struct sgx535_frozen_reg_action *actions = payload;
        if (count != 29 || actions[0].offset != 0x618)
            return -1;
        observation->load_flags = 0x1f;
        observation->status2 = m->bad_status ? 3 : 7;
        observation->initend = 0x400000;
    }
    if (stage == SGX535_FIXED_STATUS_BASELINE && m->bad_baseline)
        observation->status1 = 1U << 13;
    if (stage == SGX535_FIXED_TA_SCHEDULE) {
        const struct sgx535_frozen_reg_action *actions = payload;
        if (count != 8 || actions[0].offset != 0x204 ||
            actions[7].kind != SGX535_REG_WMB)
            return -1;
    }
    if (stage == SGX535_FIXED_ISP_RESET_ASSERT ||
        stage == SGX535_FIXED_ISP_RESET_CLEAR) {
        const struct sgx535_frozen_reg_action *actions = payload;
        if (count != 1 || actions[0].offset != 0x80 ||
            actions[0].value !=
                (stage == SGX535_FIXED_ISP_RESET_ASSERT ? 0x20U : 0U))
            return -1;
    }
    if (stage == SGX535_FIXED_RASTER_SCHEDULE) {
        const struct sgx535_frozen_reg_action *actions = payload;
        if (count != 27 || actions[0].offset != 0x400 ||
            actions[26].kind != SGX535_REG_WMB)
            return -1;
    }
    if (stage == SGX535_FIXED_TA_FIRE || stage == SGX535_FIXED_RASTER_FIRE) {
        const struct sgx535_fixed_fire_payload *fire = payload;
        if (count != 1 || fire->wire.words[0] != 2 ||
            fire->wire.words[23] !=
                (stage == SGX535_FIXED_RASTER_FIRE ? 1U : 0U) ||
            fire->action_count !=
                (stage == SGX535_FIXED_RASTER_FIRE ? 20U : 31U) ||
            fire->actions[fire->action_count - 1].offset !=
                (stage == SGX535_FIXED_RASTER_FIRE ? 0x428U : 0x200U))
            return -1;
    }
    return 0;
}

static void valid_bos(struct sgx535_frozen_bo *bos)
{
    static const sgx535_u64 addresses[SGX535_BO_COUNT] = {
        0x20000000, 0x20080000, 0x40000000, 0x30000000, 0,
        0x40001000, 0x40002000, 0x42000000, 0x31000000, 0
    };
    sgx535_u32 i;
    for (i = 0; i < SGX535_BO_COUNT; i++) {
        struct sgx535_frozen_requirement req;
        sgx535_frozen_get_requirement(i, &req);
        bos[i] = (struct sgx535_frozen_bo){i, req.domain, req.size,
                                           addresses[i], i + 1U};
    }
}

static void valid_commands(struct sgx535_frozen_command_pairs *commands)
{
    size_t i;
    memset(commands, 0, sizeof(*commands));
    for (i = 0; i < 7; i++)
        commands->ta[2 * i] = ta_offsets[i];
    for (i = 0; i < 26; i++)
        commands->raster[2 * i] = raster_offsets[i];
}

static int new_service(struct sgx535_fixed_service *service,
                       struct sgx535_frozen_session *session)
{
    static const struct sgx535_frozen_request request = {1, 1, 0, 0};
    struct sgx535_frozen_bo bos[SGX535_BO_COUNT];
    struct sgx535_frozen_command_pairs commands;
    valid_bos(bos);
    valid_commands(&commands);
    memset(session, 0, sizeof(*session));
    memset(service, 0, sizeof(*service));
    if (sgx535_frozen_session_begin(session, &request, sizeof(request),
                                    bos, SGX535_BO_COUNT, 0x80000000))
        return -1;
    if (sgx535_frozen_session_advance(session, SGX535_PHASE_HOST_IMAGE_FINAL))
        return -1;
    return sgx535_fixed_service_bind(service, session, bos, SGX535_BO_COUNT,
                                     0x80000000, &commands);
}

int main(void)
{
    const struct sgx535_fixed_ops ops = {run};
    const struct sgx535_fixed_status_source source = {sample_and_ack};
    struct sgx535_fixed_service service;
    struct sgx535_frozen_session session;
    struct mock m;
    unsigned fail_at, total_stages;

    CHECK(new_service(&service, &session) == 0);
    CHECK(sgx535_fixed_service_may_release(&service) == 1);
    memset(&m, 0, sizeof(m));
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) == 0);
    CHECK(session.phase == SGX535_PHASE_SERVICE_READY);
    CHECK(sgx535_fixed_service_fire_ta(&service, &ops, &m, 42, 10) == 0);
    CHECK(session.phase == SGX535_PHASE_FIRE_POSSIBLE);
    CHECK(sgx535_fixed_service_may_release(&service) == 0);
    CHECK(sgx535_fixed_service_fire_ta(&service, &ops, &m, 43, 10) != 0);
    CHECK(sgx535_fixed_service_status(&service, &ops, &m, 42,
                                      1U << 18, 0, 1) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);

    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) == 0);
    CHECK(sgx535_fixed_service_fire_ta(&service, &ops, &m, 44, 10) == 0);
    {
        struct sgx535_fixed_service wrong = service;
        struct sgx535_frozen_session wrong_session = session;
        wrong.session = &wrong_session;
        CHECK(sgx535_fixed_service_status(&wrong, &ops, &m, 45,
                                          1U << 13, 0, 1) != 0);
        CHECK(wrong_session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    }
    CHECK(sgx535_fixed_service_status(&service, &ops, &m, 44,
                                      1U << 13, 0, 1) == 0);
    CHECK(session.raster_started == 1);
    CHECK(sgx535_fixed_service_status(&service, &ops, &m, 44,
                                      (1U << 18) | 1U, 0, 1) == 0);
    CHECK(session.phase == SGX535_PHASE_SCENE_COMPLETED);
    CHECK(sgx535_fixed_service_may_release(&service) == 0);
    CHECK(sgx535_fixed_service_retire(&service, &ops, &m) == 0);
    CHECK(session.phase == SGX535_PHASE_RETIRED);
    CHECK(sgx535_fixed_service_may_release(&service) == 1);
    CHECK(m.seen == ((1U << 20) - 2U));
    total_stages = m.calls;
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) != 0);

    for (fail_at = 1; fail_at <= total_stages; fail_at++) {
        CHECK(new_service(&service, &session) == 0);
        memset(&m, 0, sizeof(m));
        m.fail_at = fail_at;
        if (sgx535_fixed_service_prepare(&service, &ops, &m,
                                         0x00010201) == 0) {
            if (sgx535_fixed_service_fire_ta(&service, &ops, &m,
                                              50 + fail_at, 10) == 0 &&
                sgx535_fixed_service_status(&service, &ops, &m,
                                             50 + fail_at, 1U << 13,
                                             0, 1) == 0 &&
                sgx535_fixed_service_status(&service, &ops, &m,
                                             50 + fail_at,
                                             (1U << 18) | 1U, 0, 1) == 0)
                (void)sgx535_fixed_service_retire(&service, &ops, &m);
        }
        CHECK(m.calls == fail_at);
        CHECK(session.phase == (fail_at <= 2
              ? SGX535_PHASE_ABORTED_BEFORE_SUBMIT
              : SGX535_PHASE_HELD_AFTER_FAILURE));
        CHECK(sgx535_fixed_service_may_release(&service) == (fail_at <= 2));
    }
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.bad_status = 1;
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.bad_baseline = 1;
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) == 0);
    CHECK(sgx535_fixed_service_fire_ta(&service, &ops, &m, 77, 10) == 0);
    CHECK(sgx535_fixed_service_timeout(&service) == 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(sgx535_fixed_service_status(&service, &ops, &m, 77,
                                      1U << 13, 0, 1) != 0);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    CHECK(sgx535_fixed_service_prepare(&service, &ops, &m,
                                       0x00010201) == 0);
    CHECK(sgx535_fixed_service_fire_ta(&service, &ops, &m, 78, 10) == 0);
    CHECK(sgx535_fixed_service_status(&service, &ops, &m, 78,
                                      1U << 28, 0, 1) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(sgx535_fixed_service_may_release(&service) == 0);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 1;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 90, 10, 4) == 0);
    CHECK(session.phase == SGX535_PHASE_RETIRED);
    CHECK(m.samples == 2);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 5;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 94, 10, 3) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(m.samples == 1);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 2;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 91, 10, 3) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(m.samples == 3);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 3;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 92, 10, 3) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(m.samples == 1);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 4;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 93, 10, 3) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(m.samples == 2);
    /* DPM_TA_MEM_FREE is a scene-memory event, not completion or a fault.
     * A duplicate is contradictory and must hold the scene. */
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 6;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 95, 10, 4) == 0);
    CHECK(session.phase == SGX535_PHASE_RETIRED);
    CHECK(session.ta_memory_free_seen == 1);
    CHECK(m.samples == 3);
    CHECK(new_service(&service, &session) == 0);
    memset(&m, 0, sizeof(m));
    m.sample_mode = 7;
    CHECK(sgx535_fixed_service_run_once(&service, &ops, &source, &m,
                                        0x00010201, 96, 10, 4) != 0);
    CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
    CHECK(m.samples == 2);
    return 0;
}
