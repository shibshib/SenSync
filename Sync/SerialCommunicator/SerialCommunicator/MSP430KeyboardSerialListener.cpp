#include <iostream>
#include <windows.h> 
#include "SerialComm.h"
#include <fstream>
#include <ctime>
#include <chrono>
#include <string>
#include <sstream>
#include <atlstr.h>
#include <thread>

#define TIMESTAMP_LENGTH 16
#define MINIMUM_ACCEL_SAMPLE_LENGTH 31
#define MINIMUM_KEY_SAMPLE_LENGTH 19

using namespace std::chrono;
bool finished = false;
LPWSTR KEYBOARD_COM = L"\\\\.\\COM12";
LPWSTR ACCEL_COM = L"\\\\.\\COM6";				// May be COM9, depending on MCU used.

std::string KEYBOARD_COM_string = "COM12";
std::string ACCEL_COM_string = "COM6";
std::string crystal_speed = "4MHz";
std::string dco_speed = "2MHz";
std::string filenum = "3";
std::string dataPath = "C:\\Users\\Ala\\Documents\\GitHub\\SenSync\\Sync\\HM\\Python\\Data\\";

void separateAccelFileOutput(std::string portName) {
	std::ifstream inputFile(dataPath + "raw_output_" + portName + ".csv");
	std::ofstream cfile, dfile;

	cfile.open(dataPath + "Crystal_accel_" + portName + "_" + crystal_speed + "_" + filenum + ".csv");
	dfile.open(dataPath + "DCO_accel_" + portName + "_" + dco_speed + "_" + filenum + ".csv");

	if (cfile.is_open() && dfile.is_open() && inputFile.is_open()) {
		std::string line;

		while (std::getline(inputFile, line)) {
			if (line.back() == ',') {
				if ((line[TIMESTAMP_LENGTH + 1] == 'C') && strlen(line.c_str()) > MINIMUM_ACCEL_SAMPLE_LENGTH) {
					// erase 'C,'
					line.erase(TIMESTAMP_LENGTH, 2);
					// erase final comma
					line.pop_back();

					cfile << line << std::endl;
				}
				else if ((line[TIMESTAMP_LENGTH + 1] == 'D') && strlen(line.c_str()) > MINIMUM_ACCEL_SAMPLE_LENGTH) {
					// erase 'C,'
					line.erase(TIMESTAMP_LENGTH, 2);
					// erase final comma
					line.pop_back();

					dfile << line << std::endl;
				}
			}
		}

		inputFile.close();
		cfile.close();
		dfile.close();
	}
	else {
		std::cout << "Open failed!" << std::endl;
	}
}

void keyboardFileOutput(std::string portName) {
	std::ifstream inputFile(dataPath + "raw_output_" + portName + ".csv");
	std::ofstream file;
	
	file.open(dataPath + "keyboard_" + portName + "_" + crystal_speed + "_"  + filenum + ".csv");
	std::string line;

	while (std::getline(inputFile, line)) {
		file << line << std::endl;
	}
	inputFile.close();
	file.close();
	
}

BOOL WINAPI ConsoleHandler(DWORD CEvent) {
	switch (CEvent) {
	case CTRL_CLOSE_EVENT:
		// Write contents of raw_output to crystal and DCO files
		separateAccelFileOutput(ACCEL_COM_string);
		keyboardFileOutput(KEYBOARD_COM_string);
		break;
	default:
		return FALSE;
	}
}

/*
* Function to collect data from given COM port.
* Used by each thread separately.
*/
void collectData(LPWSTR port, std::string portName) {
	char *buf = NULL;
	SerialComm  *SC = new SerialComm(port);
	std::ofstream outputFile;

	// open output file
	outputFile.open(dataPath + "raw_output_" + portName + ".csv");

	if (outputFile.is_open())
	{
		// Timestamp in microseconds since EPOCH (00:00:00 Jan 1, 1970)
		using time_stamp = std::chrono::time_point<std::chrono::system_clock,
			std::chrono::microseconds>;

		if (SC->IsConnected())
			std::cout << "Connected to " << portName << std::endl;

		std::cout << "Collecting from " << portName << "..." << std::endl;

		char incomingData[256] = "";			// don't forget to pre-allocate memory
		int dataLength = 255;
		int readResult = 0;

		while (SC->IsConnected()) {
			readResult = SC->ReadData(incomingData, dataLength);
			incomingData[readResult] = 0;

			while (strcmp(incomingData, "\\n") != 0) {
				readResult = SC->ReadData(incomingData, dataLength);
				incomingData[readResult] = 0;

				if (strcmp(incomingData, "") != 0) {
					// get timestamp
					std::time_t result = std::time(nullptr);
					microseconds ms = duration_cast<microseconds>(
						system_clock::now().time_since_epoch());

					// data comes in chunks, reduce chunks to lines and apply timestamp.
					std::string line;
					std::istringstream f(incomingData);

					while (std::getline(f, line)) {
					//	std::cout << port << "," << ms.count() << "," << line << std::endl;
						if (portName == KEYBOARD_COM_string) {
							outputFile << ms.count() << "," << line;
						}
						else {
							outputFile << ms.count() << "," << line << std::endl;
						}

					}
				}
			}
		}
		outputFile.close();
	}
	else {
		std::cout << "Open failed! PORT:" << portName << std::endl;
	}
}

int main() {
	
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE)) {
		std::cout << "Handler Installed!\n";
	}

	std::thread KEYBOARD_thread(collectData, KEYBOARD_COM, KEYBOARD_COM_string);
	std::thread ACCEL_thread(collectData, ACCEL_COM, ACCEL_COM_string);


	// synchronize threads:
	KEYBOARD_thread.join();                // pauses until first finishes
	ACCEL_thread.join();               // pauses until second finishes
	
//	separateAccelFileOutput(ACCEL_COM_string);
	return 0;
}
