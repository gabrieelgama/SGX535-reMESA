/* SPDX-License-Identifier: MIT */
#include "remesa/draw_packet.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

static enum remesa_packet_status check(const float (*p)[4], uint32_t nv,
                                      const uint16_t *ix, uint32_t ni)
{
    uint32_t i, j;
    if (!p || !ix || !nv || !ni || ni % 3)
        return REMESA_PACKET_INVALID;
    if (nv > REMESA_DRAW_MAX_VERTICES || ni > REMESA_DRAW_MAX_INDICES)
        return REMESA_PACKET_UNSUPPORTED;
    for (i = 0; i < nv; ++i) {
        for (j = 0; j < 4; ++j)
            if (!isfinite(p[i][j])) return REMESA_PACKET_INVALID;
        if (p[i][2] < 0 || p[i][2] > 1 || p[i][3] != 1)
            return REMESA_PACKET_UNSUPPORTED;
    }
    for (i = 0; i < ni; ++i)
        if (ix[i] >= nv) return REMESA_PACKET_INVALID;
    return REMESA_PACKET_OK;
}

static int overlaps(const void *a, size_t as, const void *b, size_t bs)
{
    uintptr_t x = (uintptr_t)a, y = (uintptr_t)b;
    if (as > UINTPTR_MAX - x || bs > UINTPTR_MAX - y) return 1;
    return x < y + bs && y < x + as;
}

enum remesa_packet_status remesa_draw_packet_validate(
    const struct remesa_draw_packet *p)
{
    if (!p) return REMESA_PACKET_INVALID;
    if (p->width != 32 || p->height != 32)
        return REMESA_PACKET_UNSUPPORTED;
    return check(p->positions, p->vertex_count, p->indices, p->index_count);
}

enum remesa_packet_status remesa_draw_packet_build(
    struct remesa_draw_packet *out, const float (*p)[4], uint32_t nv,
    const uint16_t *ix, uint32_t ni, uint32_t argb)
{
    enum remesa_packet_status status;
    if (!out || !p || !ix) return REMESA_PACKET_INVALID;
    /* Check counts before calculating byte ranges or dereferencing inputs. */
    if (nv > REMESA_DRAW_MAX_VERTICES || ni > REMESA_DRAW_MAX_INDICES)
        return REMESA_PACKET_UNSUPPORTED;
    if (overlaps(out, sizeof(*out), p, (size_t)nv * sizeof(*p)) ||
        overlaps(out, sizeof(*out), ix, (size_t)ni * sizeof(*ix)))
        return REMESA_PACKET_INVALID;
    status = check(p, nv, ix, ni);
    if (status != REMESA_PACKET_OK) return status;
    memset(out, 0, sizeof(*out));
    out->width = out->height = 32;
    out->argb = argb;
    out->vertex_count = nv;
    out->index_count = ni;
    memcpy(out->positions, p, (size_t)nv * sizeof(*p));
    memcpy(out->indices, ix, (size_t)ni * sizeof(*ix));
    return REMESA_PACKET_OK;
}
