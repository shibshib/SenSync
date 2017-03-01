/*
 * UART.h
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *      Simple UART program with only TX as RX is not used in this project
 *        Simple polling implementation,the output data size should be less than 20.
 */

#ifndef UART_H_
#define UART_H_

void initUART();
void print(float data, int comma);
void println(float data);
void printchar(char data);
void printstring(char *buff,char length);
inline void sendUART(char *buff,unsigned char length);

#endif /* UART_H_ */
