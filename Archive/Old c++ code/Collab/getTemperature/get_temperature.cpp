//#include <iostream>
//#include <string>
//using namespace std;
//
//int main() {
//    int number = 42;
//    int* ptr = &number; //stores the address(or reference) of number
//    cout << "The value of number is: " << number << endl;
//    cout << ptr << endl;
//    *ptr = 20;
//    cout << "The value of number is: " << number << endl;
//
//    return 0;
//}

#include <windows.h>
#include <iostream>

using namespace std;

int main() {
    // Change COM3 to the actual COM port assigned to your oven device
    HANDLE hSerial = CreateFile(
        L"\\\\.\\COM9",                  // COM port name
        GENERIC_READ | GENERIC_WRITE,   // Access type
        0,                              // No sharing
        NULL,                           // Default security
        OPEN_EXISTING,                  // Open existing port
        0,                              // No special attributes
        NULL);                          // No template

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to open COM port.\n";
        return 1;
    }

    // Configure serial port
	//Device Control Block (DCB) structure to set parameters
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error getting port state.\n";
        CloseHandle(hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_9600;     // Check your device manual
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error setting port state.\n";
        CloseHandle(hSerial);
        return 1;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(hSerial, &timeouts);

    // Send temperature read command
    const char* command = "R\r";  // Confirm this is the correct command for your model
    DWORD bytesWritten;
    if (!WriteFile(hSerial, command, strlen(command), &bytesWritten, NULL)) {
        std::cerr << "Error writing to port.\n";
        CloseHandle(hSerial);
        return 1;
    }

    // Read response
    char buffer[100];
    DWORD bytesRead;
    if (!ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
        std::cerr << "Error reading from port.\n";
        CloseHandle(hSerial);
        return 1;
    }

    buffer[bytesRead] = '\0';  // Null-terminate

    std::cout << "Received temperature: " << buffer << "\n";

    CloseHandle(hSerial);
    return 0;
}


/*
get_temperature
arguments: none
returns: temperature reading, double
*/
//double get_temperature() {
//    return 0.0
//}