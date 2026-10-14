/* SPDX-License-Identifier: MIT
 * Original mathematical preview of Gama's Xpsb-informed PDS hypothesis.
 * Field positions/operand projections are candidates, not rev121 ISA facts.
 * This interface has no upload, relocation, program or execution operation.
 */
#ifndef REMESA_PDS_CANDIDATE_H
#define REMESA_PDS_CANDIDATE_H
#include <stdint.h>
enum remesa_pds_candidate_policy {
 REMESA_PDS_CANDIDATE_ANALYSIS_ONLY = 1
};
struct remesa_pds_candidate_request {
 uint32_t policy;
 uint32_t source[2],selector[4];
};
struct remesa_pds_candidate_operand { uint32_t bank,index; };
struct remesa_pds_candidate_preview {
 uint32_t word;
 struct remesa_pds_candidate_request decoded;
 struct remesa_pds_candidate_operand pair[4],scalar[4];
 /* Four encoded selectors do not prove four consumed output words.
  * Zero means UNKNOWN, not zero outputs. No production admission follows. */
 uint32_t output_arity_known,fetch_verified,semantics_verified;
};
/* Explicit opt-in. Success verifies only numerical packing/projections.
 * Source limits are candidate field widths, not hardware register bounds.
 * Outputs are unchanged on failure; input/output overlap is rejected. */
int remesa_pds_candidate_preview(const struct remesa_pds_candidate_request *,
                               struct remesa_pds_candidate_preview *);
/* Only the proposed group0/type0/condition7/destination5/reserved0 subset.
 * This is not an architectural disassembler or a machine-code validator. */
int remesa_pds_candidate_decode(uint32_t,struct remesa_pds_candidate_request *);
/* Explicit numerical condition-field variant for the two observed CPU word
 * families (3 and 7). Neither number has qualified predicate semantics.
 * The existing functions retain the condition-7-only behavior. These variants
 * still return only ANALYSIS_ONLY previews, never executable program CODE.
 * Decoder requires the caller-selected number to match exactly. */
int remesa_pds_candidate_preview_condition(const struct remesa_pds_candidate_request *,
 uint32_t numeric_condition,struct remesa_pds_candidate_preview *);
int remesa_pds_candidate_decode_condition(uint32_t word,uint32_t numeric_condition,
 struct remesa_pds_candidate_request *);
#endif
