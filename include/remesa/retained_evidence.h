/* SPDX-License-Identifier: MIT */
#ifndef REMESA_RETAINED_EVIDENCE_H
#define REMESA_RETAINED_EVIDENCE_H
#include <stdint.h>
/* Internal normalized evidence, NOT an ioctl ABI or execution authorization.
 * Decoder must preserve raw originals and reject missing/duplicate fields.
 * Digests are SHA-256 of complete immutable request/response snapshots. This
 * verifier compares them; it does not compute hashes or trust userspace claims.
 * Observer SGXNATIVE1 attests these hashes and same-task syscall closure; it
 * does not expose a resource ticket/generation. Exact kernel snapshots and
 * GEM ownership evidence must corroborate those fields independently.
 */
struct remesa_evidence_id {
 uint64_t context, operation, serial;
 uint32_t slot, generation;
};
struct remesa_evidence_expected {
 struct remesa_evidence_id id;
 uint8_t boot[16], source[32], request_sha256[32];
 uint64_t request_bytes;
 uint32_t resources;
};
struct remesa_evidence_admission {
 uint8_t boot[16], source[32];
 uint32_t records, source_valid, capsule_unused, native_unused;
 uint32_t owner_valid, mappings_valid, pfn_valid, commands_retained;
};
/* All boolean fields must be exactly 0 or 1. Fields ending _records must be 1.
 * The lifecycle flags are decoded from existing event/phase evidence; they
 * are NOT proposed SGX register encodings. */
struct remesa_evidence_response {
 struct remesa_evidence_id id;
 uint8_t request_sha256[32], response_sha256[32];
 uint64_t request_bytes, response_bytes;
 int32_t ioctl_result, operation_errno;
 uint32_t records, attempted, ta_complete, end_render, memory_free;
 uint32_t retired, visible, acknowledged, quarantined;
};
struct remesa_evidence_observer {
 uint8_t boot[16], source[32], request_sha256[32], response_sha256[32];
 uint64_t request_bytes, response_bytes;
 int32_t ioctl_result, syscall_result;
 uint32_t native_records, capsule_records, source_records;
 uint32_t request_present, response_present, closed, syscall_exit_seen;
 uint32_t invalid, capsule_invalid, source_valid;
 uint32_t ta_complete, end_render, memory_free, retired;
};
/* PFN/mapping flags attest the held-through-completion snapshot, not mappings
 * after release. Observer retirement is SGX lifecycle; detached/released below
 * are separate GEM service observations and must not be inferred from it. */
struct remesa_evidence_ownership {
 struct remesa_evidence_id id;
 uint32_t records, resources, pfn_valid, mappings_valid;
 uint32_t held_through_completion, visibility_ordered, detached, released;
 uint32_t quarantined;
};
enum remesa_evidence_status {
 REMESA_EVIDENCE_OK, REMESA_EVIDENCE_MALFORMED, REMESA_EVIDENCE_IDENTITY,
 REMESA_EVIDENCE_BINDING, REMESA_EVIDENCE_ADMISSION,
 REMESA_EVIDENCE_LIFECYCLE, REMESA_EVIDENCE_OWNERSHIP
};
/* Success establishes consistency only, never physical execution by itself.
 * Mock records can pass the same checker. Physical claim requires preserved
 * raw kernel/observer records, authenticated capture and artifact provenance.
 * Neither function changes an execution gate or creates a completion record. */
enum remesa_evidence_status remesa_retained_admission_verify(
 const struct remesa_evidence_expected *, const struct remesa_evidence_admission *);
enum remesa_evidence_status remesa_retained_completion_verify(
 const struct remesa_evidence_expected *, const struct remesa_evidence_response *,
 const struct remesa_evidence_observer *, const struct remesa_evidence_ownership *);
#endif
