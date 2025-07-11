
/*
get_lapjack_reading
arguments: none
returns: Lapjack reading, double
*/

#include <iostream>
#include "LabJackUD.h"   // Header for UD library

#pragma comment(lib, "LabJackUD.lib")  // Link UD lib (Windows)

using namespace std;


int main() {
    LJ_HANDLE h;       // Handle for the device
    int errorcode;
    double voltage;

    // 1. Open the first found LabJack U3
    errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when opening the LabJack." << endl;
        return -1;
    }
    
    // 2. Initialize settings on the LabJack
    ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, 0, 1, 0);  // Set channel 0 to analog input

    // Optional but recommended: set range and resolution
    // ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RANGE, LJ_rgUNI5V, 0);        // 0–5V
    ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, 1, 0);            // Resolution index

    // 2. Read analog input AIN0 (single-ended)
    // IOType = LJ_ioGET_AIN, ChannelP = 0, ChannelN = 31 (ground)
    errorcode = eGet(h, LJ_ioGET_AIN, 0, &voltage, 0);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when reading the LabJack." << endl;
        return -2;
    }

    // 3. Print voltage
    cout << "AIN0 voltage = " << voltage << " V\n";

    // 4. Close the device
    Close();
    return (int)voltage;
}
