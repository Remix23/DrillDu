import numpy as np
import matplotlib.pyplot as plt
import seaborn as sn
from os.path import exists
import pandas as pd
from math import fabs

def calculate_velocity (time_set, altitude_set):

    velocity = []

    for i in range (1, len(time_set)):
        v = fabs(altitude_set[i] - altitude_set[i - 1]) / fabs(time_set[i] - time_set[i - 1]) 
        velocity.append(v)

    velocity.insert(0, 0)

    return np.array(velocity)

def calculate_acceleration (time_set, velocity_set):

    acceleration = []

    for i in range (1, len(time_set)):
        a = fabs(velocity_set[i] - velocity_set[i - 1]) / fabs(time_set[i] - time_set[i - 1])
        acceleration.append(a)

    acceleration.insert(0, 0)

    return np.array(acceleration)

def load_data_from_sd (filename):
    all_lines = []
    with open(filename) as f:
        for line in f:
            all_lines.append(line.rstrip())

    data = np.zeros((len(all_lines) // 7, 7), dtype=float)

    for i in range(0, len(all_lines), 8):
        t = float(all_lines[i])
        r = float(all_lines[i + 2][8: len(all_lines[i + 2])])
        tc = float(all_lines[i + 3][22: len(all_lines[i + 3]) - 6])
        pc = float(all_lines[i + 4][17: len(all_lines[i + 4]) - 4])
        tg = float(all_lines[i + 5][22: len(all_lines[i + 5]) - 6])
        pg = float(all_lines[i + 6][17: len(all_lines[i + 6]) - 4])
        a = float(all_lines[i + 7][11: len(all_lines[i + 7]) - 2])

        data[i // 8,0] = t
        data[i // 8,1] = r
        data[i // 8,2] = tc
        data[i // 8,3] = pc
        data[i // 8,4] = tg
        data[i // 8,5] = pg
        data[i // 8,6] = a

    return data

def gen_chart(df):

    # Data for plotting
    t = np.arange(0.0, 2.0, 0.01)
    s = 1 + np.sin(2 * np.pi * t)

    fig, ax = plt.subplots()
    ax.plot(t, s)

    ax.set(xlabel='time (s)', ylabel='voltage (mV)',
        title='About as simple as it gets, folks')
    ax.grid()

    fig.savefig("test.png")
    plt.show()



structured_data = (load_data_from_sd('test.txt'))

v = calculate_velocity(structured_data[:, 0], structured_data[:, -1])

a = calculate_acceleration(structured_data[:, 0], v)

structured_data = np.hstack((structured_data, v.reshape(5, 1), a.reshape(5, 1)))

df = pd.DataFrame(structured_data, columns=['time', 'rssi', 'temperature_cansat', 'pressure_cansat', 'temperature_ground', 'pressure_ground', 'alti', 'speed', 'acceleration'])   

print(df)
