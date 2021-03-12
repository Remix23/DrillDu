import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import getdata

colours = {'rssi': 'grey', 'temperature_cansat':'orange', 'pressure_cansat': 'blue', 'temperature_ground': 'red', 'pressure_ground': 'darkblue', 'altitude': 'green', 'acceleration': 'yellow', 'speed': 'black'}

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

structured_data = (getdata.load_data_from_sd_cansat('plot_can.txt', 4))

df = pd.DataFrame(structured_data, columns=['time', 'temperature_cansat', 'pressure_cansat'])

gen_default_charts(df)


print(df)