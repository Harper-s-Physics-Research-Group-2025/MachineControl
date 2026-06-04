#include "sm_manual_controller.h"

using namespace std;
using namespace sFnd;

int ManualController::control() {
    pInstance = this; // Bind instance pointer
    InitializeCriticalSection(&cs);
    g_mainThreadId = GetCurrentThreadId();      
    
    // Pass the static wrapper wrapper callback instead
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, StaticKeyboardProc, GetModuleHandle(NULL), 0);
    if (g_keyboardHook == NULL) {
        cerr << "Failed to install keyboard hook. Error: " << GetLastError() << endl;
        DeleteCriticalSection(&cs);
        return 1;
    }

    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

    try {
        while (g_running) {
            if (!g_initialized_motors) {
                Mgr = SysManager::Instance();
                SysManager::FindComHubPorts(comHubPorts);

                if (comHubPorts.size() == 1) {
                    Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
                } else {
                    printf("Found number (%llu) ports that is not 1\n", comHubPorts.size()); 
                    msgUser("Press any key to confirm and exit");
                    return -1;
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

                printf("Motor X  Serial #: %d\n", motorX->Info.SerialNumber.Value());
                printf("Motor Z  Serial #: %d\n", motorZ->Info.SerialNumber.Value());

                g_initialized_motors = true;
                cout << "Use arrow keys to move motors. Press 'q' to quit.\n";
            }

            if(GetMessage(&msg, NULL, 0, 0)) {
                if (msg.message == WM_KEY_EVENT) {
                    EnterCriticalSection(&cs); 

                    while (!keyQueue.empty()) {
                        KeyEvent evt = keyQueue.front();
                        keyQueue.pop();
                        LeaveCriticalSection(&cs);  

                        if (evt.pressed) {
                            keys.insert(evt.vkCode);
                        } else {
                            keys.erase(evt.vkCode);
                        }

                        EnterCriticalSection(&cs);  
                    }
                    LeaveCriticalSection(&cs);

                    v_x = (keys.find(VK_RIGHT) != keys.end()) - (keys.find(VK_LEFT) != keys.end());
                    v_z = (keys.find(VK_UP) != keys.end()) - (keys.find(VK_DOWN) != keys.end());

                    motorX->Motion.MoveVelStart(50 * v_x);
                    motorZ->Motion.MoveVelStart(50 * v_z);
                } else {
                    TranslateMessage(&msg);     
                    DispatchMessage(&msg);
                }
            }
        }
    } catch (mnErr& theErr) {
        cout << "Caught error: " << theErr.ErrorMsg << "\n";
    } catch (exception& e) {
        cout << "Caught error: " << e.what() << endl;
    }

    if (motorX) motorX->EnableReq(false);
    if (motorZ) motorZ->EnableReq(false);
    if (Mgr) Mgr->PortsClose();

    UnhookWindowsHookEx(g_keyboardHook);
    DeleteCriticalSection(&cs);
    cout << "exiting..." << endl;
    return 0;
}


LRESULT CALLBACK ManualController::StaticKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (pInstance) {
            return pInstance->KeyboardProc(nCode, wParam, lParam);
        }
        return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK ManualController::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
        if (nCode == HC_ACTION) {
            KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
            KeyEvent evt{ pKeyInfo->vkCode, wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN };

            if (evt.pressed && evt.vkCode == VK_ESCAPE) {
                printf("E-stop called\n");
                if (Port) {
                    Port->GrpShutdown.ShutdownInitiate();
                }
                g_running = false;
                PostQuitMessage(0);
            }

            EnterCriticalSection(&cs);
            keyQueue.push(evt);
            LeaveCriticalSection(&cs);

            PostThreadMessage(g_mainThreadId, WM_KEY_EVENT, 0, 0);

            if (evt.pressed && evt.vkCode == 'Q') {
                g_running = false;
                PostQuitMessage(0);
            }

            if (evt.vkCode == VK_LEFT || evt.vkCode == VK_RIGHT || evt.vkCode == VK_UP || evt.vkCode == VK_DOWN || evt.vkCode == 'Q') {
                return 1; 
            }
        }
        return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

void ManualController::msgUser(const char* msg) const {
        cout << msg;
        getchar(); 
}

void ManualController::home(SysManager* manager, INode* motor, const char* name, const int timeout) {
        if (motor->Motion.Homing.HomingValid()) {
            if (motor->Motion.Homing.WasHomed()) {
                printf("%s has already been homed, Rehoming...\n", name);
            } else {
                printf("%s has not been homed.  Homing Node now...\n", name);
            }

            motor->Motion.Homing.Initiate();
            int home_timestamp = manager->TimeStampMsec() + timeout;    
            
            while (!motor->Motion.Homing.WasHomed()) {
                MSG msg;
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                if (manager->TimeStampMsec() > home_timestamp) {
                    stringstream error_message;
                    error_message << "Homing timed out after "<< timeout << " milliseconds" << endl;
                    throw runtime_error(error_message.str());
                }
                Sleep(10);
            }

            motor->Motion.PosnMeasured.Refresh();      
            printf("%s completed homing with soft limits now active, current position:\t%8.0f\tsoft limits:\t[%d,%d]\n", name, motor->Motion.PosnMeasured.Value(), motor->Limits.SoftLimit1.Value(), motor->Limits.SoftLimit2.Value());
        }
}

void ManualController::print_motor_info(INode* motor) const{
    printf("   Node[%d]: type=%d\n", 1, motor->Info.NodeType());
    printf("            userID: %s\n", motor->Info.UserID.Value());
    printf("        FW version: %s\n", motor->Info.FirmwareVersion.Value());
    printf("          Serial #: %d\n", motor->Info.SerialNumber.Value());
    printf("             Model: %s\n", motor->Info.Model.Value());
}

// Allocate the static member translation pointer
ManualController* ManualController::pInstance = nullptr;