/* SPDX-License-Identifier: MIT */
/* Simulated verifier flags only. No GPU command, rasterization or ioctl. */
#include "remesa_render_pass.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error Render-pass tests require enabled assertions
#endif

static const struct remesa_pass_completion verified = {true, true, true};
static unsigned cases;
static void rejection(void)
{
 struct remesa_render_pass p, old;
 struct remesa_pass_ticket out = {123,456};
 assert(!remesa_render_pass_init(&p, 1));
 assert(!remesa_render_pass_cpu_write(&p)); /* partial CPU update is content */
 old=p;
 assert(remesa_render_pass_begin(&p,REMESA_PASS_PRESERVE,REMESA_PASS_STORE,false,&out)==ENOTSUP);
 assert(!memcmp(&old,&p,sizeof(p)) && out.resource_cookie==123 && out.serial==456);
 assert(remesa_render_pass_begin(&p,(enum remesa_pass_load)-1,REMESA_PASS_STORE,true,&out)==EINVAL);
 assert(remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,(enum remesa_pass_store)-1,true,&out)==EINVAL);
 assert(remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,REMESA_PASS_STORE,true,NULL)==EINVAL);
 assert(!memcmp(&old,&p,sizeof(p)));
 p.serial=UINT64_MAX; old=p;
 assert(remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,REMESA_PASS_STORE,true,&out)==EOVERFLOW);
 assert(!memcmp(&old,&p,sizeof(p)));
 assert(remesa_render_pass_init(&p,0)==EINVAL);
 p.phase=(enum remesa_pass_phase)-1;
 assert(remesa_render_pass_cpu_write(&p)==EINVAL);
 ++cases;
}
static void cancellation(void)
{
 struct remesa_render_pass p;
 struct remesa_pass_ticket first, second;
 assert(!remesa_render_pass_init(&p,2));
 assert(!remesa_render_pass_cpu_write(&p));
 assert(!remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,REMESA_PASS_STORE,false,&first));
 assert(remesa_render_pass_cpu_write(&p)==EBUSY);
 assert(!remesa_render_pass_cancel(&p,&first));
 assert(p.content==REMESA_PASS_PRESENT);
 assert(!remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,REMESA_PASS_STORE,false,&second));
 assert(second.serial==first.serial+1);
 assert(remesa_render_pass_submitted(&p,&first)==ESTALE);
 assert(!remesa_render_pass_submitted(&p,&second));
 assert(remesa_render_pass_cancel(&p,&second)==EBUSY);
 assert(!remesa_render_pass_finish(&p,&second,&verified));
 ++cases;
}
static void proof_and_identity(void)
{
 struct remesa_render_pass p, other, old;
 struct remesa_pass_ticket t, wrong;
 assert(!remesa_render_pass_init(&p,3));
 assert(!remesa_render_pass_init(&other,4));
 assert(!remesa_render_pass_import_writer(&p,&t));
 assert(!remesa_render_pass_import_writer(&other,&wrong));
 old=p;
 assert(remesa_render_pass_finish(&p,&wrong,&verified)==ESTALE);
 assert(remesa_render_pass_finish(&p,&t,NULL)==EINVAL);
 assert(!memcmp(&old,&p,sizeof(p)));
 for(unsigned mask=0;mask<7;mask++) {
  struct remesa_pass_completion partial={!!(mask&1),!!(mask&2),!!(mask&4)};
  assert(remesa_render_pass_finish(&p,&t,&partial)==EAGAIN);
  assert(!memcmp(&old,&p,sizeof(p)));
 }
 assert(remesa_render_pass_cpu_write(&p)==EBUSY);
 assert(remesa_render_pass_admit(&p,REMESA_PASS_DISCARD,true)==EBUSY);
 assert(!remesa_render_pass_finish(&p,&t,&verified));
 assert(p.content==REMESA_PASS_PRESENT && p.phase==REMESA_PASS_READY);
 assert(remesa_render_pass_finish(&p,&t,&verified)==EALREADY);
 assert(!remesa_render_pass_finish(&other,&wrong,&verified));
 ++cases;
}
static void quarantine(void)
{
 struct remesa_render_pass p, old;
 struct remesa_pass_ticket t, out;
 assert(!remesa_render_pass_init(&p,5));
 assert(!remesa_render_pass_import_writer(&p,&t));
 assert(remesa_render_pass_fail(&p,&t,0)==EINVAL);
 assert(!remesa_render_pass_fail(&p,&t,EIO));
 old=p;
 assert(remesa_render_pass_fail(&p,&t,EIO)==EALREADY);
 assert(remesa_render_pass_cpu_write(&p)==EIO);
 assert(remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,REMESA_PASS_STORE,true,&out)==EIO);
 assert(remesa_render_pass_finish(&p,&t,&verified)==EBUSY);
 assert(!memcmp(&old,&p,sizeof(p)));
 ++cases;
}
static void sequence(unsigned count)
{
 struct remesa_render_pass p;
 struct remesa_pass_ticket t, prior={0,0};
 assert(!remesa_render_pass_init(&p,6+count));
 for(unsigned i=0;i<count;i++) {
  assert(!remesa_render_pass_begin(&p,i?REMESA_PASS_PRESERVE:REMESA_PASS_DISCARD,
    REMESA_PASS_STORE,true,&t)); /* preservation support is mocked */
  assert(t.serial==i+1);
  assert(!remesa_render_pass_submitted(&p,&t));
  if(i) assert(remesa_render_pass_finish(&p,&prior,&verified)==ESTALE);
  struct remesa_pass_completion delayed={true,false,true};
  assert(remesa_render_pass_finish(&p,&t,&delayed)==EAGAIN);
  assert(!remesa_render_pass_finish(&p,&t,&verified));
  prior=t;
 }
 assert(p.phase==REMESA_PASS_READY && p.serial==count);
 /* A store-discard operation intentionally makes content undefined only after
  * complete authenticated retirement; it never implies a hardware command. */
 assert(!remesa_render_pass_begin(&p,REMESA_PASS_DISCARD,REMESA_PASS_DONT_STORE,true,&t));
 assert(!remesa_render_pass_submitted(&p,&t));
 assert(p.content==REMESA_PASS_UNCERTAIN);
 assert(!remesa_render_pass_finish(&p,&t,&verified));
 assert(p.content==REMESA_PASS_UNDEFINED);
 ++cases;
}
int main(void)
{
 rejection(); cancellation(); proof_and_identity(); quarantine();
 sequence(2); sequence(10); sequence(100);
 printf("PASS %u render-pass cases; simulated completion, no SGX\n",cases);
 return 0;
}
