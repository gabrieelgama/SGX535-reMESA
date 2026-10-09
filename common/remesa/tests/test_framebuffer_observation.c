/* SPDX-License-Identifier: MIT */
#include "remesa/framebuffer_observation.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
static void put(unsigned char *p, uint32_t c)
{ for (unsigned i = 0; i < 4; i++) p[i] = (unsigned char)(c >> (i * 8)); }
static void test(unsigned w, unsigned h)
{
    size_t pitch = ((size_t)w * 4 + 63) & ~(size_t)63;
    /* Include padding even for otherwise aligned resolutions. */
    pitch += 64;
    struct remesa_framebuffer_observation_layout l = {
        w,h,(uint32_t)pitch,4096,4096,pitch*h,128u*1024u*1024u};
    struct remesa_framebuffer_observation_identity identity = {1,2,3,4,5,6,7}, bad;
    struct remesa_framebuffer_observation_report r, saved;
    unsigned char *before = malloc(l.bytes), *after = malloc(l.bytes);
    unsigned char *draw = malloc(l.bytes), *mask = calloc((size_t)w*h, 1);
    assert(before && after && draw && mask);
    memset(before,0xa5,l.bytes); memset(draw,0xcc,l.bytes);
    for (size_t y=0;y<h;y++) for(size_t x=0;x<w;x++) {
        put(before+y*pitch+x*4,0xff102030);
        put(draw+y*pitch+x*4,0xffff00ff);
        /* Declared finite update region, not a software rasterizer. */
        mask[y*w+x]=(x==w/2 && y==h/2);
    }
    memcpy(after,before,l.bytes);
    assert(!remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r));
    assert(r.result==REMESA_FB_UNCHANGED && !r.changed_pixels);
    put(after+(h/2)*pitch+(w/2)*4,0xffff00ff);
    assert(!remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r));
    assert(r.result==REMESA_FB_PRESERVE_MATCH && r.changed_pixels==1 && !r.padding_byte_mismatches);
    for(size_t y=0;y<h;y++) for(size_t x=0;x<w;x++) if(!mask[y*w+x])put(after+y*pitch+x*4,0);
    assert(!remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r));
    assert(r.result==REMESA_FB_DISCARD_MATCH);
    after[pitch-1]^=1;
    assert(!remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r));
    assert(r.result==REMESA_FB_MISMATCH && r.padding_byte_mismatches==1);
    saved=r; bad=identity; bad.operation_generation++;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&bad,before,after,draw,mask,(size_t)w*h,0,&r)==ESTALE);
    assert(!memcmp(&r,&saved,sizeof(r)));
    for (unsigned field=0; field<7; field++) {
        bad=identity;
        switch(field) {
        case 0: bad.owner++; break;
        case 1: bad.resource++; break;
        case 2: bad.resource_generation++; break;
        case 3: bad.mapping_generation++; break;
        case 4: bad.operation++; break;
        case 5: bad.operation_generation++; break;
        case 6: bad.content_epoch++; break;
        }
        assert(remesa_framebuffer_observation_compare(&l,&identity,&bad,before,after,draw,mask,(size_t)w*h,0,&r)==ESTALE);
        assert(!memcmp(&r,&saved,sizeof(r)));
    }
    bad=identity; bad.owner=0;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&bad,before,after,draw,mask,(size_t)w*h,0,&r)==EINVAL);
    mask[0]=2;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r)==EINVAL);
    assert(!memcmp(&r,&saved,sizeof(r))); mask[0]=0;
    l.bytes--;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r)==EINVAL);
    l.bytes++; l.max_bytes=l.bytes-1;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r)==EINVAL);
    l.max_bytes=l.bytes;
    l.pitch--;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r)==EOVERFLOW);
    l.pitch++;
    l.max_width=w-1;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,&r)==EINVAL);
    l.max_width=4096;
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h-1,0,&r)==EINVAL);
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0,(void*)after)==EINVAL);
    assert(remesa_framebuffer_observation_compare(&l,&identity,&identity,before,after,draw,mask,(size_t)w*h,0xff102030,&r)==EINVAL);
    free(mask); free(draw); free(after); free(before);
}
int main(void)
{
    const unsigned dimensions[][2]={{32,32},{320,240},{640,480},{800,600},
        {1024,768},{1280,720},{1280,800},{1366,768},{1600,900},{1920,1080},{17,13}};
    for(unsigned i=0;i<sizeof(dimensions)/sizeof(dimensions[0]);i++)test(dimensions[i][0],dimensions[i][1]);
    puts("11 dimension observation contracts PASS; CPU comparison only");
    return 0;
}
