//#include "get_position_XY.h"
#include "pubSysCls.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;
using namespace sFnd;

/*
get_position_XY
arguments: none
returns: a vector of doubles
*/



//TODO:figure out what to return that indicates an error taht isn't 0.0
//TODO:figure out how to return an object in c++
vector<double> get_position_XY() {
    try {
        //store initial X and Y positions of our sample holder
        double xPos;
        double yPos;
        //Vector for storing the connected ports' names
        vector<string> comHubPorts;
        // Create instance of a System Manager. 
        SysManager* myMgr = SysManager::Instance();
        //*FindComHubPorts* finds the names of all the connected ports(SC Hubs) and stores them in the string vector comHubPorts.
        SysManager::FindComHubPorts(comHubPorts);
        if (comHubPorts.empty()) {
            cout << "No SC Hubs found." << endl;
            return { 0.0,0.0 };
        }

        for (size_t i = 0; i < comHubPorts.size(); ++i) {
            //*ComHubPort* associates different numbers to port names.
            myMgr->ComHubPort(i, comHubPorts[i].c_str());
        }
        myMgr->PortsOpen((int)comHubPorts.size()); //*PortsOpen* opens all the ports that were previously associated with numbers. 
        IPort& myPort = myMgr->Ports(0); //We have one SC hub connected, so we only need the first port

        // Test for presence of nodes
        if (myPort.NodeCount() == 0) {
            // Bail out, no nodes
            cout << "No nodes\n";
            return { 0.0,0.0 };
        }
        for (size_t i = 0; i < myPort.NodeCount(); i++) {
            //9. Create instances of the x and y node
            INode& node = myPort.Nodes(i); //i don't know for sure which is the x or y node
            node.EnableReq(false);               //Ensure Node is disabled before loading config file

            myMgr->Delay(200);
            //printf("   Node[%d]: type=%d\n", int(i), node.Info.NodeType());
            //printf("            userID: %s\n", node.Info.UserID.Value());
            //printf("        FW version: %s\n", node.Info.FirmwareVersion.Value());
            //printf("          Serial #: %d\n", node.Info.SerialNumber.Value());
            //printf("             Model: %s\n", node.Info.Model.Value());
            //TODO:Clear all errors and enable nodes
            node.Status.AlertsClear();
            node.Motion.NodeStopClear();
            //xNode.Motion.MovePosnStart(1000);
            if(i == 0) xPos = node.Motion.PosnMeasured;
            else yPos = node.Motion.PosnMeasured;
            
            
        }
        // Close down the ports
        myMgr->PortsClose();

        return { xPos, yPos };
       

    }
    catch (mnErr& theErr) {
        printf("Caught error: %s\n", theErr.ErrorMsg);
        printf("Aborting with error code: 0x%x\n", theErr.ErrorCode);
        return { 0.0,0.0 };
    }
}

int main() {
	vector<double> position = get_position_XY(); // Call the function to test it
    cout << "X-position: " << position[0] << ".\nY-position: " << position[1] << endl; // Call the function to test it
    return 0;
}