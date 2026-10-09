/* SPDX-License-Identifier: MIT */
#include "remesa_sampler_state.h"
#include "pipe/p_state.h"
#include "compiler/shader_enums.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NDEBUG
#error Tests require assertions
#endif
struct memory { unsigned calls,live,fail; };
static void *alloc(void *p,size_t n)
{
   struct memory *m=p; if (++m->calls==m->fail) return NULL;
   void *v=malloc(n); if(v)m->live++; return v;
}
static void release(void *p,void *v)
{ struct memory *m=p; assert(v&&m->live); --m->live; free(v); }
static struct pipe_sampler_state nearest(void)
{
   struct pipe_sampler_state s={0};
   s.wrap_s=s.wrap_t=s.wrap_r=PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   s.min_mip_filter=PIPE_TEX_MIPFILTER_NONE;
   return s;
}
int main(void)
{
   unsigned cases=0;
   struct memory m={0};
   struct remesa_sampler_bank *b=remesa_sampler_bank_create(alloc,release,&m);
   struct remesa_sampler_bank *other=remesa_sampler_bank_create(NULL,NULL,NULL);
   assert(b&&other&&m.live==1); assert(!remesa_sampler_draw_admit(b)); ++cases;
   struct pipe_sampler_state state=nearest(); void *s=NULL,*foreign=NULL;
   assert(!remesa_sampler_create(b,&state,&s)&&s); ++cases;
   assert(!remesa_sampler_create(other,&state,&foreign)&&foreign);
   state.min_img_filter=PIPE_TEX_FILTER_LINEAR; /* ownership is immutable */
   void *binding[]={s};
   assert(!remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,0,1,binding));
   assert(remesa_sampler_draw_admit(b)==ENOTSUP); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,0,1,NULL));
   assert(!remesa_sampler_draw_admit(b)); ++cases;
   binding[0]=foreign;
   assert(remesa_sampler_bind(b,PIPE_SHADER_VERTEX,7,1,binding)==ESTALE);
   assert(remesa_sampler_draw_admit(b)==ESTALE); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,7,1,NULL));
   assert(remesa_sampler_draw_admit(b)==ESTALE); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_VERTEX,7,1,NULL));
   assert(!remesa_sampler_draw_admit(b));
   binding[0]=s;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,31,1,binding));
   assert(!remesa_sampler_delete(b,s));
   assert(remesa_sampler_draw_admit(b)==ESTALE); ++cases;
   assert(remesa_sampler_delete(b,s)==ESTALE); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,31,1,NULL));
   assert(remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,0,1,binding)==ESTALE); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_FRAGMENT,0,1,NULL));
   binding[0]=(void *)(uintptr_t)1;
   assert(remesa_sampler_bind(b,PIPE_SHADER_COMPUTE,0,1,binding)==ESTALE); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_COMPUTE,0,1,NULL));
   /* Mesa 25.2 rejected views retain their caller references. No GPU view or
    * completion is simulated here. Arbitrary pointer must not be inspected. */
   struct pipe_sampler_view view={0};view.reference.count=19;
   struct pipe_sampler_view *views[]={&view,(void *)(uintptr_t)1};
   assert(remesa_sampler_views(b,PIPE_SHADER_FRAGMENT,126,2,0,views)==ENOTSUP);
   assert(view.reference.count==19);
   assert(remesa_sampler_draw_admit(b)==ENOTSUP); ++cases;
   assert(!remesa_sampler_views(b,PIPE_SHADER_FRAGMENT,126,1,1,NULL));
   assert(!remesa_sampler_draw_admit(b)); ++cases;
   assert(!remesa_sampler_views(b,PIPE_SHADER_VERTEX,128,0,0,NULL)); ++cases;
   /* Unsupported descriptors never manufacture default state. */
   state=nearest(); state.lod_bias=NAN; s=(void *)(uintptr_t)1;
   assert(remesa_sampler_create(b,&state,&s)==ENOTSUP&&!s); ++cases;
   state=nearest(); state.border_color.ui[3]=1;
   assert(remesa_sampler_create(b,&state,&s)==ENOTSUP&&!s); ++cases;
   state=nearest(); state.compare_mode=1;
   assert(remesa_sampler_create(b,&state,&s)==ENOTSUP&&!s); ++cases;
   state=nearest(); state.min_img_filter=PIPE_TEX_FILTER_LINEAR;
   assert(remesa_sampler_create(b,&state,&s)==ENOTSUP&&!s); ++cases;
   state=nearest(); m.fail=m.calls+1;
   assert(remesa_sampler_create(b,&state,&s)==ENOMEM&&!s&&m.live==2); ++cases;
   m.fail=0;
   for(unsigned i=1;i<1024;i++)assert(!remesa_sampler_create(b,&state,&s));
   assert(remesa_sampler_create(b,&state,&s)==ENOSPC&&!s); ++cases;
   assert(remesa_sampler_bind(b,PIPE_SHADER_VERTEX,UINT32_MAX,1,NULL)==EINVAL);
   assert(remesa_sampler_draw_admit(b)==EINVAL); ++cases;
   assert(!remesa_sampler_bind(b,PIPE_SHADER_VERTEX,0,32,NULL));
   assert(remesa_sampler_draw_admit(b)==EINVAL); ++cases;
   remesa_sampler_bank_destroy(b);assert(!m.live);++cases;
   remesa_sampler_bank_destroy(other);
   m=(struct memory){.fail=1};
   assert(!remesa_sampler_bank_create(alloc,release,&m)&&!m.live);++cases;
   assert(!remesa_sampler_bank_create(alloc,NULL,&m));++cases;
   b=remesa_sampler_bank_create(NULL,NULL,NULL);assert(b);
   assert(remesa_sampler_views(b,PIPE_SHADER_FRAGMENT,127,1,1,NULL)==EINVAL);
   assert(remesa_sampler_draw_admit(b)==EINVAL);++cases;
   remesa_sampler_bank_destroy(b);
   b=remesa_sampler_bank_create(NULL,NULL,NULL);assert(b);
   assert(remesa_sampler_bind(b,PIPE_SHADER_TYPES,0,0,NULL)==EINVAL);
   assert(remesa_sampler_draw_admit(b)==EINVAL);++cases;
   remesa_sampler_bank_destroy(b);
   printf("sampler metadata ownership: %u cases PASS; no GPU sampling or completion\n",cases);
   return 0;
}
