"""CPU-only structural binding of one provenance-qualified witness, never live IO."""
import ast,base64,hashlib,re
OLD_SOURCE='2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555'
PROVENANCE=('boot_id','module_build_id','observer_build_id','image_sha256','source_boundary')
def require(x,why):
 if not x:raise ValueError(why)
def field(node,key):
 require(isinstance(node,ast.Dict),'record must be a literal dictionary')
 keys=[ast.literal_eval(k) for k in node.keys]
 require(len(keys)==len(set(keys)),'unexpected duplicate record field')
 require(keys.count(key)==1,'missing structural reference '+key)
 return node.values[keys.index(key)]
def assignment(tree,name):
 rows=[n for n in tree.body if isinstance(n,ast.Assign) and len(n.targets)==1 and isinstance(n.targets[0],ast.Name) and n.targets[0].id==name]
 require(len(rows)==1,'missing/duplicate structural object '+name)
 return rows[0].value

def predicate(tree):
 nodes=[n for n in ast.walk(tree) if isinstance(n,ast.Compare) and ast.unparse(n.left)=='hashlib.sha256(source).hexdigest()']
 require(len(nodes)==1,'source predicate must be unique')
 n=nodes[0];require(len(n.ops)==1 and isinstance(n.ops[0],ast.Eq) and len(n.comparators)==1,'wrong source predicate operator')
 value=n.comparators[0];require(isinstance(value,ast.Constant) and isinstance(value.value,str) and re.fullmatch('[0-9a-f]{64}',value.value),'malformed witness predicate hash')
 return value

def bind_predicate(source,authoritative_sha,edit):
 require(isinstance(authoritative_sha,str) and re.fullmatch('[0-9a-f]{64}',authoritative_sha),'malformed authoritative hash')
 node=predicate(ast.parse(source))
 require(node.value==OLD_SOURCE,'unexpected qualified-template witness predicate')
 return edit(source,[(node,repr(authoritative_sha))])

def validate(source,authority,phase):
 require(all(k in authority for k in PROVENANCE),'incomplete authoritative provenance')
 h=authority['source_boundary'];require(isinstance(h,str) and re.fullmatch('[0-9a-f]{64}',h),'malformed authoritative hash')
 t=ast.parse(source);allowed=set();pred=predicate(t);require(pred.value==h,'conflicting witness predicate');allowed.add(id(pred))
 ctxnode=assignment(t,'CONTEXT');ctx=ast.literal_eval(ctxnode)
 require(all(ctx.get(k)==authority[k] for k in PROVENANCE),'wrong/stale surrounding witness provenance')
 require(ast.literal_eval(assignment(t,'BOOT'))==authority['boot_id'],'boot/provenance mismatch')
 cnode=field(ctxnode,'source_boundary');require(isinstance(cnode,ast.Constant) and cnode.value==h,'missing/malformed/conflicting context witness');allowed.add(id(cnode))
 if phase in ('final','precheck','fire'):
  enode=assignment(t,'EXPECTED');expected=ast.literal_eval(enode)
  require(expected.get('boot_id')==authority['boot_id'],'stale protected receipt boot')
  observations=field(enode,'source_observations');require(isinstance(observations,ast.List) and len(observations.elts)==2,'two ordered before/after source observations required')
  for row in observations.elts:
   value=ast.literal_eval(row);node=field(row,'sha256');require(isinstance(node,ast.Constant) and node.value==h,'conflicting observation witness');allowed.add(id(node))
   try:raw=base64.b64decode(value['payload_base64'],validate=True)
   except Exception as e:raise ValueError('malformed source observation') from e
   require(hashlib.sha256(raw).hexdigest()==h,'observation bytes/hash mismatch')
   require(value.get('decoded',{}).get('supplied_record_consistency')=='CONFIRMED','source observation not valid')
  ops=field(enode,'operations');require(isinstance(ops,ast.List),'operations list required');created=[];verified=[]
  for row in ops.elts:
   value=ast.literal_eval(row)
   if value.get('name')=='source.preparation.original.txt':
    if value.get('phase')=='created':created.append(row)
    elif value.get('phase')=='verified':verified.append(row)
    else:raise ValueError('unexpected witness file lifecycle record')
  require(len(created)==len(verified)==1,'unique witness creation and verified file receipts required')
  row=verified[0];node=field(row,'sha256');require(isinstance(node,ast.Constant) and node.value==h,'conflicting persisted witness');allowed.add(id(node))
 if phase=='fire':
  cardnode=assignment(t,'CARD');card=ast.literal_eval(cardnode)
  require(card.get('boot_id')==authority['boot_id'],'stale execution card')
  witnessnode=field(cardnode,'witness');witness=ast.literal_eval(witnessnode)
  require(all(witness.get(k)==authority[k] for k in PROVENANCE),'execution-card witness provenance mismatch')
  node=field(witnessnode,'source_boundary');require(isinstance(node,ast.Constant) and node.value==h,'execution-card witness conflict');allowed.add(id(node))
 for node in ast.walk(t):
  if isinstance(node,ast.Constant) and node.value in (h,OLD_SOURCE):require(id(node) in allowed,'unexpected duplicate/stale witness reference')
 return {'classification':'STRUCTURAL_WITNESS_BINDING_PASS','phase':phase,'expected_hash_references':len(allowed),'same_authoritative_witness':True,'provenance':{k:authority[k] for k in PROVENANCE}}
