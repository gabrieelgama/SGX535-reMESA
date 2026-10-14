/* SPDX-License-Identifier: MIT. Original CPU-only native transport regression.
 * Interposed resource service. No device, MMIO, SGX VA or dispatch. */
int inherited_transport_main(void);
#define main inherited_transport_main
#define __wrap_ioctl inherited_ioctl
#include "test_shader_memory_transport.c"
#undef __wrap_ioctl
#undef main
static unsigned fail_action;static bool inject,alias_create,initial_corrupt;
static int allocation_failure=-1;
void *__real_malloc(size_t);void *__real_calloc(size_t,size_t);void __real_free(void *);
void *__wrap_malloc(size_t);void *__wrap_calloc(size_t,size_t);void __wrap_free(void *);
static void *tracked[8];static unsigned tracked_count;
static bool allocation_denied(void)
{if(allocation_failure<0)return false;if(!allocation_failure)return true;--allocation_failure;return false;}
static void track(void *p,bool active){if(p&&active){CHECK(tracked_count<8);tracked[tracked_count++]=p;}}
void *__wrap_malloc(size_t n){bool active=allocation_failure>=0;if(allocation_denied())return NULL;void *p=__real_malloc(n);track(p,active);return p;}
void *__wrap_calloc(size_t n,size_t m){bool active=allocation_failure>=0;if(allocation_denied())return NULL;void *p=__real_calloc(n,m);track(p,active);return p;}
void __wrap_free(void *p){for(unsigned i=0;i<tracked_count;i++)if(tracked[i]==p){tracked[i]=tracked[--tracked_count];break;}__real_free(p);}
int __wrap_ioctl(int,unsigned long,...);
int __wrap_ioctl(int fd,unsigned long command,...)
{
 va_list args;va_start(args,command);struct remesa_memory_request *r=va_arg(args,void *);va_end(args);
 if(inject&&command==DRM_IOCTL_REMESA_MEMORY) {
  if(alias_create&&r->action==REMESA_MEM_CREATE){r->id=32;return 0;}
  if(r->action==fail_action){errno=ENOMEM;return -1;}
 }
 int result=inherited_ioctl(fd,command,r);
 if(!result&&initial_corrupt&&command==DRM_IOCTL_REMESA_MEMORY&&r->action==REMESA_MEM_READ&&r->id==33&&!geometry_write_lease)r->data[0]^=1;
 return result;
}
int main(int argc,char **argv)
{
 const char *mode=argc>1?argv[1]:"good";
 struct remesa_device device={0};struct remesa_linux_memory *memory=NULL;void *token=NULL;
 native_storage(&memory,&device);struct remesa_shader_draw_programs pair=programs();
 struct remesa_usse_vertex_mapping map={0};
 for(unsigned c=0;c<4;c++){map.input[c]=(struct remesa_usse_cpu_register){0,32+c};map.output[c]=(struct remesa_usse_cpu_register){2,64+c};}
 CHECK(!remesa_linux_set_vertex_candidate_mapping(&device,&map));
 CHECK(device.ops->prepare_shader_programs(device.priv,&pair,&token)==ENOTSUP&&token);
 struct remesa_shader_request request;struct remesa_shader_bindings bindings;
 CHECK(!remesa_linux_shader_bindings(&device,token,&request,&bindings));
 struct remesa_pds_request pds[2];
 for(unsigned i=0;i<2;i++){
  pds[i]=(struct remesa_pds_request){.operation=9,.generation=request.generation,.owner_cookie=request.owner,
   .code={.allocation_cookie=64+i,.owner_cookie=request.owner,.mapping_generation=19+i,
    .address=0x20000000+4096*i,.offset=64,.bytes=16,.allocation_bytes=4096}};
  pds[i].data=pds[i].code;pds[i].data.offset=16;pds[i].data.bytes=i?12:20;
 }
 /* Split original host CODE/DATA windows must not need a T+8 projection
  * merely to apply the checked USE entry relocation to immutable DATA. */
 if(!strcmp(mode,"split"))for(unsigned i=0;i<2;i++) {
  pds[i].data.allocation_cookie=80+i;
  pds[i].data.address=UINT64_C(0x21000000)+4096*i;
 }
 uint32_t banks[2]={3,4};uint64_t offsets[2]={0,8};
 CHECK(!remesa_linux_shader_retain_pds_links(&device,token,0x10000000,banks,pds,offsets));
 unsigned char source[2][20];memset(source,0x6b,sizeof(source));
 struct remesa_pds_host_data data[2];for(unsigned i=0;i<2;i++)data[i]=(struct remesa_pds_host_data){.binding=pds[i].data,.bytes=source[i],.byte_length=(size_t)pds[i].data.bytes};
 if(!strcmp(mode,"split")) {
  struct remesa_pds_record_preview record[2],sentinel;
  memset(record,0xa5,sizeof(record));sentinel=record[0];
  CHECK(remesa_linux_shader_preview_pds_records(&device,token,0x10000000,pds,record)==ENOTSUP);
  CHECK(!memcmp(&record[0],&sentinel,sizeof(sentinel))&&!memcmp(&record[1],&sentinel,sizeof(sentinel)));
 }
 if(!strcmp(mode,"split")) {
  struct remesa_pds_host_data invalid[2]={data[0],data[1]};
  unsigned char unchanged[2][20];memcpy(unchanged,source,sizeof(source));
  ++invalid[1].binding.mapping_generation;
  CHECK(remesa_linux_shader_apply_host_pds_fixups(&device,token,0x10000000,pds,invalid)==ESTALE);
  CHECK(!memcmp(unchanged,source,sizeof(source)));
  invalid[1]=data[1];invalid[1].bytes=invalid[0].bytes;
  CHECK(remesa_linux_shader_apply_host_pds_fixups(&device,token,0x10000000,pds,invalid)==EINVAL);
  CHECK(!memcmp(unchanged,source,sizeof(source)));
 }
 CHECK(!remesa_linux_shader_retain_host_pds_data(&device,token,0x10000000,pds,data));
 struct remesa_pds_host_data_view view[2];
 CHECK(!remesa_linux_shader_host_pds_data(&device,token,0x10000000,pds,view));
 if(!strcmp(mode,"split"))for(unsigned i=0;i<2;i++) {
  uint64_t relative=bindings.resources[i].offset%UINT64_C(0x80000);
  uint32_t entry=banks[i]+(uint32_t)(relative/32768)*16+
   (uint32_t)((relative/16)%2048)*256;
  uint32_t expected=(UINT32_C(0x6b6b6b6b)&~UINT32_C(0x7ffff))|entry;
  const unsigned char *bytes=view[i].storage.bytes;
  for(unsigned b=0;b<data[i].byte_length;b++) {
   uint32_t want=b>=offsets[i]&&b<offsets[i]+4?
    (expected>>(8*(b-(unsigned)offsets[i])))&255u:0x6bu;
   CHECK(bytes[b]==want&&source[i][b]==0x6b);
  }
 }
 /* A live token is not enough if its DATA relocation no longer names the
  * retained USE entry. Reject before allocating/pinning provider backing. */
 if(!strcmp(mode,"entry-corrupt")) {
  for(unsigned stage=0;stage<2;stage++) {
   unsigned char *word=(unsigned char *)view[stage].storage.bytes+offsets[stage];
   *word^=1u;
   struct remesa_pds_host_data_view output[2],before_view[2];
   memset(output,0xa5,sizeof(output));memcpy(before_view,output,sizeof(output));
   unsigned provider_calls=calls;
   CHECK(remesa_linux_shader_host_pds_data(&device,token,0x10000000,pds,output)==EPROTO);
   CHECK(!memcmp(output,before_view,sizeof(output))&&calls==provider_calls);
   CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==EPROTO);
   CHECK(calls==provider_calls&&!geometry_live&&!geometry_pinned&&!geometry_leased);
   CHECK(remesa_linux_shader_verify(&device,token)==EPROTO&&calls==provider_calls);
   *word^=1u;
   CHECK(!remesa_linux_shader_host_pds_data(&device,token,0x10000000,pds,output));
  }
 }
 memset(source,0xa5,sizeof(source));
 struct remesa_persistent_pds_backing backing,saved;memset(&backing,0x91,sizeof(backing));saved=backing;
 unsigned before=calls;
 CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EAGAIN&&!memcmp(&backing,&saved,sizeof(backing))&&calls==before);
 struct remesa_pds_request stale[2];memcpy(stale,pds,sizeof(stale));++stale[1].data.mapping_generation;
 CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,stale)==ESTALE&&calls==before);
 CHECK(remesa_linux_shader_stage_pds_data(&device,(void *)1,0x10000000,pds)==EINVAL&&calls==before);
 if(!strcmp(mode,"alloc")) {
  for(int failure=0;failure<4;failure++){
   before=calls;allocation_failure=failure;
   int result=remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds);allocation_failure=-1;
   CHECK(result==ENOMEM&&calls==before&&!geometry_live&&!tracked_count);
   CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EAGAIN&&!memcmp(&backing,&saved,sizeof(backing)));
  }
 } else if(!strcmp(mode,"budget")) {
  void *other=NULL;CHECK(device.ops->prepare_shader_programs(device.priv,&pair,&other)==ENOTSUP&&other);
  before=calls;CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==ENOSPC&&calls==before);
  device.ops->destroy_shader_programs(device.priv,other);CHECK(!geometry_live);
 } else if(!strcmp(mode,"initial-corrupt")) {
  initial_corrupt=true;CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==EPROTO);initial_corrupt=false;
  CHECK(!geometry_live&&!geometry_pinned&&!geometry_leased);
  CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EAGAIN&&!memcmp(&backing,&saved,sizeof(backing)));
 } else if(!strcmp(mode,"create")||!strcmp(mode,"pin")||!strcmp(mode,"lease")) {
  fail_action=!strcmp(mode,"create")?REMESA_MEM_CREATE:!strcmp(mode,"pin")?REMESA_MEM_PIN:REMESA_MEM_LEASE;inject=true;
  CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==ENOMEM);
  inject=false;CHECK(!geometry_live&&!geometry_pinned&&!geometry_leased);
  CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EAGAIN&&!memcmp(&backing,&saved,sizeof(backing)));
 } else if(!strcmp(mode,"alias")||!strcmp(mode,"write")||!strcmp(mode,"unlease")) {
  alias_create=!strcmp(mode,"alias");inject=alias_create||!strcmp(mode,"unlease");fail_action=REMESA_MEM_UNLEASE;fail_write=!strcmp(mode,"write");
  CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==EOWNERDEAD);
  inject=false;fail_write=false;
  CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EOWNERDEAD&&!memcmp(&backing,&saved,sizeof(backing)));
  before=calls;CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==EOWNERDEAD&&calls==before);
  CHECK(remesa_linux_close_checked(&device)==EBUSY);
  printf("PASS: %u PDS backing checks; %s quarantined, no GPU\n",checks,mode);return 0;
 }
 CHECK(!remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds));
 CHECK(!remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing));
 CHECK(backing.handle==33&&backing.owner==request.owner&&backing.generation==request.generation&&backing.allocation_bytes==4096&&backing.pinned&&backing.staged);
 CHECK(backing.data_offset[0]==0&&backing.data_offset[1]==32&&backing.data_bytes[0]==20&&backing.data_bytes[1]==12);
 CHECK(!memcmp(geometry_bytes,view[0].storage.bytes,20)&&!memcmp(geometry_bytes+32,view[1].storage.bytes,12));
 bool zero_padding=true;for(unsigned i=20;i<4096;i++)if((i<32||i>=44)&&geometry_bytes[i])zero_padding=false;CHECK(zero_padding);
 before=calls;CHECK(remesa_linux_shader_stage_pds_data(&device,token,0x10000000,pds)==EBUSY&&calls==before);
 CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,(void *)view[0].storage.bytes)==EINVAL&&calls==before);
 CHECK(!remesa_linux_shader_verify(&device,token));
 if(!strcmp(mode,"entry-corrupt"))for(unsigned stage=0;stage<2;stage++) {
  unsigned char *word=(unsigned char *)view[stage].storage.bytes+offsets[stage];
  *word^=1u;saved=backing;unsigned provider_calls=calls;
  CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EPROTO);
  CHECK(!memcmp(&backing,&saved,sizeof(backing))&&calls==provider_calls);
  CHECK(remesa_linux_shader_verify(&device,token)==EPROTO&&calls==provider_calls);
  CHECK(geometry_live&&geometry_pinned&&!geometry_leased);
  *word^=1u;
  CHECK(!remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing));
 }
 if(!strcmp(mode,"corrupt")) {
  geometry_bytes[31]=1;
  CHECK(remesa_linux_shader_verify(&device,token)==EOWNERDEAD);
  saved=backing;CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EOWNERDEAD&&!memcmp(&backing,&saved,sizeof(backing)));
  CHECK(remesa_linux_close_checked(&device)==EBUSY);
  printf("PASS: %u PDS backing checks; corrupt padding quarantined, no GPU\n",checks);return 0;
 }
 void *fence=(void *)1;before=calls;
 CHECK(device.ops->submit_shader_programs(device.priv,(void *)1,(void *)1,(void *)1,token,&fence)==ENOTSUP&&!fence&&calls==before);
 device.ops->destroy_shader_programs(device.priv,token);
 CHECK(!live&&!pinned&&!geometry_live&&!geometry_pinned&&!geometry_leased);
 before=calls;saved=backing;
 CHECK(remesa_linux_shader_pds_backing(&device,token,0x10000000,pds,&backing)==EINVAL&&!memcmp(&backing,&saved,sizeof(backing))&&calls==before);
 CHECK(!remesa_linux_close_checked(&device));CHECK(!remesa_linux_memory_detach(memory));
 printf("PASS: %u PDS backing checks; %s, exact CPU readback, no GPU\n",checks,mode);return 0;
}
