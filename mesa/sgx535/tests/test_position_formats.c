/* SPDX-License-Identifier: MIT */
#include "remesa_draw.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error Tests require assertions
#endif
static unsigned cases;
#define CHECK(x) do { assert(x); ++cases; } while(0)
static struct remesa_draw_packet packet, original, reference;
static struct remesa_prepared_triangles prepared, old_prepared;
int main(void)
{
   const enum pipe_format formats[]={PIPE_FORMAT_R32G32_FLOAT,
       PIPE_FORMAT_R32G32B32_FLOAT,PIPE_FORMAT_R32G32B32A32_FLOAT};
   const float raw[3][4]={{8,8,0,1},{24,8,0,1},{8,24,0,1}};
   const float clip[3][4]={{-.5f,-.5f,0,1},{.5f,-.5f,0,1},{-.5f,.5f,0,1}};
   const float window[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
   struct pipe_resource vr={.target=PIPE_BUFFER},ir={.target=PIPE_BUFFER};
   struct pipe_vertex_buffer vb={.buffer.resource=&vr,.buffer_offset=1};
   struct pipe_vertex_element ve={.src_offset=2};
   struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};
   struct pipe_draw_start_count_bias draw={.count=3};
   struct pipe_viewport_state vp={0};vp.scale[0]=vp.scale[1]=16;
   vp.translate[0]=vp.translate[1]=16;vp.scale[2]=vp.translate[2]=.5f;
   unsigned char bytes[160];uint16_t indices[]={2,3,2};ir.width0=sizeof(indices);
   memset(&original,0x5a,sizeof(original));memset(&old_prepared,0x5a,sizeof(old_prepared));
   CHECK(remesa_position_format_bytes(PIPE_FORMAT_R32_FLOAT)==0);
   CHECK(remesa_position_format_bytes(PIPE_FORMAT_R16G16_FLOAT)==0);
   CHECK(remesa_position_format_bytes(PIPE_FORMAT_R32G32_UINT)==0);
   for(unsigned f=0;f<3;f++) {
      ve.src_format=formats[f];unsigned n=remesa_position_format_bytes(ve.src_format);
      CHECK(n==8+f*4);ve.src_stride=n+5;vr.width0=3+2*ve.src_stride+n;
      memset(bytes,0xff,sizeof(bytes));
      for(unsigned i=0;i<3;i++)memcpy(bytes+3+i*ve.src_stride,raw[i],n);
#define PACK() remesa_pack_triangle_draw(&info,&draw,&vb,&ve,bytes,vr.width0,indices,sizeof(indices),0xffff00ff,&packet)
      info.index_size=0;draw.start=0;packet=original;
      CHECK(PACK()==REMESA_DRAW_VALIDATED);
      CHECK(packet.vertex_count==3&&!memcmp(packet.positions,raw,sizeof(raw)));
      if(!f)reference=packet;else CHECK(!memcmp(&packet,&reference,sizeof(packet)));
      vr.width0--;packet=original;
      CHECK(PACK()==REMESA_DRAW_INVALID);CHECK(!memcmp(&packet,&original,sizeof(packet)));vr.width0++;
      CHECK(remesa_pack_triangle_draw(&info,&draw,&vb,&ve,bytes,vr.width0-1,
            NULL,0,0,&packet)==REMESA_DRAW_INVALID);
      CHECK(!memcmp(&packet,&original,sizeof(packet)));
      ve.src_stride=0;CHECK(PACK()==REMESA_DRAW_UNSUPPORTED);ve.src_stride=n+5;
      info.index_size=2;info.index.resource=&ir;draw.index_bias=-1;
      CHECK(PACK()==REMESA_DRAW_VALIDATED);
      CHECK(packet.vertex_count==2&&packet.indices[0]==0&&packet.indices[1]==1&&packet.indices[2]==0);
      CHECK(!memcmp(packet.positions[0],raw[1],16)&&!memcmp(packet.positions[1],raw[2],16));
      struct remesa_draw_plan plan;
      CHECK(remesa_validate_indexed_triangles(&info,&draw,&vb,&ve,indices,sizeof(indices),&plan)==REMESA_DRAW_VALIDATED);
      CHECK(plan.vertex_end_byte==vr.width0&&plan.min_vertex==1&&plan.max_vertex==2);
      vr.width0--;packet=original;
      CHECK(PACK()==REMESA_DRAW_INVALID);CHECK(!memcmp(&packet,&original,sizeof(packet)));vr.width0++;
      draw.index_bias=-3;CHECK(PACK()==REMESA_DRAW_INVALID);draw.index_bias=INT32_MAX;
      CHECK(PACK()==REMESA_DRAW_INVALID);draw.index_bias=-1;
      ve.src_stride=UINT32_MAX;CHECK(PACK()==REMESA_DRAW_INVALID);ve.src_stride=n+5;
      vb.buffer_offset=UINT32_MAX;CHECK(PACK()==REMESA_DRAW_INVALID);vb.buffer_offset=1;
      /* Defaults are inserted before host viewport and clipping. */
      for(unsigned i=0;i<3;i++)memcpy(bytes+3+i*ve.src_stride,clip[i],n);
      info.index_size=0;draw.index_bias=INT32_MIN;
      CHECK(remesa_pack_triangle_draw_vertex(&info,&draw,&vb,&ve,bytes,vr.width0,NULL,0,
            0xffff00ff,&vp,&packet)==REMESA_DRAW_VALIDATED);
      CHECK(!memcmp(packet.positions,window,sizeof(window)));
      CHECK(remesa_prepare_constant_draw(&info,&draw,&vb,&ve,bytes,vr.width0,NULL,0,
            0xffff00ff,&vp,32,32,&prepared)==REMESA_DRAW_VALIDATED);
      CHECK(prepared.vertex_count==3&&!memcmp(prepared.positions,window,sizeof(window)));
      vr.width0--;prepared=old_prepared;
      CHECK(remesa_prepare_constant_draw(&info,&draw,&vb,&ve,bytes,vr.width0,NULL,0,
            0xffff00ff,&vp,32,32,&prepared)==REMESA_DRAW_INVALID);
      CHECK(!memcmp(&prepared,&old_prepared,sizeof(prepared)));vr.width0++;
      float nan=NAN;memcpy(bytes+3,&nan,4);packet=original;
      CHECK(remesa_pack_triangle_draw_vertex(&info,&draw,&vb,&ve,bytes,vr.width0,NULL,0,
            0xffff00ff,&vp,&packet)==REMESA_DRAW_INVALID);
      CHECK(!memcmp(&packet,&original,sizeof(packet)));
   }
   ve.src_format=PIPE_FORMAT_R32_UINT;packet=original;prepared=old_prepared;
   CHECK(PACK()==REMESA_DRAW_UNSUPPORTED);CHECK(!memcmp(&packet,&original,sizeof(packet)));
   CHECK(remesa_prepare_constant_draw(&info,&draw,&vb,&ve,bytes,vr.width0,NULL,0,
         0xffff00ff,&vp,32,32,&prepared)==REMESA_DRAW_UNSUPPORTED);
   CHECK(!memcmp(&prepared,&old_prepared,sizeof(prepared)));
   printf("PASS: %u FLOAT2/3/4 host fetch cases; no GPU shader or rendering\n",cases);
   return 0;
}
