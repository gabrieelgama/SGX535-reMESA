/* SPDX-License-Identifier: MIT */
/* Real Gallium resource/draw/map callbacks with explicit mocked completion.
 * Pixels are copied from the sealed reference as fixture input, NOT rendered. */
#define main existing_driver_regression
#include "test_driver.c"
#undef main
#include "remesa_retained_readback.h"
static void *(*saved_map)(struct pipe_context *,struct pipe_resource *,unsigned,unsigned,const struct pipe_box *,struct pipe_transfer **);
static unsigned layout;
static unsigned char padded[8192];
static void *layout_map(struct pipe_context *c,struct pipe_resource *r,unsigned level,unsigned flags,
 const struct pipe_box *box,struct pipe_transfer **t)
{
 void *p=saved_map(c,r,level,flags,box,t);if(!p)return NULL;
 if(layout==1)(*t)->stride=127;
 if(layout==2)(*t)->box.x=1;
 if(layout==3){(*t)->stride=256;return padded;}
 return p;
}
static void verify_snapshot(struct pipe_context *c,struct pipe_resource *target,
 struct pipe_fence_handle *f,unsigned char *out)
{
 struct remesa_evidence_expected e;struct remesa_evidence_admission a;
 struct remesa_evidence_response r;struct remesa_evidence_observer o;
 struct remesa_evidence_ownership g;

 memset(&e,0,sizeof e); memset(&a,0,sizeof a); memset(&r,0,sizeof r);
 memset(&o,0,sizeof o); memset(&g,0,sizeof g);
 e.id=(struct remesa_evidence_id){11,22,3,4,5};
 memset(e.boot,1,16); memset(e.source,2,32); memset(e.request_sha256,3,32);
 e.request_bytes=264; e.resources=10;
 memcpy(a.boot,e.boot,16); memcpy(a.source,e.source,32);
 a.records=a.source_valid=a.capsule_unused=a.native_unused=a.owner_valid=1;
 a.mappings_valid=a.pfn_valid=a.commands_retained=1;
 r.id=g.id=e.id; r.request_bytes=o.request_bytes=e.request_bytes;
 r.response_bytes=o.response_bytes=4096;
 memcpy(r.request_sha256,e.request_sha256,32); memcpy(o.request_sha256,e.request_sha256,32);
 memset(r.response_sha256,4,32); memcpy(o.response_sha256,r.response_sha256,32);
 memcpy(o.boot,e.boot,16); memcpy(o.source,e.source,32);
 r.records=r.attempted=r.ta_complete=r.end_render=r.memory_free=r.retired=1;
 r.visible=r.acknowledged=1;
 o.native_records=o.capsule_records=o.source_records=1;
 o.request_present=o.response_present=o.closed=o.syscall_exit_seen=o.source_valid=1;
 o.ta_complete=o.end_render=o.memory_free=o.retired=1;
 g.records=g.pfn_valid=g.mappings_valid=g.held_through_completion=1;
 g.visibility_ordered=g.detached=g.released=1; g.resources=e.resources;

 (void)a;
 assert(!remesa_retained_readback_verified(c,target,f,&e,&r,&o,&g,0,out,4096));
 o.response_sha256[0]^=1;
 assert(remesa_retained_readback_verified(c,target,f,&e,&r,&o,&g,0,out,4096)==EPROTO);
 o.response_sha256[0]^=1;g.id.generation++;
 assert(remesa_retained_readback_verified(c,target,f,&e,&r,&o,&g,0,out,4096)==EPROTO);
}
int main(int argc,char **argv)
{
 assert(argc==2);unsigned char reference[4096],actual[4096];unsigned mismatch;
 FILE *input=fopen(argv[1],"rb");assert(input && fread(reference,1,4096,input)==4096 && fgetc(input)==EOF);fclose(input);
 struct recorder rec={0};struct remesa_device device={.ops=&ops,.priv=&rec,.offline_test_backend=true};
 struct pipe_screen *s=remesa_screen_create(&device);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0);assert(c);
 struct pipe_resource *v=buffer(s,48,PIPE_BIND_VERTEX_BUFFER);
 struct pipe_resource rt={0};rt.target=PIPE_TEXTURE_2D;rt.format=PIPE_FORMAT_B8G8R8A8_UNORM;
 rt.width0=rt.height0=32;rt.depth0=rt.array_size=1;rt.bind=PIPE_BIND_RENDER_TARGET;
 struct pipe_resource *r=s->resource_create(s,&rt);assert(v&&r);
 float positions[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
 assert(remesa_retained_upload(c,v,1,positions,48)==EINVAL);
 assert(remesa_retained_upload(c,v,0,positions,0)==EINVAL);
 assert(!remesa_retained_upload(c,v,0,positions,48));
 struct pipe_vertex_buffer vb={0};vb.buffer.resource=v;++v->reference.count;c->set_vertex_buffers(c,1,&vb);
 struct pipe_vertex_element e={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};
 void *ve=c->create_vertex_elements_state(c,1,&e);assert(ve);c->bind_vertex_elements_state(c,ve);
 struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=r;fb.cbufs[0].format=r->format;c->set_framebuffer_state(c,&fb);
 assert(!remesa_bind_diagnostic_pipeline(c,0xffff00ff));
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};struct pipe_draw_start_count_bias draw={.count=3};
 c->draw_vbo(c,&info,0,NULL,&draw,1);assert(!remesa_context_error(c)&&rec.draws==1);
 struct pipe_fence_handle *f=NULL;c->flush(c,&f,0);assert(f);
 struct remesa_gpu_receipt receipt={11,22,true,true,false};
 memset(actual,0x55,4096);
 assert(remesa_retained_readback_verified(c,r,f,NULL,NULL,NULL,NULL,0,actual,4096)==EPROTO);
 assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EAGAIN);
 assert(actual[0]==0x55);receipt.operation++;
 assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==ESTALE);receipt.operation--;
 receipt.retired=false;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EAGAIN);receipt.retired=true;
 receipt.published=false;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EAGAIN);receipt.published=true;
 receipt.failed=true;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EIO);receipt.failed=false;
 assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4095)==EINVAL);
 rec.last->retired=true;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EAGAIN);
 /* Fixture writes predetermined bytes after explicit mock publication. */
 rec.last->target->published=true;memcpy(rec.last->target->bytes,reference,4096);
 assert(!remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096));
 assert(!remesa_retained_compare(actual,4096,reference,4096,&mismatch)&&!mismatch);
 verify_snapshot(c,r,f,actual);
 saved_map=c->texture_map;c->texture_map=layout_map;
 layout=1;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EPROTO);
 layout=2;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==EPROTO);
 memset(padded,0xa5,sizeof(padded));for(unsigned y=0;y<32;y++)memcpy(padded+y*256,reference+y*128,128);
 layout=3;assert(!remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096));
 assert(!remesa_retained_compare(actual,4096,reference,4096,&mismatch));c->texture_map=saved_map;
 actual[4095]^=1;assert(remesa_retained_compare(actual,4096,reference,4096,&mismatch)==EILSEQ && mismatch==1);
 actual[0]^=1;assert(remesa_retained_compare(actual,4096,reference,4096,&mismatch)==EILSEQ && mismatch==2);
 assert(remesa_retained_compare(actual,0,reference,4096,&mismatch)==EINVAL);
 r->width0=31;assert(remesa_retained_readback(c,r,f,11,22,&receipt,0,actual,4096)==ENOTSUP);r->width0=32;
 c->bind_vertex_elements_state(c,NULL);c->delete_vertex_elements_state(c,ve);c->destroy(c);
 s->fence_reference(s,&f,NULL);resource_drop(v);resource_drop(r);s->destroy(s);
 assert(rec.allocs==rec.frees);
 puts("PASS actual Gallium callbacks, sealed-reference fixture, mocked fences; no SGX");return 0;
}
