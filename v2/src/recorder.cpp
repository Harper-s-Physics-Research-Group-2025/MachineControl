#include "controls/recorder.h"

using namespace std;
using namespace sFnd;

int Recorder::record(std::string CSV_FILENAME, std::string BATH_PORT, std::string TEMERATURE_PORT) const {
    sFnd::SysManager* Mgr = nullptr;
    IPort* Port;
    INode* motorX; 
    INode* motorZ; 

    LJ_HANDLE Labjack;
    
    int interval;
    cout << "Enter polling interval in seconds: ";
    cin >> interval;
    
    // FIXED: Replaced 'argv[2]' (which is out-of-scope) with the function parameter
    std::string log_file = CSV_FILENAME;

    ofstream file(log_file);
    if (!file.is_open()) {
        cerr << "Failed to open CSV file for writing.\n";
        return 1;
    }

    RTE7 Bath = RTE7(BATH_PORT); 
    Oven5R6900 Temp_controller = Oven5R6900(TEMERATURE_PORT);
    if(!init_labjack(Labjack)) return -1;

    file << "timestamp,bath_temp,sample_temp,laser_intensity\n";
    cout << "Polling started. Press Ctrl+C to stop.\n";

    float bath_temp, sample_temp, laser_intensity;
    string timestamp;

    while (true) {
        timestamp = currentTimestamp();
        Bath.get_temp(bath_temp);
        Temp_controller.get_temp(sample_temp);
        laser_intensity = poll_labjack(Labjack);

        file << timestamp << "," << to_string(bath_temp) << "," << to_string(sample_temp) << "," << to_string(laser_intensity) << "\n";
        file.flush();  

        cout << "[" << timestamp << "] Polled data: " 
                << to_string(bath_temp) << "," << to_string(sample_temp) << "," << to_string(laser_intensity) << endl;

        this_thread::sleep_for(chrono::seconds(interval));
    }
    return 0;
}


bool Recorder::init_teknic_motors(SysManager* Mgr, INode*& X, INode*& Z) const {
    vector<string> comHubPorts;
    SysManager::FindComHubPorts(comHubPorts);
    printf("Found %llu SC Hubs\n", comHubPorts.size());

    if (comHubPorts.size() == 1) {
        Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
    } else {
        printf("Found number (%llu) ports that is not 1\n", comHubPorts.size()); 
        return false;
    }

    Mgr->PortsOpen(1);
    IPort* P = &Mgr->Ports(0);
    X = &P->Nodes(0); 
    Z = &P->Nodes(1); 

    printf("current position (X, Z): (%.0f,%.0f)\n", X->Motion.PosnMeasured.Value(), Z->Motion.PosnMeasured.Value());
    return true;
}

string Recorder::currentTimestamp() const {
    time_t now = time(nullptr);
    tm* timeinfo = localtime(&now);
    ostringstream ss;
    ss << put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool Recorder::init_labjack(LJ_HANDLE& h) const {
    int errorcode;
    errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when opening the LabJack." << endl;
        return false;
    }
    ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, 0, 1, 0);  
    ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, 1, 0);            
    return true;
}

double Recorder::poll_labjack(LJ_HANDLE h) const {
    int errorcode;
    double voltage;
    errorcode = eGet(h, LJ_ioGET_AIN, 0, &voltage, 0);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when reading the LabJack." << endl;
        return -2;
    }
    return voltage;
}