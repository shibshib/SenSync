import pygame
import sys
import time
import numpy as np
import matplotlib.pyplot as plt

def collectKeyPresses():
    data = []
    end_function = False
    pygame.key.set_repeat(1, 10)
    key_down = False;
    while end_function is False:
        if key_down is False:
            ts = time.time()
            point = [ts, 1]
            data.append(point)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                end_function = True

            if event.type == pygame.KEYDOWN:        # Key is pressed
                ts = time.time()
                point = [ts, 2]
                data.append(point)
            #    print('Key Pressed!')
                key_down = True
                window.fill(RED)
                pygame.display.flip()
            elif event.type == pygame.KEYUP:        # Key is released
                ts = time.time()
                point = [ts, 1]
                data.append(point)
            #    print('Key Released!')
                key_down = False;
                window.fill(WHITE);
                pygame.display.flip()

        time.sleep(0.02);   # might need fine tuning
    pygame.quit()
    data = np.asarray(data);
    return data

def plot_data(data):
    time = [];
    status = [];

    for d in data:
        time.append(d[0])
        status.append(d[1])

    time = np.asarray(time)
    status = np.asarray(status)

    # interpolate time so we can get time elapsed as x-axis
    time = time - time[0]
    plt.plot(time, status)

    # Grab gca object of plot so we can play with labels and limits
    axes = plt.gca()
    axes.set_ylim([0,3])
    axes.set_xlabel('Seconds')
    axes.set_ylabel('Status (2 - Pressed, 1 - Not Pressed)')

    plt.grid()
    plt.show()

def save_data (data):
    fid = open("data_files/press_data_testing3.csv","w+")
    for d in data:
        fid.write(str(d[0]) + ',' + str(d[1]) + '\r\n')
    fid.close();

if __name__ == "__main__":
    pygame.init()

    HEIGHT = 500;
    WIDTH  = 500;
    window = pygame.display.set_mode((WIDTH, HEIGHT))
    WHITE = (255,255,255);
    RED   = (255, 0, 0);
    window.fill(WHITE);

    key_press_data = collectKeyPresses()
#    plot_data(key_press_data)
    save_data(key_press_data)
# def plotData(data):
