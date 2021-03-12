import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from math import fabs
import getdata

colours = {'rssi': 'grey', 'temperature_cansat':'orange', 'pressure_cansat': 'blue', 'temperature_ground': 'red', 'pressure_ground': 'darkblue', 'altitude': 'green', 'acceleration': 'yellow', 'speed': 'black'}
endings = {'rssi': 'dBm', 'temperature_cansat':'orange', 'pressure_cansat': 'blue', 'temperature_ground': 'red', 'pressure_ground': 'darkblue', 'altitude': 'green', 'acceleration': 'yellow', 'speed': 'black'}

def calculate_velocity (time_set, altitude_set):

    velocity = []

    for i in range (1, len(time_set)):
        if fabs(time_set[i] - time_set[i - 1]) != 0:
            v = fabs(altitude_set[i] - altitude_set[i - 1]) / fabs(time_set[i] - time_set[i - 1]) 
        else:
            v = -1
        velocity.append(v)

    velocity.insert(0, 0)

    return np.array(velocity)

def calculate_acceleration (time_set, velocity_set):

    acceleration = []

    for i in range (1, len(time_set)):
        if fabs(time_set[i] - time_set[i - 1]) != 0:
            a = fabs(velocity_set[i] - velocity_set[i - 1]) / fabs(time_set[i] - time_set[i - 1])
        else:
            a = -1
        acceleration.append(a)

    acceleration.insert(0, 0)

    return np.array(acceleration)

def gen_default_charts (df):

    fig, (ax0, ax1, ax2, ax3) = plt.subplots(nrows = 4, figsize = (15, 10))

    for column in df.columns.values.tolist():

        if column != 'time':

            y = df[column].values

            x = df['time'].values

            ax0.plot(x, y, label = column, color = colours[column])

    ax0.set_title('All measurments ordered by time of receiving package over time')
    ax0.set_ylim(auto=True)
    ax0.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    ax1.plot(df['time'].values, df['temperature_cansat'].values, label = 'temperature_cansat', color = colours['temperature_cansat'])
    ax1.plot(df['time'].values, df['temperature_ground'].values, label = 'temperature_ground', color = colours['temperature_ground'])

    ax1.set_title("Temp cansat and temp ground  over time")
    ax1.set_ylim(auto=True)

    ax1.set(ylabel = 'temperature [deg C]')
    ax1.set(xlabel = 'time [s]')

    ax1.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    ax2.plot(df['time'].values, df['pressure_cansat'].values, label = 'pressure_cansat', color = colours['pressure_cansat'])
    ax2.plot(df['time'].values, df['pressure_ground'].values, label = 'pressure_ground', color = colours['pressure_ground'])

    ax2.set_title("Press cansat and press ground over time")
    ax2.set_ylim(auto=True)

    ax2.set(ylabel = 'pressure [hPa]')
    ax2.set(xlabel = 'time [s]')

    ax2.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    ax3.plot(df['time'].values, df['speed'].values, label = 'speed [m/s]', color = colours['speed'])
    ax3.plot(df['time'].values, df['acceleration'].values, label = 'acceleration [m/s^2]', color = colours['acceleration'])
    ax3.plot(df['time'].values, df['altitude'].values, label = 'altitude [m]', color = colours['altitude'])

    ax3.set_title("Speed, high, altitude over time over time")
    ax3.set_ylim(auto=True)

    ax3.set(ylabel = 'value')
    ax3.set(xlabel = 'time [s]')

    ax3.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    fig.tight_layout()
    plt.savefig('default_chart.png')

def gen_any_charts (df):

    ile_wykresow = int(input("Podaj liczbę wykresów, które chcesz wygenerować: "))

    if ile_wykresow <= 0:
        return 

    fig, axes = plt.subplots(nrows=ile_wykresow, figsize = (5 * (ile_wykresow + 1), 5 * ile_wykresow))

    if ile_wykresow == 1:
        availbe = ['packet number', 'time', 'rssi', 'temperature_cansat', 'pressure_cansat', 'temperature_ground', 'pressure_ground', 'altitude', 'speed', 'acceleration']
        x_title = input(f'Podaj oś x (poziomą) dostępne: {availbe}: ')
        availbe.remove(x_title)
        if x_title != 'packet number':
            availbe.remove('packet number')
        y_title = input(f'Podaj oś y (poziomą) dostępne: {availbe}: ')

        if x_title == 'packet number':
            axes.plot(df.index.values.tolist(), df[y_title].values, label = y_title, color = colours[y_title])
        else:
            axes.plot(df[x_title].values, df[y_title].values, label = y_title, color = colours[y_title])

        axes.set_title(f'{y_title.title()} over {x_title.title()}')
        axes.set_ylim(auto=True)

        axes.set(ylabel = y_title)
        axes.set(xlabel = x_title)

        axes.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    else:

        for i in range (ile_wykresow):
            availbe = ['packet number', 'time', 'rssi', 'temperature_cansat', 'pressure_cansat', 'temperature_ground', 'pressure_ground', 'altitude', 'speed', 'acceleration']
            x_title = input(f'Podaj oś x (poziomą) dostępne: {availbe}: ')
            availbe.remove(x_title)
            if x_title != 'packet number':
                availbe.remove('packet number')
            y_title = input(f'Podaj oś y (poziomą) dostępne: {availbe}: ')

            if x_title == 'packet number':
                axes[i].plot(df.index.values.tolist(), df[y_title].values, label = y_title, color = colours[y_title])
            else:
                axes[i].plot(df[x_title].values, df[y_title].values, label = y_title, color = colours[y_title])

            axes[i].set_title(f'{y_title.title()} over {x_title.title()}')
            axes[i].set_ylim(auto=True)

            axes[i].set(ylabel = y_title)
            axes[i].set(xlabel = x_title)

            axes[i].legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    fig.tight_layout()
    plt.savefig(f'{y_title.title()} over {x_title.title()}.png')

structured_data = (getdata.load_data_from_sd_ground('xd.txt', 8))

print(structured_data)

v = calculate_velocity(structured_data[:, 0], structured_data[:, -1])

a = calculate_acceleration(structured_data[:, 0], v)

structured_data = np.hstack((structured_data, v.reshape((-1, 1)), a.reshape((-1, 1))))

df = pd.DataFrame(structured_data, columns=['time', 'rssi', 'temperature_cansat', 'pressure_cansat', 'temperature_ground', 'pressure_ground', 'altitude', 'speed', 'acceleration'])

gen_default_charts(df)

gen_any_charts(df)

print(df)