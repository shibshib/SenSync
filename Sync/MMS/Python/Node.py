
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
        self.cur_time = 0;
        self.sync_time = 0;
        
    def generateSkew(self):
        return random.uniform(0.1, 1)

    def generateOffset(self):
        return random.randint(1,200)

    def getTime(self):
        return (self.skew * self.cur_time) + self.offset;
