import csv
import numpy as np
import matplotlib.pyplot as plt

K = 500
M = 500
X = 2.0

def main():
    u = []
    u_single = []
    x_band = np.linspace(0.0, X, M + 1)
    with open('outmt.csv', newline='') as data_file:
        data_reader = csv.reader(data_file, quoting = csv.QUOTE_NONNUMERIC)
        for row in data_reader:
            u.append(row)
    
    with open('outout.csv', newline='') as data_file:
        data_reader = csv.reader(data_file, quoting = csv.QUOTE_NONNUMERIC)
        for row in data_reader:
            u_single.append(row)
    
    pl11 = plt.subplot(311)
    r = []
    for j in range(0, M+1):
        r.append(abs(u[1][j] - u_single[1][j]))
    #plt.plot(x_band, u[1][0:M+1])
    #plt.plot(x_band, u_single[1][0:M+1])
    plt.plot(x_band, r)
    plt.tick_params('x', labelbottom=False)
    plt.grid(True)
    pl12 = plt.subplot(312, sharex = pl11, sharey = pl11)
    r = []
    for j in range(0, M+1):
        r.append(abs(u[2][j] - u_single[2][j]))
    #plt.plot(x_band, u[K//4][0:M+1])
    #plt.plot(x_band, u_single[K//4][0:M+1])
    plt.plot(x_band, r)
    plt.tick_params('x', labelbottom=False)
    plt.grid(True)
    pl13 = plt.subplot(313, sharex = pl11, sharey = pl11)
    r = []
    for j in range(0, M+1):
        r.append(abs(u[3][j] - u_single[3][j]))
    #plt.plot(x_band, u[K//2][0:M+1])
    #plt.plot(x_band, u_single[K//2][0:M+1])
    plt.plot(x_band, r)
    plt.tick_params('x', labelsize=6)
    plt.grid(True)
    plt.show()
    
if __name__ == '__main__':
    main()
