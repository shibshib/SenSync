# -*- coding: utf-8 -*-
"""
Created on Thu Mar 16 16:28:45 2017

@author: Ala

Quick script to grab "date created" on each frame -- convert it into "microseconds 
since epoch" and record in a csv file
"""
import glob
import os

# Move to new directory
path = "Data/temp/3";
os.chdir(path);

# Get all files in 1 directory
allFiles = glob.glob("*.jpg")
fd = open('timing.csv', 'w');
counter = 1;

for file_ in allFiles:
    creation_time = os.path.getctime(file_)*1000000;
    fd.write(str(creation_time) + ',' + str(counter) + '\n');
    counter = counter + 1;
    

fd.close();

