#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "pubSysCls.h"
#include "Oven5R6900.h"
#include "RTE7.h"
#include "LabJackUD.h"   // Header for UD library
#pragma comment(lib, "LabJackUD.lib")  // Link UD lib (Windows)



using namespace std;
using namespace sFnd;


// Teknic motors
sFnd::SysManager* Mgr = nullptr;
IPort* Port;
INode* motorX; // Controlled with Left/Right
INode* motorZ; // Controlled with Up/Down


// functions
bool init_teknic_motors(SysManager* Mgr, INode*& X, INode*& Z);
string currentTimestamp();
bool init_labjack(LJ_HANDLE& h);
double poll_labjack(LJ_HANDLE h);
 


int main(int argc, char** argv) {
    // Record timestamp, bath temp, sample temp, laser intensity to csv file (specify filename with command line arg)

    // Teknic motors
    sFnd::SysManager* Mgr = nullptr;
    IPort* Port;
    INode* motorX; // Controlled with Left/Right
    INode* motorZ; // Controlled with Up/Down

    // Labjack
    LJ_HANDLE Labjack;
    
    int interval;
    cout << "Enter polling interval in seconds: ";
    cin >> interval;

    // // motors
    // Mgr = SysManager::Instance();
    // if (!init_teknic_motors(Mgr, motorX, motorZ)) return -1;
    
    // Output CSV file
    string log_file = "device_log.csv";
    if (argc > 1) {
        log_file = argv[2];
    }

    ofstream file(log_file);
    if (!file.is_open()) {
        cerr << "Failed to open CSV file for writing.\n";
        return 1;
    }

    string bath_port, temp_port;
    cout << "Enter bath port: ";
    cin >> bath_port;
    cout << "Enter Temperature Controller port: ";
    cin >> temp_port;

    RTE7 Bath = RTE7("COM" + bath_port);
    Oven5R6900 Temp_controller = Oven5R6900("COM" + temp_port);
    if(!init_labjack(Labjack)) return -1;




    // Write header
    file << "timestamp,bath_temp,sample_temp,laser_intensity\n";
    cout << "Polling started. Press Ctrl+C to stop.\n";

    float bath_temp, sample_temp, laser_intensity;
    string timestamp;

    while (true) {
        timestamp = currentTimestamp();
        Bath.get_temp(bath_temp);
        Temp_controller.get_temp(sample_temp);
        laser_intensity = poll_labjack(Labjack);

        // Write row to CSV
        file << timestamp << "," << to_string(bath_temp) << "," << to_string(sample_temp) << "," << to_string(laser_intensity) << "\n";
        file.flush();  // Ensure data is written to disk

        cout << "[" << timestamp << "] Polled data: " 
                  << to_string(bath_temp) << "," << to_string(sample_temp) << "," << to_string(laser_intensity) << endl;

        this_thread::sleep_for(chrono::seconds(interval));
    }

    // file.close(); // Not reached in this example
    return 0;
}



bool init_teknic_motors(SysManager* Mgr, INode*& X, INode*& Z) {

    vector<string> comHubPorts;

    // Find Ports
    SysManager::FindComHubPorts(comHubPorts);
    printf("Found %llu SC Hubs\n", comHubPorts.size());

    if (comHubPorts.size() == 1) {
        // assign ports
        Mgr->ComHubPort(0, comHubPorts[0].c_str()); // for our use case we will only ever be using one com port (circuit board controller)
    } else {
        printf("Found number (%llu) ports that is not 1\n", comHubPorts.size()); // handle case with more than one port found
        return false;
    }

    // Open the port(s)
    Mgr->PortsOpen(1);

    IPort* P = &Mgr->Ports(0);
    X = &P->Nodes(0); // Controlled with Left/Right
    Z = &P->Nodes(1); // Controlled with Up/Down

    // Homing
    printf("current position (X, Z): (%.0f,%.0f)\n", X->Motion.PosnMeasured.Value(), Z->Motion.PosnMeasured.Value());

    return true;
}


// Get current timestamp as string
string currentTimestamp() {
    time_t now = time(nullptr);
    tm* timeinfo = localtime(&now);
    ostringstream ss;
    ss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}



bool init_labjack(LJ_HANDLE& h) {

    int errorcode;
    double voltage;

    // 1. Open the first found LabJack U3
    errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when opening the LabJack." << endl;
        return false;
    }
    
    // 2. Initialize settings on the LabJack
    ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, 0, 1, 0);  // Set channel 0 to analog input

    // Optional but recommended: set range and resolution
    // ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RANGE, LJ_rgUNI5V, 0);        // 0–5V
    ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, 1, 0);            // Resolution index

    return true;

}


// poll labjack
double poll_labjack(LJ_HANDLE h) {
    int errorcode;
    double voltage;

    // 2. Read analog input AIN0 (single-ended)
    // IOType = LJ_ioGET_AIN, ChannelP = 0, ChannelN = 31 (ground)
    errorcode = eGet(h, LJ_ioGET_AIN, 0, &voltage, 0);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when reading the LabJack." << endl;
        return -2;
    }

    return voltage;
}