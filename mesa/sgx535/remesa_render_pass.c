/* SPDX-License-Identifier: MIT */
#include "remesa_render_pass.h"
#include <errno.h>
#include <string.h>

static int valid(const struct remesa_render_pass *p)
{
 return p && p->resource_cookie && p->phase >= REMESA_PASS_READY &&
  p->phase <= REMESA_PASS_QUARANTINED && p->content >= REMESA_PASS_UNDEFINED &&
  p->content <= REMESA_PASS_UNCERTAIN && p->store >= REMESA_PASS_STORE &&
  p->store <= REMESA_PASS_DONT_STORE;
}
static int match(const struct remesa_render_pass *p,
 const struct remesa_pass_ticket *t)
{
 if (!valid(p) || !t || !t->resource_cookie || !t->serial) return EINVAL;
 return t->resource_cookie == p->resource_cookie && t->serial == p->serial ? 0 : ESTALE;
}
int remesa_render_pass_init(struct remesa_render_pass *p, uint64_t cookie)
{
 if (!p || !cookie) return EINVAL;
 memset(p, 0, sizeof(*p)); p->resource_cookie = cookie; return 0;
}
int remesa_render_pass_cpu_write(struct remesa_render_pass *p)
{
 if (!valid(p)) return EINVAL;
 if (p->phase == REMESA_PASS_QUARANTINED) return p->error ? p->error : EIO;
 if (p->phase != REMESA_PASS_READY) return EBUSY;
 p->content = REMESA_PASS_PRESENT; return 0;
}
int remesa_render_pass_admit(const struct remesa_render_pass *p,
 enum remesa_pass_load load, bool preserve_supported)
{
 if (!valid(p) || (load != REMESA_PASS_DISCARD && load != REMESA_PASS_PRESERVE)) return EINVAL;
 if (p->phase == REMESA_PASS_QUARANTINED) return p->error ? p->error : EIO;
 if (p->phase != REMESA_PASS_READY) return EBUSY;
 if (p->content == REMESA_PASS_UNCERTAIN) return EIO;
 if (load == REMESA_PASS_PRESERVE && !preserve_supported) return ENOTSUP;
 return 0;
}
int remesa_render_pass_begin(struct remesa_render_pass *p,
 enum remesa_pass_load load, enum remesa_pass_store store, bool preserve_supported,
 struct remesa_pass_ticket *out)
{
 int e = remesa_render_pass_admit(p, load, preserve_supported);
 if (e) return e;
 if (!out || (store != REMESA_PASS_STORE && store != REMESA_PASS_DONT_STORE)) return EINVAL;
 if (p->serial == UINT64_MAX) return EOVERFLOW;
 ++p->serial; p->store = store; p->phase = REMESA_PASS_RESERVED;
 *out = (struct remesa_pass_ticket){p->resource_cookie, p->serial}; return 0;
}
int remesa_render_pass_cancel(struct remesa_render_pass *p,
 const struct remesa_pass_ticket *t)
{
 int e = match(p, t); if (e) return e;
 if (p->phase != REMESA_PASS_RESERVED) return EBUSY;
 p->phase = REMESA_PASS_READY; return 0;
}
int remesa_render_pass_submitted(struct remesa_render_pass *p,
 const struct remesa_pass_ticket *t)
{
 int e = match(p, t); if (e) return e;
 if (p->phase != REMESA_PASS_RESERVED) return EBUSY;
 p->phase = REMESA_PASS_PENDING; p->content = REMESA_PASS_UNCERTAIN; return 0;
}
int remesa_render_pass_import_writer(struct remesa_render_pass *p,
 struct remesa_pass_ticket *out)
{
 int e = remesa_render_pass_begin(p, REMESA_PASS_DISCARD, REMESA_PASS_STORE, false, out);
 return e ? e : remesa_render_pass_submitted(p, out);
}
int remesa_render_pass_finish(struct remesa_render_pass *p,
 const struct remesa_pass_ticket *t, const struct remesa_pass_completion *proof)
{
 int e = match(p, t); if (e) return e;
 if (!proof) return EINVAL;
 if (p->phase == REMESA_PASS_READY) return EALREADY;
 if (p->phase != REMESA_PASS_PENDING) return EBUSY;
 if (!proof->authenticated || !proof->cpu_visible || !proof->retired) return EAGAIN;
 p->phase = REMESA_PASS_READY;
 p->content = p->store == REMESA_PASS_STORE ? REMESA_PASS_PRESENT : REMESA_PASS_UNDEFINED;
 return 0;
}
int remesa_render_pass_fail(struct remesa_render_pass *p,
 const struct remesa_pass_ticket *t, int error)
{
 int e = match(p, t); if (e) return e;
 if (error <= 0) return EINVAL;
 if (p->phase == REMESA_PASS_QUARANTINED) return EALREADY;
 if (p->phase != REMESA_PASS_PENDING && p->phase != REMESA_PASS_RESERVED) return EBUSY;
 p->phase = REMESA_PASS_QUARANTINED; p->content = REMESA_PASS_UNCERTAIN;
 p->error = error; return 0;
}
