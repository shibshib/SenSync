/*
 * ISL29023.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *  Basic I2C driver for ISL29023 (ambient light sensor)
 *  Currently ISL29034 is set to continuous ambient light sampling mode, 16 bit.
 */

#ifndef ISL29023_H_
#define ISL29023_H_

void initISL29023();
unsigned int readISL29023();
float getISL29023AMB();

#endif /* ISL29023_H_ */
