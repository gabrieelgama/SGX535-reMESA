/* SPDX-License-Identifier: MIT */
#ifdef NDEBUG
#error Assertions are required
#endif
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "remesa_draw.h"
int main(void)
{
    const unsigned dimensions[][2]={{32,32},{320,240},{640,480},{800,600},
       {1024,768},{1280,720},{1280,800},{1366,768},{1600,900},{1920,1080},{51,39}};
    float positions[3][4]={{-2,0,0,1},{0,-1,0,1},{0,1,0,1}};
    uint16_t indices[REMESA_DRAW_MAX_INDICES];
    for (unsigned i=0;i<REMESA_DRAW_MAX_INDICES;++i) indices[i]=i%3+1;
    struct pipe_resource vb={.target=PIPE_BUFFER,.width0=sizeof(positions)};
    struct pipe_resource ib={.target=PIPE_BUFFER,.width0=sizeof(indices)};
    struct pipe_vertex_buffer vertices={.buffer.resource=&vb};
    struct pipe_vertex_element element={.src_stride=16,.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT};
    struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1,.index_size=2,.index.resource=&ib};
    struct pipe_draw_start_count_bias draw={.count=3,.index_bias=-1};
    static struct remesa_prepared_triangles output, unchanged;
    struct pipe_viewport_state viewport={0};
    viewport.scale[2]=viewport.translate[2]=.5f;
    for (unsigned i=0;i<sizeof(dimensions)/sizeof(dimensions[0]);++i) {
        unsigned w=dimensions[i][0],h=dimensions[i][1];
        viewport.scale[0]=viewport.translate[0]=w/2.f;
        viewport.scale[1]=viewport.translate[1]=h/2.f;
        assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
                  indices,sizeof(indices),0xffff00ff,&viewport,w,h,&output)==REMESA_DRAW_VALIDATED);
        assert(output.width==w && output.height==h && output.argb==0xffff00ff && output.vertex_count==6);
        for (unsigned n=0;n<output.vertex_count;++n) {
            assert(output.positions[n][0]>=0 && output.positions[n][0]<=w);
            assert(output.positions[n][1]>=0 && output.positions[n][1]<=h);
        }
    }
    info.index_size=0; draw.index_bias=INT32_MAX;
    assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
              NULL,0,0xffff00ff,&viewport,51,39,&output)==REMESA_DRAW_VALIDATED);
    assert(output.vertex_count==6);
    memset(&unchanged,0x5a,sizeof(unchanged));output=unchanged;
    draw.start=1;
    assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
              NULL,0,0,&viewport,51,39,&output)==REMESA_DRAW_INVALID);
    assert(!memcmp(&output,&unchanged,sizeof(output)));
    draw.start=0; info.index_size=2; draw.index_bias=-1; draw.count=REMESA_DRAW_MAX_INDICES;
    assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
              indices,sizeof(indices),0,&viewport,51,39,&output)==REMESA_DRAW_UNSUPPORTED);
    assert(!memcmp(&output,&unchanged,sizeof(output))); /* expansion cannot truncate */
    draw.count=3;draw.index_bias=-2;
    assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
              indices,sizeof(indices),0,&viewport,51,39,&output)==REMESA_DRAW_INVALID);
    draw.index_bias=-1; element.src_stride=UINT32_MAX;
    assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
              indices,sizeof(indices),0,&viewport,51,39,&output)==REMESA_DRAW_INVALID);
    assert(!memcmp(&output,&unchanged,sizeof(output)));
    /* Historical indexed validator must also reject extreme fetch ranges. */
    uint32_t huge_indices[3]={UINT32_MAX,UINT32_MAX,UINT32_MAX};
    info.index_size=4; draw.index_bias=0;
    vertices.buffer_offset=UINT32_MAX; element.src_offset=UINT16_MAX;
    struct remesa_draw_plan plan;
    assert(remesa_validate_indexed_triangles(&info,&draw,&vertices,&element,
              huge_indices,sizeof(huge_indices),&plan)==REMESA_DRAW_INVALID);
    static struct remesa_draw_packet packet, original;
    memset(&original,0x5a,sizeof(original)); packet=original;
    assert(remesa_pack_triangle_draw(&info,&draw,&vertices,&element,
              positions,sizeof(positions),huge_indices,sizeof(huge_indices),0,&packet)==REMESA_DRAW_INVALID);
    assert(!memcmp(&packet,&original,sizeof(packet)));
    info.index_size=0; draw.start=UINT32_MAX-2;
    assert(remesa_pack_triangle_draw(&info,&draw,&vertices,&element,
              positions,sizeof(positions),NULL,0,0,&packet)==REMESA_DRAW_INVALID);
    assert(!memcmp(&packet,&original,sizeof(packet)));
    vertices.buffer_offset=0;element.src_offset=0;
    element.src_stride=16; draw.count=0;
    assert(remesa_prepare_constant_draw(&info,&draw,&vertices,&element,positions,sizeof(positions),
              indices,sizeof(indices),0,&viewport,51,39,&output)==REMESA_DRAW_EMPTY);
    assert(output.vertex_count==0);
    puts("PASS: actual Gallium indexed/nonindexed clipping preparation, 11 resolutions, signed bias, overflow and unchanged-output rejection; CPU geometry only");
    return 0;
}
