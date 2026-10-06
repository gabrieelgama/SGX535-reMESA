import subprocess,json
p=subprocess.run(['python3', '-B', '/home/gama/sgx535-square-display-20261006T090214Z-tools/square_display_centered.py', '--inspect'],capture_output=True,timeout=30)
print(p.stdout.decode(),end="")
if p.returncode or p.stderr:raise RuntimeError(p.stderr.decode())
