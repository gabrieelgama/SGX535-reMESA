/* SPDX-License-Identifier: MIT
 * CPU transfer preparation only: no SGX execution or framebuffer preservation. */
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pipe/p_state.h"
#include "remesa_fixed_state.h"

int main(void)
{
 unsigned cases=0;
#define CHECK(x) do { ++cases; assert(x); } while (0)
 const unsigned dimensions[][2]={{32,32},{320,240},{640,480},{800,600},{1024,768},
  {1280,720},{1280,800},{1366,768},{1600,900},{1920,1080},{17,23}};
 union pipe_color_union color={.f={1,.25f,0,.5f}};
 struct remesa_color_clear plan;
 for(unsigned i=0;i<sizeof(dimensions)/sizeof(dimensions[0]);++i) {
  unsigned w=dimensions[i][0],h=dimensions[i][1],stride=(w*4+63)&~63u;
  size_t bytes=(size_t)stride*h;
  CHECK(!remesa_prepare_color_clear(PIPE_FORMAT_B8G8R8A8_UNORM,w,h,stride,bytes,&color,0,0,w,h,&plan));
  CHECK(plan.offset==0&&plan.row_bytes==w*4&&plan.rows==h&&plan.stride==stride);
  CHECK(plan.pixel[0]==0&&plan.pixel[1]==64&&plan.pixel[2]==255&&plan.pixel[3]==128);
  CHECK(!remesa_prepare_color_clear(PIPE_FORMAT_R8G8B8A8_UNORM,w,h,stride,bytes,&color,w-1,h-1,1,1,&plan));
  CHECK(plan.offset==(size_t)(h-1)*stride+(w-1)*4&&plan.row_bytes==4&&plan.rows==1);
  CHECK(plan.pixel[0]==255&&plan.pixel[1]==64&&plan.pixel[2]==0&&plan.pixel[3]==128);
  /* Exercise exactly the plan a mapping caller consumes. Preserve row padding,
   * and all pixels outside a partial rectangle. */
  unsigned char storage[23*128];
  if(w==17) {
   memset(storage,0x5a,sizeof(storage));
   CHECK(!remesa_prepare_color_clear(PIPE_FORMAT_R8G8B8A8_UNORM,w,h,stride,bytes,&color,2,3,5,7,&plan));
   for(unsigned row=0;row<plan.rows;++row)
    for(size_t column=0;column<plan.row_bytes;column+=4)
     memcpy(storage+plan.offset+row*plan.stride+column,plan.pixel,4);
   for(unsigned row=0;row<h;++row)for(unsigned column=0;column<stride;++column) {
    unsigned char expected=row>=3&&row<10&&column>=8&&column<28 ? plan.pixel[column%4]:0x5a;
    CHECK(storage[row*stride+column]==expected);
   }
  }
 }
 const unsigned w=17,h=23,stride=128;const size_t bytes=stride*h;
 struct remesa_color_clear original;memset(&original,0xa5,sizeof(original));
#define BAD(expr, error) do { plan=original; CHECK((expr)==(error)); CHECK(!memcmp(&plan,&original,sizeof(plan))); } while (0)
#define CALL(f,c,x,y,rw,rh,s,n) remesa_prepare_color_clear(f,w,h,s,n,c,x,y,rw,rh,&plan)
 BAD(CALL(PIPE_FORMAT_R8_UNORM,&color,0,0,1,1,stride,bytes),ENOTSUP);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,NULL,0,0,1,1,stride,bytes),EINVAL);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,64,bytes),EINVAL);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,129,bytes),EINVAL);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,stride,bytes-1),ENOSPC);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,18,0,0,0,stride,bytes),EINVAL);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,24,0,0,stride,bytes),EINVAL);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,1,0,UINT32_MAX,1,stride,bytes),EINVAL);
 BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,1,1,UINT32_MAX,stride,bytes),EINVAL);
 BAD(remesa_prepare_color_clear(PIPE_FORMAT_R8G8B8A8_UNORM,UINT32_MAX,1,UINT32_MAX,SIZE_MAX,&color,0,0,1,1,&plan),EOVERFLOW);
 if(SIZE_MAX==UINT32_MAX)
  BAD(remesa_prepare_color_clear(PIPE_FORMAT_R8G8B8A8_UNORM,1,UINT32_MAX,4,SIZE_MAX,&color,0,0,1,1,&plan),EOVERFLOW);
 color.f[0]=NAN;BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,stride,bytes),EINVAL);
 color.f[0]=INFINITY;BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,stride,bytes),EINVAL);
 color.f[0]=-INFINITY;BAD(CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,stride,bytes),EINVAL);
 color.f[0]=-2;color.f[1]=2;
 CHECK(!CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,0,0,1,1,stride,bytes));
 CHECK(plan.pixel[0]==0&&plan.pixel[1]==255);
 CHECK(!CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,w,h,0,0,stride,bytes));
 CHECK(!plan.offset&&!plan.rows&&!plan.row_bytes);
 CHECK(!CALL(PIPE_FORMAT_R8G8B8A8_UNORM,&color,w,0,0,h,stride,bytes));
 CHECK(!plan.offset&&!plan.rows&&!plan.row_bytes);
 CHECK(remesa_prepare_color_clear(PIPE_FORMAT_R8G8B8A8_UNORM,w,h,stride,bytes,&color,0,0,1,1,NULL)==EINVAL);
 printf("clear preparation: %u checks PASS; CPU transfer plan only, no GPU\n",cases);
 return 0;
}
