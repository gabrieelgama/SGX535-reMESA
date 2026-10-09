/* SPDX-License-Identifier: MIT; original host compilation/lifetime contract. */
#include "remesa_shader_program.h"
#include <errno.h>
#include <string.h>
static int overlaps(const void *a,size_t an,const void *b,size_t bn)
{
 if(!a||!b||!an||!bn)return 0;
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
 return x<=y ? y-x<an : x-y<bn;
}

int remesa_shader_program_prepare(const struct remesa_shader_ir *ir,
                                 struct remesa_shader_program *out)
{
 struct remesa_shader_program program={.schema=REMESA_SHADER_PROGRAM_SCHEMA,
                                      .status=REMESA_PROGRAM_SEMANTIC_ONLY};
 if(!ir||!out||overlaps(ir,sizeof(*ir),out,sizeof(*out)))return EINVAL;
 int error=remesa_usse_lower(ir,&program.usse);
 if(error)return error;
 program.semantic=*ir;
 *out=program;
 return 0;
}
int remesa_shader_program_validate(const struct remesa_shader_program *program)
{
 struct remesa_usse_program expected={0};
 if(!program)return EINVAL;
 if(program->schema!=REMESA_SHADER_PROGRAM_SCHEMA||
    program->status!=REMESA_PROGRAM_SEMANTIC_ONLY)return ENOTSUP;
 int error=remesa_usse_lower(&program->semantic,&expected);
 if(error)return error;
 error=remesa_usse_validate(&program->usse);
 if(error)return error;
 /* A separately valid lowering for another color/stage must not alias this
  * shader. Both structures are canonical host uint32 records, not binaries. */
 return memcmp(&expected,&program->usse,sizeof(expected)) ? EPROTO : 0;
}
int remesa_shader_program_report(const struct remesa_shader_program *program,
                                struct remesa_shader_program_report *out)
{
 if(!out||!program||overlaps(out,sizeof(*out),program,sizeof(*program)))return EINVAL;
 int error=remesa_shader_program_validate(program);
 if(error)return error;
 struct remesa_shader_program_report report={
  .stage=program->semantic.stage,.status=program->status,
  .virtual_instructions=program->usse.count,.binary_error=ENOTSUP
 };
 remesa_program_register_evidence(&report.register_evidence);
 *out=report;
 return 0;
}
int remesa_shader_program_encode(const struct remesa_shader_program *program,
                                void *out,size_t capacity,size_t *written)
{
 if(!written||(!out&&capacity))return EINVAL;
 int error=remesa_shader_program_validate(program);
 if(error)return error;
 return remesa_usse_encode(&program->usse,out,capacity,written);
}
void remesa_shader_program_finish(struct remesa_shader_program *program)
{if(program)memset(program,0,sizeof(*program));}
int remesa_shader_program_prepare_launch(const struct remesa_shader_program *program,
 const struct remesa_pds_request *request,struct remesa_pds_plan *out)
{
 if(!out||!program||overlaps(program,sizeof(*program),out,sizeof(*out)))return EINVAL;
 int error=remesa_shader_program_validate(program);
 if(error)return error;
 return remesa_pds_prepare(&program->usse,request,out);
}
int remesa_shader_program_encode_launch(const struct remesa_shader_program *program,
 const struct remesa_pds_plan *plan,const struct remesa_pds_request *current,
 void *out,size_t capacity,size_t *written)
{
 if(!written||(!out&&capacity))return EINVAL;
 int error=remesa_shader_program_validate(program);
 if(error)return error;
 error=remesa_pds_validate_binding(&program->usse,plan,current);
 if(error)return error;
 return remesa_pds_encode(&program->usse,plan,out,capacity,written);
}
