import subprocess,json
p=subprocess.run(['python3', '-B', '/home/gama/sgx535-square-display-20261006T090214Z-tools/square_display_centered.py', '--publish-once', '--card', '/home/gama/sgx535-square-display-20261006T090214Z-tools/card.json', '--authorization', '/home/gama/sgx535-square-display-20261006T090214Z-tools/authorization.json', '--source', '/home/gama/sgx535-square-display-20261006T090214Z-tools/source.bin', '--evidence', '/home/gama/sgx535-square-display-20261006T090214Z-evidence'],capture_output=True,timeout=35)
print(json.dumps({"exit_code":p.returncode,"stdout":p.stdout.decode(),"stderr":p.stderr.decode(),"sgx_invocations":0,"maximum_publications":1}),flush=True)
if p.returncode or p.stderr:raise SystemExit(1)
