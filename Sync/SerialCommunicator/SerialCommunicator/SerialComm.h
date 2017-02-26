#pragma once
#ifndef SERIALCLASS_H_INCLUDED
#define SERIALCLASS_H_INCLUDED

#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define MSP430_WAIT_TIME 2000

class SerialComm
{
private:
	// Serial comm handler
	HANDLE hSerial;
	// Connection status
	bool connected;
	// Get various info about conection
	COMSTAT status;
	// Keep track of last error
	DWORD errors;

public:
	// Initialize Serial communication with the given COM port
	SerialComm(LPCWSTR portName);
	// Close connecsh
	~SerialComm();
	// Read data in a buffer, if nbChar is greated than the max number
	// of bytes available, it will return only the bytes available. The 
	// function will return -1 when nothing could be read.
	int ReadData(char *buffer, unsigned int nbChar);
	// Writes data froma  buffer through the serial connecsh
	// return TRUE on success
	bool WriteData(char *buffer, unsigned int nbChar);
	// Check whether we are actually connected
	bool IsConnected();

};

#endif