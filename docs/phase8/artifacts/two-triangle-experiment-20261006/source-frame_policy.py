"""CPU-only future frame admission model; not a live SGX adapter.

Truth of supplied provider evidence is outside this policy. No production caller
exists. A successful synthetic rearm is NOT proof of hardware reuse safety.
"""
from dataclasses import dataclass

@dataclass(frozen=True)
class RearmPremises:
    same_owner: bool = False
    previous_retired: bool = False
    previous_evidence_sealed: bool = False
    previous_events_consumed: bool = False
    authoritative_source_boundary: bool = False
    continuous_isolation: bool = False
    dpm_resources_safe: bool = False
    memory_visible: bool = False
    no_evidence_loss: bool = False

class FramePolicy:
    def __init__(self):
        self.phase='NEW'
        self.epoch=0 # internal CPU frame identity, not an exported UAPI sequence
    def prepare(self, *, owned, construction_valid, source_valid, isolation_valid):
        if self.phase!='NEW' or not all(type(v) is bool and v for v in (owned,construction_valid,source_valid,isolation_valid)):
            self.phase='BLOCKED'; return False
        self.phase='PREPARED'; return True
    def admit(self, *, authorization, bindings_valid):
        if self.phase!='PREPARED' or authorization is not True or bindings_valid is not True:
            self.phase='BLOCKED'; return False
        # Mark potential issuance before any external submit adapter could run.
        self.phase='POSSIBLY_ISSUED'; return True
    def complete(self, *, retired, ledger, provenance_valid):
        if self.phase!='POSSIBLY_ISSUED' or retired is not True or type(ledger) is not int or ledger!=7 or provenance_valid is not True:
            self.phase='HOLD'; return False
        self.phase='RETIRED'; return True
    def seal(self, *, originals_complete, hashes_valid):
        if self.phase!='RETIRED' or originals_complete is not True or hashes_valid is not True:
            self.phase='HOLD'; return False
        self.phase='SEALED'; return True
    def rearm(self, premises):
        if self.phase!='SEALED' or type(premises) is not RearmPremises or not all(type(v) is bool and v for v in vars(premises).values()):
            self.phase='BLOCKED'; return False
        self.epoch+=1; self.phase='NEW'; return True
