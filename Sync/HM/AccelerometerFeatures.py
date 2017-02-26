
#########################################################################
#
# This is a class that contains functions to extract features and generate
# feature vector for accelerometer data
#
#   INPUT:      4D ndarray: timestamp, x, y, z, magnitude,
#
#########################################################################
import numpy as np
import pandas as pd
import glob


class AccelerometerFeatures:
    def mav(self, data, dim):
        meandat = data[:,dim].mean()
        val = 0;
        count = 0;
        for i in data:
            val += abs(i[dim] - meandat)
            count += 1
        return (val / count)

    def mean_value(self, dim):
        return data[:,dim].mean()

    def zero_crossings(self, data, dim, threshold):
        ZC = 0
        for i in range(0, len(data)-1):
            sample1 = abs(data[i+1, dim] - data[i, dim]);
            sample2 = max(abs(data[i+1, dim] + data[i, dim]), threshold)
            if sample1 > sample2:
                ZC = ZC + 1
        return ZC

    def sign_slope_changes(self, data, dim, threshold):
        SSC = 0
        for i in range(1, len(data)-1):
            prev_point = data[i-1, dim];
            cur_point = data[i, dim];
            next_point = data[i+1, dim];

            cond1 = cur_point > max(prev_point, next_point)
            cond2 = cur_point < min (prev_point, next_point)
            cond3 = max(abs(next_point - cur_point),abs(cur_point - prev_point)) > threshold

            if cond1 or cond2 and cond3:
                SSC = SSC + 1

        return SSC

    def waveform_length(self, data, dim):
        WL = 0;
        for i in range(1, len(data)):
            WL += abs(data[i, dim] - data[i-1, dim]);

        return WL;

    def extract_features(self, data, window_size, overlap):
        features = [];
        labels = [];
        windows = [];
        for i in range(0, len(data), window_size):
            # Define window contents
            if(i > 0):
                window = data[i-overlap:i+window_size];
            else:
                window = data[i:i+window_size];

            windows.append([window[0,0], window[-1,0]]);

            # Extract features: MAV, SSC, and ZC
            mav_x = self.mav(window, 1);
            mav_y = self.mav(window, 2);
            mav_z = self.mav(window, 3);

        #    ssc_x = self.sign_slope_changes(window, 1, 0.0);
        #    ssc_y = self.sign_slope_changes(window, 2, 0.0);
        #    ssc_z = self.sign_slope_changes(window, 3, 7.0);

            WL_x = self.waveform_length(window, 1);
            WL_y = self.waveform_length(window, 2);
            WL_z = self.waveform_length(window, 3);

            # Compute window magnitude, for labeling the window
            mag = np.square(window[:,1]) + np.square(window[:,2]) + np.square(window[:,3]);
            mag = np.sqrt(mag);
            label = self.generate_naive_label(mag);
            labels.append(label);
            features.append([mav_x, mav_y, mav_z, WL_x, WL_y, WL_z]);

        features = np.asarray(features);
        labels = np.asarray(labels);
        windows = np.asarray(windows);

        return features, labels, windows;
    # Function to generate labels.
    # Function is naive in that it performs
    # simple peak counting for onset and offset labeling
    def generate_naive_label(self, data):
        labels = [];
        for i in range(0, len(data)):
            if(data[i] > 8.0):
                return 1
        return 0;

    # Function to cycle through accelerometer files in Data directory
    # and extract features from each one.
    def extract_features_from_files(self, train_src, window_size, window_overlap):
        path = 'Data';
        columns = ['Timestamp', 'X', 'Y', 'Z'];

        # Check if we're using DCO or crystal as our training source files..
        if train_src == "crystal":
            allFiles = glob.glob(path + "/Crystal*.csv");
        else:
            allFiles = glob.glob(path + "/DCO*.csv");

        # Cycle through all files and train
        X_full = [];
        y_full = [];
        w_full = [];
        for file_ in allFiles:
            df = pd.read_csv(file_,names=columns)
            training_data = df.as_matrix();
            X, y, w = self.extract_features(training_data, window_size, window_overlap)
            #X_full = np.concatenate((X_full, X), axis=0);
            X_full.append(X);
            y_full.append(y);
            w_full.append(w);

        # Convert list of np arrays to single giant np array
        X_full = np.concatenate(X_full, axis=0);
        y_full = np.concatenate(y_full, axis=0);
        w_full = np.concatenate(w_full, axis=0);

        return X_full, y_full, w_full;
