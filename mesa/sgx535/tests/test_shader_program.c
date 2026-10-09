/* SPDX-License-Identifier: MIT
 * Actual Mesa shader callbacks and independently authored host lowering.
 * No shader bytes, rendering, completion fixtures or hardware submissions.
 */
#include "remesa_shader_program.h"
#include "vertex_nir_fixture.h"
#define main inherited_nir_main
#include "test_nir.c"
#undef main
#define CHECK(condition) do { assert(condition); ++cases; } while(0)
int main(void)
{
 unsigned cases=0;
 struct remesa_shader_ir ir={0};
 nir_shader *nir=fixture(0xffff00ff);
 CHECK(!remesa_shader_ir_lower(nir,&ir));ralloc_free(nir);
 struct remesa_shader_program program={0},copy={0};
 CHECK(!remesa_shader_program_prepare(&ir,&program));copy=program;
 CHECK(!remesa_shader_program_validate(&program));
 CHECK(!remesa_shader_program_prepare(&ir,&copy)&&!memcmp(&copy,&program,sizeof(copy)));
 memset(&ir,0,sizeof(ir));CHECK(!remesa_shader_program_validate(&program));
 struct remesa_shader_program_report report={0};
 CHECK(!remesa_shader_program_report(&program,&report));
 CHECK(report.stage==REMESA_IR_FRAGMENT&&report.status==REMESA_PROGRAM_SEMANTIC_ONLY&&
       report.virtual_instructions==8&&report.binary_error==ENOTSUP);
 CHECK(report.register_evidence.use_banks==16&&
       report.register_evidence.use_address_bits==25&&
       report.register_evidence.declared_pds_alignment_shift==20&&
       !report.register_evidence.use_va_packing_verified&&
       !report.register_evidence.rev121_verified&&
       !report.register_evidence.instruction_fields_known);
 unsigned char binary[128],before[128];memset(binary,0xa5,sizeof(binary));memcpy(before,binary,sizeof(before));
 size_t written=987;
 CHECK(remesa_shader_program_encode(&program,binary,sizeof(binary),&written)==ENOTSUP);
 CHECK(written==987&&!memcmp(binary,before,sizeof(binary)));
 CHECK(remesa_shader_program_encode(&program,NULL,1,&written)==EINVAL);
 CHECK(remesa_shader_program_encode(&program,NULL,0,&written)==ENOTSUP&&written==987);
 /* A different independently valid lowering must not become this program. */
 nir=fixture(0xff00ff00);CHECK(!remesa_shader_ir_lower(nir,&ir));ralloc_free(nir);
 CHECK(!remesa_usse_lower(&ir,&copy.usse));
 CHECK(remesa_shader_program_validate(&copy)==EPROTO);
 struct remesa_shader_program original=copy;
 CHECK(remesa_shader_program_report(&copy,&report)==EPROTO&&!memcmp(&copy,&original,sizeof(copy)));
 CHECK(remesa_shader_program_encode(&copy,binary,sizeof(binary),&written)==EPROTO&&written==987);
 CHECK(!memcmp(binary,before,sizeof(binary)));
 copy=program;copy.status=0;CHECK(remesa_shader_program_validate(&copy)==ENOTSUP);
 copy=program;copy.schema=2;CHECK(remesa_shader_program_validate(&copy)==ENOTSUP);
 /* Output aliases must not mutate an immutable compilation snapshot. */
 union program_storage {struct remesa_shader_program program;struct remesa_shader_ir ir;
                        struct remesa_shader_program_report report;struct remesa_pds_plan plan;} box,snapshot;
 memset(&box,0,sizeof(box));box.program=program;snapshot=box;
 CHECK(remesa_shader_program_prepare(&box.program.semantic,&box.program)==EINVAL);
 CHECK(!memcmp(&box,&snapshot,sizeof(box)));
 CHECK(remesa_shader_program_report(&box.program,&box.report)==EINVAL);
 CHECK(!memcmp(&box,&snapshot,sizeof(box)));
 /* Symbolic storage metadata only, never authority or a GPU launch image. */
 struct remesa_pds_request request={.operation=2,.generation=3,.owner_cookie=4,
  .code={.allocation_cookie=5,.owner_cookie=4,.mapping_generation=6,
         .address=0x1000,.offset=16,.bytes=32,.allocation_bytes=64},
  .data={.allocation_cookie=7,.owner_cookie=4,.mapping_generation=8,
         .address=0x2000,.offset=32,.bytes=32,.allocation_bytes=128}};
 struct remesa_pds_plan plan={0};
 CHECK(remesa_shader_program_prepare_launch(&box.program,&request,&box.plan)==EINVAL);
 CHECK(!memcmp(&box,&snapshot,sizeof(box)));
 CHECK(!remesa_shader_program_prepare_launch(&program,&request,&plan));
 CHECK(remesa_shader_program_encode_launch(&program,&plan,&request,binary,sizeof(binary),&written)==ENOTSUP);
 CHECK(written==987&&!memcmp(binary,before,sizeof(binary)));
 struct remesa_pds_request current=request;current.generation++;
 CHECK(remesa_shader_program_encode_launch(&program,&plan,&current,binary,sizeof(binary),&written)==ESTALE);
 CHECK(written==987&&!memcmp(binary,before,sizeof(binary)));
 current=request;current.code.mapping_generation++;
 CHECK(remesa_shader_program_encode_launch(&program,&plan,&current,binary,sizeof(binary),&written)==ESTALE);
 CHECK(remesa_shader_program_encode_launch(&program,&plan,NULL,binary,sizeof(binary),&written)==EINVAL);
 remesa_shader_program_finish(&program);remesa_shader_program_finish(&program);
 CHECK(remesa_shader_program_validate(&program)==ENOTSUP);

 static const struct remesa_device_ops device_ops={alloc,release,map,unmap,submit,wait,release};
 struct remesa_device dev={.ops=&device_ops,.offline_test_backend=true};
 struct pipe_screen *screen=remesa_screen_create(&dev);CHECK(screen);
 struct pipe_context *ctx=screen->context_create(screen,NULL,0),*other=screen->context_create(screen,NULL,0);
 CHECK(ctx&&other);
 struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR};
 state.ir.nir=vertex_fixture();unsigned freed=destroyed;
 void *vs=ctx->create_vs_state(ctx,&state);CHECK(vs);
 state.ir.nir=fixture(0xffff00ff);void *fs=ctx->create_fs_state(ctx,&state);
 CHECK(fs&&destroyed==freed+1);
 CHECK(!remesa_context_shader_program_report(ctx,PIPE_SHADER_VERTEX,vs,&report));
 CHECK(report.stage==REMESA_IR_VERTEX&&report.binary_error==ENOTSUP);
 CHECK(!remesa_context_shader_program_report(ctx,PIPE_SHADER_FRAGMENT,fs,&report));
 CHECK(report.stage==REMESA_IR_FRAGMENT&&report.binary_error==ENOTSUP);
 CHECK(report.register_evidence.use_banks==16&&
       !report.register_evidence.use_va_packing_verified&&
       !report.register_evidence.rev121_verified&&
       !report.register_evidence.instruction_fields_known);
 ctx->bind_vs_state(ctx,vs);ctx->bind_fs_state(ctx,fs);CHECK(!remesa_context_error(ctx));
 CHECK(remesa_context_shader_program_encode(ctx,PIPE_SHADER_VERTEX,vs,binary,sizeof(binary),&written)==ENOTSUP);
 CHECK(remesa_context_shader_program_encode(ctx,PIPE_SHADER_FRAGMENT,fs,binary,sizeof(binary),&written)==ENOTSUP);
 CHECK(written==987&&!memcmp(binary,before,sizeof(binary)));
 struct remesa_shader_program_report sentinel;memset(&sentinel,0xa5,sizeof(sentinel));report=sentinel;
 CHECK(remesa_context_shader_program_report(other,PIPE_SHADER_VERTEX,vs,&report)==EINVAL);
 CHECK(!memcmp(&report,&sentinel,sizeof(report)));
 CHECK(remesa_context_shader_program_report(ctx,PIPE_SHADER_VERTEX,fs,&report)==EINVAL);
 CHECK(remesa_context_shader_program_report(ctx,PIPE_SHADER_COMPUTE,vs,&report)==ENOTSUP);
 CHECK(remesa_context_shader_program_report(ctx,PIPE_SHADER_VERTEX,NULL,&report)==EINVAL);
 CHECK(remesa_context_shader_program_report(NULL,PIPE_SHADER_VERTEX,vs,&report)==EINVAL);
 CHECK(remesa_context_shader_program_encode(ctx,PIPE_SHADER_VERTEX,vs,NULL,1,&written)==EINVAL);
 ctx->delete_vs_state(ctx,vs);ctx->delete_fs_state(ctx,fs);
 CHECK(remesa_context_shader_program_report(ctx,PIPE_SHADER_VERTEX,vs,&report)==EINVAL);
 CHECK(remesa_context_shader_program_encode(ctx,PIPE_SHADER_FRAGMENT,fs,binary,sizeof(binary),&written)==EINVAL);
 CHECK(written==987&&!memcmp(binary,before,sizeof(binary)));
 state.ir.nir=fixture(0xffff00ff);freed=destroyed;fail_next_calloc=true;
 CHECK(!ctx->create_fs_state(ctx,&state)&&remesa_context_error(ctx)==ENOMEM&&destroyed==freed+1);
 state.ir.nir=fixture(0x00ff00ff);freed=destroyed;
 CHECK(!ctx->create_fs_state(ctx,&state)&&destroyed==freed+1);
 /* Leave valid unbound plans for ordinary context teardown to own. */
 state.ir.nir=vertex_fixture();CHECK(ctx->create_vs_state(ctx,&state));
 state.ir.nir=fixture(0xff00ff00);CHECK(ctx->create_fs_state(ctx,&state));
 ctx->destroy(ctx);other->destroy(other);screen->destroy(screen);
 printf("PASS %u semantic program/actual shader callback cases; no GPU bytes or submissions\n",cases);
 return 0;
}
