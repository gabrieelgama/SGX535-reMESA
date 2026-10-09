/* SPDX-License-Identifier: MIT */
/* Native Gallium -> semantic packet -> provider lifecycle integration.
 * Synthetic receipts only. No rasterizer, device, ioctl or SGX output. */
#include "remesa_screen.h"
#include "remesa_device.h"
#include "remesa/provider.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct backing {
    struct remesa_token token;
    void *bytes;
    unsigned references, mapped;
};
struct backend_fence {
    struct remesa_fence token;
    struct backing *target;
};
struct fixture {
    struct remesa_slot slots[8];
    struct remesa_registry registry;
    struct remesa_provider provider;
    struct remesa_draw_packet packet;
    struct backend_fence *submitted;
    uint64_t next_va;
    unsigned dispatches, destroyed;
};
static enum remesa_dispatch dispatch(void *data, const struct remesa_job *job)
{
    struct fixture *f=data;
    assert(job->count==1 && job->uses[0].access==REMESA_ACCESS_WRITE);
    ++f->dispatches;
    return REMESA_ISSUED; /* Test event only, never a hardware receipt. */
}
static int release(void *data, const struct remesa_job *job)
{
    (void)data;
    return job->state==REMESA_FENCE_PUBLISHED ? REMESA_OK : REMESA_BUSY;
}
static int allocate(void *data, size_t n, void **out)
{
    struct fixture *f=data;
    struct backing *b=calloc(1,sizeof(*b));
    if(!b) return ENOMEM;
    b->bytes=calloc(1,n);b->references=1;
    if(!b->bytes || remesa_registry_allocate(&f->registry,n,&b->token)!=REMESA_OK) {
        free(b->bytes);free(b);return ENOMEM;
    }
    assert(remesa_registry_map(&f->registry,b->token,f->next_va,4096)==REMESA_OK);
    f->next_va+=(n+4095)&~(uint64_t)4095;
    *out=b;return 0;
}
static void destroy(void *data, void *object)
{
    struct fixture *f=data;struct backing *b=object;
    assert(b->references && !b->mapped);
    if(--b->references) return;
    assert(remesa_registry_remove(&f->registry,b->token)==REMESA_OK);
    ++f->destroyed;free(b->bytes);free(b);
}
static int map(void *data, void *object, bool write, void **out)
{
    struct fixture *f=data;struct backing *b=object;struct remesa_resource r;
    if(remesa_registry_snapshot(&f->registry,b->token,&r)!=REMESA_OK) return EINVAL;
    if(b->mapped || r.state==REMESA_IN_FLIGHT || r.state==REMESA_QUARANTINED) return EBUSY;
    if(write && r.state!=REMESA_MAPPED) return EBUSY;
    if(!write && r.state==REMESA_RETIRED && remesa_resource_readable(&r,0,r.bytes)!=REMESA_OK) return EAGAIN;
    b->mapped=1;*out=b->bytes;return 0;
}
static void unmap(void *data, void *object, bool write)
{ (void)data;(void)write;struct backing *b=object;assert(b->mapped);b->mapped=0; }
static int submit(void *data, void *object, const struct remesa_draw_packet *p, void **out)
{
    struct fixture *f=data;struct backing *b=object;struct remesa_resource r;
    struct backend_fence *fence=calloc(1,sizeof(*fence));
    if(!fence) return ENOMEM;
    assert(remesa_draw_packet_validate(p)==REMESA_PACKET_OK);
    assert(remesa_registry_snapshot(&f->registry,b->token,&r)==REMESA_OK);
    struct remesa_use use={b->token,0,r.bytes,REMESA_ACCESS_WRITE};
    int status=remesa_provider_submit(&f->provider,&use,1,&fence->token);
    if(status!=REMESA_OK) { free(fence);return EBUSY; }
    f->packet=*p;++b->references;fence->target=b;f->submitted=fence;*out=fence;return 0;
}
static int wait(void *data, void *object, uint64_t timeout, bool visible)
{
    struct fixture *f=data;struct backend_fence *fence=object;struct remesa_job job;
    (void)timeout;
    if(remesa_provider_query(&f->provider,fence->token,&job)!=REMESA_OK) return EINVAL;
    if(job.state==REMESA_FENCE_FAILED) return EIO;
    return job.state==REMESA_FENCE_PUBLISHED || (!visible && job.state==REMESA_FENCE_RETIRED) ? 0 : EAGAIN;
}
static void destroy_fence(void *data, void *object)
{
    struct fixture *f=data;struct backend_fence *fence=object;
    assert(remesa_provider_release(&f->provider,fence->token)==REMESA_OK);
    destroy(f,fence->target);free(fence);f->submitted=NULL;
}
static struct pipe_resource *buffer(struct pipe_screen *screen, unsigned bytes, unsigned bind)
{
    struct pipe_resource t={0};t.target=PIPE_BUFFER;t.format=PIPE_FORMAT_R8_UNORM;
    t.width0=bytes;t.height0=t.depth0=t.array_size=1;t.bind=bind;
    return screen->resource_create(screen,&t);
}
int main(void)
{
    struct fixture f={0};f.next_va=4096;
    assert(remesa_registry_init(&f.registry,f.slots,8)==REMESA_OK);
    struct remesa_backend backend={dispatch,release,&f};
    assert(remesa_provider_init(&f.provider,&f.registry,101,&backend)==REMESA_OK);
    struct remesa_device_ops ops={allocate,destroy,map,unmap,submit,wait,destroy_fence};
    struct remesa_device device={.ops=&ops,.priv=&f,.offline_test_backend=true,.framebuffer_mode=REMESA_FRAMEBUFFER_REPLACE};
    struct pipe_screen *screen=remesa_screen_create(&device);assert(screen);
    struct pipe_context *ctx=screen->context_create(screen,NULL,0);assert(ctx);
    struct pipe_resource *vb=buffer(screen,48,PIPE_BIND_VERTEX_BUFFER);
    struct pipe_resource *ib=buffer(screen,6,PIPE_BIND_INDEX_BUFFER);assert(vb && ib);
    struct pipe_box box={.x=0,.width=48,.y=0,.height=1,.z=0,.depth=1};struct pipe_transfer *transfer=NULL;
    float vertices[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
    void *ptr=ctx->buffer_map(ctx,vb,0,PIPE_MAP_WRITE,&box,&transfer);assert(ptr);
    memcpy(ptr,vertices,48);ctx->buffer_unmap(ctx,transfer);
    box.width=6;ptr=ctx->buffer_map(ctx,ib,0,PIPE_MAP_WRITE,&box,&transfer);assert(ptr);
    uint16_t indices[3]={0,1,2};memcpy(ptr,indices,6);ctx->buffer_unmap(ctx,transfer);
    struct pipe_resource t={0};t.target=PIPE_TEXTURE_2D;t.format=PIPE_FORMAT_B8G8R8A8_UNORM;
    t.width0=t.height0=32;t.depth0=t.array_size=1;t.bind=PIPE_BIND_RENDER_TARGET;
    struct pipe_resource *color=screen->resource_create(screen,&t);assert(color);
    struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;
    fb.cbufs[0].texture=color;fb.cbufs[0].format=color->format;
    ctx->set_framebuffer_state(ctx,&fb);
    struct pipe_vertex_buffer binding={0};binding.buffer.resource=vb;
    ctx->set_vertex_buffers(ctx,1,&binding); /* Transfers the caller reference. */
    struct pipe_vertex_element element={0};element.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT;element.src_stride=16;
    void *state=ctx->create_vertex_elements_state(ctx,1,&element);assert(state);
    ctx->bind_vertex_elements_state(ctx,state);
    assert(!remesa_bind_diagnostic_pipeline(ctx,0xffff00ff));
    struct pipe_draw_info info={0};info.mode=MESA_PRIM_TRIANGLES;info.index_size=2;info.index.resource=ib;info.instance_count=1;
    struct pipe_draw_start_count_bias draw={0,3,0};
    ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);
    assert(!remesa_context_error(ctx) && f.dispatches==1);
    assert(f.packet.vertex_count==3 && f.packet.index_count==3 && f.packet.argb==0xffff00ff);
    assert(!memcmp(f.packet.positions,vertices,sizeof(vertices)));
    struct pipe_fence_handle *fence=NULL;ctx->flush(ctx,&fence,0);assert(fence);
    assert(!screen->fence_finish(screen,ctx,fence,0));
    struct remesa_job job;
    assert(remesa_provider_query(&f.provider,f.submitted->token,&job)==REMESA_OK);
    assert(remesa_provider_complete(&f.provider,f.submitted->token,999,job.operation,1)==REMESA_INVALID);
    assert(remesa_provider_complete(&f.provider,f.submitted->token,101,job.operation,1)==REMESA_OK);
    assert(!screen->fence_finish(screen,ctx,fence,0)); /* Retirement is NOT publication. */
    box=(struct pipe_box){.x=0,.width=32,.y=0,.height=32,.z=0,.depth=1};
    assert(!ctx->texture_map(ctx,color,0,PIPE_MAP_READ,&box,&transfer));
    assert(remesa_provider_publish(&f.provider,f.submitted->token,101,job.operation)==REMESA_OK);
    assert(screen->fence_finish(screen,ctx,fence,0));
    ptr=ctx->texture_map(ctx,color,0,PIPE_MAP_READ,&box,&transfer);assert(ptr);
    /* No renderer in this test: every byte remains allocation-zero. */
    for(unsigned i=0;i<4096;++i) assert(!((unsigned char *)ptr)[i]);
    ctx->texture_unmap(ctx,transfer);
    ctx->delete_vertex_elements_state(ctx,state);ctx->destroy(ctx);
    screen->resource_destroy(screen,ib);screen->resource_destroy(screen,color);
    assert(f.destroyed==2); /* Fence still owns the color backing. */
    screen->fence_reference(screen,&fence,NULL);assert(f.destroyed==3);
    screen->destroy(screen);
    puts("PASS: native Gallium -> clean provider; synthetic completion/publication only; NO SGX DRAW");
    return 0;
}
