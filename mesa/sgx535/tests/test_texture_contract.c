/* SPDX-License-Identifier: MIT */
#include "../remesa_texture.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error Texture qualification requires enabled assertions
#endif

struct waiter { int calls, error; uint64_t serial; };
static int wait_writer(void *opaque, uint64_t serial)
{
   struct waiter *w=opaque; ++w->calls; w->serial=serial; return w->error;
}

static void transfer_and_format(void)
{
   struct remesa_texture_desc d={4,2,REMESA_TEXTURE_RGBA8};
   struct remesa_texture *t=remesa_texture_create(&d);
   uint8_t in[32], out[32]; struct remesa_texture_pixel p;
   struct remesa_texture_sampler s={true,true,true,true,false};
   for (unsigned i=0;i<sizeof(in);i++) in[i]=(uint8_t)(i+1);
   assert(t && remesa_texture_refs(t)==1);
   assert(!remesa_texture_upload(t,0,0,4,2,in,16,sizeof(in)));
   memset(out,0,sizeof(out)); assert(!remesa_texture_readback(t,0,0,4,2,out,16,sizeof(out)));
   assert(!memcmp(in,out,sizeof(in)));
   assert(!remesa_texture_sample_nearest(t,&s,0,0,&p));
   assert(p.r==1 && p.g==2 && p.b==3 && p.a==4);
   assert(!remesa_texture_sample_nearest(t,&s,1,1,&p));
   assert(p.r==29 && p.g==30 && p.b==31 && p.a==32);
   remesa_texture_release(t);
}

static void bgra_and_bounds(void)
{
   struct remesa_texture_desc d={2,1,REMESA_TEXTURE_BGRA8};
   struct remesa_texture *t=remesa_texture_create(&d);
   const uint8_t bgra[8]={3,2,1,255,7,6,5,255};
   struct remesa_texture_pixel p; struct remesa_texture_sampler s={true,true,true,true,false};
   assert(t); assert(!remesa_texture_upload(t,0,0,2,1,bgra,8,sizeof(bgra)));
   assert(!remesa_texture_sample_nearest(t,&s,0,0,&p));
   assert(p.r==1 && p.g==2 && p.b==3 && p.a==255);
   assert(remesa_texture_upload(t,1,0,2,1,bgra,8,sizeof(bgra))==EINVAL);
   assert(remesa_texture_readback(t,0,0,2,1,(void *)bgra,7,sizeof(bgra))==EINVAL);
   assert(remesa_texture_readback(t,0,0,0,1,(void *)bgra,8,sizeof(bgra))==EINVAL);
   remesa_texture_release(t);
}

static void fences_and_lifetime(void)
{
   struct remesa_texture_desc d={2,2,REMESA_TEXTURE_RGBA8};
   struct remesa_texture *t=remesa_texture_create(&d); struct waiter w={0,EBUSY,0};
   uint8_t bytes[16]={0};
   assert(t); assert(remesa_texture_attach_writer(t,17,wait_writer,&w)==0);
   assert(remesa_texture_has_writer(t));
   assert(remesa_texture_readback(t,0,0,1,1,bytes,4,sizeof(bytes))==EBUSY);
   assert(w.calls==1 && w.serial==17 && remesa_texture_has_writer(t));
   w.error=0; assert(!remesa_texture_upload(t,0,0,1,1,bytes,4,sizeof(bytes)));
   assert(w.calls==2 && !remesa_texture_has_writer(t));
   assert(remesa_texture_attach_writer(t,17,wait_writer,&w)==ESTALE);
   assert(remesa_texture_attach_writer(t,16,wait_writer,&w)==ESTALE);
   assert(remesa_texture_attach_writer(t,18,wait_writer,&w)==0);
   remesa_texture_retain(t); assert(remesa_texture_refs(t)==2);
   assert(!remesa_texture_release(t)); assert(remesa_texture_refs(t)==1);
   /* Pending release retains the final reference until completion. */
   assert(remesa_texture_release(t)==EBUSY); assert(remesa_texture_refs(t)==1);
   w.error=0; assert(!remesa_texture_wait(t));
   remesa_texture_release(t);
}

static void terminal_quarantine(void)
{
   struct remesa_texture *t=remesa_texture_create(&(struct remesa_texture_desc){1,1,REMESA_TEXTURE_RGBA8});
   struct waiter writer={0,EIO,0}, retire={0,EBUSY,0};
   uint8_t dst[4]={9,9,9,9},src[4]={1,2,3,4};
   struct remesa_texture_pixel p={9,9,9,9};
   struct remesa_texture_sampler sampler={true,true,true,true,false};
   assert(t && !remesa_texture_attach_writer(t,12,wait_writer,&writer));
   assert(remesa_texture_wait(t)==EIO && writer.calls==1);
   assert(remesa_texture_is_quarantined(t));
   writer.error=0;
   assert(remesa_texture_wait(t)==EIO && writer.calls==1);
   assert(remesa_texture_readback(t,0,0,1,1,dst,4,4)==EIO);
   assert(remesa_texture_upload(t,0,0,1,1,src,4,4)==EIO);
   assert(remesa_texture_sample_nearest(t,&sampler,0,0,&p)==EIO);
   for(unsigned i=0;i<4;i++)assert(dst[i]==9);
   assert(p.r==9 && p.g==9 && p.b==9 && p.a==9);
   assert(remesa_texture_release(t)==EBUSY);
   assert(remesa_texture_retire_quarantine(t,11,wait_writer,&retire)==ESTALE && !retire.calls);
   assert(remesa_texture_retire_quarantine(t,12,wait_writer,&retire)==EBUSY && retire.calls==1);
   assert(remesa_texture_has_writer(t));
   retire.error=0;
   assert(!remesa_texture_retire_quarantine(t,12,wait_writer,&retire));
   assert(!remesa_texture_has_writer(t) && remesa_texture_is_quarantined(t));
   assert(remesa_texture_attach_writer(t,13,wait_writer,&writer)==EIO);
   assert(remesa_texture_readback(t,0,0,1,1,dst,4,4)==EIO);
   assert(remesa_texture_retire_quarantine(t,12,wait_writer,&retire)==EINVAL);
   assert(!remesa_texture_release(t));
}

static void interruptions_and_cycles(void)
{
   struct remesa_texture *t=remesa_texture_create(&(struct remesa_texture_desc){1,1,REMESA_TEXTURE_RGBA8});
   struct waiter w={0,EINTR,0};
   assert(t);
   for(uint64_t serial=1;serial<=100;serial++) {
      assert(!remesa_texture_attach_writer(t,serial,wait_writer,&w));
      assert(remesa_texture_wait(t)==EINTR && !remesa_texture_is_quarantined(t));
      w.error=EAGAIN; assert(remesa_texture_wait(t)==EAGAIN);
      w.error=0; assert(!remesa_texture_wait(t));
      assert(remesa_texture_attach_writer(t,serial,wait_writer,&w)==ESTALE);
      w.error=EINTR;
   }
   assert(w.calls==300 && !remesa_texture_has_writer(t));
   assert(!remesa_texture_release(t));
}

static void exhaustion_and_bad_response(void)
{
   struct remesa_texture *t=remesa_texture_create(&(struct remesa_texture_desc){1,1,REMESA_TEXTURE_RGBA8});
   struct waiter w={0,-EIO,0},retire={0,0,0}; uint8_t bytes[4]={0};
   assert(t);
   assert(!remesa_texture_test_counters(t,UINT_MAX,1));
   assert(remesa_texture_retain(t)==EOVERFLOW && remesa_texture_refs(t)==UINT_MAX);
   assert(!remesa_texture_test_counters(t,1,UINT64_MAX));
   assert(remesa_texture_upload(t,0,0,1,1,bytes,4,4)==EOVERFLOW);
   assert(remesa_texture_attach_writer(t,1,wait_writer,&w)==EOVERFLOW && !w.calls);
   assert(!remesa_texture_test_counters(t,1,UINT64_MAX-1));
   assert(remesa_texture_attach_writer(t,1,wait_writer,&w)==EOVERFLOW);
   assert(!remesa_texture_test_counters(t,1,UINT64_MAX-2));
   assert(!remesa_texture_attach_writer(t,1,wait_writer,&w));
   w.error=0; assert(!remesa_texture_wait(t));
   assert(remesa_texture_generation(t)==UINT64_MAX);
   assert(!remesa_texture_test_counters(t,1,1));
   assert(!remesa_texture_attach_writer(t,2,wait_writer,&w));
   w.error=-EIO; assert(remesa_texture_wait(t)==EPROTO);
   assert(remesa_texture_is_quarantined(t));
   assert(!remesa_texture_retire_quarantine(t,2,wait_writer,&retire));
   assert(!remesa_texture_release(t));
}

static void validation(void)
{
   struct remesa_texture_desc d={0,2,REMESA_TEXTURE_RGBA8};
   struct remesa_texture_sampler good={true,true,true,true,false};
   struct remesa_texture_sampler bad=good; bad.mipmaps=true;
   uint8_t b[4]={0}; struct remesa_texture *t;
   assert(remesa_texture_validate_desc(NULL)==EINVAL);
   assert(remesa_texture_validate_desc(&d)==EINVAL);
   d=(struct remesa_texture_desc){UINT32_MAX,UINT32_MAX,REMESA_TEXTURE_RGBA8};
   assert(remesa_texture_validate_desc(&d)==EOVERFLOW);
   d.width=2; d.format=(enum remesa_texture_format)99;
   assert(remesa_texture_validate_desc(&d)==EINVAL);
   assert(!remesa_texture_validate_sampler(&good));
   assert(remesa_texture_validate_sampler(&bad)==ENOTSUP);
   t=remesa_texture_create(&(struct remesa_texture_desc){2,2,REMESA_TEXTURE_RGBA8});
   assert(t); assert(remesa_texture_upload(t,0,0,2,1,b,3,sizeof(b))==EINVAL); remesa_texture_release(t);
}

static void transfer_capacity(void)
{
   struct remesa_texture *t=remesa_texture_create(&(struct remesa_texture_desc){2,2,REMESA_TEXTURE_RGBA8});
   uint8_t in[24]={0},out[24]; struct waiter w={0,0,0};
   assert(t); memset(out,0x5a,sizeof(out));
   assert(!remesa_texture_attach_writer(t,1,wait_writer,&w));
   assert(remesa_texture_upload(t,0,0,2,2,in,16,23)==EINVAL);
   assert(remesa_texture_readback(t,0,0,2,2,out,16,23)==EINVAL);
   assert(!w.calls && remesa_texture_has_writer(t));
   for(unsigned i=0;i<sizeof(out);i++)assert(out[i]==0x5a);
   assert(remesa_texture_upload(t,0,0,2,2,in,SIZE_MAX,sizeof(in))==EOVERFLOW);
   assert(!w.calls);
   assert(!remesa_texture_upload(t,0,0,2,2,in,16,sizeof(in)));
   assert(!remesa_texture_readback(t,0,0,2,2,out,16,sizeof(out)));
   for(unsigned i=8;i<16;i++)assert(out[i]==0x5a);
   remesa_texture_release(t);
}

int main(void)
{
   transfer_and_format(); bgra_and_bounds(); fences_and_lifetime(); validation();transfer_capacity();
   terminal_quarantine(); interruptions_and_cycles(); exhaustion_and_bad_response();
   puts("PASS: 8 offline texture contract groups, including 100 writer cycles; no GPU execution");
   return 0;
}
