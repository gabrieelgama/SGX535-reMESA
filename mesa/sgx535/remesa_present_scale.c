/* SPDX-License-Identifier: MIT */
#include "remesa_present_scale.h"
#include "remesa_present.h"
#include "pipe/p_state.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

int remesa_present_scaled_snapshot(struct pipe_context *ctx,struct pipe_resource *res,
 void *destination,size_t stride,size_t capacity,uint32_t width,uint32_t height,
 enum remesa_present_scale_mode mode,struct remesa_present_scale_receipt *receipt)
{
 size_t row,required,source_stride,source_bytes;
 unsigned char *source;int error;struct remesa_present_scale_receipt result={0};
 if(!receipt)return EINVAL;
 memset(receipt,0,sizeof(*receipt));
 if(!ctx||!res||!destination||!width||!height||width>INT32_MAX||height>INT32_MAX||
    !res->width0||!res->height0||
    (mode!=REMESA_PRESENT_STRETCH&&mode!=REMESA_PRESENT_FIT))return EINVAL;
 if((uint64_t)width*4>SIZE_MAX)return EOVERFLOW;
 row=(size_t)width*4;
 if(stride<row)return EINVAL;
 if(height>1&&stride>(SIZE_MAX-row)/(height-1))return EOVERFLOW;
 required=(height-1)*stride+row;
 if(capacity<required)return ENOSPC;
 if((uint64_t)res->width0*4>SIZE_MAX)return EOVERFLOW;
 source_stride=(size_t)res->width0*4;
 if(res->height0>SIZE_MAX/source_stride)return EOVERFLOW;
 source_bytes=source_stride*res->height0;
 result.source_width=res->width0;result.source_height=res->height0;
 result.width=width;result.height=height;result.source_bytes=source_bytes;
 result.content_width=width;result.content_height=height;
 if(mode==REMESA_PRESENT_FIT){
  if((uint64_t)width*res->height0<=(uint64_t)height*res->width0){
   result.content_height=(uint64_t)width*res->height0/res->width0;
   if(!result.content_height)result.content_height=1;
  }else{
   result.content_width=(uint64_t)height*res->width0/res->height0;
   if(!result.content_width)result.content_width=1;
  }
  result.content_x=(width-result.content_width)/2;
  result.content_y=(height-result.content_height)/2;
 }
 source=malloc(source_bytes);if(!source)return ENOMEM;
 error=remesa_present_snapshot(ctx,res,source,source_stride,source_bytes);
 if(error){free(source);return error;}
 /* No fallible operation follows readback; preserve row padding. */
 for(uint32_t y=0;y<height;y++){
  unsigned char *out=(unsigned char *)destination+(size_t)y*stride;
  for(uint32_t x=0;x<width;x++){
   if(x<result.content_x||x-result.content_x>=result.content_width||
      y<result.content_y||y-result.content_y>=result.content_height){
    const unsigned char black[4]={0,0,0,255};memcpy(out+(size_t)x*4,black,4);
   }else{
    uint32_t sx=(uint64_t)(x-result.content_x)*res->width0/result.content_width;
    uint32_t sy=(uint64_t)(y-result.content_y)*res->height0/result.content_height;
    memcpy(out+(size_t)x*4,source+(size_t)sy*source_stride+(size_t)sx*4,4);
   }
  }
 }
 free(source);result.cpu_output_bytes=(size_t)height*row;
 *receipt=result;return 0;
}
