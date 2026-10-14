/* SPDX-License-Identifier: MIT */
#ifdef NDEBUG
#error This qualification requires assertions
#endif
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pipe/p_state.h"
#include "remesa_fixed_state.h"

static struct pipe_viewport_state viewport(unsigned width, unsigned height)
{
   struct pipe_viewport_state v = {0};
   v.scale[0] = v.translate[0] = width / 2.f;
   v.scale[1] = v.translate[1] = height / 2.f;
   v.scale[2] = v.translate[2] = .5f;
   return v;
}
int main(void)
{
   const unsigned sizes[][2] = {{32,32},{320,240},{640,480},{800,600},
      {1024,768},{1280,720},{1280,800},{1366,768},{1600,900},{1920,1080},
      {37,53}};
   const float triangle[3][4] = {{-1,-1,0,1},{1,-1,0,1},{0,1,0,1}};
   struct remesa_clipped_polygon result, saved;
   for (unsigned n = 0; n < sizeof(sizes)/sizeof(sizes[0]); ++n) {
      unsigned w = sizes[n][0], h = sizes[n][1];
      struct pipe_viewport_state v = viewport(w,h);
      assert(!remesa_prepare_constant_triangle(triangle,&v,w,h,&result));
      assert(result.count == 3 && result.window[0][0] == 0 && result.window[0][1] == 0);
      assert(result.window[1][0] == w && result.window[2][1] == h);
      assert(result.window[2][0] == w / 2.f && result.window[0][2] == .5f);
      v.scale[1] = -v.scale[1];
      assert(!remesa_prepare_constant_triangle(triangle,&v,w,h,&result));
      assert(result.window[0][1] == h && result.window[2][1] == 0);
   }
   struct pipe_viewport_state v = viewport(1280,800);
   const float partial[3][4] = {{-2,0,0,1},{0,-1,0,1},{0,1,0,1}};
   assert(!remesa_prepare_constant_triangle(partial,&v,1280,800,&result));
   assert(result.count == 4);
   assert(result.window[0][0] == 0 && result.window[0][1] == 600);
   assert(result.window[1][0] == 0 && result.window[1][1] == 200);
   for (unsigned axis=0; axis<3; ++axis) {
      for (unsigned sign=0; sign<2; ++sign) {
         float crossed[3][4] = {{0,0,0,1},{0,-.5f,0,1},{0,.5f,0,1}};
         /* Keep the triangle nondegenerate on the selected clip plane. */
         unsigned other = axis == 1 ? 0 : 1;
         crossed[0][axis] = sign ? 2 : -2;
         crossed[1][other] = -.5f; crossed[2][other] = .5f;
         assert(!remesa_prepare_constant_triangle((const float (*)[4])crossed,&v,1280,800,&result));
         assert(result.count == 4);
         for (unsigned i=0;i<result.count;++i) {
            assert(result.window[i][0]>=0 && result.window[i][0]<=1280);
            assert(result.window[i][1]>=0 && result.window[i][1]<=800);
            assert(result.window[i][2]>=0 && result.window[i][2]<=1);
         }
      }
   }
   const float outside[3][4] = {{-3,-1,0,1},{-2,-1,0,1},{-2,1,0,1}};
   assert(!remesa_prepare_constant_triangle(outside,&v,1280,800,&result));
   assert(!result.count);
   const float behind[3][4] = {{0,0,0,-1},{.25f,0,0,-1},{0,.25f,0,-1}};
   assert(!remesa_prepare_constant_triangle(behind,&v,1280,800,&result));
   assert(!result.count);
   const float perspective[3][4] = {{-2,-2,0,2},{2,-2,0,2},{0,2,0,2}};
   assert(!remesa_prepare_constant_triangle(perspective,&v,1280,800,&result));
   assert(result.count == 3 && result.window[1][0] == 1280);
   const float plane[3][4] = {{-1,0,0,1},{-2,0,0,1},{0,1,0,1}};
   assert(!remesa_prepare_constant_triangle(plane,&v,1280,800,&result));
   assert(result.count == 3); /* no repeated plane-boundary vertex */
   memset(&saved,0x5a,sizeof(saved)); result = saved;
   float bad[3][4]; memcpy(bad,triangle,sizeof(bad)); bad[1][0] = NAN;
   assert(remesa_prepare_constant_triangle((const float (*)[4])bad,&v,1280,800,&result) == EINVAL);
   assert(!memcmp(&saved,&result,sizeof(saved)));
   memcpy(bad,triangle,sizeof(bad)); memset(bad[0],0,sizeof(bad[0]));
   assert(remesa_prepare_constant_triangle((const float (*)[4])bad,&v,1280,800,&result) == ENOTSUP);
   assert(!memcmp(&saved,&result,sizeof(saved)));
   v.translate[0] = 0;
   assert(remesa_prepare_constant_triangle(triangle,&v,1280,800,&result) == ENOTSUP);
   v = viewport(1280,800); v.scale[2] = INFINITY;
   assert(remesa_prepare_constant_triangle(triangle,&v,1280,800,&result) == EINVAL);
   v = viewport(1280,800); v.swizzle_x = 1;
   assert(remesa_prepare_constant_triangle(triangle,&v,1280,800,&result) == ENOTSUP);
   struct pipe_scissor_state sc = {.minx=1200,.miny=700,.maxx=1500,.maxy=900}, clipped;
   assert(!remesa_clip_scissor(&sc,1280,800,&clipped));
   assert(clipped.minx==1200 && clipped.maxx==1280 && clipped.miny==700 && clipped.maxy==800);
   sc.minx=1400;
   assert(!remesa_clip_scissor(&sc,1280,800,&clipped));
   assert(clipped.minx==clipped.maxx);
   sc.minx=1600; memset(&clipped,0x5a,sizeof(clipped));
   struct pipe_scissor_state unchanged=clipped;
   assert(remesa_clip_scissor(&sc,1280,800,&clipped)==EINVAL);
   assert(!memcmp(&clipped,&unchanged,sizeof(clipped)));
   puts("PASS: 11 resolution/origin cases; all six homogeneous clip planes, perspective host preparation, scissor and rejection contracts; CPU geometry only");
   return 0;
}
