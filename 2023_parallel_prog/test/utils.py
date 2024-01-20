import numpy as np
import sys
import re

def write_solution(data, filename):
    with open(filename, mode='w') as fout:
        fout.write('\n'.join(' '.join(str(x) for x in row) for row in data))

def parse_solution(full_text: str) -> np.ndarray:
    lines = [line for line in full_text.splitlines() 
             if line.strip() and line[0].isnumeric()]
    # m = re.search(r'((\d+\.\d+)( \d+\.\d+)*\n)+', full_text)
    # if not m:
        # raise ValueError("Not found array in stdin!")
    parsed_input = [[float(x) for x in line.strip().split()] for line in lines]
    return np.array(parsed_input)

def read_solution(filename):
    with open(filename) as fin: 
        return parse_solution(fin.read())

def read_solution_from_stdin():
    return parse_solution(sys.stdin.read())
    