/* SPDX-License-Identifier: MIT */
#include "remesa_retained_gallium.h"
#include "remesa_screen.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <errno.h>
#include <string.h>
struct borrowed {
 struct remesa_linux_memory *memory;uint32_t id;uint64_t va;
 const struct remesa_evidence_expected *expected;
 const struct remesa_evidence_response *response;
 const struct remesa_evidence_observer *observer;
 const struct remesa_evidence_ownership *ownership;
 bool allocated,mapped;int error;
};
static int verify(struct borrowed *b)
{
 if(remesa_linux_memory_context(b->memory)!=b->expected->id.context)return ESTALE;
 if(remesa_retained_completion_verify(b->expected,b->response,b->observer,b->ownership)!=REMESA_EVIDENCE_OK)return EPROTO;
 return remesa_linux_memory_validate_backing(b->memory,b->id,b->va,4096);
}
static int allocate(void *p,size_t n,void **bo)
{struct borrowed *b=p;if(n!=4096 || b->allocated)return EINVAL;int e=verify(b);if(e)return e;b->allocated=true;*bo=b;return 0;}
static void destroy(void *p,void *bo){struct borrowed *b=p;if(bo==b)b->allocated=false;}
static int map(void *p,void *bo,bool write,void **out)
{struct borrowed *b=p;if(bo!=b || write || b->mapped)return EINVAL;int e=verify(b);if(e)return e;e=remesa_linux_memory_ops()->cpu_map(b->memory,b->id,false,out);if(!e)b->mapped=true;return e;}
static void unmap(void *p,void *bo,bool write)
{struct borrowed *b=p;if(bo!=b || write || !b->mapped){b->error=EPROTO;return;}b->error=remesa_linux_memory_ops()->cpu_unmap(b->memory,b->id,false);b->mapped=false;}
static int submit(void *p,void *bo,const struct remesa_draw_packet *packet,void **f)
{(void)p;(void)bo;(void)packet;(void)f;return ENOTSUP;}
static int wait(void *p,void *f,uint64_t timeout,bool visible)
{struct borrowed *b=p;(void)timeout;if(f!=b || !visible)return EINVAL;return verify(b);}
static void destroy_fence(void *p,void *f){(void)p;(void)f;}
static const struct remesa_device_ops ops={allocate,destroy,map,unmap,submit,wait,destroy_fence};
int remesa_retained_gallium_read(struct remesa_linux_memory *m,const struct remesa_retained_request *request,uint32_t id,uint64_t va,
 const struct remesa_evidence_expected *ex,const struct remesa_evidence_response *rs,
 const struct remesa_evidence_observer *ob,const struct remesa_evidence_ownership *own,
 void *out,size_t bytes)
{
 if(!m || !request || !id || !va || !ex || !rs || !ob || !own || !out || bytes!=4096)return EINVAL;
 if(request->context!=ex->id.context || request->operation!=ex->id.operation ||
    request->serial!=ex->id.serial || request->slot!=ex->id.slot || request->generation!=ex->id.generation ||
    request->bindings[5].id!=id || request->bindings[5].va!=va || request->bindings[5].bytes!=4096)return ESTALE;
 struct borrowed b={.memory=m,.id=id,.va=va,.expected=ex,.response=rs,.observer=ob,.ownership=own};
 struct remesa_device device={.ops=&ops,.priv=&b,.resource_only_backend=true};
 struct pipe_screen *screen=NULL;struct pipe_context *ctx=NULL;struct pipe_resource *resource=NULL;struct pipe_fence_handle *fence=NULL;
 int e=verify(&b);if(e)return e;
 screen=remesa_screen_create(&device);if(!screen)return ENOMEM;
 ctx=screen->context_create(screen,NULL,0);if(!ctx){e=ENOMEM;goto done;}
 struct pipe_resource desc={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,.width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 resource=screen->resource_create(screen,&desc);if(!resource){e=EOWNERDEAD;goto done;}
 e=remesa_resource_attach_writer(ctx,resource,&b,&fence);if(e)goto done;
 e=remesa_retained_readback_verified(ctx,resource,fence,ex,rs,ob,own,0,out,bytes);
 if(!e)e=b.error;
 done:
 if(fence)screen->fence_reference(screen,&fence,NULL);
 if(resource)screen->resource_destroy(screen,resource);
 if(ctx)ctx->destroy(ctx);
 screen->destroy(screen);return e;
}
