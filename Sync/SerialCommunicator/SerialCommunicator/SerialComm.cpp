#include "SerialComm.h"

SerialComm::SerialComm(LPCWSTR portName)
{
	// We're not yet connected.
	this->connected = false;

	// Try to connect to the given port through CreateFile
	this->hSerial = CreateFile(portName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	// Check if connection was not successful
	if (this->hSerial == INVALID_HANDLE_VALUE) {
		// If not successful, error
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			// Print error
			std::cout << "ERROR: HANDLE WAS NOT ATTACHED. REASON: " << portName << " WAS NOT AVAILABLE." << std::endl;
		}
		else {
			std::cout << "UNKNOWN ERROR" << std::endl;
		}
	}
	else {
		// If connection successful
		DCB dcbSerialParams = { 0 };

		// Try to get the current
		if (!GetCommState(this->hSerial, &dcbSerialParams)) {
			// If fail, show error
			std::cout << "Failed to get serial params!" << std::endl;
		}
		else {
			// Define serial connection params for the MSP430
			dcbSerialParams.BaudRate = CBR_256000;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;
			// Setting DTR to Control_Enable ensures that the MSP430 is properly reset upon establishing connection (Optional)
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

			// Set the parameters and check for their proper application
			if (!SetCommState(hSerial, &dcbSerialParams)) {
				std::cout << "ERROR: Could not set up Serial Port Parameters" << std::endl;
			}
			else {
				// If everything went to plan, we are now connected!
				this->connected = true;
				// Flush remaining characters in buffers (Optional when reading only)
				PurgeComm(this->hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
				// Wait 5 seconds for board to stabilize and start clocks
				Sleep(MSP430_WAIT_TIME);
			}
		}
	}
}

// Destructor
SerialComm::~SerialComm()
{
	// Check if we are connected before trying to disconnect
	if (this->connected) {
		// We're no longer connected
		this->connected = false;
		// Close serial handler
		CloseHandle(this->hSerial);
	}
}

/*
Function to read the data from a buffer connected to comms port.
INPUT : character buffer, and number of chars to read. 
*/
int SerialComm::ReadData(char *buffer, unsigned int nbChar) {
	// Number of bytes we will have read
	DWORD bytesRead;
	// Number of bytes we we'll really ask to read
	unsigned int toRead;

	// Use the ClearCommError function to get status info on the serial port
	ClearCommError(this->hSerial, &this->errors, &this->status);

	// Check if there is something to read
	if (this->status.cbInQue > 0) {
		// If there is, we check if there is enough data to read the required 
		// number of characters, if not, twe will only read the available characters 
		// to prevent from locking the application. 
		if (this->status.cbInQue > nbChar) {
			toRead = nbChar;
		}
		else {
			toRead = this->status.cbInQue;
		}

		// Try to read the required number of chars, and return the number of read bytes
		if (ReadFile(this->hSerial, buffer, toRead, &bytesRead, NULL)) {
			return bytesRead;
		}
	}
	// If nothing has been read, or an error was detected, return 0
	return 0;
}

/*
Function to write the data from a buffer connected to comms port.
INPUT : character buffer, and number of chars to write.
*/
bool SerialComm::WriteData(char *buffer, unsigned int nbChar) {
	DWORD bytesSend;

	// Try to write the buffer on the Serial port
	if (!WriteFile(this->hSerial, (void *)buffer, nbChar, &bytesSend, 0)) {
		// In case it don't work, get comm error and return false
		ClearCommError(this->hSerial, &this->errors, &this->status);
		
		return false;
	}
	else {
		return true;
	}
}

bool SerialComm::IsConnected() {
	// Simply return the connection status
	return this->connected;
}