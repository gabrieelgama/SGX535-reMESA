/* SPDX-License-Identifier: MIT */
#ifdef NDEBUG
#error Assertions are required
#endif
#include "remesa_draw.h"
#include "remesa_screen.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static unsigned cases;
#define CHECK(e) do { assert(e); ++cases; } while (0)
int main(void)
{
    struct pipe_resource vb = {.target=PIPE_BUFFER, .width0=128};
    struct pipe_resource ib = {.target=PIPE_BUFFER, .width0=12};
    struct pipe_vertex_buffer vertices = {.buffer.resource=&vb};
    struct pipe_vertex_element position = {.src_stride=32,
        .src_format=PIPE_FORMAT_R32G32B32A32_FLOAT};
    struct pipe_draw_info info = {.mode=MESA_PRIM_TRIANGLES, .index_size=2,
        .instance_count=1, .index.resource=&ib};
    struct pipe_draw_start_count_bias draw = {.count=6};
    uint16_t idx[] = {0,1,2,1,3,2};
    unsigned char unaligned[sizeof(idx)+1];
    struct remesa_draw_plan p;
#define RUN remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,idx,sizeof(idx),&p)
    CHECK(remesa_screen_create(NULL)==NULL);
    CHECK(remesa_screen_create((struct remesa_device *)&vb)==NULL);
    CHECK(remesa_screen_unavailable_reason()!=NULL);
    CHECK(RUN==REMESA_DRAW_VALIDATED);
    CHECK(p.primitive_count==2 && p.min_vertex==0 && p.max_vertex==3);
    CHECK(p.index_first_byte==0 && p.index_end_byte==12 && p.vertex_end_byte==112);
    memcpy(unaligned+1,idx,sizeof(idx));
    CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,
          unaligned+1,sizeof(idx),&p)==REMESA_DRAW_VALIDATED);
    draw.count=3; draw.start=3;
    CHECK(RUN==REMESA_DRAW_VALIDATED && p.index_first_byte==6);
    draw.start=4; CHECK(RUN==REMESA_DRAW_INVALID && p.primitive_count==0);
    draw.start=0; draw.count=6;
    vb.width0=111; CHECK(RUN==REMESA_DRAW_INVALID);
    vb.width0=112; CHECK(RUN==REMESA_DRAW_VALIDATED);
    vertices.buffer_offset=1; CHECK(RUN==REMESA_DRAW_INVALID); vertices.buffer_offset=0;
    draw.index_bias=-1; CHECK(RUN==REMESA_DRAW_INVALID); draw.index_bias=1;
    CHECK(RUN==REMESA_DRAW_INVALID); draw.index_bias=0;
    info.index_bounds_valid=true; info.max_index=2;
    CHECK(RUN==REMESA_DRAW_INVALID); info.max_index=3;
    CHECK(RUN==REMESA_DRAW_VALIDATED); info.index_bounds_valid=false;
    info.primitive_restart=true; CHECK(RUN==REMESA_DRAW_UNSUPPORTED); info.primitive_restart=false;
    info.has_user_indices=true; CHECK(RUN==REMESA_DRAW_UNSUPPORTED); info.has_user_indices=false;
    info.take_index_buffer_ownership=true; CHECK(RUN==REMESA_DRAW_UNSUPPORTED); info.take_index_buffer_ownership=false;
    vertices.is_user_buffer=true; CHECK(RUN==REMESA_DRAW_UNSUPPORTED); vertices.is_user_buffer=false;
    position.instance_divisor=1; CHECK(RUN==REMESA_DRAW_UNSUPPORTED); position.instance_divisor=0;
    info.instance_count=2; CHECK(RUN==REMESA_DRAW_UNSUPPORTED); info.instance_count=0;
    CHECK(RUN==REMESA_DRAW_EMPTY); info.instance_count=1;
    draw.count=0; CHECK(RUN==REMESA_DRAW_EMPTY);
    /* Only complete TRIANGLES affect declared index bounds or vertex fetch.
     * The ignored fifth index names vertex3, outside these active bounds. */
    info.index_bounds_valid=true;info.max_index=2;
    for(unsigned tail=1;tail<=2;++tail) {
       draw.count=3+tail;
       CHECK(RUN==REMESA_DRAW_VALIDATED);
       CHECK(p.primitive_count==1 && p.max_vertex==2 &&
             p.index_end_byte==6 && p.vertex_end_byte==80);
       CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,
             idx,6,&p)==REMESA_DRAW_VALIDATED);
    }
    info.index_bounds_valid=false;draw.count=6;
    for(unsigned empty=1;empty<=2;++empty) {
       draw.count=empty;draw.start=UINT_MAX;
       CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,
             NULL,0,&p)==REMESA_DRAW_EMPTY);
       CHECK(p.primitive_count==0 && p.index_end_byte==0);
    }
    draw.start=0;draw.count=6;
    info.index_size=3; CHECK(RUN==REMESA_DRAW_INVALID); info.index_size=2;
    CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,idx,11,&p)==REMESA_DRAW_INVALID);
    draw.start=UINT_MAX; CHECK(RUN==REMESA_DRAW_INVALID); draw.start=0;
    vb.target=PIPE_TEXTURE_2D; CHECK(RUN==REMESA_DRAW_INVALID); vb.target=PIPE_BUFFER;
    info.index.resource=NULL; CHECK(RUN==REMESA_DRAW_INVALID); info.index.resource=&ib;
    CHECK(remesa_validate_indexed_triangles(NULL,&draw,&vertices,&position,idx,sizeof(idx),&p)==REMESA_DRAW_INVALID);
    CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,idx,sizeof(idx),NULL)==REMESA_DRAW_INVALID);
    CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,NULL,12,&p)==REMESA_DRAW_INVALID);
    { uint8_t small[]={0,1,2,1,3,2}; info.index_size=1;
      CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,small,sizeof(small),&p)==REMESA_DRAW_VALIDATED); }
    { uint32_t large[]={0,1,2,1,3,2}; info.index_size=4; ib.width0=sizeof(large);
      CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,large,sizeof(large),&p)==REMESA_DRAW_VALIDATED);
      large[5]=UINT32_MAX; draw.index_bias=1;
      CHECK(remesa_validate_indexed_triangles(&info,&draw,&vertices,&position,large,sizeof(large),&p)==REMESA_DRAW_INVALID); }
    printf("Gallium CPU preflight: %u checks PASS; no submission/provider\n", cases);
    return 0;
}
