/* SPDX-License-Identifier: MIT */
/* Authorized bring-up application: requests a triangle through Gallium.
 * No shell/subprocess, software rasterizer or legacy demo invocation. */
#include "remesa_screen.h"
#include "remesa_device.h"
#include "remesa_linux.h"
#include "remesa/native_uapi.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int preserve_bytes(int fd,const void *bytes,size_t size)
{
 ssize_t count=write(fd,bytes,size);int write_error=count<0 ? errno : EIO;
 int sync_error=fsync(fd) ? errno : 0; /* Also persist a partial failed write. */
 if(count!=(ssize_t)size || sync_error) { errno=count!=(ssize_t)size ? write_error : sync_error;return -1; }
 return 0;
}
static void response_fallback(const struct remesa_native_draw *response)
{
 const unsigned char *p=(const unsigned char *)response;
 fputs("UNSEALED_NATIVE_RESPONSE_FALLBACK_HEX=",stderr);
 for(size_t i=0;i<sizeof(*response);i++) fprintf(stderr,"%02x",p[i]);
 fputc('\n',stderr);fflush(stderr);
}
static struct pipe_resource *make_buffer(struct pipe_screen *s,unsigned n,unsigned bind)
{
 struct pipe_resource t={0};t.target=PIPE_BUFFER;t.format=PIPE_FORMAT_R8_UNORM;
 t.width0=n;t.height0=t.depth0=t.array_size=1;t.bind=bind;return s->resource_create(s,&t);
}
static void drop(struct pipe_resource *r) { if(r && !__atomic_sub_fetch(&r->reference.count,1,__ATOMIC_ACQ_REL)) r->screen->resource_destroy(r->screen,r); }
static int upload(struct pipe_context *c,struct pipe_resource *r,const void *data,unsigned n)
{
 struct pipe_box b={.width=(int)n,.height=1,.depth=1};struct pipe_transfer *t=NULL;
 void *p=c->buffer_map(c,r,0,PIPE_MAP_WRITE,&b,&t);if(!p) return EIO;
 memcpy(p,data,n);c->buffer_unmap(c,t);return 0;
}
int main(int argc,char **argv)
{
 struct remesa_device d={0};struct pipe_screen *s=NULL;struct pipe_context *c=NULL;
 struct pipe_resource *v=NULL,*i=NULL,*color=NULL;struct pipe_fence_handle *f=NULL;
 struct pipe_vertex_element element={0};void *state=NULL;int fd=-1,response_fd=-1,status=1,error;char *response_path=NULL;
 struct remesa_linux_receipt receipt={0};
 if(argc!=4 || strcmp(argv[1],"--execute-once")) {
  fprintf(stderr,"Usage: %s --execute-once /dev/dri/cardN NEW-READBACK-FILE\n"
   "This performs one SGX draw. Separate execution-card authorization is required.\n",argv[0]);return 2;
 }
 /* Reserve the output before any possible issuance; never overwrite evidence. */
 fd=open(argv[3],O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC,0600);if(fd<0) { perror("readback destination");return 1; }
 size_t path_length=strlen(argv[3]);
 if(path_length>SIZE_MAX-32) goto done;
 response_path=malloc(path_length+32);if(!response_path) goto done;
 snprintf(response_path,path_length+32,"%s.native-response",argv[3]);
 response_fd=open(response_path,O_CREAT|O_EXCL|O_WRONLY|O_CLOEXEC,0600);
 if(response_fd<0) { perror("native response destination");goto done; }
 error=remesa_linux_open(&d,argv[2]);if(error) { fprintf(stderr,"provider discovery: %s\n",strerror(error));goto done; }
 s=remesa_screen_create(&d);if(!s) goto done;c=s->context_create(s,NULL,0);if(!c) goto done;
 const float vertices[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
 const uint16_t indices[3]={0,1,2};v=make_buffer(s,sizeof(vertices),PIPE_BIND_VERTEX_BUFFER);i=make_buffer(s,sizeof(indices),PIPE_BIND_INDEX_BUFFER);
 if(!v || !i || upload(c,v,vertices,sizeof(vertices)) || upload(c,i,indices,sizeof(indices))) goto done;
 struct pipe_resource rt={0};rt.target=PIPE_TEXTURE_2D;rt.format=PIPE_FORMAT_B8G8R8A8_UNORM;
 rt.width0=rt.height0=32;rt.depth0=rt.array_size=1;rt.bind=PIPE_BIND_RENDER_TARGET;color=s->resource_create(s,&rt);if(!color) goto done;
 struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=color;fb.cbufs[0].format=color->format;c->set_framebuffer_state(c,&fb);
 struct pipe_vertex_buffer vb={0};vb.buffer.resource=v;__atomic_add_fetch(&v->reference.count,1,__ATOMIC_RELAXED);c->set_vertex_buffers(c,1,&vb);
 element.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT;element.src_stride=16;state=c->create_vertex_elements_state(c,1,&element);if(!state) goto done;c->bind_vertex_elements_state(c,state);
 if(remesa_bind_diagnostic_pipeline(c,0xffff00ff)) goto done;
 struct pipe_draw_info info={0};info.mode=MESA_PRIM_TRIANGLES;info.index_size=2;info.index.resource=i;info.instance_count=1;
 struct pipe_draw_start_count_bias draw={.count=3};
 /* The only draw callback. No retry even when the returned status is ambiguous. */
 c->draw_vbo(c,&info,0,NULL,&draw,1);
 error=remesa_context_error(c);remesa_linux_last_receipt(&d,&receipt);
 struct remesa_native_draw original;
 if(!remesa_linux_response(&d,&original)) {
  if(preserve_bytes(response_fd,&original,sizeof(original))) {
   perror("preserve complete native response");response_fallback(&original);goto done;
  }
 } else if(receipt.attempted) { fprintf(stderr,"missing original attempted response\n");goto done; }
 fprintf(stdout,"operation=%llu ioctl_errno=%d operation_errno=%d phase=%u events=0x%x issued=%u cpu_published=%u attempted=%u draw_error=%d\n",
  (unsigned long long)receipt.operation,receipt.ioctl_errno,receipt.operation_errno,receipt.phase,receipt.events,receipt.issued,receipt.cpu_published,receipt.attempted,error);
 fflush(stdout);
 if(error) goto done;
 c->flush(c,&f,0);if(!f || !s->fence_finish(s,c,f,0)) goto done;
 struct pipe_box box={.width=32,.height=32,.depth=1};struct pipe_transfer *t=NULL;
 const unsigned char *p=c->texture_map(c,color,0,PIPE_MAP_READ,&box,&t);if(!p) goto done;
 if(preserve_bytes(fd,p,4096)) { perror("preserve readback");c->texture_unmap(c,t);goto done; }
 /* Interpretation follows successful original-byte preservation. */
 unsigned magenta=0,zero=0,other=0,mismatch=0;
 for(unsigned y=0;y<32;y++) for(unsigned x=0;x<32;x++) {
  const unsigned char *pixel=p+(y*32+x)*4;
  uint32_t value=(uint32_t)pixel[0]|(uint32_t)pixel[1]<<8|(uint32_t)pixel[2]<<16|(uint32_t)pixel[3]<<24;
  if(value==0xffff00ff) ++magenta;else if(!value) ++zero;else ++other;
  uint32_t expected=x>=8 && y>=8 && x+y<31 ? 0xffff00ff : 0;
  if(value!=expected) ++mismatch;
 }
 c->texture_unmap(c,t);
 fprintf(stdout,"magenta=%u zero=%u other=%u mismatched_pixels=%u classification=%s\n",magenta,zero,other,mismatch,
  mismatch ? "UNEXPECTED_OUTPUT" : "EXPECTED_GALLIUM_TRIANGLE_READBACK");
 status=mismatch ? 3 : 0;
 done:
 if(c) { if(state) { c->bind_vertex_elements_state(c,NULL);c->delete_vertex_elements_state(c,state); } c->destroy(c); }
 if(s && f) s->fence_reference(s,&f,NULL);
 drop(v);drop(i);drop(color);if(s) s->destroy(s);remesa_linux_close(&d);
 if(fd>=0) close(fd);
 if(response_fd>=0) close(response_fd);
 free(response_path);return status;
}
