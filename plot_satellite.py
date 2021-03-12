import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from math import fabs

colours = {'rssi': 'grey', 'temperature_cansat':'orange', 'pressure_cansat': 'blue', 'temperature_ground': 'red', 'pressure_ground': 'darkblue', 'altitude': 'green', 'acceleration': 'yellow', 'speed': 'black'}

lines_of_data = 4

def load_data_from_sd (filename):
    all_lines = []
    with open(filename) as f:
        for line in f:
            all_lines.append(line.rstrip())

    data = np.zeros((len(all_lines) // lines_of_data, 3), dtype=float)

    for i in range(0, len(all_lines), lines_of_data):

        try:
            t = float(all_lines[i][19: len(all_lines[i])]) / 1000
            tc = float(all_lines[i + 2][21: len(all_lines[i + 2]) - 6])
            pc = float(all_lines[i + 3][17: len(all_lines[i + 3]) - 4])
            data[i // lines_of_data, 0] = t
            data[i // lines_of_data, 1] = tc
            data[i // lines_of_data, 2] = pc
        except Exception as e:
            data[i // lines_of_data,0] = -1
            data[i // lines_of_data,1] = -1
            data[i // lines_of_data,2] = -1

    return data

def gen_default_charts (df):

    fig, (ax0, ax1) = plt.subplots(nrows = 2, figsize = (8, 5))

    ax0.plot(df.index.values.tolist(), df['temperature_cansat'].values, label = 'temperature_cansat', color = colours['temperature_cansat'])

    ax0.set_title("Temp cansat")
    ax0.set_ylim(auto=True)

    ax0.set(ylabel = 'temperature [deg C]')
    ax0.set(xlabel = 'packet nr')

    ax0.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    ax1.plot(df.index.values.tolist(), df['pressure_cansat'].values, label = 'pressure_cansat', color = colours['pressure_cansat'])

    ax1.set_title("Pressure cansat")
    ax1.set_ylim(auto=True)

    ax1.set(ylabel = 'pressure [HPa]')
    ax1.set(xlabel = 'packet nr')

    ax1.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)

    fig.tight_layout()
    plt.savefig('default_chart.png')

def gen_any_charts (df):

    ile_wykresow = int(input("Podaj liczbę wykresów, które chcesz wygenerować: "))

    if ile_wykresow <= 0:
        return 

    fig, axes = plt.subplots(nrows=ile_wykresow, figsize = (5 * (ile_wykresow + 1), 5 * ile_wykresow))

    if ile_wykresow == 1:
        availbe = ['packet number', 'time', 'temperature_cansat', 'pressure_cansat']
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
            availbe = ['packet number', 'time', 'temperature_cansat', 'pressure_cansat']
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

structured_data = (load_data_from_sd('plot_can.txt'))

df = pd.DataFrame(structured_data, columns=['time', 'temperature_cansat', 'pressure_cansat'])

gen_default_charts(df)


print(df)