/* CPU-only source-boundary policy qualification. No ioctl or device access. */
#include <stdio.h>
#include <string.h>
#include "frozen_source_guard.h"

static int a, b, failures, checks;
#define CHECK(x) do { checks++; if (!(x)) { \
    fprintf(stderr,"line %d: %s\n",__LINE__,#x); failures++; } } while (0)
static const struct sgx535_source_facts clean = {1,0,0,0,0,0,1,0};

static void fresh(struct sgx535_source_guard *g)
{
    memset(g,0,sizeof(*g));
    sgx535_source_boot_start(g,&clean);
    sgx535_source_producer(g,1);
    sgx535_source_boot_assert(g,SGX535_SOURCE_RESET_MASK);
    sgx535_source_boot_finish(g,&clean);
    sgx535_source_producer(g,0);
    CHECK(sgx535_source_boot_valid(g));
    CHECK(sgx535_source_probe(g,&clean));
    CHECK(sgx535_source_begin(g,&a)==1);
}
int main(void)
{
    struct sgx535_source_guard g;
    struct sgx535_source_facts f;
    sgx535_u32 reasons;

    /* Model the documented initialization reset lifecycle, not hardware. */
    fresh(&g); CHECK(sgx535_source_boundary(&g,&a,&clean)==1);
    CHECK(sgx535_source_valid(&g,&a));
    CHECK(sgx535_source_close(&g,&a)==1);
    CHECK(g.state==SGX535_SOURCE_CLOSED && !g.reasons);

    fresh(&g); f=clean; f.pending=1;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    CHECK(g.reasons & SGX535_SOURCE_PENDING);
    CHECK(!sgx535_source_valid(&g,&a));

    memset(&g,0,sizeof(g)); CHECK(sgx535_source_begin(&g,&a)); f=clean;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    CHECK(g.reasons & SGX535_SOURCE_DELAYED_UNKNOWN);
    CHECK(g.state==SGX535_SOURCE_BLOCKED); /* Clean raw words are insufficient. */

    fresh(&g); f=clean; f.busy=1;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    CHECK(g.reasons & SGX535_SOURCE_BUSY);

    fresh(&g); sgx535_source_producer(&g,1); sgx535_source_producer(&g,0);
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK(g.reasons & SGX535_SOURCE_INTERFERENCE);

    memset(&g,0,sizeof(g)); sgx535_source_producer(&g,1);
    CHECK(sgx535_source_begin(&g,&a)==1);
    sgx535_source_producer(&g,0);
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK(g.reasons & SGX535_SOURCE_INTERFERENCE); /* Already in-flight work. */

    fresh(&g); CHECK(sgx535_source_boundary(&g,&a,&clean));
    sgx535_source_producer(&g,1); sgx535_source_producer(&g,0);
    CHECK(!sgx535_source_valid(&g,&a));
    CHECK(!sgx535_source_close(&g,&a));

    fresh(&g); f=clean; f.sample_valid=0;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    CHECK(g.reasons & SGX535_SOURCE_SAMPLE_FAILED);

    fresh(&g); f=clean; f.coverage_complete=0;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    CHECK(g.reasons & SGX535_SOURCE_COVERAGE_UNKNOWN);

    fresh(&g); sgx535_source_lost(&g);
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK(g.reasons & SGX535_SOURCE_LOST);

    fresh(&g); CHECK(!sgx535_source_boundary(&g,&b,&clean));
    CHECK(g.reasons & SGX535_SOURCE_WRONG_LIFETIME);

    fresh(&g); CHECK(sgx535_source_boundary(&g,&a,&clean));
    CHECK(!sgx535_source_close(&g,&b));
    CHECK(g.reasons & SGX535_SOURCE_WRONG_LIFETIME);

    fresh(&g); f=clean; f.pending=1;
    CHECK(!sgx535_source_boundary(&g,&a,&f)); reasons=g.reasons;
    CHECK(!sgx535_source_begin(&g,&b));
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK((g.reasons & reasons)==reasons); /* No retry/clean replacement. */
    CHECK(!sgx535_source_valid(&g,&a));

    fresh(&g); CHECK(sgx535_source_boundary(&g,&a,&clean));
    CHECK(sgx535_source_close(&g,&a));
    sgx535_source_producer(&g,1); sgx535_source_producer(&g,0);
    CHECK(g.state==SGX535_SOURCE_CLOSED && !g.reasons); /* Interval ended. */
    CHECK(!sgx535_source_begin(&g,&a)); /* Never rearm. */

    memset(&g,0,sizeof(g)); sgx535_source_producer(&g,0);
    CHECK(g.reasons & SGX535_SOURCE_LOST); /* Unmatched end is loss. */
    memset(&g,0,sizeof(g)); g.producers_active=~0U;
    sgx535_source_producer(&g,1);
    CHECK(g.reasons & SGX535_SOURCE_LOST); /* Overflow cannot appear clean. */
    fresh(&g); CHECK(!sgx535_source_boundary(&g,&a,NULL));
    CHECK(g.reasons & SGX535_SOURCE_SAMPLE_FAILED);

    /* Clean raw observations cannot reconstruct lost lifecycle history. */
    memset(&g,0,sizeof(g)); CHECK(sgx535_source_begin(&g,&a));
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK(g.reasons & SGX535_SOURCE_DELAYED_UNKNOWN);

    /* Pending TA/3D or BIF requests survive as failed evidence even if a
     * modeled reset later makes all raw reads clean. No reset-to-PASS. */
    { unsigned mode;
      for (mode=0; mode<10; mode++) {
        memset(&g,0,sizeof(g)); f=clean;
        if (mode==0) f.pending=1U<<13;
        if (mode==1) f.pending=1U;
        if (mode==2) f.bif_reads=1;
        if (mode==3) f.bif_fault=1;
        if (mode==4) f.busy=1;
        if (mode==5) f.reset=1;
        if (mode==6) f.sample_valid=0;
        if (mode==8) f.autonomous=SGX535_SOURCE_EVENT_TIMER_ENABLE;
        if (mode==9) f.autonomous=SGX535_SOURCE_EVENT_KICK_NOW;
        if (mode==7) sgx535_source_producer(&g,1); /* software-delayed work */
        sgx535_source_boot_start(&g,&f);
        sgx535_source_producer(&g,1);
        sgx535_source_boot_assert(&g,SGX535_SOURCE_RESET_MASK);
        sgx535_source_boot_finish(&g,&clean);
        sgx535_source_producer(&g,0);
        CHECK(!sgx535_source_boot_valid(&g));
        reasons=g.reasons;
        CHECK(sgx535_source_begin(&g,&a));
        CHECK(!sgx535_source_boundary(&g,&a,&clean));
        CHECK((g.reasons & reasons)==reasons);
        CHECK(!sgx535_source_valid(&g,&a));
      }
    }
    /* Wrong/incomplete reset, delayed IRQ, power/reset continuity loss and
     * missing source coverage all remain unknown. Never establish by sleeping. */
    memset(&g,0,sizeof(g)); sgx535_source_boot_start(&g,&clean);
    sgx535_source_producer(&g,1); sgx535_source_boot_assert(&g,0x3f);
    sgx535_source_boot_finish(&g,&clean); sgx535_source_producer(&g,0);
    CHECK(!sgx535_source_boot_valid(&g));
    fresh(&g); sgx535_source_pending(&g,1U<<13);
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK(g.startup_pending==(1U<<13)); /* Raw quiet now cannot erase prior TA. */
    fresh(&g); sgx535_source_pending(&g,1U);
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    CHECK(g.startup_pending==1U); /* IRQ/service-delayed 3D evidence retained. */
    fresh(&g); sgx535_source_lifecycle_lost(&g);
    CHECK(!sgx535_source_boundary(&g,&a,&clean));
    fresh(&g); f=clean; f.bif_reads=1;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    fresh(&g); f=clean; f.bif_fault=1;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    fresh(&g); f=clean; f.autonomous=SGX535_SOURCE_EVENT_TIMER_ENABLE;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    fresh(&g); f=clean; f.autonomous=SGX535_SOURCE_EVENT_KICK_NOW;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    fresh(&g); f=clean; f.reset=SGX535_SOURCE_RESET_MASK;
    CHECK(!sgx535_source_boundary(&g,&a,&f));
    fresh(&g); sgx535_source_boot_start(&g,&clean); /* reseed/rebind */
    CHECK(!sgx535_source_boundary(&g,&a,&clean));

    /* Accepted operation's own events do not become prior events; only the
     * qualified acceptance path may classify them. Closed evidence immutable. */
    fresh(&g); CHECK(sgx535_source_boundary(&g,&a,&clean));
    sgx535_source_pending(&g,7U);
    CHECK(sgx535_source_valid(&g,&a));
    CHECK(sgx535_source_close(&g,&a));
    { struct sgx535_source_guard saved=g;
      sgx535_source_lifecycle_lost(&g); sgx535_source_pending(&g,~0U);
      sgx535_source_producer(&g,1); sgx535_source_producer(&g,0);
      CHECK(!memcmp(&g,&saved,sizeof(g)));
    }

    printf("%d source-guard checks; %d failures; SYNTHETIC ONLY\n",checks,failures);
    return failures ? 1 : 0;
}
