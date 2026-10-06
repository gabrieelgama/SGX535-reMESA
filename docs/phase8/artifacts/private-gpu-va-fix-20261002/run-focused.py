from pathlib import Path
import re,subprocess,sys,json
r=Path('/home/gama/sgx535-gfx');e=Path(__file__).parent
source=(r/'kernel/sgx535_frozen/gma500_bo_owner.c').read_text()
flags={name:re.search(pattern,source)[1] for name,pattern in [('ROOT_FLAGS',r'owner->va_root.flags = ([^;]+);'),('EXCLUSION_FLAGS',r'owner->gtt_exclusion.flags = ([^;]+);'),('BO_FLAGS',r'bo->va.flags = ([^;]+);')]}
probe=(e/'allocator-probe-template.c').read_text()
for k,v in flags.items():probe=probe.replace(k,v)
label=sys.argv[1];(e/(label+'.c')).write_text(probe)
cp=subprocess.run(['/usr/bin/gcc-14','-std=c11','-Wall','-Wextra','-Wno-unused-parameter','-fsanitize=undefined','-fno-sanitize-recover=all',str(e/(label+'.c')),'-o','/tmp/sgx535-va-focused-probe'],capture_output=True,text=True)
assert cp.returncode==0,cp.stderr
cp=subprocess.run(['/tmp/sgx535-va-focused-probe'],capture_output=True,text=True)
(e/(label+'.stdout')).write_text(cp.stdout);(e/(label+'.stderr')).write_text(cp.stderr)
(e/(label+'.json')).write_text(json.dumps({'source_flags':flags,'exit_code':cp.returncode,'stdout':cp.stdout,'stderr':cp.stderr},indent=2)+'\n')
print(label,cp.returncode,cp.stdout);sys.exit(cp.returncode)
