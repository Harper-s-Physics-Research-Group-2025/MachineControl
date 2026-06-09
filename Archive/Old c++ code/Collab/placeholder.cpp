// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//This example shows how to connect to a SC Hub and get the first node's information. Names that appear like this *ExampleName* refer to methods.

#include <iostream>
#include <string.h>
#include <iostream>
#include "pubSysCls.h"
#include <vector> //needed for creating vectors
#include <string> //needed for creating strings

using namespace std;
using namespace sFnd;

#define ACC_LIM_RPM_PER_SEC 100000
#define VEL_LIM_RPM         700
#define MOVE_DISTANCE_CNTS  100000 //The distance to move in encoder counts. This is a positive number, but the node will move in the negative direction because of the MovePosnStart function below.   
#define NUM_MOVES           5
#define TIME_TILL_TIMEOUT   10000   //The timeout used for homing(ms)

int main()

{
    try {
		// Create instance of a System Manager. Note: you cannot use the SysManager constructor for this. This is the main interface to the system.
        SysManager* myMgr = SysManager::Instance();
        size_t ind = 0;

        
        vector<string> comHubPorts;
		SysManager::FindComHubPorts(comHubPorts); //*FindComHubPorts* finds the names of all the connected ports(SC Hubs) and stores them in the string vector comHubPorts.
        if (comHubPorts.empty()) {
            cout << "No SC Hubs found." << endl;
            return 0;
        }

        for (size_t i = 0; i < comHubPorts.size(); ++i) {
			myMgr->ComHubPort(i, comHubPorts[i].c_str()); //*ComHubPort* associates different numbers to port names. For example, 0 -> COM7, 1 -> COM8, etc. This is useful for referring to ports later in the code.
        }
		myMgr->PortsOpen((int)comHubPorts.size()); //*PortsOpen* opens all the ports that were previously associated with numbers. This is necessary to communicate with the SC Hubs.

        //myMgr->Delay(10000);-- this works
       // This example configures the first port(netNumber=0) to refer to windows COM7
        //myMgr->ComHubPort(0, 7, MN_BAUD_24X);
        // Tell system to open this one port
        //myMgr->PortsOpen(ind);
        // Create useful reference to the first port(SC Hub) object
        IPort& myPort = myMgr->Ports(ind);
        

        // Test for presence of nodes
        if (myPort.NodeCount() == 0) {
            // Bail out, no nodes
            cout << "No nodes\n";
            return 0;
        }
        //// If there are nodes(Motors) present, get a reference to the first one
        INode& theNode = myPort.Nodes(0);
        //// You can now interact with myNode
        theNode.EnableReq(true);

       

        printf("            userID: %s\n", theNode.Info.UserID.Value());
        printf("        FW version: %s\n", theNode.Info.FirmwareVersion.Value());
        printf("          Serial #: %d\n", theNode.Info.SerialNumber.Value());
        printf("             Model: %s\n", theNode.Info.Model.Value());
        cout << theNode.Motion.MovePosnDurationMsec(1000) << endl;

        /*Movement and positioning*/
        theNode.Motion.MoveWentDone();                      //Clear the rising edge Move done register

        theNode.AccUnit(INode::RPM_PER_SEC);                //Set the units for Acceleration to RPM/SEC
        theNode.VelUnit(INode::RPM);                        //Set the units for Velocity to RPM
        theNode.Motion.AccLimit = ACC_LIM_RPM_PER_SEC;      //Set Acceleration Limit (RPM/Sec)
        theNode.Motion.VelLimit = VEL_LIM_RPM;
        theNode.Status.AlertsClear();                   //Clear Alerts on node 
        theNode.Motion.NodeStopClear(); //if you don't clear alerts or nodestop, your motor won't move after it is forced to stop.
		theNode.Motion.MovePosnStart(MOVE_DISTANCE_CNTS);           //Execute 100000 encoder count move. *MovePosnStart* is a non-blocking call, so the node will start moving the motor and return immediately.
        myMgr->Delay(200);
        theNode.Motion.MovePosnStart(-MOVE_DISTANCE_CNTS);           //Execute 100000 encoder count move. *MovePosnStart* is a non-blocking call, so the node will start moving the motor and return immediately.
        printf("%f estiomated time.\n", theNode.Motion.MovePosnDurationMsec(MOVE_DISTANCE_CNTS));
        double timeout = myMgr->TimeStampMsec() + theNode.Motion.MovePosnDurationMsec(MOVE_DISTANCE_CNTS) + 100;         //define a timeout in case the node is unable to enable

        while (!theNode.Motion.MoveIsDone()) {
            if (myMgr->TimeStampMsec() > timeout) {
                printf("Error: Timed out waiting for move to complete\n");
                return -2;
            }
        }

        cout << "Done...\n";

        for (size_t iNode = 0; iNode < myPort.NodeCount(); iNode++) {
            // Create a shortcut reference for a node
            myPort.Nodes(iNode).EnableReq(false);
        }
        


    }
    catch (mnErr& theErr) {
        printf("Caught error: %s\n", theErr.ErrorMsg);
        printf("Aborting with error code: 0x%x\n", theErr.ErrorCode);
       
    }
   


    return 0;

}


