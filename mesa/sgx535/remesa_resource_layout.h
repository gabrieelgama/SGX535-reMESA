/* SPDX-License-Identifier: MIT */
#ifndef REMESA_RESOURCE_LAYOUT_H
#define REMESA_RESOURCE_LAYOUT_H
#include "remesa_device.h"
#include <errno.h>
#include <limits.h>
struct remesa_resource_layout {
 uint32_t stride,pages;
 size_t bytes;
};
/* Checked host-storage layout only. Neither successful layout nor allocation
 * establishes SGX visibility, hardware dimensions, or sampling support. */
static inline int remesa_resource_layout(const struct remesa_resource_limits *limits,
 uint32_t width,uint32_t height,uint32_t cpp,struct remesa_resource_layout *out)
{
 if(!limits||!out)return EINVAL;
 uint32_t maxw=limits->max_width?limits->max_width:4096;
 uint32_t maxh=limits->max_height?limits->max_height:4096;
 uint32_t align=limits->row_alignment?limits->row_alignment:1;
 uint64_t budget=limits->max_resource_bytes?limits->max_resource_bytes:UINT64_C(64)*1024*1024;
 uint64_t row,total;
 if(!width||!height||!cpp||width>maxw||height>maxh||
    (align&(align-1)))return EINVAL;
 row=(uint64_t)width*cpp;
 if(row>UINT32_MAX||row+align-1>UINT32_MAX)return EOVERFLOW;
 row=(row+align-1)&~((uint64_t)align-1);
 total=row*height;
 /* Conservative <=4GiB storage admission keeps layouts portable to the
  * i386 target; pipe_transfer.layer_stride is uintptr_t in pinned Mesa. */
 if(total>SIZE_MAX||total>UINT32_MAX)return EOVERFLOW;
 if(total>budget)return ENOMEM;
 *out=(struct remesa_resource_layout){.stride=(uint32_t)row,
  .bytes=(size_t)total,.pages=(uint32_t)((total+4095)/4096)};
 return 0;
}
#endif
