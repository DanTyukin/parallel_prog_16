import sys
import itertools
import subprocess
import re

with sys.stdin as fin:
    repeat = int(fin.readline())
    Ms = [int(x) for x in fin.readline().split()]
    Ps = [int(x) for x in fin.readline().split()]
    tau = float(fin.readline())
    h = float(fin.readline())
    a = float(fin.readline())
    NPs = [int(x) for x in fin.readline().split()]

print("id,M,P,NP,user,system,elapsed")
 
run_id = 1
for M, P, NP, _ in itertools.product(Ms, Ps, NPs, range(repeat)):
    args = sys.argv[1:].copy()
    for i, ar in enumerate(args):
        if ar == "{NP}":
            args[i] = str(NP)
    sp = subprocess.Popen(["time", "-f", "%U %S %e"] + args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = sp.communicate(f"{M} {P} {tau} {h} {a}".encode())
    m = re.search(r'(\d+\.\d+) (\d+\.\d+) (\d+.\d+)', err.decode())
    user = m.group(1)
    system = m.group(2)
    elapsed = m.group(3)
    # print(out, err)
    # break
    print(run_id, M, P, NP, user, system, elapsed, sep=',')
    run_id += 1
