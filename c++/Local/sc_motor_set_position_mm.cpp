#include "pubSysCls.h"
#include <iostream>
#include <windows.h>
#include <algorithm>

using namespace std;
using namespace sFnd;



// Teknic motors
SysManager* Mgr = nullptr;
IPort* Port;
INode* motorX; // Horizontal axis
INode* motorZ; // Vertical axis
    
vector<string> comHubPorts;                 // usually only one port (board) and up to 4 motors per port

const int vel_limit = 1000;     // max velocity rpm

vector<int> args = {0, 0, 240, 10000};     // (x, z), velocity, timeout


int main(int argc, char **argv) {
    // Set position of the lipid holder interface with the sc motors using mm (800 counts / mm)
    // Input: Desired position in mm (X, Z) and velocity to move in rpm
    // Output: Motor hub name and echos desired position in mm
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc < 3 || argc > 4) {
        cerr << "Usage <command> <x_pos> <z_pos> <vel (rpm)> <timeout (s)>" << endl;
        return 1;
    } 
     
    for (int i = 1; i < argc; i++) {
        try {
            args[i - 1] = stoi(argv[i]);    // populate argument array and convert types        
        } catch (const invalid_argument& e) {
            cerr << "Invalid input: " << e.what() << " '" << argv[i] << "'" << endl;
            return 2;
        }
    }
    
    
    if (args[2] > vel_limit) {
        cerr << "Desired velocity exceeds limit" << endl;
        return 1;
    }

    args[0] *= 800;     // convert to counts
    args[1] *= 800;     // convert to counts
    cout << args[1] << endl;

    // Run Program
    try {
        
        Mgr = SysManager::Instance();
        
        // Find Ports
        SysManager::FindComHubPorts(comHubPorts);
        

        if (comHubPorts.size() == 1) {
            // assign ports
            Mgr->ComHubPort(0, comHubPorts[0].c_str()); // for our use case we will only ever be using one com port (circuit board controller)
        } else {
            cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
            return 1;
        }

        // Open the port(s)
        Mgr->PortsOpen(1);

        Port = &Mgr->Ports(0);
        motorX = &Port->Nodes(0);
        motorZ = &Port->Nodes(1);

        if (!(motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) ||       // Ensure motors are homed
            !(motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid())) {
                cerr << "Motors are not homed. Please home before continuing" << endl;
                if (motorX) motorX->EnableReq(false);
                if (motorZ) motorZ->EnableReq(false);
                if (Mgr) Mgr->PortsClose();
                return 1;
        }

        motorX->Status.AlertsClear();                   //Clear Alerts on node 
        motorZ->Status.AlertsClear();

        motorX->Motion.NodeStopClear();                  // Clear Nodestops ?
        motorZ->Motion.NodeStopClear();
        
        motorX->EnableReq(true);
        motorZ->EnableReq(true);
        Sleep(200); // wait for enabling

        motorX->VelUnit(INode::RPM);                        //Set the units for Velocity to RPM
        motorZ->VelUnit(INode::RPM);                        //Set the units for Velocity to RPM
        
        motorX->Motion.VelLimit = args[2];              //Set Velocity Limit (RPM)
        motorZ->Motion.VelLimit = args[2];              //Set Velocity Limit (RPM)
        
        // Move motors to position (counts)
        motorX->Motion.MovePosnStart(args[0], true);
        motorZ->Motion.MovePosnStart(args[1], true);

        // args[3] = Mgr->TimeStampMsec() + 10000;
        args[3] = Mgr->TimeStampMsec() + 100 + max(motorX->Motion.MovePosnDurationMsec(args[0], true), motorZ->Motion.MovePosnDurationMsec(args[1], true));

        // Wait for moves to complete or timeout
        while (!motorX->Motion.MoveIsDone() || !motorZ->Motion.MoveIsDone()) {
            if (Mgr->TimeStampMsec() > args[3]) {
                cerr << "Movement timed out" << endl;
                return 1;
            }
        }

        cout << "(" << motorX->Motion.PosnMeasured.Value() / 800 << ", " << motorZ->Motion.PosnMeasured.Value() / 800 << ")" << endl;

    } catch (mnErr& theErr) {
        cerr << "Caught error: " << theErr.ErrorMsg << "\n";
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();
        return 1;
    
    } catch (exception& e) {
        cerr << "Caught error: " << e.what() << endl;
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();
        return 1;

    }

    if (motorX) motorX->EnableReq(false);
    if (motorZ) motorZ->EnableReq(false);
    if (Mgr) Mgr->PortsClose();

    return 0;
}