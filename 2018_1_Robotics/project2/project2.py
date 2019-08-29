import os
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from matplotlib.patches import FancyArrowPatch
from mpl_toolkits.mplot3d import proj3d


class Arrow3D(FancyArrowPatch):
    def __init__(self, xs, ys, zs, *args, **kwargs):
        FancyArrowPatch.__init__(self, (0, 0), (0, 0), *args, **kwargs)
        self._verts3d = xs, ys, zs

    def draw(self, renderer):
        xs3d, ys3d, zs3d = self._verts3d
        xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, renderer.M)
        self.set_positions((xs[0], ys[0]), (xs[1], ys[1]))
        FancyArrowPatch.draw(self, renderer)


def kinematic(ths, a, d, alpha):
    cos = np.cos
    sin = np.sin
    A = []
    for th in ths:
        T6 = np.identity(4)
        for i in range(6):
            T = np.identity(4)
            T[0, 0] = cos(th[i])
            T[0, 1] = -sin(th[i]) * cos(alpha[i])
            T[0, 2] = sin(th[i]) * sin(alpha[i])
            T[0, 3] = a[i] * cos(th[i])
            T[1, 0] = sin(th[i])
            T[1, 1] = cos(th[i]) * cos(alpha[i])
            T[1, 2] = -cos(th[i]) * sin(alpha[i])
            T[1, 3] = a[i] * sin(th[i])
            T[2, 1] = sin(alpha[i])
            T[2, 2] = cos(alpha[i])
            T[2, 3] = d[i]
            T6 = T6.dot(T)
        A.append(T6)
    return A


def inverse_kinematic():
    os.system('g++ project1.cc -std=c++11 -I/usr/include/eigen3 -o project1')
    os.system('./project1 1 0,-1,0,0,0,-1,1,0,0,0.1,0.5,0.3')
    os.system('./project1 1 0,0,-1,0,1,0,1,0,0,0.3,0.3,0.2')
    os.system('./project1 1 0,0,-1,1,0,0,0,-1,0,-0.3,-0.25,0.25')
    print('Use Joint Variables Solution:\n'
          '    A: 78.6901, -21.9802, -30.5766, 52.5567, 168.69, 0\n'
          '    B: 45, 12.2384, -88.9084, 76.67, 135, 90\n'
          '    C: -140.194, 2.1035, -87.5413, 85.4378, 39.8056, 90')
    A = np.array([78.6901, -21.9802, -30.5766, 52.5567, 168.69, 0])
    B = np.array([45, 12.2384, -88.9084, 76.67, 135, 90])
    C = np.array([-140.194, 2.1035, -87.5413, 85.4378, 39.8056, 90])
    return np.deg2rad(A), np.deg2rad(B), np.deg2rad(C)


def path_planning(A, B, C):
    t = np.arange(-0.5, 0.5002, 0.002)
    th = np.zeros((t.shape[0], A.shape[0]))
    vel = np.zeros((t.shape[0], A.shape[0]))
    acc = np.zeros((t.shape[0], A.shape[0]))
    dC = C - B
    dB = (A - B) * 2 / 5
    tacc = 0.2
    T = 0.5
    for idx, dt in enumerate(t):
        if (dt >= -tacc and dt <= tacc):
            h = (dt + tacc) / (2 * tacc)
            tmp = dC * tacc / T + dB
            th[idx] = tmp * (2 - h) * (h ** 3) + B + (1 - 2 * h) * dB
            vel[idx] = (tmp * (3 - 2 * h) * (h ** 2) - dB) / tacc
            acc[idx] = tmp * (1 - h) * 3 * h / (tacc ** 2)
        elif (dt < -tacc):
            th[idx] = (B - A) / T * dt + B
            vel[idx] = (B - A) / T
            acc[idx] = 0
        else:
            h = dt / T
            th[idx] = dC * h + B
            vel[idx] = dC / T
            acc[idx] = 0
    return t, th, vel, acc


def plot_param(t, th, vel, acc, title):
    fig = plt.figure(title, figsize=(12, 10))
    fig.suptitle(title, fontsize=16)
    xyz = ['x', 'y', 'z']
    pva = ['position', 'velocity', 'acceration']
    for i in range(th.shape[1]):
        for j in range(3):
            plt.subplot(th.shape[1], 3, i * 3 + j + 1)
            if (title == 'Joint Motion'):
                plt.title('joint %d: %s' % (i + 1, pva[j]))
            else:
                plt.title('%s: %s' % (xyz[i], pva[j]))
            if (j == 0):
                plt.plot(t, th[:, i])
            elif (j == 1):
                plt.plot(t, vel[:, i])
            else:
                plt.plot(t, acc[:, i])
    plt.subplots_adjust(hspace=1)


def plot_3dpath(title, points, TA, TB, TC):
    A = TA[:3, 3]
    B = TB[:3, 3]
    C = TC[:3, 3]
    AA = np.c_[A, A, A] + 0.1 * np.array(TA[:3, :3])
    BB = np.c_[B, B, B] + 0.1 * np.array(TB[:3, :3])
    CC = np.c_[C, C, C] + 0.1 * np.array(TC[:3, :3])
    fig = plt.figure(title)
    ax = Axes3D(fig)
    ax.set_title(title)
    ax.set_xlim(-0.4, 0.4)
    ax.set_ylim(-0.4, 0.6)
    ax.set_zlim(0.1, 0.3)
    ax.set_xlabel('X-axis (m)')
    ax.set_ylabel('Y-axis (m)')
    ax.set_zlabel('Z-axis (m)')
    x = [p[0, 3] for p in points]
    y = [p[1, 3] for p in points]
    z = [p[2, 3] for p in points]
    xx = [p[0, 3] + 0.05 * p[0, 2] for p in points]
    yy = [p[1, 3] + 0.05 * p[1, 2] for p in points]
    zz = [p[2, 3] + 0.05 * p[2, 2] for p in points]
    ax.plot(x[:150], y[:150], z[:150], color='r')
    ax.plot(x[150:350], y[150:350], z[150:350], color='g')
    ax.plot(x[350:], y[350:], z[350:], color='b')
    ax.plot([A[0], B[0]], [A[1], B[1]], [A[2], B[2]],
            linestyle='--', color='k')
    ax.plot([B[0], C[0]], [B[1], C[1]], [B[2], C[2]],
            linestyle='--', color='k')
    ax.text(A[0], A[1], A[2],
            'A(%.2f, %.2f, %.2f)' % (A[0], A[1], A[2]))
    ax.text(B[0], B[1], B[2],
            'B(%.2f, %.2f, %.2f)' % (B[0], B[1], B[2]))
    ax.text(C[0], C[1], C[2],
            'C(%.2f, %.2f, %.2f)' % (C[0], C[1], C[2]))
    ax.text(x[149], y[149], z[149],
            'A\'(%.3f, %.3f, %.3f)' % (x[150], y[150], z[150]))
    ax.text(x[349], y[349], z[349],
            'B\'(%.3f, %.3f, %.3f)' % (x[350], y[350], z[350]))
    color = ['r', 'g', 'b']
    for t, tt in zip([A, B, C], [AA, BB, CC]):
        for i in range(3):
            ax.add_artist(
                Arrow3D([t[0], tt[0, i]], [t[1], tt[1, i]], [t[2], tt[2, i]],
                        mutation_scale=15, lw=2,
                        arrowstyle='-|>', color=color[i]))
    for i in range(len(x)):
        if (i % 5 == 0 and title == '3D Path of Joint Motion'):
            ax.add_artist(
                Arrow3D([x[i], xx[i]], [y[i], yy[i]], [z[i], zz[i]],
                        mutation_scale=5, lw=2,
                        arrowstyle='-|>', color='k'))


if __name__ == '__main__':
    # Robot Parameters
    dh_a = np.array([0.12, 0.25, 0.26, 0, 0, 0])
    dh_d = np.zeros(6)
    dh_alpha = np.deg2rad(np.array([-90, 0, 0, -90, 90, 0]))

    # Matrix of A, B and C
    TA = np.array([[0, 0, 1, 0.1],
                   [-1, 0, 0, 0.5],
                   [0, -1, 0, 0.3],
                   [0, 0, 0, 1]])
    TB = np.array([[0, 0, 1, 0.3],
                   [0, 1, 0, 0.3],
                   [-1, 0, 0, 0.2],
                   [0, 0, 0, 1]])
    TC = np.array([[0, 1, 0, -0.3],
                   [0, 0, -1, -0.25],
                   [-1, 0, 0, 0.25],
                   [0, 0, 0, 1]])

    # Joint Move
    A, B, C = inverse_kinematic()
    t, th, vel, acc = path_planning(A, B, C)
    plot_param(t, th, vel, acc, 'Joint Motion')
    cartesian_points = kinematic(th, dh_a, dh_d, dh_alpha)
    plot_3dpath('3D Path of Joint Motion', cartesian_points, TA, TB, TC)

    # Cartesian Move
    A = np.array([0.1, 0.5, 0.3])
    B = np.array([0.3, 0.3, 0.2])
    C = np.array([-0.3, -0.25, 0.25])
    t, pos, vel, acc = path_planning(A, B, C)
    plot_param(t, pos, vel, acc, 'Cartesian Motion')
    cartesian_points = []
    for p in pos:
        tmp = np.identity(4)
        tmp[:3, 3] = p
        cartesian_points.append(tmp)
    plot_3dpath('3D Path of Cartesian Motion', cartesian_points, TA, TB, TC)
    plt.show()
