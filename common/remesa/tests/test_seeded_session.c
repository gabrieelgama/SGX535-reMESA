/* SPDX-License-Identifier: MIT */
#include "remesa/seeded_session.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
static unsigned checks;
#define CHECK(x) do { assert(x); ++checks; } while(0)
struct fixture { unsigned calls, fail_at; };
/* SIMULATED source verification, not a physical authenticator. */
static int simulated(void *opaque,const struct remesa_seeded_session_record *r)
{
    struct fixture *f=opaque;
    f->calls++;
    return (unsigned)r->event==f->fail_at?EACCES:0;
}
static void initialize(struct remesa_seeded_session_record *r,
    struct remesa_framebuffer_observation_identity id,
    const void *before,const void *after,const void *independent)
{
    memset(r,0,14*sizeof(*r));
    for(unsigned i=0;i<14;i++) {
        unsigned event=i+1;
        r[i].event=(enum remesa_seeded_session_event)event;
        r[i].identity=id;r[i].sequence=100+i;r[i].provider=1;
        r[i].record_id=event;r[i].gpu_va=0x4000;r[i].pfn=7;
        r[i].pins=r[i].mapped_pages=r[i].verified_pages=1;
        r[i].live_resources=10;
        r[i].submissions=event>=REMESA_SEEDED_SUBMITTED;
        r[i].held_jobs=event>=REMESA_SEEDED_SUBMITTED && event<=REMESA_SEEDED_ACKNOWLEDGED;
        if(event>=REMESA_SEEDED_RELEASED) {
            r[i].pins=r[i].mapped_pages=r[i].verified_pages=0;
            r[i].live_resources=0;r[i].gpu_va=r[i].pfn=0;
        }
    }
    const void *payloads[3]={before,after,independent};
    const unsigned indices[3]={1,10,11};
    for(unsigned i=0;i<3;i++) {
        unsigned j=indices[i];r[j].cpu_leases=1;r[j].payload=payloads[i];
        r[j].payload_bytes=4096;r[j].capture_id=500+i;
    }
    r[11].provider=2;
}
int main(int argc,char **argv)
{
    unsigned char before[4096], reference[4096], after[4096], independent[4096];
    struct remesa_framebuffer_observation_identity id={1,2,3,4,5,6,7};
    struct remesa_seeded_session_record r[14], valid[14];
    struct remesa_seeded_session_report report={0}, saved;
    struct fixture f={0};
    FILE *file;
    CHECK(argc==2);file=fopen(argv[1],"rb");CHECK(file);
    CHECK(fread(reference,1,4096,file)==4096);CHECK(fgetc(file)==EOF);CHECK(!fclose(file));
    CHECK(!remesa_seeded_color_initialize(before,4096,32,32,128));
    memcpy(after,before,4096);
    for(unsigned i=0;i<1024;i++)
        if(reference[4*i]) memcpy(after+4*i,reference+4*i,4);
    memcpy(independent,after,4096);
    initialize(r,id,before,after,independent);memcpy(valid,r,sizeof(r));
#define RUN() remesa_seeded_session_validate(&id,r,14,reference,4096,simulated,&f,&report)
    CHECK(!RUN());CHECK(f.calls==14);CHECK(report.verified_records==14);
    CHECK(report.observation.pixels.result==REMESA_FB_PRESERVE_MATCH);
    saved=report;
    for(unsigned i=0;i<14;i++) {
        f.fail_at=0;
        memcpy(r,valid,sizeof(r));r[i].identity.operation_generation++;
        CHECK(RUN()==ESTALE);CHECK(!memcmp(&report,&saved,sizeof(report)));
        memcpy(r,valid,sizeof(r));r[i].sequence=0;CHECK(RUN()==EINVAL);
        memcpy(r,valid,sizeof(r));r[i].event=REMESA_SEEDED_RELEASED;CHECK(RUN()!=0 || i==12);
        memcpy(r,valid,sizeof(r));r[i].quarantined=1;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].submissions=2;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].live_resources++;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].cpu_leases++;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].pins++;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].mapped_pages++;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].verified_pages++;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));r[i].held_jobs++;CHECK(RUN()==EBUSY);
        memcpy(r,valid,sizeof(r));f.fail_at=i+1;CHECK(RUN()==EACCES);
    }
    f.fail_at=0;memcpy(r,valid,sizeof(r));
    r[5].sequence=r[4].sequence;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[5].record_id=r[4].record_id;CHECK(RUN()==EALREADY);
    memcpy(r,valid,sizeof(r));r[11].provider=r[10].provider;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[11].capture_id=r[10].capture_id;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[10].payload=before;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[10].payload_bytes=4095;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[4].payload=before;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[4].record_id=0;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[3].provider=0;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[0].gpu_va=0x4001;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[0].gpu_va=UINT64_MAX;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[0].pfn=0;CHECK(RUN()==EINVAL);
    memcpy(r,valid,sizeof(r));r[6].gpu_va+=4096;CHECK(RUN()==EBUSY);
    memcpy(r,valid,sizeof(r));r[6].pfn++;CHECK(RUN()==EBUSY);
    memcpy(r,valid,sizeof(r));
    CHECK(remesa_seeded_session_validate(&id,r,12,reference,4096,simulated,&f,&report)==EINVAL);
    CHECK(remesa_seeded_session_validate(&id,r,14,reference,4095,simulated,&f,&report)==EINVAL);
    CHECK(remesa_seeded_session_validate(&id,r,14,reference,4096,NULL,&f,&report)==EINVAL);
    CHECK(remesa_seeded_session_validate(&id,r,14,reference,4096,simulated,&f,(void*)r)==EINVAL);
    CHECK(remesa_seeded_session_validate(&id,r,14,reference,4096,simulated,&f,(void*)after)==EINVAL);
    CHECK(!memcmp(&saved,&report,sizeof(report)));
    memcpy(after,reference,4096);memcpy(independent,after,4096);
    CHECK(!RUN());CHECK(report.observation.pixels.result==REMESA_FB_DISCARD_MATCH);
    memcpy(after,before,4096);memcpy(independent,after,4096);
    CHECK(RUN()==ENODATA);CHECK(report.observation.pixels.result==REMESA_FB_UNCHANGED);
    independent[0]^=1;CHECK(RUN()==EILSEQ);CHECK(report.observation.corroboration_byte_mismatches==1);
    memcpy(independent,after,4096);after[0]^=1;independent[0]^=1;
    CHECK(RUN()==EILSEQ);CHECK(report.observation.pixels.result==REMESA_FB_MISMATCH);
    saved=report;reference[0]^=1;CHECK(RUN()==EILSEQ);CHECK(!memcmp(&saved,&report,sizeof(report)));
    printf("%u checks PASS; simulated evidence verifier and CPU byte comparison only\n",checks);
    return 0;
}
