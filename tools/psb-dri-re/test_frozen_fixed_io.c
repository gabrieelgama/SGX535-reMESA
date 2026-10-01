#include "frozen_fixed_io.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); return 1; \
} } while (0)

struct fake {
    unsigned writes;
    unsigned reads;
    unsigned barriers;
    unsigned read_ready_at;
    unsigned fail_write_at;
    unsigned fail_read_at;
    sgx535_u32 last_offset;
    sgx535_u32 last_value;
};

static int write_reg(void *ctx, sgx535_u32 off, sgx535_u32 val)
{
    struct fake *f = ctx;
    f->writes++;
    f->last_offset = off;
    f->last_value = val;
    return f->writes == f->fail_write_at ? -1 : 0;
}

static int read_reg(void *ctx, sgx535_u32 off, sgx535_u32 *val)
{
    struct fake *f = ctx;
    f->reads++;
    f->last_offset = off;
    if (f->reads == f->fail_read_at)
        return -1;
    *val = f->reads >= f->read_ready_at ? 0x44U : 0;
    return 0;
}

static int barrier(void *ctx)
{
    struct fake *f = ctx;
    f->barriers++;
    return 0;
}

int main(void)
{
    const struct sgx535_fixed_io io = {write_reg, read_reg, barrier};
    const struct sgx535_frozen_reg_action actions[] = {
        {SGX535_REG_WRITE, 0xad4, 1, 0},
        {SGX535_REG_POLL_SET, 0x138, 0x44, 0x44},
        {SGX535_REG_WRITE, 0x140, 0x44, 0},
        {SGX535_REG_WMB, 0, 0, 0}
    };
    struct fake f = {0};

    f.read_ready_at = 2;
    CHECK(sgx535_fixed_run_actions(&io, &f, actions, 4, 3) == 0);
    CHECK(f.writes == 2 && f.reads == 2 && f.barriers == 1);
    CHECK(f.last_offset == 0x140 && f.last_value == 0x44);
    memset(&f, 0, sizeof(f));
    f.read_ready_at = 4;
    CHECK(sgx535_fixed_run_actions(&io, &f, actions, 4, 3) != 0);
    CHECK(f.writes == 1 && f.reads == 3 && f.barriers == 0);
    memset(&f, 0, sizeof(f));
    f.fail_write_at = 1;
    CHECK(sgx535_fixed_run_actions(&io, &f, actions, 4, 3) != 0);
    CHECK(f.reads == 0 && f.barriers == 0);
    memset(&f, 0, sizeof(f));
    f.fail_read_at = 1;
    CHECK(sgx535_fixed_run_actions(&io, &f, actions, 4, 3) != 0);
    CHECK(f.writes == 1 && f.reads == 1 && f.barriers == 0);
    CHECK(sgx535_fixed_run_actions(&io, &f, actions, 4, 0) != 0);
    CHECK(sgx535_fixed_run_actions(&io, &f, actions, 32, 3) != 0);
    {
        struct sgx535_frozen_reg_action invalid =
            {SGX535_REG_WRITE, 0x1000, 1, 0};
        CHECK(sgx535_fixed_run_actions(&io, &f, &invalid, 1, 3) != 0);
        invalid = (struct sgx535_frozen_reg_action){
            SGX535_REG_POLL_SET, 0x138, 0x80, 0x44};
        CHECK(sgx535_fixed_run_actions(&io, &f, &invalid, 1, 3) != 0);
        invalid = (struct sgx535_frozen_reg_action){
            SGX535_REG_WMB, 4, 0, 0};
        CHECK(sgx535_fixed_run_actions(&io, &f, &invalid, 1, 3) != 0);
    }
    return 0;
}
