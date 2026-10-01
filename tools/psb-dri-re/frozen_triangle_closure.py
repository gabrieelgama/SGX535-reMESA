#!/usr/bin/env python3
"""Shared static closure ledger. Never performs device operations.

Derived CPU transactions and policy checks do not authenticate architectural
postconditions. UNKNOWN evidence obligations survive successful simulated polls.
The canonical ledger prevents callers from promoting evidence by editing JSON.
"""
import argparse
import csv
import json
import hashlib
from pathlib import Path

import frozen_triangle_bo as bo
import frozen_triangle_contracts as contracts
import frozen_triangle_image as image

DOC = Path(__file__).resolve().parents[2] / 'docs/phase7/psb-dri-re'
ORDER = ('allocate_map', 'initialize_cpu_payloads', 'validate_bind',
         'relocate_validated_backing', 'scene_ta_ready_join', 'finish_last_cpu_write',
         'cpu_memory_publication', 'translation_publication',
         'device_invalidation_complete', 'first_consumer')


def retained_poll_return(completion, cleared):
    """Xpsb0x4e10 CPU return policy: second poll only logs; not recommended."""
    if type(completion) is not bool or type(cleared) is not bool:
        raise ValueError('boolean observations required')
    return 0 if completion else -16


def require_polls(results):
    if not results or any(pair != (True, True) for pair in results):
        raise ValueError('CLEANROOM_POLICY abort on completion OR clear failure')
    return True


def check_order(sequence, results):
    # Policy models an ordered provider, not a trace that was executed.
    if tuple(sequence) != ORDER:
        raise ValueError('incomplete/reordered publication: no consumer allowed')
    if len(results) != 3:
        raise ValueError('all three invalidation phases required')
    return require_polls(results)


def strong_publication_policy(m, sequence, results, published_bos, phase_cycles=None):
    """Enforce host ordering and BO coverage, not SGX visibility semantics."""
    p = bo.build(m)
    bo.check(m, p)
    check_order(sequence, results)
    required = {n for n, b in p['bos'].items() if b['domain'] != 'LOCAL'}
    if set(published_bos) != required:
        raise ValueError('partial or undeclared GPU BO publication')
    masks=[r['status_mask'] for r in contracts.publication_trace(0xffff)]
    if phase_cycles is None or len(phase_cycles)!=len(masks):
        raise ValueError('missing fresh publication cycle observations')
    for mask, cycle in zip(masks, phase_cycles):
        if len(cycle)!=3:
            raise ValueError('publication cycle requires before/completed/cleared')
        check_completion_cycle(mask, mask, *cycle)
    return 'ENFORCED_CONTRACT_ONLY'


def ta_load(flags):
    if type(flags) is not int or flags < 0 or flags & ~0x3f:
        raise ValueError('unknown TA load flag')
    selected = [(bit, kick, poll) for bit, kick, poll in
                ((1, 0x684, 1), (2, 0x680, 2), (4, 0x688, 4), (8, 0x690, 4))
                if flags & bit]
    mask = 0
    for _, _, poll in selected:
        mask |= poll
    return dict(flags=flags, kicks=[k for _, k, _ in selected],
                status_offset=0x118, clear_offset=0x114,
                retained_poll_mask=mask, header_load_mask=flags & 15,
                unpolled_header_bits=(flags & 15) & ~mask,
                init_kick=0x6a8 if flags & 16 else None,
                init_status_offset=0x12c, init_mask=0x400000,
                ready=False, classification='CPU_CONFIRMED; header applicability separate',
                evidence='P7H-052; Xpsb0x3df0/0x3f51; SGX535 header status2')


def check_completion_cycle(required_mask, polled_mask, before, completed, cleared):
    """Policy observation guard, conditional on an applicable completion contract.

    `before` is observed after clearing and before issuing a new request, with
    earlier requests quiescent. This function neither establishes quiescence
    nor asserts that a status bit means the requested hardware work is done.
    """
    values = (required_mask, polled_mask, before, completed, cleared)
    if any(type(v) is not int or not 0 <= v <= 0xffffffff for v in values):
        raise ValueError('32-bit observation/mask required')
    if not required_mask or polled_mask & required_mask != required_mask:
        raise ValueError('missing required completion bit')
    if before & required_mask:
        raise ValueError('no clear pre-request baseline; stale completion possible')
    if completed & required_mask != required_mask:
        raise ValueError('required load/invalidation not observed complete')
    if cleared & required_mask:
        raise ValueError('completion not cleared')
    return 'OBSERVATIONS_ONLY'


def strong_bootstrap_policy(raw_revision, provenance, before, completed,
                            cleared, init_completed, tables_ready,
                            timed_out=False):
    """Reject incomplete selected observations; no device-ready assertion."""
    if provenance != 'QUALIFIED_SGX_REVISION_EVIDENCE':
        raise ValueError('selected SGX revision not qualified')
    if timed_out is not False:
        raise ValueError('bootstrap timeout or unqualified timeout state')
    contracts.bootstrap(raw_revision, provenance)
    check_completion_cycle(0xf, 0xf, before, completed, cleared)
    if init_completed is not True or tables_ready is not True:
        raise ValueError('INITEND or required TA tables not observed ready')
    return 'ENFORCED_CONTRACT_ONLY'


def source_dominance(possible, deterministic, unresolved, envelope_proven=False):
    """Set implication checker; an asserted inventory is not a proven envelope."""
    if any(type(x) is not set for x in (possible, deterministic, unresolved)):
        raise ValueError('source-domain sets required')
    if envelope_proven is not True or unresolved or not possible <= deterministic:
        raise ValueError('possible pre-definition input not proven deterministic')
    return 'DOMINATED_BY_CONTRACT'


def cleanroom_source_domains(family_rows, auxiliary_rows, plan):
    """Five auxiliary contexts plus primary; UNKNOWN is an eligibility gap."""
    by_id={r['aux_id']:r for r in auxiliary_rows}
    rows=[]
    for family in family_rows + [dict(family_id='PRIMARY', members=[])]:
        family_id=family['family_id']
        programs=([by_id[n]['program'] for n in family['members']]
                  if family_id!='PRIMARY' else ['primary_pds'])
        bos={plan['bindings'][n]['bo'] for n in programs}
        if len(bos)!=1:
            raise ValueError('mixed PDS backing needs separate source model')
        backing=next(iter(bos))
        # These are deterministic CPU values, not certified architectural
        # sources. The unresolved class includes their preload mapping.
        known={'cpu_backing_image', 'encoded_launch_control'}
        deterministic=set(known)
        unresolved={'pds_architectural_source_state_not_proven_from_initialized_backing_or_controls'}
        try:
            source_dominance(known,deterministic,unresolved)
        except ValueError:
            result='UNKNOWN_ARCHITECTURAL_ELIGIBILITY'
        rows.append(dict(context=family_id, programs=programs,
            cpu_backing_bo=backing,
            cpu_backing_allocation_bytes=plan['bos'][backing]['size'],
            known_cpu_candidate_inputs=sorted(known),
            cleanroom_deterministic_cpu_domain=sorted(deterministic),
            architectural_envelope='UNKNOWN',
            unresolved_eligibility=sorted(unresolved),result=result,
            backing_policy='ENFORCED_CONTRACT: zero entire user BO before all object writes',
            evidence='P7H-038;P7H-041;P7H-053;P7H-056; clean-room BO initializer'))
    return rows


def cleanroom_publication_domains(plan):
    """Every selected GPU-facing BO; no inferred device cache-domain success."""
    return [dict(bo=n, owner=b['owner'], payload_bytes=b['size'],
                 producer='CPU full-zero + fields + relocations' if b['owner']=='user'
                          else 'service/kernel producer; complete bytes still conditional',
                 relocation_writes=sum(r['owner_bo']==n for r in plan['wire_relocations']),
                 mapping=b['map'], translation=b['domain'],
                 required_policy='all writes then CPU publication then translation publication then three successful device phases',
                 completion_observation='status0x138:0x44; status0x138:0x1; status0x12c:0x04000000',
                 completion_postcondition='UNKNOWN', first_consumer=b['role'],
                 evidence='P7H-047;P7H-051;P7H-057')
            for n,b in plan['bos'].items() if b['domain']!='LOCAL']


def cleanroom_contract_rules():
    return [dict(rule=rule, historical_unknown=historical,
                 stronger_contract=policy, software_enforceable='YES',
                 historical_dependency_eliminated=eliminated,
                 remaining_hardware_fact=fact, result='OPEN', evidence=evidence)
            for rule,historical,policy,eliminated,fact,evidence in [
        ('R1', 'historical callers may continue after maintenance timeout',
         'zero and populate full BOs; relocate after validation; require fresh success of all three phases for every GPU BO; abort on failure and defer first consumer',
         'historical timeout continuation is not required',
         'For each selected GPU BO, mapping and successful CPU/translation publication plus status0x138:0x44, status0x138:0x1 and status0x12c:0x04000000 completion must imply initialized payload and translation visibility to its first consumer',
         'P7H-047;P7H-051;P7H-054;P7H-057'),
        ('R2', 'historical mask0x7 does not observe LOAD3 bit0x8',
         'qualify raw SGX revision; require fresh status0x118 mask0xf, INITEND and required table observations; abort any failure before selected TA',
         'historical 0x7 wait is not required',
         'For the qualified target, fresh bit0x8 completion plus INITEND and table/service success must imply LOAD3 and revision-conditioned state are consumable before first selected TA use',
         'P7H-050;P7H-052;P7H-055;P7H-057'),
        ('R3', 'historical recycled PDS holes may be stale and exact reads unknown',
         'zero every byte of user BOs before program/constant writes and relocations; do not reuse dirty allocations',
         'historical stale-hole values are not required',
         'For each selected auxiliary family and primary launch, every architecturally eligible pre-definition DS/temp/implicit source must derive from initialized backing or deterministic control or be deterministically defined before first read',
         'P7H-036;P7H-041;P7H-053;P7H-056')]]


def selected_irq_ack(status2):
    # Candidate psb_irq.c184 and254: status2 mask is BIF_REQUESTER_FAULT only.
    return status2 & 0x10


def load_paths():
    result = []
    for i, name, kick, regs, poll in (
        (0, 'TA', 0x684, [0x618, 0x61c, 0x648], 1),
        (1, '3D/RASTER', 0x680, [0x600, 0x604, 0x64c], 2),
        (2, 'HOST/HOSTA', 0x688, [0x610, 0x614, 0x654], 4),
        (3, 'DHOST/HOSTD', 0x690, [0x608, 0x60c, 0x650], 4)):
        result.append(dict(load_id=f'LOAD{i}', request_bit=1 << i, header_name=name,
            initiator='psb_scene.c flags0x1f -> op9 -> Xpsb0x3df0',
            source='cookie0=GPU(ta_page_table); cookie10/11/12=packed page endpoints',
            descriptor_registers=regs, descriptor_values=['cookie0', '(cookie11<<16)|cookie10', 'cookie12<<16'],
            kick=kick, kick_value=1, status=0x118, header_completion_bit=1 << i,
            retained_poll_contribution=poll,
            purpose='header names a DPM free-load event; internal table consumer not specified',
            completion_producer='device status, hardware production rule UNKNOWN',
            revision_guard=f'none in selected flag{1 << i} branch',
            first_software_successor='INITEND wait -> op9 reply -> scene validation return -> selected TA op2',
            first_hardware_consumer='UNKNOWN', required_by_frozen_draw='CPU requests it; architectural necessity UNKNOWN',
            confidence='CONFIRMED CPU actions; header correlation; consumer rule UNKNOWN',
            evidence='P7H-055; Xpsb0x3df0;psb_scene.c257-265;SGX535 status2 definitions'))
    return result


def selected_later_waits():
    """Scoped explicit waits AFTER the mask7 load wait, not inferred dependencies."""
    rows = [dict(stage='TA load INIT', status=0x12c, mask=0x400000,
                 evidence='Xpsb0x3df0; P7H-055'),
            dict(stage='fresh TA scene prepare', status=0x12c, mask=0x100a40,
                 evidence='Xpsb0x4550 flags4; P7H-055')]
    for stage in ('TA', 'RASTER'):
        for r in contracts.publication_trace(0xffff):
            rows.append(dict(stage=stage + ' cache phase', status=r['status_offset'],
                mask=r['status_mask'], evidence='Xpsb0x4f10/0x5040; P7H-051'))
    # Selected raster flags15: (flags&9)!=1, cookie14==0 -> no helper wait.
    # TA scheduling events are in another status bank and imply no LOAD3 rule.
    return rows


def canonical_families(m):
    rows = auxiliary_matrix(m)
    by_id = {r['aux_id']: r for r in rows}
    with (DOC / 'pds-semantic-corpus.csv').open(newline='') as f:
        prior = list(csv.DictReader(f))
    with (DOC / 'pds-differential-matrix.csv').open(newline='') as f:
        diffs = list(csv.DictReader(f))
    if len(prior) != 42:
        raise ValueError('prior corpus changed; review applicability rather than silently rebuilding')
    literal_rows = {}
    for row in prior:
        if row['construction'] == 'literal':
            literal_rows.setdefault(int(row['word'], 16), []).append(row['row'])
    result = []
    for i, (code, ids) in enumerate(families(rows).items(), 1):
        instances = [by_id[n] for n in ids]
        result.append(dict(family_id=f'FAMILY-{i}', members=ids, instructions=list(code),
            data_prefix_bytes=instances[0]['data_prefix_bytes'],
            initialized_offsets=list(range(0, instances[0]['data_prefix_bytes'], 4)),
            launch_contexts=[dict(program=r['program'], words=r['launch_words'], sites=r['launch_sites']) for r in instances],
            roles=[r['role'] for r in instances],
            use_linkages=[r for r in m['relocations'] if r['owner'] in [x['program'] for x in instances]
                          and r['kind'] == 'USE_THREE_RECORDS'],
            old_literal_rows={f'{word:08x}': literal_rows[word] for word in code if word in literal_rows},
            unmatched_old_corpus_words=sorted(set(code) - literal_rows.keys()),
            inherited_comparisons=[r['comparison'] for r in diffs
                                   if int(r['left'], 16) in code or int(r['right'], 16) in code],
            predefinition_timeline=[dict(instruction=i, word=w, architectural_reads='UNKNOWN',
                                        architectural_definitions='UNKNOWN') for i, w in enumerate(code)],
            possible_sources=['deterministic CPU prefix IF mapped/preloaded', 'encoded launch control',
                              'UNCLASSIFIED pre-definition domain; not an asserted external read'],
            finite_architectural_domain='UNKNOWN; CPU extent is not an eligibility bound',
            coverage='UNKNOWN', evidence='P7H-053;P7H-056; prior semantic/differential results consumed without regeneration'))
    return result


def families(rows):
    out = {}
    for row in rows:
        out.setdefault(tuple(row['instructions']), []).append(row['aux_id'])
    return out


def differentials(rows):
    by_id = {r['aux_id']: r for r in rows}
    result = []
    for code, members in families(rows).items():
        if len(members) < 2:
            continue
        words = [by_id[n]['data_words'] for n in members]
        changed = [i for i in range(len(words[0])) if len({str(w[i]) for w in words}) > 1]
        result.append(dict(members=members, instructions=list(code),
            changed_data_dwords=changed, instruction_variation=False,
            source_domain_closed=False,
            limitation='CPU data changes without code change; does not expose an architectural source-class selector'))
    return result


def launch_words(m, name):
    """Existing serialized records only; no architectural entry/preload claim."""
    def words(owner):
        return [f['value'] for f in m['objects'][owner]['fields']]
    if name in ('vertex_pds', 'bounds_vertex_pds'):
        return words('triangle_index' if name == 'vertex_pds' else 'bounds_index')
    if name.endswith('_state_pds'):
        return words(name[:-4] + '_ta')
    if name in ('background_pds', 'background_secondary'):
        return words('background_object')[:3]
    if name in ('primary_pds', 'secondary_pds'):
        return words('triangle_state')[2:5]
    if name == 'event_pds':
        return words('raster_registers')[-6:]
    return []  # allocated vertex secondary has no explicit selected TA site


def auxiliary_matrix(m):
    base = contracts.auxiliary(m)
    contracts.check_auxiliary(m, base)
    rows = []
    for i, r in enumerate(base, 1):
        name = r['program']
        obj = m['objects'][name]
        split = r['data_prefix_bytes'] // 4
        fields = obj['fields']
        instructions = [f['value'] for f in fields[split:]]
        if any(type(w) is not int for w in instructions):
            raise ValueError('instruction image is not fully defined')
        sites = [f"{s['owner']}+0x{s['offset']:x}: {s['formula']}"
                 for s in m['relocations'] if s['target'] == name]
        rows.append(dict(aux_id=f'AUX-{i:02}', program=name, size=obj['size'],
            data_prefix_bytes=r['data_prefix_bytes'],
            data_words=[f['value'] for f in fields[:split]],
            historical_hole_offsets=[f['offset'] for f in fields[:split]
                                     if f['historical_producer_unwritten']],
            instructions=instructions, launch_sites=sites, launch_words=launch_words(m, name),
            launch_status='EXPLICIT_CPU_REFERENCE' if sites else 'ALLOCATED_NO_EXPLICIT_SCOPED_LAUNCH_REFERENCE',
            role=r['role'], producer=obj['producer'],
            cpu_definedness='ALL_BYTES_DEFINED_WITH_SYMBOLIC_RELOCATIONS',
            source_classes=['INITIALIZED_CPU_PREFIX_IF_PRELOADED',
                            'ENCODED_LAUNCH_CONTROL', 'UNCLASSIFIED_PREDEFINITION_SOURCE'],
            first_read='UNKNOWN architectural ordering; no invented write-before-read guarantee',
            coverage='UNKNOWN', missing_rule=r['missing_rule'],
            evidence=r['evidence'] + ';P7H-053'))
    return rows


def publication_edges():
    # The conceptual total order is a strengthened clean-room requirement.
    # Translation binding can already occur during validation; this list requires
    # its completion before the consumer, not a second invented MMU operation.
    return [dict(step=n, kind=kind, scope=scope, primitive=primitive,
                 postcondition=status, evidence=evidence)
            for n, kind, scope, primitive, status, evidence in [
        ('allocate_map', 'BACKING', 'user and kernel payloads',
         'selected nonfixed TTM same backing; mapped protections', 'CONDITIONAL_PROVIDER', 'P7H-047'),
        ('initialize_cpu_payloads', 'CPU_DATA', 'all six user BOs; service/kernel producers separate',
         'full zero then exact fields; no historical stale replacement claim', 'CPU_CONFIRMED', 'P7H-036;P7H-045;P7H-047'),
        ('validate_bind', 'PLACEMENT', 'six user validations; separate scene/TA validation',
         'reject errors; validated addresses; same TTM route', 'CPU_CONFIRMED', 'psb_sgx.c1283;psb_scene.c;P7H-047'),
        ('relocate_validated_backing', 'CPU_DATA', '49 wire writes',
         'psb_fixup_relocs AFTER validation; reject errors', 'CPU_CONFIRMED', 'psb_sgx.c1289;P7H-047'),
        ('scene_ta_ready_join', 'BOOTSTRAP_JOIN', 'kernel scene and TA backing; B4 sub-sequence',
         'scene validation after user relocation; require service-generated table readiness; not a CPU zeroed-table claim', 'B4_UNKNOWN', 'psb_sgx.c1394;psb_scene.c257-265;P7H-052'),
        ('finish_last_cpu_write', 'ORDER', 'payload relocations plus scene/table producers',
         'no later write without renewed publication', 'CLEANROOM_POLICY', 'P7H-054'),
        ('cpu_memory_publication', 'CPU_VISIBILITY', 'UC/WC payloads; cached scene handled separately',
         'mapping protection and required store/cache completion; wmb is not GPU cache invalidation', 'CONDITIONAL_PROVIDER', 'drm_vm.c;psb_sgx.c487-503;P7H-051'),
        ('translation_publication', 'TRANSLATION', 'bound pages/PTEs',
         'PTE clflush; BIF invalidate/flush; readback; not payload flush', 'CPU_SEQUENCE_CONFIRMED', 'psb_mmu.c140-180;P7H-051'),
        ('device_invalidation_complete', 'GPU_VISIBILITY', 'selected TA and raster first consumers',
         'three Xpsb phases; REQUIRE both polls each; abort failure', 'ARCHITECTURAL_POSTCONDITION_UNKNOWN', 'Xpsb0x4e10/0x4f10/0x5040;P7H-051;P7H-054'),
        ('first_consumer', 'ORDER', 'TA then selected raster service',
         'no kick until publication AND bootstrap obligations closed', 'CLEANROOM_POLICY', 'P7H-054')]]


def bootstrap_prerequisites():
    return [dict(state=state, producer=producer, first_consumer=consumer,
                 observed_success=success, remaining=remaining, evidence=evidence)
            for state, producer, consumer, success, remaining, evidence in [
        ('address spaces and BO backing', 'provider memory managers; validation/bind',
         'relocator and TA/raster fetches', 'creation/validation returns success and addresses fit explicit ranges',
         'B3 publication', 'P7H-047'),
        ('USE base reservations', 'provider register allocator; USE op5/4/4 relocations',
         'selected vertex/fragment/event USE tasks', 'CPU resolver checks explicit distinct DM reservations and window',
         'B3 publication; B4 compatible initialized service', 'P7H-047'),
        ('revision-conditioned register state', 'Xpsb0x4af0 ->0x3820',
         'Xpsb0x4f10/0x5040 via selected op2', 'init function returns zero after writes; op4 reply explicitly zero',
         'B4 target-qualified ready postcondition; no init-ready poll', 'P7H-050;P7H-052'),
        ('scene context and cookie', 'psb_scene.c; Xpsb0x3a40; hardware context allocator',
         'op2 bind/fire offset and hw_context; cookie2..8 or7/14',
         'allocation/validation and service reply; known first-use cookie formulas',
         'B4 service readiness; B3 publication; cookie is not full hardware scene state', 'P7H-008;P7H-046;P7H-047'),
        ('TA table/load state', 'psb_scene.c257-265 -> psb_xhw_ta_mem_load ->Xpsb0x3df0',
         'first selected TA op2', 'request done and ret0; retained mask7 then INITEND poll',
         'B4 applicability/completeness of ready predicate including fourth load', 'P7H-052'),
        ('request interface and completion', 'psb_xhw enqueue/done and XpsbThread dispatcher',
         'op2 TA/raster requests', 'reply delivered; op9 helper result propagated',
         'transport completion is not publication or initialized-state proof', 'P7H-046;P7H-052')]]


def scope_audit(m):
    p = bo.build(m)
    bo.check(m, p)
    return dict(enumerated_wire_coverage='COMPLETE', wire_count=len(p['wire_relocations']),
                missing_enumerated_wire_sites=[], backing_roles=len(p['bos']),
                user_validation_entries=len(p['validation']),
                kernel_generated_payloads=[n for n in ('scene_hw','ta_page_table','ta_parameter')
                                           if m['objects'][n]['bytes'] is None],
                whole_path_contract='PARTIAL',
                reason='B4-generated state and B3 publication unproved; not an omitted enumerated CPU relocation',
                evidence='P7H-047;P7H-057')


def rule_discriminators():
    return [dict(rule=rule, status='UNKNOWN', proposition=proposition,
                 model_a=a, model_b=b, distinguishing_fact=fact,
                 narrowest_evidence=evidence)
            for rule, proposition, a, b, fact, evidence in [
        ('R1', 'Fresh successful selected publication covers every first-consumer payload',
         'All required payload/translation domains are covered by completed maintenance',
         'The same observed bits complete but one required payload domain is not covered',
         'Applicable completion-domain implication for the selected mapping and invalidation sequence',
         'Qualified cache/mapping contract including status138 masks44/1 and MADD completion; not merely caller continuation'),
        ('R2', 'Selected service/TA ready predicate implies all required initialized state is available',
         'INITEND and subsequent selected scene preparation dominate every required load including LOAD3',
         'Those observations may precede availability of LOAD3 or another required revision-conditioned state',
         'Applicable readiness implication including LOAD3 completion/dependency and revision-conditioned initialization',
         'Qualified DHOST-load consumer/dependency rule plus service initialization postcondition; a bit-name list alone is insufficient'),
        ('R3', 'Every selected family/context pre-definition input lies in deterministic launch state',
         'All pre-definition inputs are supplied by initialized prefix and established controls',
         'An additional launch/state producer is needed but absent from the reconstructed clean-room contract',
         'Complete pre-definition source eligibility for the five streams and primary launch contexts',
         'Applicable source-class/preload rule or bounded accessible-domain initializer; CPU literal equality alone does not establish it')]]


def build(m):
    image.validate(m)
    p = bo.build(m)
    bo.check(m, p)
    aux = auxiliary_matrix(m)
    family_rows = canonical_families(m)
    primary = m['objects']['primary_pds']
    return dict(schema=1, hardware_ready=False, gate_b='BLOCKED',
        publication_order=list(ORDER), publication_edges=publication_edges(),
        backing_consumers=[dict(bo=n, owner=v['owner'], role=v['role'],
            publication='CPU_INTERFACE_ONLY' if n in ('control', 'xhw_comm') else 'B3_REQUIRED',
            initialization='CPU_SERIALIZER' if v['owner'] == 'user' else 'B4_PROVIDER_PRODUCER')
            for n, v in p['bos'].items()],
        bootstrap=dict(revision_codes=[107, 108, 109, 111, 113], ready=False,
            init_reply='XpsbThread op4 sets ret=0 after writes; no ready poll in0x3820',
            ta_request=ta_load(0x1f),
            ta_reply='op9 returns helper result; kernel done/ret acknowledge reply, not full ready state',
            postcondition='UNKNOWN: target-qualified service initialization and all required TA loads ready',
            evidence='P7H-050;P7H-052; psb_scene.c257-265;psb_xhw.c300-337;Xpsb0x30f0/0x3820/0x3df0'),
        bootstrap_prerequisites=bootstrap_prerequisites(), auxiliary=aux,
        load_paths=load_paths(), later_waits=selected_later_waits(),
        canonical_families=family_rows,
        cleanroom_source_domains=cleanroom_source_domains(family_rows, aux, p),
        cleanroom_publication_domains=cleanroom_publication_domains(p),
        cleanroom_contract_rules=cleanroom_contract_rules(),
        scope_audit=scope_audit(m), rule_discriminators=rule_discriminators(),
        inherited_constraint_files={name: hashlib.sha256((DOC / name).read_bytes()).hexdigest()
            for name in ('pds-semantic-corpus.csv', 'pds-field-constraints.csv',
                         'pds-differential-matrix.csv', 'pds-bit-influence.csv')},
        controlled_differences=differentials(aux),
        primary=dict(instructions=[f['value'] for f in primary['fields'][12:]],
            data_prefix_bytes=48, coverage='UNKNOWN', launch_words=launch_words(m, 'primary_pds'),
            source_classes=['INITIALIZED_CPU_PREFIX_IF_PRELOADED', 'ENCODED_LAUNCH_CONTROL',
                            'UNCLASSIFIED_PREDEFINITION_SOURCE'],
            actual_outside_read_established=False, persistence_established=False,
            evidence='P7H-041;P7H-053'),
        obligations=[
            dict(rule='PUB', blockers=['B3'], status='UNKNOWN',
                 missing='Successful selected publication sequence must publish every initialized/relocated payload to its first consumer; status0x138 masks0x44/1 lack an applicable completion-domain contract',
                 evidence='P7H-051;P7H-054;P7H-057'),
            dict(rule='READY', blockers=['B4'], status='UNKNOWN',
                 missing='Target-qualified ready postcondition for revision-conditioned initialization and TA loads, including whether the fourth load is complete when retained mask7 succeeds',
                 evidence='P7H-050;P7H-052;P7H-055'),
            dict(rule='SOURCE_DOMAIN', blockers=['B1', 'L12'], status='UNKNOWN',
                 missing='Complete pre-definition source eligibility for the five auxiliary instruction streams in their launch contexts and the primary pair; CPU bytes and equal instructions do not prove context-independent coverage',
                 evidence='P7H-041;P7H-049;P7H-053;P7H-056')])


def decision(model):
    unresolved = [r for r in model['obligations'] if r['status'] != 'CONFIRMED']
    remaining = list(dict.fromkeys(b for r in unresolved for b in r['blockers']))
    return dict(B1='OPEN' if 'B1' in remaining else 'CLOSED', B2='CLOSED',
        B3='CONDITIONAL' if 'B3' in remaining else 'CLOSED',
        B4='OPEN' if 'B4' in remaining else 'CLOSED', L12='OPEN' if 'L12' in remaining else 'CLOSED',
        FG01='CLOSED', FG02='OPEN' if remaining else 'CLOSED',
        complete=not remaining, remaining=remaining,
        minimum_rule_groups=[r['rule'] for r in unresolved],
        hardware_ready=False, gate_b='BLOCKED')


def check(m, model, complete=False):
    # A caller-supplied CONFIRMED label is not an evidence update.
    if model != build(m):
        raise ValueError('missing/altered closure inventory or unsupported evidence promotion')
    d = decision(model)
    if complete and not d['complete']:
        raise ValueError('PARTIAL: ' + ', '.join(d['remaining']))
    return d


def write_tables(model):
    for filename, rows in [('frozen-triangle-auxiliary-domains.csv', model['auxiliary']),
                           ('frozen-triangle-publication-edges.csv', model['publication_edges']),
                           ('frozen-triangle-bootstrap-prerequisites.csv', model['bootstrap_prerequisites']),
                           ('frozen-triangle-load-paths.csv', model['load_paths']),
                           ('frozen-triangle-later-waits.csv', model['later_waits']),
                           ('frozen-triangle-pds-families.csv', model['canonical_families']),
                           ('cleanroom-source-domains.csv', model['cleanroom_source_domains']),
                           ('cleanroom-publication-domains.csv', model['cleanroom_publication_domains']),
                           ('cleanroom-final-rules.csv', model['cleanroom_contract_rules']),
                           ('frozen-triangle-rule-discriminators.csv', model['rule_discriminators']),
                           ('frozen-triangle-closure-obligations.csv', model['obligations'])]:
        with (DOC / filename).open('w', newline='') as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0]), lineterminator='\n')
            w.writeheader()
            w.writerows({k: json.dumps(v, separators=(',', ':')) if isinstance(v, (list, dict)) else v
                         for k, v in row.items()} for row in rows)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--complete', action='store_true')
    ap.add_argument('--write-tables', action='store_true')
    ap.add_argument('--json', action='store_true')
    args = ap.parse_args()
    m = image.build()
    model = build(m)
    try:
        result = check(m, model, complete=args.complete)
    except ValueError as e:
        ap.exit(1, str(e) + '\n')
    if args.write_tables:
        write_tables(model)
    print(json.dumps(model if args.json else result, indent=2))


if __name__ == '__main__':
    main()
