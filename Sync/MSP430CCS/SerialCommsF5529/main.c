#include <msp430f5529.h>
#include "driverlib.h"

#include "USB_API/USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                     //USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_app/usbConstructs.h"

/*
 * main.c
 */

/*
 * NOTE: Modify hal.h to select a specific evaluation board and customize for
 * your own board.
 */
#include "hal.h"

volatile uint8_t sendCrystalTimestampToHost = FALSE;
volatile uint8_t sendDCOTimestampToHost = FALSE;
volatile uint8_t crystalCounter = 0;
volatile uint8_t dcoCounter = 0;

uint32_t mclk = 0;
uint32_t smclk = 0;
uint32_t aclk = 0;

int main(void) {
    WDT_A_hold(WDT_A_BASE);   // Stop watchdog timer

    initClocks();             // Initialize SMCLK and ACLK

    aclk=UCS_getACLK();
    mclk=UCS_getMCLK();
    smclk=UCS_getSMCLK();

    // Minimum Vcore setting required for hte USB API is PMM_CORE_LEVEL_2
    PMM_setVCore(PMM_CORE_LEVEL_2);
    USBHAL_initPorts();           // Config GPIOS for low-power (output low)

    USB_setup(TRUE,TRUE);  // Init USB & events; if a host is present, connect
    __enable_interrupt();  // Enable interrupts globally

    P1DIR |= (BIT0);
    P4DIR |= (BIT7);

    TA0CCR0 = 65000;                 // Count limit (16 bits)
    TA0CCTL0 = 0x10;                // Enable counter interrupts
    TA0CTL = TASSEL__ACLK + MC_1;          // Timer A0 with crystal, SMCLK (1 MHz), count UP

    TA1CCR0 = 800;                 // Count limit (16 bits)
    TA1CCTL0 = 0x10;                // Enable counter interrupts
    TA1CTL = TASSEL__SMCLK + MC_1;       // Timer A1 with ACLK (12 KHz), count UP


    while(1){
        // Enter LPM0, which keeps the DCO/FLL active but shuts off the
        // CPU.  For USB, you can't go below LPM0!
        __bis_SR_register(LPM0_bits + GIE);
        // If USB is present, sent the time to the host.  Flag is set every sec
        if (sendCrystalTimestampToHost){
            sendCrystalTimestampToHost = FALSE;
            P1OUT ^= BIT0;                      // Toggle red LED
            if (USBCDC_sendDataInBackground("C", 1, CDC0_INTFNUM, 1000)){
                _NOP();                // If it fails, it'll end up here.  Could happen if
                                       // the cable was detached after the connectionState()
            }                          // check, or if somehow the retries failed
        }

        if(sendDCOTimestampToHost) {
            sendDCOTimestampToHost = FALSE;
            P4OUT ^= BIT7;                     // Toggle green LED
            if (USBCDC_sendDataInBackground("D", 1, CDC0_INTFNUM, 1000)){
                  _NOP();  // If it fails, it'll end up here.  Could happen if
       //                     // the cable was detached after the connectionState()
            }               // check, or if somehow the retries failed
        }
    }
    return (0);
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void) {
    crystalCounter++;
    if (crystalCounter == 3){
        crystalCounter = 0;
        sendCrystalTimestampToHost = TRUE;
        __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM
    }
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void) {
    dcoCounter++;
    if (dcoCounter == 3){
        dcoCounter = 0;
        sendDCOTimestampToHost = TRUE;
        __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM
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
