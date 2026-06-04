#include "pubSysCls.h"
#include <iostream>

using namespace std;
using namespace sFnd;



// Teknic motors
SysManager* Mgr = nullptr;
IPort* Port;
INode* motorX; // Horizontal axis
INode* motorZ; // Vertical axis
    
vector<string> comHubPorts;                 // usually only one port (board) and up to 4 motors per port



int main(int argc, char **argv) {
    // Read position of the lipid holder interface from the sc motors in counts (6400 counts / revolution)
    // Input: none
    // Output: Motor hub name and position in counts
    // Returns: Errorcode, 0 = success, 1 = failure

    // check for correct number of command line arguments
    if (argc != 1) {
        cerr << "Usage: <command>" << endl;
        return 2;
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
        
        cout << "Counts: (" << motorX->Motion.PosnMeasured.Value() << ", " << motorZ->Motion.PosnMeasured.Value() << ")" << endl;

    } catch (mnErr& theErr) {
        cerr << "Caught error: " << theErr.ErrorMsg << "\n";
        if (Mgr) Mgr->PortsClose();
        return 1;
    
    } catch (exception& e) {
        cerr << "Caught error: " << e.what() << endl;
        if (Mgr) Mgr->PortsClose();
        return 1;

    }

    if (Mgr) Mgr->PortsClose();

    return 0;
}