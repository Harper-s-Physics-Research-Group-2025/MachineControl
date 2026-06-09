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
        //If there are nodes(Motors) present, get a reference to the first one
        INode& xNode = myPort.Nodes(0);
        // INode& yNode = myPort.Nodes(1);

        //TODO:Clear all errors and enable nodes
        xNode.Status.AlertsClear();
        xNode.Motion.NodeStopClear();
        //xNode.Motion.MovePosnStart(1000);
        xPos = xNode.Motion.PosnMeasured;
        return { xPos,0.0 };

    }
    catch (mnErr& theErr) {
        printf("Caught error: %s\n", theErr.ErrorMsg);
        printf("Aborting with error code: 0x%x\n", theErr.ErrorCode);
        return { 0.0,0.0 };
    }
}