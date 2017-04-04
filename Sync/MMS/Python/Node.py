
#########################################################################
#
# This is a class that contains functions for each node simulated in the network
#
#
#
#########################################################################
import numpy as np
import random
import time

class Node:
    def __init__(self):
        self.skew = self.generateSkew();
        self.offset = self.generateOffset();
        self.stderr = self.generateError();
        self.sync_time = self.generateLastSyncTime();

    # Skew and offset currently randomly generated, these can be fixed
    # in simulator.py code to specific values if need be.
    def generateSkew(self):
        return random.uniform(0.1, 1)

    def generateOffset(self):
        return random.randint(1,200)

    def generateError(self):
        return random.uniform(0, 1)

    def getTime(self):
        return (self.skew * self.cur_time) + self.offset;

    def generateLastSyncTime(self):
        curtime = lambda: int(round(time.time() * 1000));
        # Generate a random time between now and a year that the last sync happened
        last_year = 1459779808000;
        return random.uniform(last_year, curtime())
