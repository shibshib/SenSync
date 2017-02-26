import numpy as np
import csv
import matplotlib.pyplot as plt
from scipy.signal import butter, lfilter, freqz

def butter_highpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='high', analog=False)
    return b, a

def butter_highpass_filter(data, cutoff, fs, order=5):
    b, a = butter_highpass(cutoff, fs, order=order)
    y = lfilter(b, a, data)
    return y

# Read passed accelerometer and key files
def read_files(acc, keys):
    acc_data = np.genfromtxt(acc, delimiter=',')
    key_data = np.genfromtxt(keys, delimiter=',')
    return (acc_data, key_data)

# Plot data passed to it
def plot_acc_data(data):
    time = [];
    magnitude = [];

    for d in data:
        time.append(d[0])
        magnitude.append(d[4])

    time = np.asarray(time)
    magnitude = np.asarray(magnitude)

    # interpolate time so we can get time elapsed as x-axis
    time = time - time[0]
    plt.plot(time, magnitude)

    # Grab gca object of plot so we can play with labels and limits
    axes = plt.gca()
    axes.set_xlabel('Seconds')
    axes.set_ylabel('Magnitude')

    plt.grid()
    plt.show()

def plot_key_data(data):
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

def plot_both_data(acc, keys):
    acc_time = [];
    key_time = [];

    magnitude = [];
    status = [];

    # Set up accelerometer data
    for a in acc:
        acc_time.append(a[0]/1000)
        magnitude.append(a[4])

    # Filter requirements.
    order = 6
    fs = 50.0       # sample rate, Hz
    cutoff = 14.0  # desired cutoff frequency of the filter, Hz
    magnitude = butter_highpass_filter(magnitude, cutoff, fs, order)


    acc_time = np.asarray(acc_time)
    magnitude = np.asarray(magnitude)
    magnitude = abs(magnitude)

    # Set up key data
    for k in keys:
        key_time.append(k[0])
        status.append(k[1])

    key_time = np.asarray(key_time)
    status = np.asarray(status)

    # Make up for different start times (for this experiment,
    # there is 4 seconds between each start time)
    diff = abs(key_time[0] - acc_time[0]);
    if key_time[0] > acc_time[0]:
        acc_time = acc_time + diff
    else:
        key_time = key_time + diff

    # Seconds elapsed
    key_time = key_time - key_time[0];
    acc_time = acc_time - acc_time[0];


    f, axarr = plt.subplots(2, sharex = True)
    axarr[0].plot(acc_time, magnitude)
    axarr[0].set_xlabel('Seconds Elapsed')
    axarr[0].set_ylabel('Magnitude')

    axarr[1].plot(key_time, status, 'r')
    axarr[1].set_xlabel('Seconds Elapsed')
    axarr[1].set_ylabel('Status')
    axarr[1].set_ylim([0,3])
    axarr[0].grid()
    axarr[1].grid()
    f.subplots_adjust(hspace=0)
    plt.setp([a.get_xticklabels() for a in f.axes[:-1]], visible=False)
    plt.show()



if __name__ == "__main__":
    acc, keys = read_files('data_files/accelerometer_50 Hz.csv', 'data_files/press_data_testing.csv');
    plot_both_data(acc, keys)
#    plot_acc_data(acc)
#    plot_key_data(keys)
