import AccelerometerFeatures as af
import numpy as np
from sklearn import svm
from sklearn.model_selection import cross_val_score
import pandas as pd
import glob

# Create feature extraction object
ae = af.AccelerometerFeatures();

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

    def find_events(self, node, window_size, window_overlap, classifier, ax=None):
        if(node.size > 0):
                    X2, y2, w2 = ae.extract_features(node, window_size, window_overlap);
            # Compute time and magnitude
            ntime = node[:,0];
            ntime = ntime - ntime[0];
            ntime = ntime / 1000;                # Convert to milliseconds for the sake of display

            node_mag = np.square(node[:,1]) + np.square(node[:,2]) + np.square(node[:,3]);
            node_mag = np.sqrt(node_mag);

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
