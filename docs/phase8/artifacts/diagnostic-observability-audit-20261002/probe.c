/* New reporting edge cases only. Reuse the existing hardware-free fixture. */
#define main established_suite_not_run
#include "/home/gama/sgx535-gfx/tools/psb-dri-re/test_frozen_fixed_service.c"
#undef main

static int edge_sample(void *context, sgx535_u32 sequence,
                       sgx535_u32 *status1, sgx535_u32 *status2,
                       int *exclusive_owned)
{
    struct mock *m = context;
    if (sequence != 1U) return -1;
    m->samples++;
    *status1 = m->sample_mode == 1U ? (1U << 28) : (1U << 24);
    *status2 = 0;
    *exclusive_owned = m->sample_mode == 1U ? 1 : 0;
    return 0;
}

int main(void)
{
    const struct sgx535_fixed_ops ops = {run};
    const struct sgx535_fixed_status_source status_source = {edge_sample};
    unsigned mode;
    for (mode = 1; mode <= 2; mode++) {
        struct sgx535_fixed_service service;
        struct sgx535_frozen_session session;
        struct mock m = {0};
        int result;
        CHECK(new_service(&service, &session) == 0);
        m.sample_mode = mode;
        result = sgx535_fixed_service_run_once(&service, &ops, &status_source,
                         &m, 0x00010201U, 1U, 10U, 1U);
        CHECK(result == SGX535_FROZEN_BAD_REQUEST);
        CHECK(session.phase == SGX535_PHASE_HELD_AFTER_FAILURE);
        CHECK(session.observed_events == 0U);
        CHECK(service.diagnostic.stage_reached == SGX535_FIXED_STATUS_PROCESS);
        CHECK(service.diagnostic.observation_stage == SGX535_FIXED_STATUS_PROCESS);
        CHECK(service.diagnostic.failure_stage == SGX535_FIXED_STAGE_NONE);
        CHECK(service.diagnostic.failure_source ==
              (mode == 1 ? SGX535_FIXED_FAILURE_NONE : SGX535_FIXED_FAILURE_SERVICE_CHECK));
        CHECK(service.diagnostic.raw_result == (mode == 1 ? 0 : -1));
        CHECK(service.diagnostic.observation_status1 ==
              (mode == 1 ? (1U << 28) : (1U << 24)));
        CHECK(service.diagnostic.observation_status2 == 0U);
        CHECK(m.calls == 14U && m.samples == 1U);
        CHECK(!(m.seen & ((1U << SGX535_FIXED_ISP_RESET_ASSERT) |
                         (1U << SGX535_FIXED_ISP_RESET_CLEAR) |
                         (1U << SGX535_FIXED_RASTER_SCHEDULE) |
                         (1U << SGX535_FIXED_RASTER_FIRE) |
                         (1U << SGX535_FIXED_USE_RELEASE))));
        printf("{\"case\":\"%s\",\"result\":%d,\"phase\":%u,\"events\":%u,"
               "\"stage_reached\":%u,\"failure_stage\":%u,\"failure_source\":%u,"
               "\"raw_result\":%d,\"status1\":%u,\"callbacks\":%u,\"samples\":%u}\n",
               mode == 1 ? "recognized-fault" : "exclusive-ownership-rejected",
               result, (unsigned)session.phase, session.observed_events,
               (unsigned)service.diagnostic.stage_reached,
               (unsigned)service.diagnostic.failure_stage,
               (unsigned)service.diagnostic.failure_source,
               service.diagnostic.raw_result, service.diagnostic.observation_status1,
               m.calls, m.samples);
    }
    return 0;
}
