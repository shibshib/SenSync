/*
 * SH21.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *  Basic I2C driver for SHT21
 */

#ifndef SHT21_H_
#define SHT21_H_


void initSHT21();
unsigned int readSHT21TEMP();
unsigned int readSHT21HUM();
float getSHT21HUM();
float getSHT21TEMP();

#endif /* SHT21_H_ */
