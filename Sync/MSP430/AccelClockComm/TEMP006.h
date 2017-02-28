/*
 * TEMP006.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *      Basic I2C driver for TMP006
 */

#ifndef TEMP006_H_
#define TEMP006_H_

void initTEMP006();
unsigned int readTMP006AMB();
float getTMP006AMB();

#endif /* TEMP006_H_ */
