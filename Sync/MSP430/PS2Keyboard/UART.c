/*
 * UART.c
 *
 *  Created on: Feb 4, 2017
 *      Author: Ala
 *
 *        Simple UART program with only TX as RX is not used in this project
 *        Simple polling implementation,the output data size should be less than 20.
 */

#include <msp430.h>
#include <stdio.h>
#include "UART.h"


char _buff[20]={0};
void initUART()
{
  P4SEL |= BIT5+BIT4;                       // P4.5,4 = USCI_A0 TXD/RXD
  UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA1CTL1 |= UCSSEL_2;                     // SMCLK
  UCA1BR0 = 15;                              // 4MHz 256000 (see User's Guide)
  UCA1BR1 = 0;                              // 4MHz 256000
  UCA1MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
  UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  __bis_SR_register(GIE);                   // Enter LPM0, interrupts enabled
}

void print(float data, int comma)
{
    int n;
    if (comma)
        n=sprintf (_buff, "%.2f,", data);
    else
        n=sprintf (_buff, "%.2f", data);
    sendUART(_buff,n);
}

void printstring(char *buff,char length)
{
      int i;
      for(i=0;i<length;i++,buff++)
      {
        while (!(UCA1IFG&UCTXIFG));
        UCA1TXBUF = *buff;
      }
}

void printchar(char data)
{
    while (!(UCA1IFG&UCTXIFG));
    UCA1TXBUF = data;
}

void println(float data)
{
    int n;
    n=sprintf (_buff, "%.2f\n", data);
    sendUART(_buff,n);
}

inline void sendUART(char *buff,unsigned char length)
{
  int i;
  for(i=0;i<length;i++,buff++)
  {
    while (!(UCA1IFG&UCTXIFG));
    UCA1TXBUF = *buff;
  }
}

