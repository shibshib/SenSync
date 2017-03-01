/*
 * Filter.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *  Average filter: calculate the mean value of pass 3 data, this function
 *  contains a static buffer, so it shouldn't be used for different sensors.
 *
 *  Median filter: returns the median value of pass 5 data, a bubble sort is
 *  used because there are only 5 data. This function contains a static buffer, so it
 *  shouldn't be used for different sensors.
 *
 *  FIR filter: A LPF. corner freq = 3Hz, cutoff freq = 6Hz.
 *  Used for accelerometer. This function contains a static variable,
 *  so it shouldn't be used for different sensors.
 */

#ifndef FILTER_H
#define FILTER_H

float FIR(float in);
float medianFilter(float data);
float averageFilter(float data);
float AngleCalculate (float angle,float gyroZ,float accX,float accZ);

#endif

