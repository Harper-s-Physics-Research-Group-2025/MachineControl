// listen on  COM port for Omega hh802u thermocouple

#include <windows.h>
#include <iostream>

int main() {
    // Set your COM port here
    const char* portName = "\\\\.\\COM6";

    // Open COM port
    HANDLE hSerial = CreateFileA(
        portName,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening COM port!" << std::endl;
        return 1;
    }

    // Configure serial port
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Failed to get current serial parameters!" << std::endl;
        CloseHandle(hSerial);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_19200; // For HH802U
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    // Disable all flow control
    dcbSerialParams.fOutxCtsFlow = FALSE;                 // No CTS
    dcbSerialParams.fRtsControl  = RTS_CONTROL_DISABLE;   // No RTS
    dcbSerialParams.fInX         = FALSE;                 // No XON/XOFF (input)
    dcbSerialParams.fOutX        = FALSE;                 // No XON/XOFF (output)

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Could not set serial port parameters!" << std::endl;
        CloseHandle(hSerial);
        return 1;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;

    SetCommTimeouts(hSerial, &timeouts);

    std::cout << "Listening on " << portName << " at 19200 baud..." << std::endl;

    // Read loop
    char buffer[256];
    DWORD bytesRead;

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';  // Null-terminate string
                std::cout << buffer;       // Print raw output
                std::flush(std::cout);
            }
        } else {
            std::cerr << "ReadFile failed." << std::endl;
            break;
        }
    }

    CloseHandle(hSerial);
    return 0;
}
