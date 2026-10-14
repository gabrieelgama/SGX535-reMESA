/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SEEDED_SESSION_H
#define REMESA_SEEDED_SESSION_H
#include "remesa/seeded_observation.h"

/* Host transcript validation, not an admission path or hardware authenticator.
 * Independent implementation of evidence-order constraints; no GPU encodings.
 * This does not open devices, issue ioctls, flush caches, or release resources.
 * The verifier must bind each normalized record AND its payload to an original
 * externally captured source. A verifier returning zero is a caller-supplied
 * trust assertion: it cannot itself establish physical execution or visibility.
 * Different provider IDs are necessary, not proof of independent observation.
 * No resulting field means PRESERVATION_CONFIRMED or permits another draw. */
enum remesa_seeded_session_event {
    REMESA_SEEDED_RETAINED = 1, REMESA_SEEDED_BEFORE,
    REMESA_SEEDED_SYNCED, REMESA_SEEDED_SUBMITTED,
    REMESA_SEEDED_TA_COMPLETE, REMESA_SEEDED_RENDER_COMPLETE,
    REMESA_SEEDED_GPU_RETIRED, REMESA_SEEDED_VISIBLE,
    REMESA_SEEDED_ACKNOWLEDGED, REMESA_SEEDED_TICKET_RETIRED,
    REMESA_SEEDED_PRIMARY_CAPTURE, REMESA_SEEDED_CORROBORATING_CAPTURE,
    REMESA_SEEDED_RELEASED,
    REMESA_SEEDED_BASELINE
};
#define REMESA_SEEDED_SESSION_RECORDS 14u
struct remesa_seeded_session_record {
    enum remesa_seeded_session_event event;
    struct remesa_framebuffer_observation_identity identity;
    uint64_t sequence, provider, record_id, capture_id;
    uint64_t gpu_va, pfn;
    uint32_t mapped_pages, verified_pages, pins, cpu_leases, held_jobs;
    uint32_t quarantined, submissions, live_resources;
    const void *payload;
    size_t payload_bytes;
};
/* No record structs or payloads may mutate during verification. The verifier
 * must not dispatch GPU work. It must return an errno for missing attribution,
 * uncertain completion, unknown visibility or contradictory source evidence. */
typedef int (*remesa_seeded_record_verifier)(void *opaque,
    const struct remesa_seeded_session_record *record);
struct remesa_seeded_session_report {
    struct remesa_seeded_observation_report observation;
    size_t verified_records;
    /* Correlated transcript assertions only, never physical proof. */
    uint64_t primary_provider, corroborating_provider;
};
/* Exact full successful-lifecycle record sequence required. Any missing,
 * duplicate, stale, out-of-order, early-release, or unaccounted event STOPs.
 * UNCHANGED returns ENODATA; contradictory pixels return EILSEQ. For these two
 * negative controls the complete report is returned; malformed/custody/verifier
 * failures leave report unchanged. No cleanup/retry/dispatch occurs on failure.
 * Original sealed reference must be separately authenticated by the caller. */
int remesa_seeded_session_validate(
    const struct remesa_framebuffer_observation_identity *expected,
    const struct remesa_seeded_session_record *records, size_t count,
    const void *reference, size_t reference_bytes,
    remesa_seeded_record_verifier verifier, void *opaque,
    struct remesa_seeded_session_report *report);
#endif
