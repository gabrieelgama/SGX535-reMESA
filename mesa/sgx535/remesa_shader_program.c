/* SPDX-License-Identifier: MIT; original host compilation/lifetime contract. */
#include "remesa_shader_program.h"
#include "remesa_fragment_body.h"
#include "remesa/shader_request.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
static int overlaps(const void *a,size_t an,const void *b,size_t bn)
{
 if(!a||!b||!an||!bn)return 0;
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
 if(an>UINTPTR_MAX-x||bn>UINTPTR_MAX-y)return 1;
 return x<y+bn&&y<x+an;
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
int remesa_shader_program_prepare_color(uint32_t argb,struct remesa_shader_program *out)
{
 if(!out)return EINVAL;
 struct remesa_shader_ir ir={.schema=REMESA_SHADER_IR_SCHEMA,
  .stage=REMESA_IR_FRAGMENT,.count=2};
 ir.instructions[0]=(struct remesa_ir_instruction){.op=REMESA_IR_CONSTANT,
  .components=4,.type=REMESA_IR_FLOAT32};
 ir.instructions[1]=(struct remesa_ir_instruction){.op=REMESA_IR_COLOR,
  .dst=REMESA_SHADER_IR_NO_REGISTER,.components=4,.type=REMESA_IR_FLOAT32};
 const unsigned shifts[4]={16,8,0,24};
 for(unsigned c=0;c<4;c++) {
  uint32_t channel=(argb>>shifts[c])&255;
  if(channel!=0&&channel!=255)return ENOTSUP;
  ir.instructions[0].bits[c]=channel ? UINT32_C(0x3f800000) : 0;
 }
 return remesa_shader_program_prepare(&ir,out);
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
int remesa_shader_program_preview_pds(const struct remesa_shader_program *program,
 const struct remesa_pds_candidate_request *request,struct remesa_shader_pds_preview *out)
{
 if(!program||!request||!out||overlaps(out,sizeof(*out),program,sizeof(*program))||
    overlaps(out,sizeof(*out),request,sizeof(*request)))return EINVAL;
 int error=remesa_shader_program_validate(program);if(error)return error;
 struct remesa_shader_pds_preview preview={.stage=program->semantic.stage};
 error=remesa_usse_identity(&program->usse,&preview.semantic_identity);if(error)return error;
 error=remesa_pds_candidate_preview(request,&preview.candidate);if(error)return error;
 *out=preview;return 0;
}
int remesa_shader_pipeline_prepare(const struct remesa_shader_program *vertex,
 const struct remesa_shader_program *fragment,struct remesa_shader_pipeline *out)
{
 if(!vertex||!fragment||!out||overlaps(out,sizeof(*out),vertex,sizeof(*vertex))||
    overlaps(out,sizeof(*out),fragment,sizeof(*fragment)))return EINVAL;
 int error=remesa_shader_program_validate(vertex);if(error)return error;
 error=remesa_shader_program_validate(fragment);if(error)return error;
 if(vertex->semantic.stage!=REMESA_IR_VERTEX||fragment->semantic.stage!=REMESA_IR_FRAGMENT)
  return ENOTSUP;
 struct remesa_shader_pipeline pair={.schema=REMESA_SHADER_PIPELINE_SCHEMA,
  .status=REMESA_PROGRAM_SEMANTIC_ONLY,.binary_error=ENOTSUP};
 error=remesa_shader_ir_vertex_input(&vertex->semantic,&pair.position_input_slot);
 if(error)return error;
 error=remesa_shader_ir_fragment_color(&fragment->semantic,&pair.fragment_argb);
 if(error)return error;
 error=remesa_usse_identity(&vertex->usse,&pair.identity[0]);if(error)return error;
 error=remesa_usse_identity(&fragment->usse,&pair.identity[1]);if(error)return error;
 pair.virtual_instructions[0]=vertex->usse.count;
 pair.virtual_instructions[1]=fragment->usse.count;
 *out=pair;return 0;
}
int remesa_shader_pipeline_revalidate(const struct remesa_shader_pipeline *pair,
 const struct remesa_shader_program *vertex,const struct remesa_shader_program *fragment)
{
 if(!pair)return EINVAL;
 struct remesa_shader_pipeline current={0};
 int error=remesa_shader_pipeline_prepare(vertex,fragment,&current);if(error)return error;
 /* Compare semantic fields explicitly: compiler padding is not state. The
  * identities do not authorize execution, and every call validates the actual
  * immutable programs before consulting their correlation keys. */
 if(pair->schema!=current.schema||pair->status!=current.status||
    pair->identity[0]!=current.identity[0]||pair->identity[1]!=current.identity[1]||
    pair->virtual_instructions[0]!=current.virtual_instructions[0]||
    pair->virtual_instructions[1]!=current.virtual_instructions[1]||
    pair->position_input_slot!=current.position_input_slot||
    pair->fragment_argb!=current.fragment_argb||pair->binary_error!=current.binary_error)
  return ESTALE;
 return 0;
}
int remesa_shader_draw_programs_prepare(const struct remesa_shader_program *vertex,
 const struct remesa_shader_program *fragment,struct remesa_shader_draw_programs *out)
{
 if(!vertex||!fragment||!out||overlaps(out,sizeof(*out),vertex,sizeof(*vertex))||
    overlaps(out,sizeof(*out),fragment,sizeof(*fragment)))return EINVAL;
 struct remesa_shader_draw_programs programs={0};
 int error=remesa_shader_pipeline_prepare(vertex,fragment,&programs.pipeline);
 if(error)return error;
 programs.vertex=*vertex;programs.fragment=*fragment;
 *out=programs;return 0;
}
int remesa_shader_draw_programs_validate(const struct remesa_shader_draw_programs *programs)
{
 if(!programs)return EINVAL;
 return remesa_shader_pipeline_revalidate(&programs->pipeline,&programs->vertex,&programs->fragment);
}
static int same_program(const struct remesa_shader_program *a,
                        const struct remesa_shader_program *b)
{
 /* Padding is not shader state. Compare the complete canonical semantic
  * representation, including unused fields already checked by validation.
  * The lowering is determined exactly by these fields, never by its hash. */
 if(a->schema!=b->schema||a->status!=b->status||
    a->semantic.schema!=b->semantic.schema||a->semantic.stage!=b->semantic.stage||
    a->semantic.count!=b->semantic.count)return 0;
 for(unsigned n=0;n<REMESA_SHADER_IR_INSTRUCTIONS;n++) {
  const struct remesa_ir_instruction *x=&a->semantic.instructions[n];
  const struct remesa_ir_instruction *y=&b->semantic.instructions[n];
  if(x->op!=y->op||x->dst!=y->dst||x->components!=y->components||
     x->type!=y->type||x->input_slot!=y->input_slot)return 0;
  for(unsigned c=0;c<4;c++)
   if(x->src[c].reg!=y->src[c].reg||x->src[c].channel!=y->src[c].channel||
      x->bits[c]!=y->bits[c])return 0;
 }
 return 1;
}
int remesa_shader_draw_programs_revalidate(const struct remesa_shader_draw_programs *programs,
 const struct remesa_shader_program *vertex,const struct remesa_shader_program *fragment)
{
 int error=remesa_shader_draw_programs_validate(programs);if(error)return error;
 error=remesa_shader_program_validate(vertex);if(error)return error;
 error=remesa_shader_program_validate(fragment);if(error)return error;
 return same_program(&programs->vertex,vertex)&&same_program(&programs->fragment,fragment) ? 0 : ESTALE;
}
struct remesa_shader_image {
 struct remesa_shader_draw_programs programs;
 struct remesa_shader_storage *fragment,*vertex;
 struct remesa_usse_vertex_mapping vertex_mapping;
 enum remesa_shader_candidate_form fragment_form;
};
static int image_records(const struct remesa_usse_program *program,
 unsigned char records[REMESA_FRAGMENT_BODY_BYTES],size_t *length,
 enum remesa_shader_candidate_form *form)
{
 struct remesa_fragment_constant constant;
 int error=remesa_fragment_constant_prepare(program,&constant);
 if(!error) {
  memcpy(records,constant.record,sizeof(constant.record));
  *length=sizeof(constant.record);*form=REMESA_SHADER_PACKED_CONSTANT;return 0;
 }
 if(error!=ENOTSUP)return error;
 /* Preserve the separately validated larger candidate for endpoint profiles
  * outside the two-color-correlated packed form. Neither form is executable. */
 struct remesa_fragment_body body;
 error=remesa_fragment_body_prepare(program,&body);if(error)return error;
 memcpy(records,body.records,sizeof(body.records));
 *length=sizeof(body.records);*form=REMESA_SHADER_FLOAT_PACK_BODY;return 0;
}
static int image_prepare(const struct remesa_shader_draw_programs *programs,
 const struct remesa_usse_vertex_mapping *mapping,
 struct remesa_shader_image **out)
{
 unsigned char records[REMESA_FRAGMENT_BODY_BYTES];size_t length;
 unsigned char vertex_records[REMESA_USSE_VERTEX_CANDIDATE_BYTES];size_t vertex_length=0;
 enum remesa_shader_candidate_form form;
 struct remesa_shader_image *image;
 if(!programs||!out||overlaps(out,sizeof(*out),programs,sizeof(*programs)))return EINVAL;
 if(mapping&&overlaps(out,sizeof(*out),mapping,sizeof(*mapping)))return EINVAL;
 if(*out)return EBUSY;
 int error=remesa_shader_draw_programs_validate(programs);if(error)return error;
 if(mapping) {
  error=remesa_usse_vertex_candidate(&programs->vertex.usse,mapping,vertex_records,
   sizeof(vertex_records),&vertex_length);if(error)return error;
 }
 error=image_records(&programs->fragment.usse,records,&length,&form);if(error)return error;
 image=calloc(1,sizeof(*image));if(!image)return ENOMEM;
 image->programs=*programs;
 image->fragment_form=form;
 const struct remesa_shader_storage_request request={
  .stage=REMESA_IR_FRAGMENT,.format=REMESA_SHADER_STORAGE_HOST_CPU_RECORDS,
  .bytes=records,.byte_length=length,.byte_alignment=8
 };
 error=remesa_shader_storage_create(&request,&image->fragment);
 if(error){free(image);return error;}
 if(mapping) {
  const struct remesa_shader_storage_request vertex_request={
   .stage=REMESA_IR_VERTEX,.format=REMESA_SHADER_STORAGE_HOST_CPU_RECORDS,
   .bytes=vertex_records,.byte_length=vertex_length,.byte_alignment=8
  };
  image->vertex_mapping=*mapping;
  error=remesa_shader_storage_create(&vertex_request,&image->vertex);
  if(error){remesa_shader_image_destroy(image);return error;}
 }
 *out=image;return 0;
}
int remesa_shader_image_prepare(const struct remesa_shader_draw_programs *programs,
 struct remesa_shader_image **out)
{return image_prepare(programs,NULL,out);}
int remesa_shader_image_prepare_vertex_candidate(const struct remesa_shader_draw_programs *programs,
 const struct remesa_usse_vertex_mapping *mapping,struct remesa_shader_image **out)
{return mapping?image_prepare(programs,mapping,out):EINVAL;}
int remesa_shader_image_validate(const struct remesa_shader_image *image)
{
 unsigned char expected[REMESA_FRAGMENT_BODY_BYTES];size_t length;
 enum remesa_shader_candidate_form form;
 struct remesa_shader_storage_view view;
 if(!image)return EINVAL;
 int error=remesa_shader_draw_programs_validate(&image->programs);if(error)return error;
 error=image_records(&image->programs.fragment.usse,expected,&length,&form);if(error)return error;
 error=remesa_shader_storage_inspect(image->fragment,&view);if(error)return error;
 if(view.stage!=REMESA_IR_FRAGMENT||view.format!=REMESA_SHADER_STORAGE_HOST_CPU_RECORDS||
    view.status!=REMESA_SHADER_STORAGE_CANDIDATE||view.byte_length!=length||
    image->fragment_form!=form||view.byte_alignment!=8||memcmp(view.bytes,expected,length))return EPROTO;
 if(image->vertex) {
  unsigned char vertex[REMESA_USSE_VERTEX_CANDIDATE_BYTES];size_t vertex_length;
  error=remesa_usse_vertex_candidate(&image->programs.vertex.usse,&image->vertex_mapping,
   vertex,sizeof(vertex),&vertex_length);if(error)return error;
  error=remesa_shader_storage_inspect(image->vertex,&view);if(error)return error;
  if(view.stage!=REMESA_IR_VERTEX||view.format!=REMESA_SHADER_STORAGE_HOST_CPU_RECORDS||
     view.status!=REMESA_SHADER_STORAGE_CANDIDATE||view.byte_alignment!=8||
     view.byte_length!=vertex_length||memcmp(view.bytes,vertex,vertex_length))return EPROTO;
 }
 return 0;
}
int remesa_shader_image_inspect(const struct remesa_shader_image *image,
 struct remesa_shader_image_view *out)
{
 if(!out||!image||overlaps(out,sizeof(*out),image,sizeof(*image)))return EINVAL;
 int error=remesa_shader_image_validate(image);if(error)return error;
 struct remesa_shader_image_view view={.programs=&image->programs,
  .vertex_byte_length=0,.execution_error=ENOTSUP,.fragment_form=image->fragment_form};
 error=remesa_shader_storage_inspect(image->fragment,&view.fragment);if(error)return error;
 if(image->vertex) {
  error=remesa_shader_storage_inspect(image->vertex,&view.vertex);if(error)return error;
  view.vertex_byte_length=view.vertex.byte_length;
 }
 if(overlaps(out,sizeof(*out),view.programs,sizeof(*view.programs))||
    overlaps(out,sizeof(*out),view.fragment.bytes,view.fragment.byte_length)||
    overlaps(out,sizeof(*out),view.vertex.bytes,view.vertex.byte_length))return EINVAL;
 *out=view;return 0;
}
int remesa_shader_image_preview_use_entry(const struct remesa_shader_image *image,
 const struct remesa_use_entry_request *request,struct remesa_use_entry_plan *out)
{
 struct remesa_shader_image_view view;
 if(!image||!request||!out)return EINVAL;
 int error=remesa_shader_image_inspect(image,&view);if(error)return error;
 if(overlaps(out,sizeof(*out),image,sizeof(*image))||
    overlaps(out,sizeof(*out),view.programs,sizeof(*view.programs))||
    overlaps(out,sizeof(*out),view.fragment.bytes,view.fragment.byte_length)||
    overlaps(out,sizeof(*out),view.vertex.bytes,view.vertex.byte_length))return EINVAL;
 size_t bytes;
 if(request->data_master==REMESA_USE_PIXEL)bytes=view.fragment.byte_length;
 else if(request->data_master==REMESA_USE_VERTEX) {
  if(!view.vertex.byte_length)return ENOTSUP;
  bytes=view.vertex.byte_length;
 } else return ENOTSUP;
 if(request->program_bytes!=bytes)return EPROTO;
 struct remesa_use_entry_plan entry;
 uint32_t address_bits;
 error=remesa_use_entry_prepare(request,&entry);if(error)return error;
 /* Do not publish a native entry that the observed address representation
  * would truncate. This checks lossless software packing, not hardware
  * instruction alignment or a complete task-control word. */
 error=remesa_use_entry_preview_fixup(&entry,&address_bits);if(error)return error;
 *out=entry;return 0;
}
int remesa_shader_image_revalidate(const struct remesa_shader_image *image,
 const struct remesa_shader_draw_programs *programs)
{
 int error=remesa_shader_image_validate(image);if(error)return error;
 error=remesa_shader_draw_programs_validate(programs);if(error)return error;
 return remesa_shader_draw_programs_revalidate(&image->programs,&programs->vertex,&programs->fragment);
}
int remesa_shader_image_validate_request(const struct remesa_shader_image *image,
 const struct remesa_shader_request *request)
{
 struct remesa_shader_image_view view;
 if(!request)return EINVAL;
 int error=remesa_shader_image_inspect(image,&view);if(error)return error;
 if(remesa_shader_request_validate(request,request->owner,request->generation)!=
    REMESA_SHADER_REQUEST_OK)return EPROTO;
 unsigned mask=1u<<(REMESA_SHADER_FRAGMENT_USSE-1);
 if(view.vertex.byte_length)mask|=1u<<(REMESA_SHADER_VERTEX_USSE-1);
 if(request->resource_mask!=mask || request->byte_count!=
    view.vertex.byte_length+view.fragment.byte_length)return EPROTO;
 const struct remesa_shader_storage_view *stages[]={&view.vertex,&view.fragment};
 size_t offset=0;
 for(unsigned n=0;n<2;n++) {
  const struct remesa_shader_storage_view *stage=stages[n];
  if(!stage->byte_length)continue;
  const struct remesa_shader_resource *resource=&request->resources[n];
  if(resource->offset!=offset||resource->length!=stage->byte_length||
     resource->alignment!=stage->byte_alignment||resource->state!=REMESA_SHADER_CPU_CANDIDATE||
     memcmp(request->data+offset,stage->bytes,stage->byte_length))return EPROTO;
  offset+=stage->byte_length;
 }
 return 0;
}
int remesa_shader_image_validate_execution(const struct remesa_shader_image *image)
{
 int error=remesa_shader_image_validate(image);return error?error:ENOTSUP;
}
void remesa_shader_image_destroy(struct remesa_shader_image *image)
{
 if(image){(void)remesa_shader_storage_reference(&image->vertex,NULL);
  (void)remesa_shader_storage_reference(&image->fragment,NULL);free(image);}
}
/* Called only after each request's overflow/bounds checks have succeeded. */
static int stage_storage_disjoint(const struct remesa_pds_request *vertex,
                                 const struct remesa_pds_request *fragment)
{
 if(vertex->owner_cookie!=fragment->owner_cookie||vertex->operation!=fragment->operation||
    vertex->generation!=fragment->generation)return ESTALE;
 const struct remesa_pds_binding *a[2]={&vertex->code,&vertex->data};
 const struct remesa_pds_binding *b[2]={&fragment->code,&fragment->data};
 for(unsigned i=0;i<2;i++)for(unsigned j=0;j<2;j++) {
  if(a[i]->allocation_cookie==b[j]->allocation_cookie) {
   if(a[i]->address!=b[j]->address||a[i]->allocation_bytes!=b[j]->allocation_bytes||
      a[i]->mapping_generation!=b[j]->mapping_generation)return ESTALE;
  } else if(a[i]->address<b[j]->address+b[j]->allocation_bytes&&
            b[j]->address<a[i]->address+a[i]->allocation_bytes)return EINVAL;
  uint64_t x=a[i]->address+a[i]->offset,y=b[j]->address+b[j]->offset;
  if(x<y+b[j]->bytes&&y<x+a[i]->bytes)return EINVAL;
 }
 return 0;
}
int remesa_shader_pipeline_prepare_launch(const struct remesa_shader_program *vertex,
 const struct remesa_shader_program *fragment,const struct remesa_pds_request *vertex_request,
 const struct remesa_pds_request *fragment_request,struct remesa_shader_pipeline_launch *out)
{
 if(!out||!vertex||!fragment||!vertex_request||!fragment_request||
    overlaps(out,sizeof(*out),vertex,sizeof(*vertex))||
    overlaps(out,sizeof(*out),fragment,sizeof(*fragment))||
    overlaps(out,sizeof(*out),vertex_request,sizeof(*vertex_request))||
    overlaps(out,sizeof(*out),fragment_request,sizeof(*fragment_request)))return EINVAL;
 struct remesa_shader_pipeline_launch launch={0};
 int error=remesa_shader_pipeline_prepare(vertex,fragment,&launch.semantic);if(error)return error;
 error=remesa_shader_program_prepare_launch(vertex,vertex_request,&launch.stage[0]);if(error)return error;
 error=remesa_shader_program_prepare_launch(fragment,fragment_request,&launch.stage[1]);if(error)return error;
 error=stage_storage_disjoint(vertex_request,fragment_request);if(error)return error;
 *out=launch;return 0;
}
int remesa_shader_pipeline_revalidate_launch(const struct remesa_shader_pipeline_launch *launch,
 const struct remesa_shader_program *vertex,const struct remesa_shader_program *fragment,
 const struct remesa_pds_request *vertex_current,const struct remesa_pds_request *fragment_current)
{
 if(!launch)return EINVAL;
 int error=remesa_shader_pipeline_revalidate(&launch->semantic,vertex,fragment);if(error)return error;
 error=remesa_pds_validate_binding(&vertex->usse,&launch->stage[0],vertex_current);if(error)return error;
 error=remesa_pds_validate_binding(&fragment->usse,&launch->stage[1],fragment_current);if(error)return error;
 return stage_storage_disjoint(vertex_current,fragment_current);
}
