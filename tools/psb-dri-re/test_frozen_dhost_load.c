/* Synthetic MMIO model for the actual generated load plan and executor.
 * No device, SGX operation or claim about native completion timing. */
#define main existing_contract_regression_main
#include "test_frozen_kernel_contract.c"
#undef main
#include "frozen_fixed_io.h"

struct load_model {
    sgx535_u32 status2, status1;
    unsigned delayed, samples, kicks, acknowledgements, init_kicks;
    unsigned missing, bad_read, stuck_clear;
    sgx535_u32 acknowledged;
};
static int model_write(void *v, sgx535_u32 reg, sgx535_u32 value)
{
    struct load_model *m = v;
    if (reg == 0x684) { m->status2 |= 1; m->kicks++; }
    if (reg == 0x680) { m->status2 |= 2; m->kicks++; }
    if (reg == 0x688) { m->status2 |= 4; m->kicks++; }
    if (reg == 0x690) {
        m->kicks++;
        if (!m->delayed && !m->missing) m->status2 |= 8;
    }
    if (reg == 0x114) {
        m->acknowledgements++; m->acknowledged |= value;
        if (!m->stuck_clear) m->status2 &= ~value;
    }
    if (reg == 0x6a8) { m->init_kicks++; m->status1 |= 0x400000; }
    if (reg == 0x134) m->status1 &= ~value;
    return 0;
}
static int model_read(void *v, sgx535_u32 reg, sgx535_u32 *value)
{
    struct load_model *m = v;
    if (reg == 0x118) {
        if (m->bad_read) return -1;
        m->samples++;
        if (m->delayed && !m->missing && m->samples == m->delayed)
            m->status2 |= 8;
        *value = m->status2;
    } else *value = m->status1;
    return 0;
}
static int model_barrier(void *v) { (void)v; return 0; }
static const struct sgx535_fixed_io model_io = {model_write,model_read,model_barrier};
int main(void)
{
    struct sgx535_frozen_bo bos[SGX535_BO_COUNT];
    struct sgx535_frozen_reg_action plan[29], old[29];
    struct load_model m;
    struct sgx535_frozen_session session;
    valid_bos(bos);
    CHECK(sgx535_frozen_ta_load_plan(bos,SGX535_BO_COUNT,0x50000000ULL,plan,29),0);
    memset(&m,0,sizeof(m));
    CHECK(sgx535_fixed_run_actions(&model_io,&m,plan,29,8),0);
    CHECK(m.kicks,4); CHECK(m.acknowledged,15); CHECK(m.status2,0); CHECK(m.init_kicks,1);
    memset(&m,0,sizeof(m)); m.delayed=4;
    CHECK(sgx535_fixed_run_actions(&model_io,&m,plan,29,8),0);
    CHECK(m.samples,5); CHECK(m.acknowledged,15); CHECK(m.status2,0);
    memset(&m,0,sizeof(m)); m.missing=1;
    CHECK(sgx535_fixed_run_actions(&model_io,&m,plan,29,8),-1);
    CHECK(m.status2,7); CHECK(m.acknowledgements,0); CHECK(m.init_kicks,0);
    memset(&m,0,sizeof(m)); m.bad_read=1;
    CHECK(sgx535_fixed_run_actions(&model_io,&m,plan,29,8),-1);
    CHECK(m.acknowledgements,0); CHECK(m.init_kicks,0);
    memset(&m,0,sizeof(m)); m.stuck_clear=1;
    CHECK(sgx535_fixed_run_actions(&model_io,&m,plan,29,8),-1);
    CHECK(m.status2,15); CHECK(m.init_kicks,0);
    memset(&m,0,sizeof(m)); m.status2=16;
    CHECK(sgx535_fixed_run_actions(&model_io,&m,plan,29,8),0);
    CHECK(m.acknowledged,15); CHECK(m.status2,16); /* Fault never erased. */
    /* Replay the public old plan's three mask7 actions. All four completions
     * can already be present and bit8 still escapes into the scene loop. */
    memcpy(old,plan,sizeof(old)); old[22].value=old[22].mask=7;
    old[23].value=7; old[24].mask=7;
    memset(&m,0,sizeof(m));
    CHECK(sgx535_fixed_run_actions(&model_io,&m,old,29,8),0);
    CHECK(m.status2,8);
    memset(&session,0,sizeof(session));session.phase=SGX535_PHASE_FIRE_POSSIBLE;session.sequence=1;
    CHECK(sgx535_frozen_session_observe_status(&session,1,0,m.status2,1),-1);
    CHECK(session.phase,SGX535_PHASE_HELD_AFTER_FAILURE);CHECK(session.observed_events,0);
    /* Missing/delayed fourth completion cannot be hidden by INITEND. */
    memset(&m,0,sizeof(m));m.delayed=4;
    CHECK(sgx535_fixed_run_actions(&model_io,&m,old,29,8),0);
    CHECK(m.acknowledged,7);CHECK(m.samples,2);
    puts("8 DHOST load/executor regression cases PASS; SYNTHETIC ONLY");
    return 0;
}
