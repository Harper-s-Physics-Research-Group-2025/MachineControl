
/*
get_lapjack_reading
arguments: none
returns: Lapjack reading, double
*/

#include <iostream>
#include "LabJackUD.h"   // Header for UD library

#pragma comment(lib, "LabJackUD.lib")  // Link UD lib (Windows)

using namespace std;


double get_lapjack_reading(){
    // Author: ChatGPT
    LJ_HANDLE handle;       // Handle for the device
    int errorcode;
    double voltage;

    // 1. Open the first found LabJack U3
    errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 0, &handle);
    if (errorcode != LJ_NOERROR) {
        std::cerr << "Failed opening device: error " << errorcode << "\n";
        return 1;
    }

    // 2. Read analog input AIN0 (single-ended)
    // IOType = LJ_ioGET_AIN, ChannelP = 0, ChannelN = 31 (ground)
    errorcode = eGet(handle, LJ_ioGET_AIN, 0, &voltage, 0);
    if (errorcode != LJ_NOERROR) {
        cerr << "Failed reading AIN0: error " << errorcode << "\n";
        CloseLabJack(h);
        return 1;
    }

    // 3. Print voltage
    cout << "AIN0 voltage = " << voltage << " V\n";

    // 4. Close the device
    Close();
    return voltage;
}
