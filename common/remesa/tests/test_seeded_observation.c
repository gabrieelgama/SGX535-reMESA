/* SPDX-License-Identifier: MIT */
#include "remesa/seeded_observation.h"
#include "remesa_render_pass.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
static uint32_t pixel(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static unsigned cases;
#define CHECK(condition) do { assert(condition); ++cases; } while (0)
int main(int argc, char **argv)
{
    unsigned char reference[4096], before[4096], after[4096], independent[4096];
    unsigned char corrupt[4096], reflected[4096];
    struct remesa_framebuffer_observation_identity id = {1,2,3,4,5,6,7}, stale;
    struct remesa_seeded_observation_report report = {0}, saved;
    FILE *file;
    CHECK(argc==2);
    file=fopen(argv[1],"rb"); CHECK(file!=NULL);
    CHECK(fread(reference,1,sizeof(reference),file)==sizeof(reference));
    CHECK(fgetc(file)==EOF); CHECK(!fclose(file));
    CHECK(!remesa_seeded_color_initialize(before,sizeof(before),32,32,128));
#define OBSERVE(expected,observed,b,bn,ref,rn,a,an,c,cn,r) \
    remesa_seeded_triangle_observe(expected,observed,b,bn,ref,rn,a,an,c,cn,r)
#define RUN() OBSERVE(&id,&id,before,4096,reference,4096,after,4096,independent,4096,&report)
    memcpy(after,before,4096); memcpy(independent,after,4096);
    CHECK(!RUN()); CHECK(report.pixels.result==REMESA_FB_UNCHANGED);
    CHECK(!report.pixels.changed_pixels && report.preserve_byte_mismatches==360);
    /* Expected bytes are a comparison fixture, not GPU output or a rasterizer. */
    for(size_t i=0;i<1024;i++) if(pixel(reference+i*4)) memcpy(after+i*4,reference+i*4,4);
    memcpy(independent,after,4096);
    CHECK(!RUN()); CHECK(report.pixels.result==REMESA_FB_PRESERVE_MATCH);
    CHECK(report.pixels.changed_pixels==120 && !report.preserve_byte_mismatches);
    CHECK(report.discard_byte_mismatches==904*4);
    memcpy(after,reference,4096); memcpy(independent,after,4096);
    CHECK(!RUN()); CHECK(report.pixels.result==REMESA_FB_DISCARD_MATCH);
    CHECK(!report.discard_byte_mismatches && report.preserve_byte_mismatches==904*4);
    independent[7]^=1;
    CHECK(RUN()==EILSEQ); CHECK(report.corroboration_byte_mismatches==1);
    memcpy(independent,after,4096); after[7]^=1; independent[7]^=1;
    CHECK(!RUN()); CHECK(report.pixels.result==REMESA_FB_MISMATCH);
    saved=report;
    stale=id; stale.operation_generation++;
    CHECK(OBSERVE(&id,&stale,before,4096,reference,4096,after,4096,independent,4096,&report)==ESTALE);
    CHECK(!memcmp(&saved,&report,sizeof(report)));
    for(unsigned f=0;f<7;f++) {
        stale=id;
        switch(f) {
        case 0: stale.owner++; break;
        case 1: stale.resource++; break;
        case 2: stale.resource_generation++; break;
        case 3: stale.mapping_generation++; break;
        case 4: stale.operation++; break;
        case 5: stale.operation_generation++; break;
        case 6: stale.content_epoch++; break;
        }
        CHECK(OBSERVE(&id,&stale,before,4096,reference,4096,after,4096,independent,4096,&report)==ESTALE);
        CHECK(!memcmp(&saved,&report,sizeof(report)));
    }
    memcpy(corrupt,before,4096); corrupt[2]^=1;
    CHECK(OBSERVE(&id,&id,corrupt,4096,reference,4096,after,4096,independent,4096,&report)==EILSEQ);
    for(unsigned span=0;span<4;span++) {
        size_t sizes[4]={4096,4096,4096,4096}; sizes[span]--;
        CHECK(OBSERVE(&id,&id,before,sizes[0],reference,sizes[1],after,sizes[2],independent,sizes[3],&report)==EINVAL);
        CHECK(!memcmp(&saved,&report,sizeof(report)));
        sizes[span]+=2;
        CHECK(OBSERVE(&id,&id,before,sizes[0],reference,sizes[1],after,sizes[2],independent,sizes[3],&report)==EINVAL);
    }
    memcpy(corrupt,reference,4096); corrupt[0]^=1;
    CHECK(OBSERVE(&id,&id,before,4096,corrupt,4096,after,4096,independent,4096,&report)==EILSEQ);
    /* Same120/904counts, different coordinates: counts-only acceptance fails. */
    for(unsigned y=0;y<32;y++) for(unsigned x=0;x<32;x++)
        memcpy(reflected+(y*32+x)*4,reference+(y*32+31-x)*4,4);
    CHECK(memcmp(reflected,reference,4096)!=0);
    CHECK(OBSERVE(&id,&id,before,4096,reflected,4096,after,4096,independent,4096,&report)==EILSEQ);
    CHECK(!memcmp(&saved,&report,sizeof(report)));
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,after,4096,independent,4096,(void*)after)==EINVAL);
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,after,4096,independent,4096,(void*)&id)==EINVAL);
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,after,4096,after,4096,&report)==EINVAL);
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,before,4096,independent,4096,&report)==EINVAL);
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,after,4096,before,4096,&report)==EINVAL);
    CHECK(!memcmp(&saved,&report,sizeof(report)));
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,reference,4096,independent,4096,&report)==EINVAL);
    CHECK(OBSERVE(&id,&id,before,4096,reference,4096,after,4096,reference,4096,&report)==EINVAL);
    /* Simulated lifecycle proof only: successful image comparison alone does
     * not make a resource CPU-visible or retired. */
    struct remesa_render_pass pass;
    struct remesa_pass_ticket ticket;
    struct remesa_pass_completion proof = {false,false,false};
    CHECK(!remesa_render_pass_init(&pass,99));
    CHECK(!remesa_render_pass_cpu_write(&pass));
    CHECK(remesa_render_pass_begin(&pass,REMESA_PASS_PRESERVE,REMESA_PASS_STORE,false,&ticket)==ENOTSUP);
    CHECK(!remesa_render_pass_begin(&pass,REMESA_PASS_DISCARD,REMESA_PASS_STORE,false,&ticket));
    CHECK(!remesa_render_pass_submitted(&pass,&ticket));
    CHECK(remesa_render_pass_cpu_write(&pass)==EBUSY);
    CHECK(remesa_render_pass_finish(&pass,&ticket,&proof)==EAGAIN);
    proof.authenticated=true;
    CHECK(remesa_render_pass_finish(&pass,&ticket,&proof)==EAGAIN);
    proof.cpu_visible=true;
    CHECK(remesa_render_pass_finish(&pass,&ticket,&proof)==EAGAIN);
    proof.retired=true;
    CHECK(!remesa_render_pass_finish(&pass,&ticket,&proof));
    CHECK(remesa_render_pass_finish(&pass,&ticket,&proof)==EALREADY);
    CHECK(remesa_render_pass_admit(&pass,REMESA_PASS_PRESERVE,false)==ENOTSUP);
    printf("%u checks PASS; sealed pixels, CPU comparison and simulated lifecycle only\n",cases);
    return 0;
}
