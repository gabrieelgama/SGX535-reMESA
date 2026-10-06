/* CPU-only integration with the real qualified acceptance/service contract. */
#define main unchanged_service_regression_main
#include "test_frozen_fixed_service.c"
#undef main
#include "frozen_capsule.h"

static struct sgx535_capsule evidence;
static const struct sgx535_fixed_ops observed_ops;
static const struct sgx535_fixed_status_source observed_source;
static unsigned taps;

static int observed_run(void *m, enum sgx535_fixed_stage stage,
    const void *p, size_t n, struct sgx535_fixed_observation *o)
{
    if (stage == SGX535_FIXED_TA_FIRE) sgx535_capsule_issued(&evidence,m);
    return run(m,stage,p,n,o);
}
static int observed_sample(void *m, sgx535_u32 q, sgx535_u32 *a,
    sgx535_u32 *d, int *exclusive)
{
    int result = sample_and_ack(m,q,a,d,exclusive);
    if (!result) sgx535_capsule_sample(&evidence,m,q,*a,*d,*exclusive);
    return result;
}
static void begin(void *b, struct sgx535_fixed_service *s,
    const struct sgx535_fixed_ops *ops,
    const struct sgx535_fixed_status_source *source, void *context)
{
    taps++;
    sgx535_capsule_service(&evidence,b,s,
        ops == &observed_ops && source == &observed_source && context == b);
}
static void before(void *b, struct sgx535_fixed_service *s, sgx535_u32 q,
    sgx535_u32 a, sgx535_u32 d, int exclusive)
{
    taps++;
    sgx535_capsule_before(&evidence,b,s,q,a,d,exclusive);
}
static void after(void *b, struct sgx535_fixed_service *s)
{
    taps++;
    sgx535_capsule_after(&evidence,b,s,s->session->observed_events,s->session->phase);
}
static void terminal(void *b, struct sgx535_fixed_service *s,int result)
{
    taps++;
    sgx535_capsule_terminal(&evidence,b,s,result,s->session->phase,
        s->session->observed_events);
}
static const struct sgx535_fixed_ops observed_ops = {observed_run};
static const struct sgx535_fixed_status_source observed_source = {observed_sample};
static const struct sgx535_fixed_service_observer observer = {begin,before,after,terminal};

int main(void)
{
    struct sgx535_fixed_service service, control;
    struct sgx535_frozen_session session, control_session;
    struct mock m, baseline;
    unsigned mode;
    int result, original;
    sgx535_u8 color[4096], saved[4096];

    memset(color,0xa5,sizeof(color));
    for (mode=1; mode<=7; mode++) {
        CHECK(new_service(&service,&session)==0);
        CHECK(new_service(&control,&control_session)==0);
        memset(&evidence,0,sizeof(evidence));
        memset(&m,0,sizeof(m)); memset(&baseline,0,sizeof(baseline));
        m.sample_mode = baseline.sample_mode = mode;
        taps=0;
        sgx535_capsule_call(&evidence,&m);
        sgx535_capsule_admit(&evidence,&m,&m,&service,&session,
            session.phase,session.observed_events,session.sequence,1);
        service.observer=&observer; service.observer_context=&m;
        result=sgx535_fixed_service_run_once(&service,&observed_ops,
            &observed_source,&m,0x00010201,42,10,8);
        {
            const struct sgx535_fixed_ops ops={run};
            const struct sgx535_fixed_status_source source={sample_and_ack};
            original=sgx535_fixed_service_run_once(&control,&ops,&source,
                &baseline,0x00010201,42,10,8);
        }
        CHECK(taps>=2);
        CHECK(result==original);
        CHECK(session.phase==control_session.phase);
        CHECK(session.observed_events==control_session.observed_events);
        CHECK(m.calls==baseline.calls && m.samples==baseline.samples);
        CHECK(!memcmp(m.stages,baseline.stages,sizeof(m.stages)));
        if (!result) {
            CHECK(!evidence.invalid);
            CHECK(evidence.events==7 && evidence.phase==SGX535_PHASE_RETIRED);
            sgx535_capsule_color(&evidence,&session,session.phase,color,sizeof(color));
            sgx535_capsule_release(&evidence,&session);
        } else {
            CHECK(!sgx535_capsule_success(&evidence));
            CHECK(evidence.terminal_present && evidence.result==result);
        }
        sgx535_capsule_close(&evidence,&m,result ? -5 : 0);
        CHECK(!!sgx535_capsule_success(&evidence)==!result);
        memcpy(saved,evidence.color,sizeof(saved));
        sgx535_capsule_call(&evidence,&m);
        CHECK(evidence.invalid && !sgx535_capsule_success(&evidence));
        CHECK(!memcmp(saved,evidence.color,sizeof(saved)));
    }
    puts("7 service/observer differential cases PASS; synthetic CPU only");
    return 0;
}
