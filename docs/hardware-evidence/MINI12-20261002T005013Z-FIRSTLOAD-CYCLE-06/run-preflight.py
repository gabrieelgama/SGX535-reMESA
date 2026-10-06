from pathlib import Path
import sys,json,hashlib
R=Path('/home/gama/sgx535-gfx');E=Path(Path('/tmp/sgx535-cycle06-live-evidence').read_text())
sys.path.insert(0,str(R/'tools/psb-dri-re'))
import frozen_first_load_capture_v2 as timing
import frozen_first_load_procedure as base
import frozen_first_load_procedure_v2 as gates
for pin in json.loads((E/'tooling-pins.json').read_text()):
 assert hashlib.sha256((R/pin['path']).read_bytes()).hexdigest()==pin['sha256'],'tooling drift'
gates.validate_plan(gates.load_plan());p=base.load_plan(R);base.validate_plan(p,R)
ready=json.loads((E/'operator-preflight-readiness.json').read_text());timing.require_ready(ready)
source=(E/'stock-root-template.py').read_text()
rows,verdict=timing.capture_once(E/'preflight',json.loads((E/'pinned-ssh-prefix.json').read_text()),source,ready,'stock_preparation')
creation=json.loads((R/'docs/hardware-evidence/MINI12-20261001T062604Z-FIRSTLOAD-CYCLE-02/staging/decoded-records.json').read_text())
receipts=base.validate_existing_stage_files(p,creation,rows[1]['destinations'])
old_id='22aae08e-fea5-492f-957b-5cffea6f33f4';reference=(E/'experimental-root-reference.py').read_text()
assert reference.count(old_id)==1
source=reference.replace(old_id,rows[0]['boot_id']);(E/'experimental-root.py').write_text(source)
(E/'future-capture-source-provenance.json').write_text(json.dumps({'experimental_root_sha256':hashlib.sha256(source.encode()).hexdigest(),'stock_template_sha256':hashlib.sha256((E/'stock-root-template.py').read_bytes()).hexdigest(),'only_experimental_reference_change':'fresh prior STOCK boot ID','fresh_stock_boot_id':rows[0]['boot_id']},indent=2)+'\n')
(E/'preflight-decision.json').write_text(json.dumps({'classification':'PASS','stock_boot_id':rows[0]['boot_id'],'guards_passed':len(rows[1]['guards']),'existing_creation_receipts':receipts,'sgx_authorized':False},indent=2)+'\n')
print('Fresh STOCK preflight PASS:',len(rows[1]['guards']),'guards; boot ID',rows[0]['boot_id'])
