/* SPDX-License-Identifier: MIT */
/* Host geometry translation only; no device, ioctl or rasterizer. */
#ifdef NDEBUG
#error Assertions are required
#endif
#include "remesa_draw.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned cases;
static void check(enum remesa_draw_status actual, enum remesa_draw_status expected)
{ assert(actual==expected); ++cases; }
int main(void)
{
 const float triangle[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
 struct pipe_resource vr={.target=PIPE_BUFFER,.width0=320080}, ir={.target=PIPE_BUFFER};
 struct pipe_vertex_buffer vb={.buffer.resource=&vr,.buffer_offset=3};
 struct pipe_vertex_element ve={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=32,.src_offset=5};
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};
 struct pipe_draw_start_count_bias d={.count=3};
 unsigned char *vertices=malloc(vr.width0);assert(vertices);memset(vertices,0xff,vr.width0);
 struct remesa_draw_packet *p=malloc(sizeof(*p)), *saved=malloc(sizeof(*saved));assert(p&&saved);
 uint32_t indices[4]={999999,8001,9001,10001};
 for(unsigned i=0;i<3;i++) memcpy(vertices+8+(8000+i*1000)*32,triangle[i],16);
 ir.width0=sizeof(indices);info.index_size=4;info.index.resource=&ir;d.start=1;d.index_bias=-1;
#define PACK() remesa_pack_triangle_draw(&info,&d,&vb,&ve,vertices,vr.width0,indices,sizeof(indices),0xffff00ff,p)
 check(PACK(),REMESA_DRAW_VALIDATED);
 assert(p->vertex_count==3 && !memcmp(p->positions,triangle,sizeof(triangle)));
 assert(p->indices[0]==0&&p->indices[1]==1&&p->indices[2]==2);
 /* An incomplete final primitive has no index storage requirement. The BO
  * and snapshot end immediately after the three consumed indices. */
 for(unsigned tail=1;tail<=2;++tail) {
  d.count=3+tail;check(PACK(),REMESA_DRAW_VALIDATED);
  assert(p->index_count==3&&p->vertex_count==3);
 }
 d.count=3;
 /* Large sparse indices do not allocate or validate unreferenced vertices. */
 indices[3]=8001;check(PACK(),REMESA_DRAW_VALIDATED);assert(p->vertex_count==2&&p->indices[2]==0);indices[3]=10001;
 info.index_bounds_valid=true;info.min_index=8001;info.max_index=10001;check(PACK(),REMESA_DRAW_VALIDATED);
 info.max_index=10000;check(PACK(),REMESA_DRAW_INVALID);info.index_bounds_valid=false;
 d.index_bias=-8002;check(PACK(),REMESA_DRAW_INVALID);d.index_bias=-1;
 ir.width0=15;check(PACK(),REMESA_DRAW_INVALID);ir.width0=16;
 check(remesa_pack_triangle_draw(&info,&d,&vb,&ve,vertices,vr.width0-1,indices,sizeof(indices),0,p),REMESA_DRAW_INVALID);
 check(remesa_pack_triangle_draw(&info,&d,&vb,&ve,vertices,vr.width0,indices,15,0,p),REMESA_DRAW_INVALID);
 ve.src_stride=UINT32_MAX;check(PACK(),REMESA_DRAW_INVALID);ve.src_stride=32;
 d.count=REMESA_DRAW_MAX_INDICES+3;check(PACK(),REMESA_DRAW_UNSUPPORTED);d.count=3;
 info.instance_count=2;check(PACK(),REMESA_DRAW_UNSUPPORTED);info.instance_count=1;
 info.has_user_indices=true;check(PACK(),REMESA_DRAW_UNSUPPORTED);info.has_user_indices=false;
 d.count=0;check(PACK(),REMESA_DRAW_EMPTY);d.count=3;
 /* Unaligned native index snapshots; all widths yield the same compact packet. */
 for(unsigned width=1;width<=4;width*=2) {
  unsigned char ix[13]={0};d.start=0;d.index_bias=0;info.index_size=width;ir.width0=3*width;
  for(unsigned i=0;i<3;i++) { uint32_t v=i;memcpy(ix+1+i*width,&v,width);memcpy(vertices+8+i*32,triangle[i],16); }
  check(remesa_pack_triangle_draw(&info,&d,&vb,&ve,vertices,vr.width0,ix+1,3*width,0xffff00ff,p),REMESA_DRAW_VALIDATED);
  assert(!memcmp(p->positions,triangle,sizeof(triangle)));
  for(unsigned tail=1;tail<=2;++tail) {
   d.count=3+tail;
   check(remesa_pack_triangle_draw(&info,&d,&vb,&ve,vertices,vr.width0,
             ix+1,3*width,0xffff00ff,p),REMESA_DRAW_VALIDATED);
   assert(p->index_count==3&&p->vertex_count==3);
  }
  d.count=3;
 }
 /* Nonindexed input deliberately poisons the index union and index bias. */
 info.index_size=0;info.index.user=(void *)1;info.has_user_indices=true;
 d.index_bias=INT32_MIN;d.start=0;check(PACK(),REMESA_DRAW_VALIDATED);assert(p->vertex_count==3);
 /* The vertex BO need only contain the complete primitive, even when the
  * API count includes one or two additional, unreferenced vertices. */
 uint32_t available=vr.width0;vr.width0=8+2*32+16;
 for(unsigned tail=1;tail<=2;++tail) {
  d.count=3+tail;check(PACK(),REMESA_DRAW_VALIDATED);
  assert(p->index_count==3&&p->vertex_count==3);
 }
 d.count=3;vr.width0=available;
 d.start=UINT32_MAX;check(PACK(),REMESA_DRAW_INVALID);d.start=0;
 vb.buffer_offset=UINT32_MAX;ve.src_offset=UINT16_MAX;check(PACK(),REMESA_DRAW_INVALID);vb.buffer_offset=3;ve.src_offset=5;
 memcpy(saved,p,sizeof(*p));float bad=NAN;memcpy(vertices+8,&bad,4);
 check(PACK(),REMESA_DRAW_INVALID);assert(!memcmp(saved,p,sizeof(*p)));
 memcpy(vertices+8,triangle[0],16);bad=2;memcpy(vertices+8+12,&bad,4);
 check(PACK(),REMESA_DRAW_UNSUPPORTED);assert(!memcmp(saved,p,sizeof(*p)));
 /* More than the compact-vertex capacity fails before packet construction. */
 for(unsigned i=0;i<1026;i++) memcpy(vertices+8+i*32,triangle[0],16);
 d.count=1026;check(PACK(),REMESA_DRAW_UNSUPPORTED);assert(!memcmp(saved,p,sizeof(*p)));
 /* Empty primitives neither read a snapshot nor dereference resource storage.
  * Packing keeps its existing unchanged-output EMPTY contract. */
 for(unsigned empty=1;empty<=2;++empty) {
  d.count=empty;d.start=UINT32_MAX;
  check(remesa_pack_triangle_draw(&info,&d,&vb,&ve,NULL,0,NULL,0,0,p),REMESA_DRAW_EMPTY);
  assert(!memcmp(saved,p,sizeof(*p)));
 }
 /* Capacity applies to complete primitives, not to ignored trailing indices. */
 uint16_t repeated[REMESA_DRAW_MAX_INDICES];
 for(unsigned i=0;i<REMESA_DRAW_MAX_INDICES;++i)repeated[i]=i%3;
 info.index_size=2;info.has_user_indices=false;info.index.resource=&ir;
 ir.width0=sizeof(repeated);d.start=0;d.index_bias=0;
 for(unsigned i=0;i<3;++i)memcpy(vertices+8+i*32,triangle[i],16);
 for(unsigned tail=0;tail<=2;++tail) {
  d.count=REMESA_DRAW_MAX_INDICES+tail;
  check(remesa_pack_triangle_draw(&info,&d,&vb,&ve,vertices,vr.width0,
            repeated,sizeof(repeated),0xffff00ff,p),REMESA_DRAW_VALIDATED);
  assert(p->index_count==REMESA_DRAW_MAX_INDICES&&p->vertex_count==3);
 }
 free(vertices);free(p);free(saved);
 printf("PASS: %u geometry packing cases; no hardware\n",cases);return 0;
}
