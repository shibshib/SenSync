/*
 * main.c
 *
 * Retrieve and decode the scan key from basic PS2 keyboard
 * and send them over serial comms
 *
 * -----------------------------------------------------

 */
#include <msp430f5529.h>
#include "UART.h"
#include "hal.h"


#define BufferSize          32
#define LEFT_SHIFT          0x12
#define RIGHT_SHIFT         0x59
#define OPEN_FLAG           0xf0
#define TRUE 1
#define FALSE 0

#define KEYBOARD_PS2_PDIR P1DIR
#define KEYBOARD_PS2_PIN  P1IN
#define KEYBOARD_PS2_POUT P1OUT
#define KEYBOARD_PS2_PIFG P1IFG
#define KEYBOARD_PS2_PIES P1IES
#define KEYBOARD_PS2_PIE  P1IE
#define KEYBOARD_PS2_PREN P1REN
#define KEYBOARD_PS2_PSEL P1SEL

#define SIDval  P1IN & BIT4
#define KEYBOARD_PS2_DATA BIT5
#define KEYBOARD_PS2_CLK BIT4

unsigned char bitcount=11;
unsigned char kb_buffer[BufferSize];
unsigned char input=0;
unsigned char output=0;
unsigned char pebit=0xff;
unsigned char recdata=0;


unsigned char flag;

// 7 bit segment code, do not display characters 0x00
unsigned char scandata[17] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,
                        0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};

unsigned char DispBuf = 16;

/*
 * Defining all non-shifted characters (lowercase)
 */
const unsigned char unshifted[][2] =
{
    0x0d,9,     //Table
    0x0e,'`',
    0x15,'q',
    0x16,'1',
    0x1a,'z',
    0x1b,'s',
    0x1c,'a',
    0x1d,'w',
    0x1e,'2',
    0x21,'c',
    0x22,'x',
    0x23,'d',
    0x24,'e',
    0x25,'4',
    0x26,'3',
    0x29,' ',
    0x2a,'v',
    0x2b,'f',
    0x2c,'t',
    0x2d,'r',
    0x2e,'5',
    0x31,'n',
    0x32,'b',
    0x33,'h',
    0x34,'g',
    0x35,'y',
    0x36,'6',
    0x39,',',
    0x3a,'m',
    0x3b,'j',
    0x3c,'u',
    0x3d,'7',
    0x3e,'8',
    0x41,',',
    0x42,'k',
    0x43,'i',
    0x44,'o',
    0x45,'0',
    0x46,'9',
    0x49,'.',
    0x4a,'/',
    0x4b,'l',
    0x4c,';',
    0x4d,'p',
    0x4e,'-',
    0x52,0x27,
    0x54,'[',
    0x55,'=',
    0x5a,13,     //Enter
    0x5b,']',
    0x5d,0x5c,
    0x61,'<',
    0x66,8,  //Back Space
    0x69,'1',
    0x6b,'4',
    0x6c,'7',
    0x70,'0',
    0x71,',',
    0x72,'2',
    0x73,'5',
    0x74,'6',
    0x75,'8',
    0x79,'+',
    0x7a,'3',
    0x7b,'-',
    0x7c,'*',
    0x7d,'9',
    0,0
};

/*
 * Shifted characters (uppercase)
 */
const unsigned char shifted[][2] =
{
    0x0d,9,     //Table
    0x0e,'~',
    0x15,'Q',
    0x16,'!',
    0x1a,'Z',
    0x1b,'S',
    0x1c,'A',
    0x1d,'W',
    0x1e,'@',
    0x21,'C',
    0x22,'X',
    0x23,'D',
    0x24,'E',
    0x25,'$',
    0x26,'#',
    0x29,' ',
    0x2a,'V',
    0x2b,'F',
    0x2c,'T',
    0x2d,'R',
    0x2e,'%',
    0x31,'N',
    0x32,'B',
    0x33,'H',
    0x34,'G',
    0x35,'Y',
    0x36,'^',
    0x39,'L',
    0x3a,'M',
    0x3b,'J',
    0x3c,'U',
    0x3d,'&',
    0x3e,'*',
    0x41,'<',
    0x42,'K',
    0x43,'I',
    0x44,'O',
    0x45,')',
    0x46,'(',
    0x49,'>',
    0x4a,'?',
    0x4b,'L',
    0x4c,':',
    0x4d,'P',
    0x4e,'_',
    0x52,'"',
    0x54,'{',
    0x55,'+',
    0x5a,13,    //Enter
    0x5b,'}',
    0x5d,'|',
    0x61,'>',
    0x66,8,     //Back Space
    0x69,'1',
    0x6b,'4',
    0x6c,'7',
    0x70,'0',
    0x71,',',
    0x72,'2',
    0x73,'5',
    0x74,'6',
    0x75,'8',
    0x79,'+',
    0x7a,'3',
    0x7b,'-',
    0x7c,'*',
    0x7d,'9',
    0,0
};

void delayMs(unsigned int n)
{
    unsigned int i;
    for(i=0;i<n;i++)
        __delay_cycles(1000);//1ms at 1Mhz
}

/*
 * Push a character into the display cache if the cache is in clear of the previous data.
 * Input: c - character to be displayed.
 * Output: None
 */

void PutChar(unsigned char c)
{
    kb_buffer[input] = c;
    if (input < (BufferSize-1))
        input++;
    else
        input = 0;
}

/*
 * Remove a character from display cache.
 * Input: None
 * Output: feteched char
 */
unsigned char GetChar(void)
{
    unsigned char temp;


    if(output == input)
        return 0xff;
     else
    {
        temp = kb_buffer[output];
        if(output < (BufferSize-1))
        {
            output++;
        }
        else
        {
            output = 0;
        }
        return temp;
    }
}

/*
 * Initialize keyboard associated with IO
 * Input: None
 * Output: None
 */

void Init_KB(void)
{
    P1DIR &= ~BIT4;
    P1DIR &= ~BIT5;

    P1IES |= BIT4;
    P1IFG = 0x00;

    P1IE |= BIT5;
    P1SEL = 0x00;

/*
    KEYBOARD_PS2_PDIR &= ~(KEYBOARD_PS2_DATA | KEYBOARD_PS2_CLK);       // Clock connected to P2.5 and set to input AND SID connected to P2.4, set to input
    KEYBOARD_PS2_PREN |= KEYBOARD_PS2_DATA | KEYBOARD_PS2_CLK;
    KEYBOARD_PS2_POUT |= KEYBOARD_PS2_DATA | KEYBOARD_PS2_CLK;

    KEYBOARD_PS2_PIES |= KEYBOARD_PS2_CLK;                                  // Falling edge interrupt
    KEYBOARD_PS2_PIFG &= ~KEYBOARD_PS2_CLK;
    KEYBOARD_PS2_PIE |= KEYBOARD_PS2_CLK;                                    // Enable clock port interrupt
    KEYBOARD_PS2_PSEL = 0x00;
*/

    P1DIR |= BIT0;         // P1.0 LED1 for sending data to PC
    P4DIR |= BIT7;
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

/*
 * Converts incoming information from the keyboard to pairs. Should be
 * encoded in ASCII and sent into the cache.
 *
 * INPUTS: sc - information sent by keyboard
 * OUTPUT: whether valid data was received. T/F
 *
 * NOTE: This function only works with basic keys (No function keys)
 *
 * Scan code contains 3 bytes:
 *  Byte 1: connected code
 *  Byte 2: Open flag 0xf0 (Use this to detect key release/press)
 *  Byte 3: Open code
 */

unsigned char decode(unsigned char sc)
{
 //   printstring("decoding\n", 9);
    static unsigned char shift = 0;         // Whether or not shift key is pressed. 1 - pressed, 0 - not pressed
    static unsigned char up = 0;            // Whether key has been released: 1 - released, 0 - pressed
    unsigned char i, flag = 0;

    if (sc == OPEN_FLAG)                         // If we received the second byte of the scan code, then the key is released.
    {
        up = TRUE;
        printstring("OPEN FLAG!\n", 11);
        return 0;
    }
    else if (up == TRUE)                       // If we received the third byte of the scan code
    {
        up = 0;
        if((sc == LEFT_SHIFT) || (sc == RIGHT_SHIFT))
            shift = 0;
        return 0;
    }

    // If we receive the first byte of the scan code
    if((sc == LEFT_SHIFT) || (sc == RIGHT_SHIFT))        // Left OR right shift key
    {
        shift = TRUE;          // Shift is pressed
        flag = 0;
    }
    else {
        if(shift)           // Shift key is pressed
        {
            for(i = 0; (shifted[i][0] != sc) && shifted[i][0]; i++);
            if(shifted[i][0] == sc)
            {
                PutChar(shifted[i][1]);
                flag = TRUE;
            }
        } else          // Pressed some other button
        {
            for(i = 0;(unshifted[i][0] != sc) && unshifted[i][0];i++);
            if(unshifted[i][0] == sc)
            {
                 PutChar(unshifted[i][1]);
                 flag = TRUE;
            }
        }
    }

    if(flag)
        return TRUE;
    else
        return FALSE;

}

int main(void) {
    unsigned char disptmp;

    WDTCTL = WDTPW + WDTHOLD;	// Stop watchdog timer

    initUART();
    __no_operation();
    initClocks();       // Select MCLK 4MHz
    __no_operation();
    Init_KB();                          // Initialize keyboard
    __no_operation();

    _EINT();                            // Turn on global interrupts

    printstring("initialize success\n",19);
    printstring("start working\n",14);

    while(1)
    {
     //   LPM1;                           // Low power mode

        disptmp = GetChar();            // Read the ASCII code corresponding to the key value

        if((disptmp > 0x2f) && (disptmp < 0x3a))    // If the received characters are 0 - 9
        {
            DispBuf = disptmp - 0x30;               // Get the actual character
       //     LED1on();
       //     LED1off();
       //     print(scandata[DispBuf], 1);
        }
        else if ((disptmp > 0x40) && (disptmp < 0x47))  // If received characters are A - F
        {
            DispBuf = disptmp - 0x37;
        //    LED1on();
        //    LED1off();
      //      print(scandata[DispBuf], 1);
        }
        else if((disptmp > 0x60) && (disptmp < 0x67))     // If received characters are a - f
        {
            DispBuf = disptmp - 0x57;
       //     LED1on();
       //     LED1off();
      //      print(scandata[DispBuf], 1);
        }
        else
        {
            DispBuf = 16;
        }

   //  print(scandata[DispBuf], 1);
    }
}
/*
 * P1 port interrupt service.
 *
 */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    if(P2IFG & BIT4)                // Clock interrupted
    {
        P1IFG &=~ BIT4;             // Clear interrupt flag

        if(bitcount == 11)          // Receive the first bit
        {
            if(SIDval)              // If not the start bit
                return;
            else
                bitcount--;
        }
        else if(bitcount == 2)      // Receive parity bit
        {
            if(SIDval)              // If parity bit is 1
                pebit = 1;
            else
                pebit = 0;
            bitcount--;
        }
        else if(bitcount == 1)      // Receive stop bit
        {
            if(SIDval)              // If stop bit is True
            {
                bitcount = 11;      // Reset bit count
                if( decode(recdata) ) {  // Decode ASCII  value of this key and save it
                  //  LPM3_EXIT;          // Exit low power mode

                }
                recdata = 0;            // clear received data
            }
            else                        // If an error occurs
            {
                bitcount = 11;
                recdata = 0;
            }
        }
        else            // Receive 8 data bits
        {
            recdata >>= 1;
            if(SIDval)
                recdata |= 0x80;
            bitcount--;
        }
    }
}
