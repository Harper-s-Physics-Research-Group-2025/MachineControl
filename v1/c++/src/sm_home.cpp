#include "pubSysCls.h"
#include <iostream>
#include <windows.h>



using namespace std;
using namespace sFnd;


// Teknic motors
SysManager* Mgr = nullptr;
IPort* Port;
INode* motorX; // Controlled with Left/Right
INode* motorZ; // Controlled with Up/Down
    
vector<string> comHubPorts;                 // usually only one port (board) and up to 4 motors per port

vector<int> args = {20000};    // timeout milliseconds

void home(SysManager* manager, INode* motor, const char* name, const int timeout); // home axes of machine. Specifications can be set with ClearView Software



int main(int argc, char** argv) {
    // Home the motors to switches (A, A) which corresponds to the top left of the area they can move in. Adjust this in the Clearview software.
    // Input: Optional timeout in milliseconds
    // Output: Position in counts
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc > 2) {
        cerr << "Usage <command>" << endl;
        return 1;
    } 
    
    for (int i = 1; i < argc; i++) {
        args[i - 1] = stoi(argv[i]);    // populate argument array and convert types
    }


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

        motorX->Status.AlertsClear();                   //Clear Alerts on node 
        motorZ->Status.AlertsClear();

        motorX->Motion.NodeStopClear();                  // Clear Nodestops ?
        motorZ->Motion.NodeStopClear();
        
        motorX->EnableReq(true);
        motorZ->EnableReq(true);
        Sleep(200); // wait for enabling

        // Homing
        home(Mgr, motorX, "X-axis", args[0]);
        home(Mgr, motorZ, "Z-axis", args[0]);

        cout << "Final position (mm):  (" << motorX->Motion.PosnMeasured.Value() / 800 << ", " << motorZ->Motion.PosnMeasured.Value() / 800 << ")" << endl;
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



void home(SysManager* manager, INode* motor, const char* name, const int timeout) {
    // requires Sysmanager, motor, motor name, and timeout in ms.

    //home the Node
    motor->Motion.Homing.Initiate();

    int home_timestamp = manager->TimeStampMsec() + timeout;    //define a timeout in case the node is unable to enable
    
    // Basic mode - Poll until disabled
    while (!motor->Motion.Homing.WasHomed()) {
        if (manager->TimeStampMsec() > home_timestamp) {
            cerr << "Homing timed out after "<< timeout << " milliseconds" << endl;
            return;
        }
        Sleep(10);
    }

    motor->Motion.PosnMeasured.Refresh();      //Refresh our current measured position
    cout << name << " completed homing with soft limits now active, current position (mm):  " << motor->Motion.PosnMeasured.Value()/800 << "  soft limits:  [ " << motor->Limits.SoftLimit1.Value()/800 << ", " << motor->Limits.SoftLimit2.Value()/800 << " ]" << endl;

}