/* SPDX-License-Identifier: MIT */
#ifndef REMESA_DRAW_PACKET_H
#define REMESA_DRAW_PACKET_H
#include <stdint.h>

/* Internal semantic snapshot, NOT a userspace/kernel wire ABI or SGX stream.
 * Limits bound host allocation only; they advertise no hardware capability.
 * Positions are already projected into framebuffer coordinates. No shader,
 * perspective interpolation, depth testing or clipping is implied. */
#define REMESA_DRAW_MAX_VERTICES 1024u
#define REMESA_DRAW_MAX_INDICES 3072u
struct remesa_draw_packet {
    uint32_t width, height, argb, vertex_count, index_count;
    float positions[REMESA_DRAW_MAX_VERTICES][4];
    uint16_t indices[REMESA_DRAW_MAX_INDICES];
};
enum remesa_packet_status {
    REMESA_PACKET_OK, REMESA_PACKET_INVALID, REMESA_PACKET_UNSUPPORTED
};
enum remesa_packet_status remesa_draw_packet_validate(
    const struct remesa_draw_packet *packet);
/* Input storage must remain immutable through the call. Copies all active
 * positions/indices, zeroes unused storage. Failure leaves output unchanged.
 * Aliasing output with either input is rejected. Backend must independently
 * validate packets; validation grants no resource access or execution rights. */
enum remesa_packet_status remesa_draw_packet_build(
    struct remesa_draw_packet *out, const float (*positions)[4],
    uint32_t vertex_count, const uint16_t *indices, uint32_t index_count,
    uint32_t argb);
#endif
