#include "controls/sm_homer.h"

using namespace std;
using namespace sFnd;

 

int Homer::homing(int milliseconds){
    SysManager* Mgr = nullptr;
    IPort* Port = nullptr;
    INode* motorX = nullptr; 
    INode* motorZ = nullptr; 
        
    vector<string> comHubPorts;                 
    vector<int> args = {milliseconds};    

    try {
        Mgr = SysManager::Instance();
        SysManager::FindComHubPorts(comHubPorts);
        
        if (comHubPorts.size() == 1) {
            Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
        } else {
            cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
            return 1;
        }

        Mgr->PortsOpen(1);
        Port = &Mgr->Ports(0);
        motorX = &Port->Nodes(0);
        motorZ = &Port->Nodes(1);

        motorX->Status.AlertsClear();                   
        motorZ->Status.AlertsClear();

        motorX->Motion.NodeStopClear();                  
        motorZ->Motion.NodeStopClear();
        
        motorX->EnableReq(true);
        motorZ->EnableReq(true);
        Sleep(200); 

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

void Homer::home(SysManager* manager, INode* motor, const char* name, const int timeout) {
    motor->Motion.Homing.Initiate();
    int home_timestamp = manager->TimeStampMsec() + timeout;    
    
    while (!motor->Motion.Homing.WasHomed()) {
        if (manager->TimeStampMsec() > home_timestamp) {
            cerr << "Homing timed out after "<< timeout << " milliseconds" << endl;
            return;
        }
        Sleep(10);
    }

    motor->Motion.PosnMeasured.Refresh();      
    cout << name << " completed homing with soft limits now active, current position (mm):  " 
            << motor->Motion.PosnMeasured.Value()/800 << "  soft limits:  [ " 
            << motor->Limits.SoftLimit1.Value()/800 << ", " << motor->Limits.SoftLimit2.Value()/800 << " ]" << endl;
}
