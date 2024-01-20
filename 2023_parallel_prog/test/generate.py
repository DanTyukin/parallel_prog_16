import numpy as np
from visualize import plot_wave
import utils


DEFAULT_VALUES = [500, 500, 0.1, 0.1, 0.5]


def generate_data(P, M, h, tau, a):
    l = M * h
    t = np.linspace(
        np.zeros((M,)),
        np.full((M,), P*tau),
        P,
        axis=0
    )
    x = np.linspace(
        np.zeros((P,)),
        np.full((P,), l),
        M,
        axis=1
    )
    return np.sin(np.pi * x / l)*np.sin(np.pi*a*t/l)

def generate_input(P, M, h, tau, a):
    l = h * M
    return (
        np.zeros((M,)),
        np.pi*a/l*np.sin(np.linspace(0, l, M)*np.pi/l),
        np.zeros((P,)),
        np.zeros((P,))
    )

if __name__ == "__main__":
    pass