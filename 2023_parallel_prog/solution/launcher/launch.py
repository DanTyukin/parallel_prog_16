import argparse
import subprocess
from typing import Dict
import os
from os import path
import datetime
import sys
import re
import time

PARTITIONS = ["tornado", "tornado-k40", "cascade"]

FILENAME = f"launch-{datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')}"


def check_scc() -> bool:
    try:
        subprocess.call(["sbatch", "--version"], stdout=subprocess.DEVNULL)
    except:
        return False
    return True


def idle_nodes() -> Dict[str, int]:
    p = subprocess.Popen(
        ["sinfo", "-o", "%R %D", "-t", "idle", "-h"],
        stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    out, _ = p.communicate()
    result: Dict[str, int] = {}
    for line in out.decode().splitlines():
        partition, n_idle = line.split()
        result[partition] = int(n_idle)
    for p in PARTITIONS:
        if p not in result:
            result[p] = 0
    return result


def wait_for_job(job_id: str):
    while True:
        time.sleep(5)
        p = subprocess.Popen(["squeue"], stdout=subprocess.PIPE)
        out, err = p.communicate()
        if job_id not in out.decode():
            return


parser = argparse.ArgumentParser(
    prog="launcher",
    description="Launches different implementation",)
parser.add_argument(
    "-p", "--partition",
    choices=PARTITIONS, default=PARTITIONS[0],
    help="Choose partition, default: tornado")
parser.add_argument(
    "-f", "--free-partition", action="store_true",
    help="Switch to free partition if requested partition is not available")
parser.add_argument(
    "-o", "--stdout",
    default=FILENAME + ".out",
    help="File for stdout of slurm")
parser.add_argument(
    "-e", "--stderr",
    default=FILENAME + ".err",
    help="File for stdin of slurm")
parser.add_argument(
    "-i", "--stdin", help="For 'run' option read from given file name")
parser.add_argument(
    "-c", "--console", action="store_true",
    help="Will wait for process to end and print everything in console")
parser.add_argument(
    "-t", "--time", action="store_true",
    help="Measure runtime")
parser.add_argument(
    "-n", "--number-of-processes", type=int, default=4,
    help="Number of processes for running program")
parser.add_argument(
    "action", choices=["rebuild", "test", "run", "speedtest", "speedtest2", "speedtest5"],
    help="Action of program")
parser.add_argument(
    "implementation", choices=["all", "pthreads", "cmpi", "openmp", "pympi"],
    help="Implementation to run program")

args = parser.parse_args()

stdout_abspath = path.abspath(args.stdout)
stderr_abspath = path.abspath(args.stderr)
stdin_abspath = path.abspath(args.stdin or (FILENAME + ".in"))
has_input_file = args.stdin is not None

os.chdir(path.dirname(__file__))
os.chdir("..")

if check_scc():
    requested_nodes = 1 if args.action in ["rebuild", "test", "run", "speedtest"] \
        else 2 if args.action == "speedtest2" else 5
    idle_nodes_map = idle_nodes()
    if idle_nodes_map.get(args.partition, 0) < requested_nodes:
        print(f"Not enough node in {args.partition} partition")
        if args.free_partition:
            for partition, nodes in idle_nodes_map.items():
                if nodes >= requested_nodes:
                    print(
                        f"Partition {partition} has {nodes} idle nodes, switching to it")
                    args.partition = partition
                    break
            else:
                print("No other partition has available nodes, exiting")
                sys.exit("No available nodes")
        else:
            sys.exit("No available nodes")
    if args.action in ["speedtest2", "speedtest5"] \
            and args.implementation not in ["cmpi", "pympi"]:
        print(
            f"Action {args.action} is not available for {args.implementation}")
        sys.exit("Invalid action")

    script_args = ["sbatch",
                   "-J", f"LabPP-{args.action}-{args.implementation}",
                   "-p", args.partition,
                   "-N", str(requested_nodes),
                   "-o", stdout_abspath,
                   "-e", stderr_abspath,
                   "./launcher/slurm.sh"] + (["-T"] if args.time else []) + [
        args.implementation,
        args.action,
    ]
    if args.action == "run":
        input_file = args.stdin
        if not has_input_file:
            print("Input parameters (Ctl+D for end):")
            with open(stdin_abspath, "w") as fout:
                fout.write(sys.stdin.read())
        script_args += [stdin_abspath, str(args.number_of_processes)]
    r = subprocess.Popen(
        script_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = r.communicate()
    m = re.search(r'job (\d+)', out.decode())
    if not m:
        print("Cannot get job id, exiting")
        sys.exit("Job id error")
    job_id = m.group(1)
    print(f"Started job {job_id}")
    if args.console:
        print(f"Waiting for job to finish")
        wait_for_job(job_id)
        print("\nSTDOUT\n")
        with open(stdout_abspath, 'r') as fin:
            print(fin.read())
        print("\nSTDERR\n")
        with open(stderr_abspath, 'r') as fin:
            print(fin.read())
        os.remove(stdout_abspath)
        os.remove(stderr_abspath)
    if args.action == "run" and not has_input_file:
        os.remove(stdin_abspath)
else:
    if args.action in ["speedtest2", "speedtest5"]:
        print("This action is not supported!")
        sys.exit("Invalid action")
    script_args = ["./launcher/runner.sh", args.implementation, args.action]
    if args.time:
        script_args.insert(1, "-T")
    if args.action == "run":
        input_file = args.stdin
        if not has_input_file:
            print("Input parameters (Ctl+D for end):")
            with open(stdin_abspath, "w") as fout:
                fout.write(sys.stdin.read())
        script_args += [stdin_abspath, str(args.number_of_processes)]
    subprocess.run(script_args)
    if args.action == "run" and not has_input_file:
        os.remove(stdin_abspath)
