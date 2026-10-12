"""Rebuild everything in order: parts -> checks -> plates -> renders.
python make_all.py            (takes 2-4 minutes)
"""
import subprocess
import sys

for step in (["momo_enclosures.py"], ["momo_hero.py"], ["checks.py"], ["plates.py"], ["render.py", "all"]):
    print(f"\n### python {' '.join(step)}", flush=True)
    r = subprocess.run([sys.executable] + step)
    if r.returncode:
        sys.exit(f"step {step[0]} failed")
print("\nDone: out/ (STEP + model STL), stl_print_ready/, plates/ (3MF), renders/ (PNG)")
