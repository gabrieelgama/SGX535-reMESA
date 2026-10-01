"""Static closure guards; successful simulated polls are not hardware evidence."""
import copy
import unittest

import frozen_triangle_image as image
import frozen_triangle_closure as c


class ClosureTests(unittest.TestCase):
    def setUp(self):
        self.m = image.build()
        self.model = c.build(self.m)

    def test_auxiliary_families(self):
        rows = self.model['auxiliary']
        self.assertEqual(len(rows), 11)
        self.assertEqual(sorted(len(v) for v in c.families(rows).values()), [1, 1, 2, 3, 4])
        self.assertEqual(rows[0]['instructions'], rows[1]['instructions'])
        self.assertNotEqual(rows[0]['data_words'], rows[1]['data_words'])

    def test_controlled_family_differences(self):
        d = c.differentials(self.model['auxiliary'])
        self.assertEqual(d[0]['changed_data_dwords'], [0, 1, 4, 8])
        self.assertEqual(d[1]['changed_data_dwords'], [0, 1, 2])
        self.assertTrue(all(not r['source_domain_closed'] for r in d))

    def test_primary_shares_background_first_word_not_launch(self):
        row = next(r for r in self.model['auxiliary'] if r['program'] == 'background_pds')
        self.assertEqual(row['instructions'][0], self.model['primary']['instructions'][0])
        self.assertNotEqual(row['launch_words'], self.model['primary']['launch_words'])

    def test_allocated_vertex_secondaries_have_no_explicit_launch_site(self):
        rows = {r['program']: r for r in self.model['auxiliary']}
        for name in ('vertex_secondary', 'bounds_vertex_secondary'):
            self.assertEqual(rows[name]['launch_sites'], [])
            self.assertEqual(rows[name]['coverage'], 'UNKNOWN')

    def test_missing_auxiliary(self):
        self.model['auxiliary'].pop()
        with self.assertRaises(ValueError): c.check(self.m, self.model)

    def test_uninitialized_auxiliary(self):
        self.m['objects']['event_pds']['bytes'][24] = None
        with self.assertRaises(ValueError): c.build(self.m)

    def test_undeclared_source(self):
        self.model['auxiliary'][0]['source_classes'].append('invented_register')
        with self.assertRaises(ValueError): c.check(self.m, self.model)

    def test_auxiliary_unknown_cannot_be_promoted(self):
        self.model['auxiliary'][0]['coverage'] = 'CONFIRMED'
        with self.assertRaises(ValueError): c.check(self.m, self.model)

    def test_primary_nondeterministic_source(self):
        self.model['primary']['source_classes'].append('uninitialized_readable_state')
        with self.assertRaises(ValueError): c.check(self.m, self.model, complete=True)

    def test_missing_primary_domain(self):
        self.model['primary']['source_classes'].remove('UNCLASSIFIED_PREDEFINITION_SOURCE')
        with self.assertRaises(ValueError): c.check(self.m, self.model)

    def test_four_loads_only_three_distinct_polled_bits(self):
        t = c.ta_load(0x1f)
        self.assertEqual(t['kicks'], [0x684, 0x680, 0x688, 0x690])
        self.assertEqual(t['retained_poll_mask'], 7)
        self.assertEqual(t['header_load_mask'], 15)
        self.assertEqual(t['unpolled_header_bits'], 8)
        self.assertEqual(c.ta_load(8)['retained_poll_mask'], 4)

    def test_no_false_ta_ready(self):
        self.assertFalse(c.ta_load(0x1f)['ready'])
        self.assertFalse(self.model['bootstrap']['ready'])

    def test_unknown_ta_flag(self):
        with self.assertRaises(ValueError): c.ta_load(0x40)

    def test_timeout_policy_stronger_than_retained_return(self):
        self.assertEqual(c.retained_poll_return(True, False), 0)
        with self.assertRaises(ValueError): c.require_polls([(True, False)])
        self.assertEqual(c.retained_poll_return(False, True), -16)

    def test_publication_success_is_only_a_policy_check(self):
        self.assertTrue(c.check_order(self.model['publication_order'], [(True, True)] * 3))
        self.assertFalse(c.decision(self.model)['complete'])

    def test_publication_timeout(self):
        with self.assertRaises(ValueError):
            c.check_order(self.model['publication_order'], [(True, True), (False, True), (True, True)])

    def test_missing_invalidation(self):
        self.model['publication_order'].remove('device_invalidation_complete')
        with self.assertRaises(ValueError): c.check_order(self.model['publication_order'], [(True, True)] * 3)

    def test_consumer_before_publication(self):
        seq = self.model['publication_order']
        seq[-1], seq[-2] = seq[-2], seq[-1]
        with self.assertRaises(ValueError): c.check_order(seq, [(True, True)] * 3)

    def test_relocations_after_validation(self):
        seq = self.model['publication_order']
        a, b = seq.index('validate_bind'), seq.index('relocate_validated_backing')
        seq[a], seq[b] = seq[b], seq[a]
        with self.assertRaises(ValueError): c.check_order(seq, [(True, True)] * 3)

    def test_missing_bootstrap_postcondition(self):
        del self.model['bootstrap']['postcondition']
        with self.assertRaises(ValueError): c.check(self.m, self.model, complete=True)

    def test_incompatible_revision(self):
        self.model['bootstrap']['revision_codes'].append(999)
        with self.assertRaises(ValueError): c.check(self.m, self.model)

    def test_missing_provenance(self):
        self.model['obligations'][0]['evidence'] = ''
        with self.assertRaises(ValueError): c.check(self.m, self.model)

    def test_forged_proof(self):
        for r in self.model['obligations']: r['status'] = 'CONFIRMED'
        with self.assertRaises(ValueError): c.check(self.m, self.model, complete=True)

    def test_all_four_blockers_recomputed(self):
        d = c.decision(self.model)
        self.assertEqual(set(d['remaining']), {'B3', 'B4', 'B1', 'L12'})
        self.assertEqual(len(d['minimum_rule_groups']), 3)
        self.assertEqual(d['B2'], 'CLOSED')

    def test_complete_refuses(self):
        c.check(self.m, self.model)
        with self.assertRaisesRegex(ValueError, 'PARTIAL'): c.check(self.m, self.model, complete=True)

    def test_load_paths_named_without_inventing_hardware_consumer(self):
        paths = c.load_paths()
        self.assertEqual([p['load_id'] for p in paths], ['LOAD0','LOAD1','LOAD2','LOAD3'])
        self.assertEqual(paths[3]['kick'], 0x690)
        self.assertEqual(paths[3]['first_hardware_consumer'], 'UNKNOWN')
        self.assertEqual(paths[3]['revision_guard'], 'none in selected flag8 branch')

    def test_no_later_load3_bit_poll_in_scoped_path(self):
        waits = c.selected_later_waits()
        self.assertFalse(any(w['status'] == 0x118 and w['mask'] & 8 for w in waits))
        self.assertEqual(waits[1]['mask'], 0x100a40)

    def test_irq_cannot_certify_load3(self):
        self.assertEqual(c.selected_irq_ack(0x8), 0)
        self.assertEqual(c.selected_irq_ack(0x18), 0x10)

    def test_full_mask_observations_not_architectural_readiness(self):
        self.assertEqual(c.check_completion_cycle(15, 15, 0, 15, 0), 'OBSERVATIONS_ONLY')
        self.assertFalse(c.decision(self.model)['complete'])

    def test_missing_load3_mask(self):
        with self.assertRaises(ValueError): c.check_completion_cycle(15, 7, 0, 7, 0)

    def test_missing_load3_completion(self):
        with self.assertRaises(ValueError): c.check_completion_cycle(15, 15, 0, 7, 0)

    def test_stale_load3_completion(self):
        with self.assertRaises(ValueError): c.check_completion_cycle(15, 15, 8, 15, 0)

    def test_uncleared_completion(self):
        with self.assertRaises(ValueError): c.check_completion_cycle(15, 15, 0, 15, 8)

    def test_canonical_families_consume_prior_constraints(self):
        rows = c.canonical_families(self.m)
        self.assertEqual(len(rows), 5)
        self.assertEqual([r['family_id'] for r in rows], [f'FAMILY-{i}' for i in range(1,6)])
        self.assertEqual(rows[0]['unmatched_old_corpus_words'], [])
        self.assertEqual(len(rows[2]['unmatched_old_corpus_words']), 15)
        self.assertTrue(all(r['coverage'] == 'UNKNOWN' for r in rows))

    def test_no_false_program_write_before_read(self):
        for row in c.canonical_families(self.m):
            self.assertEqual(len(row['predefinition_timeline']),len(row['instructions']))
            self.assertTrue(all(x['architectural_reads'] == 'UNKNOWN' and
                                x['architectural_definitions'] == 'UNKNOWN'
                                for x in row['predefinition_timeline']))

    def test_ledger_partial_is_not_a_missing_known_wire_site(self):
        a = c.scope_audit(self.m)
        self.assertEqual(a['wire_count'],49)
        self.assertEqual(a['enumerated_wire_coverage'],'COMPLETE')
        self.assertEqual(a['whole_path_contract'],'PARTIAL')
        self.assertEqual(a['kernel_generated_payloads'],['scene_hw','ta_page_table','ta_parameter'])

    def test_rule_models_remain_explicit(self):
        rows = c.rule_discriminators()
        self.assertEqual([r['rule'] for r in rows],['R1','R2','R3'])
        for row in rows:
            self.assertTrue(row['model_a'] and row['model_b'] and row['distinguishing_fact'])
            self.assertEqual(row['status'],'UNKNOWN')

    def test_strong_publication_policy_covers_every_gpu_bo(self):
        required={n for n,v in c.bo.build(self.m)['bos'].items() if v['domain']!='LOCAL'}
        cycles=[(0,mask,0) for mask in (0x44,1,0x4000000)]
        result=c.strong_publication_policy(self.m, c.ORDER, [(True,True)]*3,
                                           required, cycles)
        self.assertEqual(result,'ENFORCED_CONTRACT_ONLY')
        with self.assertRaises(ValueError):
            c.strong_publication_policy(self.m, c.ORDER, [(True,True)]*3,
                                        required-{'scene_hw'}, cycles)
        with self.assertRaises(ValueError):
            c.strong_publication_policy(self.m, c.ORDER, [(True,True),(False,True),(True,True)],
                                        required, cycles)
        with self.assertRaises(ValueError):
            c.strong_publication_policy(self.m, c.ORDER, [(True,True)]*3, required)
        stale=[(0x44,0x44,0),(0,1,0),(0,0x4000000,0)]
        with self.assertRaises(ValueError):
            c.strong_publication_policy(self.m, c.ORDER, [(True,True)]*3,
                                        required, stale)

    def test_strong_bootstrap_policy_requires_full_fresh_completion(self):
        self.assertEqual(c.strong_bootstrap_policy(7,'QUALIFIED_SGX_REVISION_EVIDENCE',
            before=0,completed=15,cleared=0,init_completed=True,tables_ready=True),
            'ENFORCED_CONTRACT_ONLY')
        for changes in ({'completed':7},{'before':8},{'init_completed':False},
                        {'tables_ready':False},{'provenance':'SYNTHETIC_MODEL_INPUT'},
                        {'raw_revision':99},{'timed_out':True}):
            kwargs=dict(raw_revision=7,provenance='QUALIFIED_SGX_REVISION_EVIDENCE',
                        before=0,completed=15,cleared=0,init_completed=True,tables_ready=True)
            kwargs.update(changes)
            with self.assertRaises(ValueError):c.strong_bootstrap_policy(**kwargs)

    def test_source_dominance_cannot_assume_unknown_domain(self):
        with self.assertRaises(ValueError):c.source_dominance(
            {'cpu_backing','launch_control'},{'cpu_backing','launch_control'},set())
        self.assertEqual(c.source_dominance({'cpu_backing','launch_control'},
            {'cpu_backing','launch_control'},set(),envelope_proven=True),
            'DOMINATED_BY_CONTRACT')
        with self.assertRaises(ValueError):c.source_dominance(
            {'cpu_backing','unclassified_pds_state'},{'cpu_backing'},set(),
            envelope_proven=True)
        with self.assertRaises(ValueError):c.source_dominance(
            {'cpu_backing'},{'cpu_backing'},{'unclassified_pds_state'})
        model=c.build(self.m)
        self.assertEqual(len(model['cleanroom_source_domains']),6)
        self.assertTrue(all(r['result']=='UNKNOWN_ARCHITECTURAL_ELIGIBILITY'
                            for r in model['cleanroom_source_domains']))

    def test_stronger_contracts_remain_separate_from_hardware_facts(self):
        rows=self.model['cleanroom_contract_rules']
        self.assertEqual([r['rule'] for r in rows],['R1','R2','R3'])
        self.assertTrue(all(r['software_enforceable']=='YES' for r in rows))
        self.assertTrue(all(r['remaining_hardware_fact'] and r['result']=='OPEN'
                            for r in rows))
        domains=self.model['cleanroom_publication_domains']
        expected={n for n,v in c.bo.build(self.m)['bos'].items() if v['domain']!='LOCAL'}
        self.assertEqual({r['bo'] for r in domains},expected)
        self.assertTrue(all(r['first_consumer'] and r['completion_postcondition']=='UNKNOWN'
                            for r in domains))
        source=self.model['cleanroom_source_domains']
        self.assertEqual([r['context'] for r in source],
                         [f'FAMILY-{i}' for i in range(1,6)]+['PRIMARY'])
        self.assertTrue(all(r['cpu_backing_bo']=='pds' for r in source))
        self.assertTrue(all(r['cpu_backing_allocation_bytes']==0x20000 for r in source))


if __name__ == '__main__': unittest.main()
