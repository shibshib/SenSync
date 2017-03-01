/*********************************************************************************
/*
 *  Ala Shaabana
 *  McMaster University
 *  Jan 2016
 *  Built with CCSv7
 *
 *  Version: v0.2
 *
 *  Description:
 *  This is a program to evaluate the TI's sensor hub boards.Due to the tight
 *  schedule,not all the functions of sensors are developed.For example,there
 *  are several modes in ISL29023 but it's only set to detect the ambient
 *  light.However,there main difference of each mode is only sending the
 *  different commands to different registers.
 *
 *  A simple user interface is used for selecting different sensors to
 *  evaluate.Two buttons on the MSP430 are used to select sensors and to switch
 *  between low power mode and working mode.All the data are sent to PC and
 *  plotted in SerialChart.
 *
 *  Note:All the functions with prefix 'read' return the raw data of a sensor,
 *  and all the functions with prefix 'get' return the converted data.
 *
 *********************************************************************************/


#include <msp430.h>
#include "I2C.h"
#include "driverlib.h"
#include "UART.h"
#include "ISL29023.h"
#include "TEMP006.h"
#include "SHT21.h"
#include "MPU9150.h"
#include "BMP180.h"
#include "filter.h"
#include "math.h"
#include "hal.h"

#include "USB_API/USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                     //USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

#define TRUE 1
#define FALSE 0

float acc[3]={0};
float gyro[3]={0};

char sensorMode=2;
char lastMode=4;
char lowPowerMode=0;
int buttonFilter=5;

uint32_t mclk = 0;
uint32_t smclk = 0;
uint32_t aclk = 0;

volatile uint8_t crystalCounter = 0;
volatile uint8_t dcoCounter = 0;
volatile uint8_t crystalReady = FALSE;
volatile uint8_t dcoReady = FALSE;

void initDebugging()
{
      WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
      P1DIR |= BIT0;//P1.0,LED1 for indicating sensors are gathering data
      P4DIR |= BIT7;//P4.7,LED2 for sending data to PC

      P2DIR &= ~BIT1;//P2.1 for wake-up/sleep mode
      P2REN |= BIT1;// Enable internal resistance
      P2OUT |= BIT1;// Set  as pull-Up resistance
      P2IES &= ~BIT1;//  Hi/Lo edge
      P2IFG &= ~BIT1;// IFG cleared
      P2IE |= BIT1;// interrupt enabled

      P1DIR &= ~BIT1;//P1.1 for selecting what sensors to evaluate
      P1REN |= BIT1;// Enable internal resistance
      P1OUT |= BIT1;// Set  as pull-Up resistance
      P1IES &= ~BIT1;//  Hi/Lo edge
      P1IFG &= ~BIT1;// IFG cleared
      P1IE |= BIT1;// interrupt enabled
}
void delayMs(unsigned int n)
{
    unsigned int i;
    for(i=0;i<n;i++)
        __delay_cycles(1000);//1ms at 1Mhz
}
inline void enblePINInt()
{
    P1IFG &= ~BIT1;                          // P1.4 IFG cleared
    P2IFG &= ~BIT1;                          // P1.4 IFG cleared
    P2IE |= BIT1;// interrupt enabled
    P1IE |= BIT1;// interrupt enabled
    }

inline void disablePINInt()
{
    P1IFG &= ~BIT1;                          // P1.4 IFG cleared
    P2IFG &= ~BIT1;                          // P1.4 IFG cleared
    P2IE &= ~BIT1;// interrupt disabled
    P1IE &= ~BIT1;// interrupt disabled
}

inline void blinkLED1(char n)
{
    int i;
    for(i=0;i<n;i++)
    {
        LED1on();
        delayMs(300);
        LED1off();
        delayMs(300);
    }

}

inline void LED1on()
{
    P1OUT|=BIT0;
}

inline void LED1off()
{
    P1OUT&=~BIT0;
}
inline void LED2on()
{
    P4OUT|=BIT7;
}

inline void LED2off()
{
    P4OUT&=~BIT7;
}

void initSensors()
{
    initISL29023();
    __no_operation();
    initTEMP006();
    __no_operation();
    initSHT21();
    __no_operation();
    initMPU9150();
    __no_operation();
    initBMP180();
    __no_operation();
}

void initTimers()
{
    TA0CCR0 = 10000;                 // Count limit (16 bits)
    TA0CCTL0 = 0x10;                // Enable counter interrupts
    TA0CTL = TASSEL__ACLK + MC_1;          // Timer A0 with DCO (2 MHz), count UP

    TA1CCR0 = 10000;                 // Count limit (16 bits)
    TA1CCTL0 = 0x10;                // Enable counter interrupts
    TA1CTL = TASSEL__SMCLK + MC_1;       // Timer A1 with Crystal (4 MHz), count UP
}

void readSensors()
{
    getMPU6050Acc(acc,&acc[1],&acc[2]);
    __no_operation();
}

void sendSensorsData()
{
    print(acc[0], TRUE);
    print(acc[1], TRUE);
    print(acc[2], FALSE);
    printchar('\n');
    __no_operation();
}

void checkState()
{
    if((--buttonFilter)<0)
        buttonFilter=0;

    if(lowPowerMode!=0)
    {
        P2IFG &= ~BIT1;                          // P1.4 IFG cleared
        printstring("low power mode\n",15);
        blinkLED1(4);
        enblePINInt();
        __bis_SR_register(LPM4_bits + GIE);       // Enter LPM4 w/interrupt
        __no_operation();
        P2IFG &= ~BIT1;                          // P1.4 IFG cleared
        printstring("working mode\n",13);
        blinkLED1(4);
        lastMode=4;
    }
    if((sensorMode!=lastMode))
    {
        if(sensorMode==0)
            printstring("sensorMode0\n",12);
        else if(sensorMode==1)
            printstring("sensorMode1\n",12);
        else if(sensorMode==2)
            printstring("sensorMode2\n",12);
        lastMode=sensorMode;
    }
    enblePINInt();              //enable selecting mode,pin interrupt disabled after ISR
}

int main(void)
{
    initDebugging();
    clearI2CPort();
    __no_operation();
    initI2C();
    __no_operation();
    initUART();
    __no_operation();
    initClocks();
    __no_operation();
    initTimers();
    __no_operation();

    initSensors();

    mclk = UCS_getMCLK();
    smclk = UCS_getSMCLK();
    aclk = UCS_getACLK();

    printstring("initialize success\n",19);
    printstring("start working\n",14);

    while(TRUE) {

     //   checkState();
  //      __bis_SR_register(LPM0_bits + GIE);
        if (dcoReady) {
            dcoReady = FALSE;
            LED1on();
            readSensors();
            __no_operation();
            LED1off();

            LED2on();
            printstring("D,", 2);
            print(acc[0], TRUE);
            print(acc[1], TRUE);
            print(acc[2], TRUE);
            printchar('\n');
            LED2off();

        }
        if (crystalReady) {
           crystalReady = FALSE;
           LED1on();
           readSensors();
           __no_operation();
           LED1off();

           LED2on();

           printstring("C,", 2);
           print(acc[0], TRUE);
           print(acc[1], TRUE);
           print(acc[2], TRUE);
           printchar('\n');
           __no_operation();
           LED2off();
        }

        __no_operation();
    }
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void) {
    crystalCounter++;
    if(crystalCounter == 3) {
        crystalCounter = 0;
        dcoReady = TRUE;
  //      __bic_SR_register_on_exit(LPM3_bits);
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void) {
    dcoCounter++;
    if(dcoCounter == 3) {
        dcoCounter = 0;
        crystalReady = TRUE;
 //       __bic_SR_register_on_exit(LPM3_bits);
    }
}

/*
* ======== UNMI_ISR ========
*/
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(UNMI_VECTOR))) UNMI_ISR (void)
#else
#error Compiler not found!
#endif
{
       switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG )) {
       case SYSUNIV_NONE:
               __no_operation();
               break;
       case SYSUNIV_NMIIFG:
               __no_operation();
               break;
       case SYSUNIV_OFIFG:

               UCS_clearFaultFlag(UCS_XT2OFFG);
               UCS_clearFaultFlag(UCS_DCOFFG);
               SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
               break;
       case SYSUNIV_ACCVIFG:
               __no_operation();
               break;
       case SYSUNIV_BUSIFG:
               // If the CPU accesses USB memory while the USB module is
               // suspended, a "bus error" can occur.  This generates an NMI.  If
               // USB is automatically disconnecting in your software, set a
               // breakpoint here and see if execution hits it.  See the
               // Programmer's Guide for more information.
               SYSBERRIV = 0;  // Clear bus error flag
               USB_disable();  // Disable
       }
}



// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
    //prevent multiple entry
    P1IFG &= ~BIT1; // P1.1 IFG cleared
    P1IE &= ~BIT1;// interrupt disabled

    if(lowPowerMode==0&&buttonFilter==0)
    {
        sensorMode=(++sensorMode%3);
        buttonFilter=5;
    }

}

// Port 2 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
    //prevent multiple entry
    if(lowPowerMode==0&&buttonFilter==0)
    {
        lowPowerMode=1;
    }
    else if(lowPowerMode==1&&buttonFilter==0)
    {
        lowPowerMode=0;
        buttonFilter=5;
    }

    if(lowPowerMode==0)
        __bic_SR_register_on_exit(LPM4_bits);   // Exit LPM4

    P2IFG &= ~BIT1;                          // P1.4 IFG cleared
}
