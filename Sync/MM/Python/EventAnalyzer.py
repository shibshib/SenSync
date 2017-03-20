import AccelerometerFeatures as af
import numpy as np
from sklearn import svm
from scipy import signal
import scipy.interpolate as interp
from sklearn.model_selection import cross_val_score
from LucasKanade import LK
import pandas as pd
import glob

# Create feature extraction object
ae = af.AccelerometerFeatures();

# Quick define a high pass filter
def butter_highpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = signal.butter(order, normal_cutoff, btype='high', analog=False)
    return b, a

def butter_highpass_filter(data, cutoff, fs, order=5):
    b, a = butter_highpass(cutoff, fs, order=order)
    y = signal.filtfilt(b, a, data)
    return y

freq = 50;
cutoff=10;

class EventAnalyzer:

    def find_indices(self, data, start, end):
        sInd = 0;
        eInd = 0;
        for i in range(0, len(data)):
            if (data[i] == start):
                sInd = i;
            elif (data[i] == end):
                eInd = i;
        return sInd, eInd;

    # Function to detect optical seesaw observed on helmet when walking.
    # Since we are only looking for the up-then-down motion of a step,
    # there's no need for a fancy classifier.
    def find_optical_seesaw(self, frames, data, window_size, window_overlap, thresh, ax=None):
        event_recording = False;
        start = 0;
        end = 0;
        events = [];
        if(ax != None):
            e, = ax.plot(frames, data, "b", label="Non-Event");
        for i in range(0, len(data)):
            if(data[i] >= thresh and not event_recording):
                event_recording = True;
                start = i-2;
            if(data[i] <= thresh and event_recording):
                end = i+2;
                if(ax != None):
                    e, = ax.plot(frames[start:end], data[start:end], "r", label="Event");
                events.append([start, end]);
                event_recording = False;

        return events;

    def retrieve_all_optical_events(self, look_in, window_size, window_overlap, T):
        path = 'Data';
        columns = ['Timestamp', 'Number'];

        all_events = [];
        lim = 600;

        # Flow data


        for l in look_in:
            video_src = "C:/Users/Ala/Documents/GitHub/SenSync/Sync/MM/Python/Data/temp/" + str(l) + "/1.avi";
            video_timing_src = "C:/Users/Ala/Documents/GitHub/SenSync/Sync/MM/Python/Data/temp/" + str(l) + "/timing.csv";

            # Extract video timings
            video_columns = ['Timestamp', 'Number'];
            frame_df = pd.read_csv(video_timing_src, names=video_columns);
            frame_timings = frame_df.as_matrix();

            # Analyze video and grab optical flow (Y-axis)
            lkanad = LK(video_src);
            lkanad.run();

            ffreq = 8    # 8 FPS
            fcutoff = 2.9

            flow = np.concatenate(np.asarray(lkanad.flow), axis=0);
            x_flow = flow[:,0,0];
            y_flow = flow[:,0,1];

            # Interpolate timings
            vtimings = frame_timings[:,0] - frame_timings[0,0];
            timing_interp = interp.interp1d(np.arange(vtimings.size), vtimings);

            # Stretch it to match atime
            frame_timings_solid = timing_interp(np.linspace(0, vtimings.size-1, lim));
            frames = np.linspace(0, frame_timings_solid[-1], len(frame_timings_solid)/3)

            y_interp = interp.interp1d(np.arange(y_flow.size), y_flow);
            y_compressed = y_interp(np.linspace(0, y_flow.size-1, frames.size));
            y_compressed = y_compressed - y_compressed[0];
            y_compressed_filtered = butter_highpass_filter(y_compressed, fcutoff, ffreq);
            y_compressed_filtered = abs(y_compressed_filtered);

            optical_events = self.find_optical_seesaw(frames, y_compressed_filtered, window_size, window_overlap, T, None);
            all_events.append(optical_events);

        all_events = np.concatenate(all_events, axis=0);

        return all_events


    def find_events(self, node, window_size, window_overlap, classifier, ax=None):
            if(node.size == 0):
                return 0;
            else:
                X2, y2, w2 = ae.extract_features(node, window_size, window_overlap);

                # Compute time and magnitude
                ntime = node[:,0];
                ntime = ntime - ntime[0];
                ntime = ntime / 1000;                # Convert to milliseconds for the sake of display

                node_mag = np.square(node[:,1]) + np.square(node[:,2]) + np.square(node[:,3]);
                node_mag = np.sqrt(node_mag);
                node_mag = butter_highpass_filter(node_mag, cutoff, freq);
                node_mag = abs(node_mag);
                # Variable to hold the start and end indices of events in
                # the signal
                events = [];
                single_event_start = 0;

                # Use classifier to find events in DCO
                # all handshake event timings are recorded
                # inside events variable. Length of the
                # active areas array should be ~ 2 x # of events
                for i in range(0, len(X2)):
                    feat = X2[i];
                    activity = classifier.predict([feat]);
                    if (activity[0] == 1):
                        # Determine the location of this window index-wise in the data
                        start, end = self.find_indices(node[:,0], w2[i][0], w2[i][1]);

                        # we haven't started recording the event yet
                        if (single_event_start == 0):
                            # begin recording
                            single_event_start = start;

                        if(ax != None):
                            e, = ax.plot(ntime[start:end], node_mag[start:end], "r", label="Event");
                    else:
                        start, end = self.find_indices(node[:,0], w2[i][0], w2[i][1]);

                        # We are recording an event
                        if (single_event_start != 0):
                            # save recording
                            events.append([single_event_start,end]);
                            # restart for the next recording
                            single_event_start = 0;

                        if(ax != None):
                            ne, = ax.plot(ntime[start:end], node_mag[start:end], "b", label="Non-event");
                            ax.set_xlabel('Time (Millis)');

                return events;

    def retrieve_all_events(self, src, window_size, window_overlap, classifier):
        path = 'Data';
        columns = ['Timestamp', 'X', 'Y', 'Z'];

        # Check if we're using DCO or crystal as our training source files..
        if src == "crystal":
            allFiles = glob.glob(path + "/Crystal*.csv");
        else:
            allFiles = glob.glob(path + "/DCO*.csv");

        # Cycle through all files and retrieve their events
        all_events = [];
        for file_ in allFiles:
            df = pd.read_csv(file_,names=columns)
            data = df.as_matrix();

            events = self.find_events(data, window_size, window_overlap, classifier, None);
            all_events.append(events);

        all_events = np.concatenate(all_events, axis=0);
        return all_events
