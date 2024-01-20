from generate import generate_data, DEFAULT_VALUES
import utils
import sys 
import numpy as np


def compare_data(filename=None):
    actual = generate_data(*DEFAULT_VALUES)
    if filename:
        got = utils.read_solution(filename)
    else:
        got = utils.read_solution_from_stdin()
    diff = np.abs(actual - got)
    # print("File:", filename if filename else "stdin")
    print("Average difference:", np.average(diff))
    print("Max difference:", np.max(diff))
    

if len(sys.argv) == 2:
    compare_data(sys.argv[1])
elif len(sys.argv) == 1:
    compare_data()
else:
    print("Too much argument")