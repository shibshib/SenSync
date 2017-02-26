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

#define TIMESTAMP_LENGTH 16				// this will eventually change with time
#define MINIMUM_SAMPLE_LENGTH 31

using namespace std::chrono;
bool finished = false;
LPWSTR COM9 = L"COM9";
LPWSTR COM6 = L"COM6";

std::string COM9String = "COM9";
std::string COM6String = "COM6";
std::string crystal_speed = "4MHz";
std::string dco_speed = "-2MHz";

void separateFileOutput(std::string portName) {
	std::ifstream inputFile("Human-Human_Data/raw_output_" + portName + ".csv");
	std::ofstream cfile, dfile;
	std::string filenum = "3";

	cfile.open("C:\\Users\\Ala\\Desktop\\Dropbox\\PhD\\Sync\\KeySync\\Python\\Data\\Crystal_accel_" + portName + "_" + crystal_speed + "_" + filenum + ".csv");
	dfile.open("C:\\Users\\Ala\\Desktop\\Dropbox\\PhD\\Sync\\KeySync\\Python\\Data\\DCO_accel_" + portName + "_" + dco_speed + "_" + filenum + ".csv");
	std::string line;

	while (std::getline(inputFile, line)) {
		if (line.back() == ',') {
			if ((line[TIMESTAMP_LENGTH + 1] == 'C') && strlen(line.c_str()) > MINIMUM_SAMPLE_LENGTH) {
				// erase 'C,'
				line.erase(TIMESTAMP_LENGTH, 2);
				// erase final comma
				line.pop_back();

				cfile << line << std::endl;
			}
			else if ((line[TIMESTAMP_LENGTH + 1] == 'D') && strlen(line.c_str()) > MINIMUM_SAMPLE_LENGTH) {
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

BOOL WINAPI ConsoleHandler(DWORD CEvent) {
	switch (CEvent) {	
		case CTRL_CLOSE_EVENT:
			// Write contents of raw_output to crystal and DCO files
			separateFileOutput(COM6String);
			separateFileOutput(COM9String);
			break;
		default:
			return FALSE;
	}
}

/*
 * Function to collect data from given COM port.
 * Used by each thread separately. 
 */
void collectData(LPWSTR portName) {
	char *buf = NULL;
	SerialComm  *SC = new SerialComm(portName);
	std::ofstream outputFile;

	std::ostringstream oss;
	oss << CW2A(portName);
	std::string port = oss.str();

	// open output file
	outputFile.open("raw_output_" + port + ".csv");

	// Timestamp in microseconds since EPOCH (00:00:00 Jan 1, 1970)
	using time_stamp = std::chrono::time_point<std::chrono::system_clock,
		std::chrono::microseconds>;

	if (SC->IsConnected())
		std::cout << "Connected to " << port << std::endl;

	std::cout << "Collecting from " << port << "..." << std::endl;

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
				microseconds ms = duration_cast< microseconds >(
					system_clock::now().time_since_epoch());

				// data comes in chunks, reduce chunks to lines and apply timetamp.
				std::string line;
				std::istringstream f(incomingData);

				while (std::getline(f, line)) {
				//	std::cout << port << "," << ms.count() << "," << line << std::endl;
					outputFile << ms.count() << "," << line << std::endl;
				}
			}
		}
	}
}


int main() {
	
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE)) {
		std::cout << "Handler Installed!\n";
	}

	std::thread COM6_thread(collectData, COM6);
	std::thread COM9_thread(collectData, COM9);


	// synchronize threads:
	COM9_thread.join();                // pauses until first finishes
	COM6_thread.join();               // pauses until second finishes

	return 0;
}





