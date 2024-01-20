import numpy as np
import sys

sys.path.append("test")
import utils


def solve(
    P: int,     # number of steps for time axis
    M: int,     # number of steps for x axis
    tau: float, # step for time axis
    h: float,   # step for x axis
    a: float,   # 1 / v
    phi1: np.ndarray, # u(x, 0):
    phi2: np.ndarray, # du/dt(x, 0)
    psi1: np.ndarray, # u(0, t)
    psi2: np.ndarray  # u(l, t)
):
    u = np.zeros((P, M))
    u[0, 1 : M-1] = phi1[1 : M-1]
    c2 = (tau*a/h)**2
    u[1, 1 : M-1] = phi1[1 : M-1] + tau * phi2[1 : M-1] \
        + c2 / 2 * (phi1[2:M] - 2 * phi1[1 : M-1] + phi1[0:M-2])
    u[: , 0] = psi1
    u[: , M-1] = psi2
    for p in range(1, P-1):
        u[p+1, 1 : M-1] = 2*u[p, 1:M-1] - u[p-1, 1:M-1] + \
            c2 * (u[p, 0:M-2] - 2*u[p, 1:M-1] + u[p, 2:M])
    return u

if __name__ == "__main__":
    P = 500
    M = 500
    h = 0.1
    tau = 0.1
    a = 0.5
    l = M * h
    res = solve(
        P, M, tau, h, a, 
        np.zeros((M,)),
        np.pi*a/l*np.sin(np.linspace(0, l, M)*np.pi/l),
        np.zeros((P,)),
        np.zeros((P,))
    )
    utils.write_solution(res, "solution/numpy/res.txt") 