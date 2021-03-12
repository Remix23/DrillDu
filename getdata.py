import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from math import fabs

rules = {
    'can' : [
        (19, 0),
        (None, None),
        (21, 6),
        (17, 4)
    ],
    'ground' : [
        (19, 0),
        (None, None),
        (8, 0),
        (21, 6),
        (17, 4),
        (21, 6),
        (17, 4),
        (11, 2)
    ]
}

def average (x, y):
    return (x + y) / 2

def merge (log_can, log_ground, lines_of_data_can, lines_of_data_ground):
    pass

def load_data_from_sd_cansat (filename, lines_of_data):

    all_lines = []
    with open(filename) as f:
        for line in f:
            all_lines.append(line.rstrip())

    data = np.zeros((len(all_lines) // lines_of_data, lines_of_data - 1), dtype=float)

    temp = []

    for i in range(0, len(all_lines), lines_of_data):

        temp.clear()

        try:
            for j in range (lines_of_data):
                if j != 1:
                    if j > 1:
                        data[i // lines_of_data, j - 1] = float(all_lines[i + j][rules['can'][j][0]: len(all_lines[i + j]) - rules['can'][j][1]])
                    else:
                        data[i // lines_of_data, j] = float(all_lines[i + j][rules['can'][j][0]: len(all_lines[i + j]) - rules['can'][j][1]])

        except Exception as e:
            for j in range (lines_of_data):
                if j != 1:
                    if j > 1:
                        data[i // lines_of_data, j - 1] = -1
                    else:
                        data[i // lines_of_data, j] = -1

    return data

def load_data_from_sd_ground (filename, lines_of_data):
    all_lines = []
    with open(filename) as f:
        for line in f:
            all_lines.append(line.rstrip())

    data = np.zeros((len(all_lines) // lines_of_data, lines_of_data - 1), dtype=float)
    temp = []

    for i in range(0, len(all_lines), lines_of_data):

        temp.clear()

        try:
            for j in range (lines_of_data):
                if j != 1:
                    if j > 1:
                        data[i // lines_of_data, j - 1] = float(all_lines[i + j][rules['ground'][j][0]: len(all_lines[i + j]) - rules['ground'][j][1]])
                    else:
                        data[i // lines_of_data, j] = float(all_lines[i + j][rules['ground'][j][0]: len(all_lines[i + j]) - rules['ground'][j][1]])

        except Exception as e:
            for j in range (lines_of_data):
                if j != 1:
                    if j > 1:
                        data[i // lines_of_data, j - 1] = -1
                    else:
                        data[i // lines_of_data, j] = -1
        
    return data