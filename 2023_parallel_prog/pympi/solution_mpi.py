from mpi4py import MPI
from dataclasses import dataclass
import numpy as np
from typing import Tuple
import sys

TAG = 1
COMM = MPI.COMM_WORLD
SPEED_TEST = False

rank = COMM.Get_rank()
number_of_processes = COMM.Get_size()
pars = None

@dataclass
class Parameters:
    M: int
    P: int 
    tau: float
    h: float 
    a: float


@dataclass
class Edge:
    phi1: np.ndarray
    phi2: np.ndarray
    psi1: np.ndarray
    psi2: np.ndarray


def read_from_stdin() -> Tuple[Parameters, Edge]:
    tokens = sys.stdin.read().split()
    if len(tokens) < 5:
        raise ValueError("Too few parameters, expected at least five")
    pars = Parameters(
        *[t(x) for x, t in zip(tokens[:5], [int, int, float, float, float])]
    )
    if SPEED_TEST:
        l = pars.M * pars.h
        edge = Edge(
            np.zeros((pars.M,)),
            np.pi*pars.a/l*np.sin(np.linspace(0, l, pars.M)*np.pi/l),
            np.zeros((pars.P,)),
            np.zeros((pars.P,))
        )
        return pars, edge
    if len(tokens) < 5 + pars.M * 2 + pars.P * 2:
        print(len(tokens), tokens)
        raise ValueError("Cannot read edge parameters: too few inputs")
    edge = Edge(
        np.array([float(x) for x in tokens[5: 5 + pars.M]]),
        np.array([float(x) for x in tokens[5 + pars.M: 5 + pars.M * 2]]),
        [float(x) for x in tokens[5 + pars.M*2: 5 + pars.M*2 + pars.P]],
        [float(x)
         for x in tokens[5 + pars.M*2 + pars.P: 5 + pars.M*2 + pars.P*2]]
    )
    return pars, edge


def exchange(left: float, right: float) -> Tuple[float, float]:
    if number_of_processes == 1:
        return right, left 
    left_n = rank - 1 if rank > 0 else number_of_processes - 1
    right_n = (rank + 1) % number_of_processes
    new_right = COMM.sendrecv(left, left_n, source=right_n)
    new_left = COMM.sendrecv(right, right_n, source=left_n)
    return new_left, new_right

def second_step(extended_row: np.ndarray, prev_row: np.ndarray) -> np.ndarray:
    c2 = (pars.a * pars.tau / pars.h) ** 2
    return (1 - c2) * extended_row[1:-1] + \
        c2 / 2 * (extended_row[0:-2] + extended_row[2:]) + \
            prev_row * pars.tau


def iteration(extended_row: np.ndarray, prev_row: np.ndarray) -> np.ndarray:
    c2 = (pars.a * pars.tau / pars.h) ** 2
    return (2 - 2 * c2) * extended_row[1:-1] - prev_row + c2 * (extended_row[0:-2] + extended_row[2:])


def calculate_row(result: np.ndarray, i: int, func, prev_row: np.ndarray, edge: Edge):
    left, right = exchange(result[i,1] if rank > 0 else edge.psi2[i], result[i,-2])
    if rank == 0:
        left = edge.psi1[i]
    result[i, 0] = left
    result[i, -1] = right
    result[i + 1, 1:-1] = func(result[i,:], prev_row)


def main():
    global rank, pars, number_of_processes
    if rank == 0:
        pars, edge = read_from_stdin()
        # print(rank, pars, edge)
    else:
        edge = None
    pars = COMM.bcast(pars, 0)
    k = max(1 << k for k in range(16) if 1 << k < number_of_processes) \
        if number_of_processes > 1 else 0
    if rank == 0:
        buffer = (edge.phi1[1:-1], edge.phi2[1:-1])
        size = pars.M - 2
    part_size = (pars.M - 2) // number_of_processes
    while k > 0:
        for i in range(0, number_of_processes - k, 2 * k):
            to_send = min(k, number_of_processes - (i + k)) * part_size
            if rank == i:
                COMM.send((buffer[0][size-to_send:size],
                          buffer[1][size-to_send:size]), i + k)
                size -= to_send
            if rank == i + k:
                buffer = COMM.recv(source=i)
                size = to_send
        k //= 2
    result = np.zeros((pars.P, size + 2))
    result[0, 1:-1] = buffer[0][:size]
    calculate_row(result, 0, second_step, buffer[1][:size], edge)
    for p in range(1, pars.P - 1):
        calculate_row(result, p, iteration, result[p - 1, 1:-1], edge)
    final_result = [result[:,m] for m in range(1, size + 1)]
    k = 1
    while(k < number_of_processes):
        for i in range(0, number_of_processes - k, 2*k):
            if rank == i:
                final_result += COMM.recv(source=i + k)
            if rank == i + k:
                COMM.send(final_result, i)
        k *= 2
    if rank == 0 and not SPEED_TEST:
        for row in zip(edge.psi1, *final_result, edge.psi2):
            print(*row)


if __name__ == "__main__":
    if len(sys.argv) == 2 and sys.argv[1] == "SPEED_TEST":
        SPEED_TEST = True 
    main()


