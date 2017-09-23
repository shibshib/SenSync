import numpy as np
import sys
import AccelerometerFeatures as af
from scipy.signal import butter, lfilter, freqz
from sklearn.svm import SVC
from sklearn.metrics import classification_report

# Read passed accelerometer and key files
def read_file(acc):
    acc_data = np.genfromtxt(acc, delimiter=',')
    return acc_data

# Butterworth High pass filter
def butter_highpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = butter(order, normal_cutoff, btype='high', analog=False)
    return b, a

def butter_highpass_filter(data, cutoff, fs, order=5):
    b, a = butter_highpass(cutoff, fs, order=order)
    y = lfilter(b, a, data)
    return y


# Extracts labels from key press data
# INPUT: key press data from data
# OUTPUT: label timings array
def extract_labels(data):
    timings = [];
    start_time = 0
    for i in range(0, len(data)):
        seccount = seccount + 1;
        if data[i][1] == 2 and start_time == 0:     # we reached a 1 after 0
            start_time = data[i,0]*1000
        if data[i][1] == 1 and start_time > 0:     # we hit a 0 after 1
            timings.append([start_time, data[i-1,0]*1000])
            start_time = 0

    return timings

# check if there are any labels within this window
def get_label(start_time, end_time, labels):
    for l in labels:
        label_start = l[0]
        label_end = l[1]

        ## WINDOW CAN:
        #   CONTAIN START LABEL
        #   BE IN BETWEEN START AND END LABELS
        #   CONTAIN END LABEL
        #   CONTAIN BOTH A START AND AN END LABEL
        if  (start_time <= label_start and end_time >= label_start) or \
            (start_time >= label_start and end_time <= label_end) or \
            (start_time <= label_end and end_time >= label_end):
            return 2

    return 1

def extract_features(data, extracted_labels, window_size, window_overlap):
    feature_vec = [];
    labels_vec = [];

    if window_size < window_overlap:
        print "Window size cannot be less than window overlap"
        sys.exit()

    for i in range(0, len(data), window_size):
        if i > 0:
            data_window = data[i-window_overlap:i+window_overlap]
        else:
            data_window = data[i:i+window_size]

        # Mean absolute value
        mav_x = acc_feat_extractor.mav(data_window, 1)
        mav_y = acc_feat_extractor.mav(data_window, 2)
        mav_z = acc_feat_extractor.mav(data_window, 3)

        # Zero Crossings
    #    zc_x = acc_feat_extractor.zero_crossings(data_window, 1, 0.5)
    #    zc_y = acc_feat_extractor.zero_crossings(data_window, 2, 0.5)
    #    zc_z = acc_feat_extractor.zero_crossings(data_window, 3, 0.5)

        # Sign Slope Changes
        ssc_x = acc_feat_extractor.sign_slope_changes(data_window, 1, 20)
        ssc_y = acc_feat_extractor.sign_slope_changes(data_window, 2, 20)
        ssc_z = acc_feat_extractor.sign_slope_changes(data_window, 3, 20)

        l = get_label(data_window[0][0], data_window[-1][0], extracted_labels)
        labels_vec.append(l)
        feature_vec.append([mav_x, mav_y, mav_z, ssc_x, ssc_y, ssc_z])

    features = np.asarray(feature_vec)
    labels = np.asarray(labels_vec)

    return features, labels

if __name__ == "__main__":
    # Read raw data file
    data = read_file('data_files/accelerometer_50 Hz 2.csv')
    test_data = read_file('data_files/accelerometer_50 Hz.csv')

    # Extract key press labels
    key_data = read_file('data_files/press_data_testing2.csv')
    test_key_data = read_file('data_files/press_data_testing.csv')

    # extract labels
    extracted_labels = extract_labels(key_data)
    test_extracted_labels = extract_labels(test_key_data)

    # Create feature extraction object
    acc_feat_extractor = af.AccelerometerFeatures()

    # specify window size (in sample points)
    # Sample points -> millis
    # num_samples = window_time_millis * sample_rate
    window_size = 10;
    window_overlap = 5;
    # Now read window by window and check label timings and extract
    # features accordingly.
    features, labels = extract_features(data, extracted_labels, window_size, window_overlap)
    test_features, test_labels = extract_features(test_data, test_extracted_labels, window_size, window_overlap)

#    print len(extracted_labels)
#    print len(test_extracted_labels)
#    print np.where(labels == 2)
#    print np.where(test_labels == 2)
#    print test_features[0]
#    print features[1]

    # train classifier
#    clf = SVC(gamma=0.001, C=100.)
#    clf.fit(features, labels)
#    pred = [];
#    for i in range(0, len(test_features)):
#        prediction = clf.predict(test_features[i])
#        pred.append(prediction[0])

#    pred = np.asarray(pred)
#    print np.where(pred == 2)

#    target_names=['No Press', 'Press']
#    print(classification_report(test_labels, pred, target_names=target_names))
