/*
 * I2C.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *  Basic I2C driver for TI MSP430F5529
 *
 */

#ifndef I2C_H_
#define I2C_H_

#define STOP  1
#define NO_STOP 0
void initI2C();
void clearI2CPort();
void setI2CAddress(unsigned char addr);
void sendI2C(unsigned char* data,unsigned char length,unsigned char _stop);
void sendI2CBytes(unsigned char* data,unsigned char length,unsigned char _stop);
unsigned char readI2C();
unsigned int readI2CWord();
void readI2CBytes(unsigned char length,unsigned char* pointer);


#endif /* I2C_H_ */
