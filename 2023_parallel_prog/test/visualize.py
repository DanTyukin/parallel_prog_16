import numpy as np
from matplotlib import pyplot as plt
from matplotlib.animation import FuncAnimation

def plot_wave(data, h, filename):
    P, M = data.shape
    l = h*M

    plt.style.use('seaborn-pastel')

    fig = plt.figure()
    ax = plt.axes(xlim=(0, l), ylim=(np.min(data), np.max(data)))
    line, = ax.plot([], [], lw=3)
    x = np.linspace(0, l, M)

    def init():
        line.set_data([], [])
        return line,
    def animate(i):
        line.set_data(x, data[i,])
        return line,

    anim = FuncAnimation(fig, animate, init_func=init,
                               frames=P, interval=50, blit=True)

    anim.save(filename, writer='imagemagick')