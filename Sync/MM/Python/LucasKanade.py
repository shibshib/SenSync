#!/usr/bin/env python
# Python 2/3 compatibility
from __future__ import print_function
'''

Author: Ala Shaabana

Usage
-----
Lucas-Kanade.py [<video_source>]

Plots optical flow overlayed on top of video and then plots using matplotlib.

Keys
----
ESC - exit
'''



import numpy as np
import cv2
import video
import sys
from scipy import signal
import scipy.interpolate as interp
import matplotlib.pyplot as plt
from common import anorm2, draw_str
from time import clock

lk_params = dict( winSize  = (15, 15),
                  maxLevel = 8,
                  criteria = (cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 10, 0.03))

feature_params = dict( maxCorners = 500,
                       qualityLevel = 0.3,
                       minDistance = 7,
                       blockSize = 7 )

# High pass filter
def butter_highpass(cutoff, fs, order=5):
    nyq = 0.5 * fs
    normal_cutoff = cutoff / nyq
    b, a = signal.butter(order, normal_cutoff, btype='high', analog=False)
    return b, a

def butter_highpass_filter(data, cutoff, fs, order=5):
    b, a = butter_highpass(cutoff, fs, order=order)
    y = signal.filtfilt(b, a, data)
    return y

freq = 8; #fps
cutoff = 3;
order=5;
class LK:
    def __init__(self, video_src):
        self.track_len = 10
        self.detect_interval = 5
        self.tracks = []
        self.cam = video.create_capture(video_src)
        self.flow = [];
        self.frame_idx = 0


    def plotFlow(self):
        self.flow = np.concatenate(np.asarray(self.flow), axis=0);
        frames = np.linspace(0, self.frame_idx);
        x = self.flow[:,0,0];
        y = self.flow[:,0,1];

        # let's try compressing X to the size of frames..
        x_interp = interp.interp1d(np.arange(x.size), x);
        x_compressed = x_interp(np.linspace(0, x.size-1, frames.size));
        x_compressed = x_compressed - x_compressed[0];
        x_compressed_filtered = butter_highpass_filter(x_compressed, cutoff, freq, order);
        x_compressed_filtered = abs(x_compressed_filtered);

        y_interp = interp.interp1d(np.arange(y.size), y);
        y_compressed = y_interp(np.linspace(0, y.size-1, frames.size));
        y_compressed = y_compressed - y_compressed[0];
        y_compressed_filtered = butter_highpass_filter(y_compressed, cutoff, freq, order);
        y_compressed_filtered = abs(y_compressed_filtered);
        
        fig, (ax1, ax2) = plt.subplots(nrows=2);

        ax1.plot(frames, y_compressed, 'r');
        ax1.grid();
        ax1.set_title("Y - Unfiltered");
        ax2.plot(frames, y_compressed_filtered, 'b');
        ax2.grid();
        ax2.set_title("Y - Filtered");

        plt.show();


    def run(self):
        while True:
            ret, frame = self.cam.read()
          #  time.sleep( 0.5 );
            if ret == True:
                frame_gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                vis = frame.copy()

                if len(self.tracks) > 0:
                    img0, img1 = self.prev_gray, frame_gray

                    # Grab optical flow
                    p0 = np.float32([tr[-1] for tr in self.tracks]).reshape(-1, 1, 2)
                    p1, st, err = cv2.calcOpticalFlowPyrLK(img0, img1, p0, None, **lk_params)
                    p0r, st, err = cv2.calcOpticalFlowPyrLK(img1, img0, p1, None, **lk_params)
                    self.flow.append(p1);

                    # Map onto video
                    d = abs(p0-p0r).reshape(-1, 2).max(-1)
                    good = d < 1
                    new_tracks = []
                    for tr, (x, y), good_flag in zip(self.tracks, p1.reshape(-1, 2), good):
                        if not good_flag:
                            continue
                        tr.append((x, y))
                        if len(tr) > self.track_len:
                            del tr[0]
                        new_tracks.append(tr)
                        cv2.circle(vis, (x, y), 2, (0, 255, 0), -1)
                    self.tracks = new_tracks
                    cv2.polylines(vis, [np.int32(tr) for tr in self.tracks], False, (0, 255, 0))
                    draw_str(vis, (20, 20), 'track count: %d' % len(self.tracks))

                if self.frame_idx % self.detect_interval == 0:
                    mask = np.zeros_like(frame_gray)
                    mask[:] = 255
                    for x, y in [np.int32(tr[-1]) for tr in self.tracks]:
                        cv2.circle(mask, (x, y), 5, 0, -1)
                    p = cv2.goodFeaturesToTrack(frame_gray, mask = mask, **feature_params)
                    if p is not None:
                        for x, y in np.float32(p).reshape(-1, 2):
                            self.tracks.append([(x, y)])


                self.frame_idx += 1
                self.prev_gray = frame_gray
                cv2.imshow('lk_track', vis)

                ch = cv2.waitKey(1)
                if ch == 27:
                    break
            else:
                break;
        #  Plot flow?
        cv2.destroyAllWindows()
      #  self.plotFlow();



def main():
    import sys
    try:
        video_src = sys.argv[1]
    except:
        video_src = 0

    print(__doc__)
    LK(video_src).run()



if __name__ == '__main__':
    main()
