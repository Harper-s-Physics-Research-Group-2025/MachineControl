

#include "pubSysCls.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstdlib> //for atoi

using namespace std;
using namespace sFnd;


#include "pubSysCls.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <string>

using namespace std;
using namespace sFnd;

#define ACC_LIM_RPM_PER_SEC 100000
#define VEL_LIM_RPM         70
#define TIME_TILL_TIMEOUT   10000 

// Send message and wait for newline
void msgUser(const char* msg) {
    std::cout << msg;
    getchar();
}



/*
move_to_position_XY
arguments: X position, Integer; Y position, Integer
returns: void
*/
void  move_to_position_XY(int x, int y) {
    double timeout;

    double xPos;
    double xPosCommanded;
    double yPos;

    //3. Access X and Y nodes
    try {
        SysManager* myMgr = SysManager::Instance();
        vector <string> comHubPorts;
        //4.
        SysManager::FindComHubPorts(comHubPorts); //*FindComHubPorts* finds the names of all the connected ports(SC Hubs) and stores them in the string vector comHubPorts.
        if (comHubPorts.empty()) {
            cout << "No SC Hubs found." << endl;
            return;
        }
        //5.
        for (size_t i = 0; i < comHubPorts.size(); ++i) myMgr->ComHubPort(i, comHubPorts[i].c_str()); //*ComHubPort* associates different numbers to port names. For example, 0 -> COM7, 1 -> COM8, etc. This is useful for referring to ports later in the code.
        //6.
        myMgr->PortsOpen((int)comHubPorts.size()); //*PortsOpen* opens all the ports(SC Hubs) that were previously associated with numbers. This is necessary to communicate with the SC Hubs.
        //7. This creates an instance of the first SC Hub if multiple hubs are connected, and only SC hub if one hub is connected
        IPort& myPort = myMgr->Ports(0);
        //8. Test for presence of nodes
        if (myPort.NodeCount() == 0) {
            // Bail out, no nodes
            cout << "No nodes\n";
            return;
        }

        for (size_t i = 0; i < myPort.NodeCount(); i++) {
            //9. Create instances of the x and y node
            INode& node = myPort.Nodes(i); //i don't know for sure which is the x or y node
            node.EnableReq(false);
            node.Status.AlertsClear();
            node.Motion.NodeStopClear();
            node.EnableReq(true);
            //10. Checks if the motor is enabled and ready to move
            timeout = myMgr->TimeStampMsec() + TIME_TILL_TIMEOUT;
            while (!node.Status.IsReady())
                if (myMgr->TimeStampMsec() > timeout) {
                    printf("Error: Timed out waiting for node to become ready\n");
                    return;
                }
            //11. Check the Node to see if it has already been homed, 
            if (node.Motion.Homing.HomingValid())
            {
                if (node.Motion.Homing.WasHomed())
                {
                    //FIXME:  here
                    printf("The Node has already been homed, current position is: \t%8.0f \n", node.Motion.PosnMeasured.Value());
                    printf("Rehoming Node... \n");
                }
                else
                {
                    printf("Node has not been homed.  Homing Node now...\n");
                }
                //Now we will home the Node
                node.Motion.Homing.Initiate();

                timeout = myMgr->TimeStampMsec() + TIME_TILL_TIMEOUT;    //define a timeout in case the node is unable to enable
                // Basic mode - Poll until disabled
                while (!node.Motion.Homing.WasHomed()) {
                    if (myMgr->TimeStampMsec() > timeout) {
                        printf("Node did not complete homing:  \n\t -Ensure Homing settings have been defined through ClearView. \n\t -Check for alerts/Shutdowns \n\t -Ensure timeout is longer than the longest possible homing move.\n");
                        return;
                    }
                }
                printf("Node completed homing\n");
            }
            else {
                printf("Node has not had homing setup through ClearView.  The node will not be homed.\n");
            }



            node.AccUnit(INode::RPM_PER_SEC);                //Set the units for Acceleration to RPM/SEC
            node.VelUnit(INode::RPM);
            node.Motion.AccLimit = ACC_LIM_RPM_PER_SEC;
            node.Motion.VelLimit = VEL_LIM_RPM;
            node.EnableReq(true);
            myMgr->Delay(1000);
            i == 0 ? node.Motion.MovePosnStart(x) : node.Motion.MovePosnStart(y);



            /*timeout = myMgr->TimeStampMsec() + i > 0 ? node.Motion.MovePosnDurationMsec(y) : node.Motion.MovePosnDurationMsec(x) + TIME_TILL_TIMEOUT;*/         //define a timeout in case the node is unable to enable
            timeout = myMgr->TimeStampMsec() + node.Motion.MovePosnDurationMsec(x) + TIME_TILL_TIMEOUT;
            while (node.Motion.MoveIsDone()) {
                if (myMgr->TimeStampMsec() > timeout) {
                    printf("Error: Timed out waiting for move to complete\n");
                    return;
                }
            }
            node.Motion.MoveWentDone(); //Clears the move done register. Returns Bool

            if (i == 0) xPos = node.Motion.PosnMeasured.Value();
            else yPos = node.Motion.PosnMeasured.Value();
            myMgr->Delay(1000);

        }




        printf("Your final position is is: \t%8.0f \t%8.0f \n", xPos, yPos);

        for (size_t iNode = 0; iNode < myPort.NodeCount(); iNode++) {
            // Create a shortcut reference for a node
            myPort.Nodes(iNode).EnableReq(false);
        }
        // Close down the ports
        myMgr->PortsClose();
        cout << "Done...\n";
        return;
    }
    catch (mnErr& theErr) {
        printf("Caught error: %s\n", theErr.ErrorMsg);
        printf("Aborting with error code: 0x%x\n", theErr.ErrorCode);
        return;

    }

}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Not enough arguments. Usage: program.exe x y" << std::endl;
        return 1;
    }

    int x = std::atoi(argv[1]);  // Convert first argument to int
    int y = std::atoi(argv[2]);  // Convert second argument to int

    move_to_position_XY(x, y);
    


    return 0;
}

//int main() {
//    //For X, Positive <--
//    move_to_position_XY(10000, 10000);
//    return 0;
//}