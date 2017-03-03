/*
 Header File: SerialPortSample.h
 Abstract: Command line tool that demonstrates how to use IOKitLib to find all serial ports on OS X. Also shows how to open, write to, read from, and close a serial port.
 Version: 1.0
 
 Author: Ala Shaabana
 WiSeR Lab 2017
 
 */


#ifndef Header_h
#define Header_h

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/serial/ioss.h>
#include <IOKit/IOBSD.h>


// Default to local echo being on. If your modem has local echo disabled, undefine the following macro.
#define LOCAL_ECHO

// Find the first device that matches the callout device path MATCH_PATH.
// If this is undefined, return the first device found.
#define MATCH_PATH "/dev/cu.usbmodem14223"

#define kATCommandString	"AT\r"

#ifdef LOCAL_ECHO
#define kOKResponseString	"AT\r\r\nOK\r\n"
#else
#define kOKResponseString	"\r\nOK\r\n"
#endif

const int kNumRetries = 3;

// Hold the original termios attributes so we can reset them
static struct termios gOriginalTTYAttrs;

// Default baud rate
#define BAUDRATE 256000

// Connection status
Boolean connected = false;

// Function prototypes
static kern_return_t findModems(io_iterator_t *matchingServices);
static kern_return_t getModemPath(io_iterator_t serialPortIterator, char *bsdPath, CFIndex maxPathSize);
static int openSerialPort(const char *bsdPath);
static char *logString(char *str);
static Boolean readSerial(char *buffer, int fileDescriptor);
static void collectData(int fileDescriptor);
static void closeSerialPort(int fileDescriptor);



#endif /* Header_h */
