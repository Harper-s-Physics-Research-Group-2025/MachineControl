#include "pubSysCls.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <sstream>
#include <queue>
#include <unordered_set>

using namespace std;
using namespace sFnd;

#pragma comment(lib, "user32.lib")

#define WM_KEY_EVENT (WM_USER + 1)      // custom message channel

// Event structure to hold key events
struct KeyEvent {
    DWORD vkCode;
    bool pressed;  // true = down, false = up
};

// Shared queue and synchronization
queue<KeyEvent> keyQueue;
CRITICAL_SECTION cs;

HHOOK g_keyboardHook;
volatile bool g_running = true;
bool g_initialized_motors = false;
volatile bool g_emergency_stop = false;
DWORD g_mainThreadId;       // for posting messages?

// Teknic motors
SysManager* Mgr = nullptr;
IPort* Port;
INode* motorX; // Controlled with Left/Right
INode* motorZ; // Controlled with Up/Down
    
vector<string> comHubPorts;                 // usually only one port (board) and up to 4 motors per port
unordered_set<DWORD> keys;
int v_x;                            // desired velocity of motors possibly should be double
int v_z;

// function definitions
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);     // keyboard callback
void msgUser(const char* msg);  // message user
void home(SysManager* manager, INode* motor, const char* name, const int timeout); // home axes of machine. Specifications can be set with ClearView Software
void print_motor_info(INode* motor);




int main(int argc, char **argv) {

    // Initialize synchronization
    InitializeCriticalSection(&cs);
    g_mainThreadId = GetCurrentThreadId();      // get for PostThreadMessage
    
    
    // Install keyboard hook
    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    if (g_keyboardHook == NULL) {
        cerr << "Failed to install keyboard hook. Error: " << GetLastError() << endl;
        DeleteCriticalSection(&cs);
        return 1;
    }

    // Run Program
    // Ensure message queue exists
    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);


    try {
        // Main message + event handling loop
        while (g_running) {

            if (!g_initialized_motors) {
                Mgr = SysManager::Instance();

                // Find Ports
                SysManager::FindComHubPorts(comHubPorts);
                printf("Found %llu SC Hubs\n", comHubPorts.size());

                if (comHubPorts.size() == 1) {
                    // assign ports
                    Mgr->ComHubPort(0, comHubPorts[0].c_str()); // for our use case we will only ever be using one com port (circuit board controller)
                } else {
                    printf("Found number (%llu) ports that is not 1\n", comHubPorts.size()); // handle case with more than one port found
                    msgUser("Press any key to confirm and exit");
                    return -1;
                }

                // Open the port(s)
                Mgr->PortsOpen(1);

                Port = &Mgr->Ports(0);
                motorX = &Port->Nodes(0); // Controlled with Left/Right
                motorZ = &Port->Nodes(1); // Controlled with Up/Down
                
                motorX->Status.AlertsClear();                   //Clear Alerts on node 
                motorZ->Status.AlertsClear();

                motorX->Motion.NodeStopClear();                  // Clear Nodestops ?
                motorZ->Motion.NodeStopClear();
                
                motorX->EnableReq(true);
                motorZ->EnableReq(true);
                Sleep(200); // wait for enabling

                
                // print motor (Node) information

                printf("Motor X  Serial #: %d\n", motorX->Info.SerialNumber.Value());
                printf("Motor Z  Serial #: %d\n", motorZ->Info.SerialNumber.Value());
                // printf("Press enter to continue and home motors, any other key to exit...");
                // char c = _getch();
                // if (c != '\n' && c != '\r') break;
                // printf("Enabling and Homing Motors\n");


                
                // // Homing
                // printf("current position (X, Z): (%.0f,%.0f)\n", motorX->Motion.PosnMeasured.Value(), motorZ->Motion.PosnMeasured.Value());
                // home(Mgr, motorX, "X-axis", 20000);
                // home(Mgr, motorZ, "Z-axis", 20000);
                // printf("current position (X, Z): (%.0f,%.0f)\n", motorX->Motion.PosnMeasured.Value(), motorZ->Motion.PosnMeasured.Value());

                g_initialized_motors = true;
                cout << "Use arrow keys to move motors. Press 'q' to quit.\n";
            }

                

            if(GetMessage(&msg, NULL, 0, 0)) {
                if (msg.message == WM_KEY_EVENT) {
                    
                    // Process key events
                    EnterCriticalSection(&cs); // Message queue

                    while (!keyQueue.empty()) {
                        KeyEvent evt = keyQueue.front();
                        keyQueue.pop();
                        LeaveCriticalSection(&cs);  // Unlock while processing

                        // Add actions to set
                        if (evt.pressed) {
                            // add to set
                            keys.insert(evt.vkCode);
                        } else {
                            // remove from set
                            keys.erase(evt.vkCode);
                        }

                        EnterCriticalSection(&cs);  // Re-lock before next iteration
                    }

                    LeaveCriticalSection(&cs);

                    // move motors according to key strokes
                    v_x = (keys.find(39) != keys.end()) - (keys.find(37) != keys.end());
                    v_z = (keys.find(38) != keys.end()) - (keys.find(40) != keys.end());

                    motorX->Motion.MoveVelStart(50*v_x);
                    motorZ->Motion.MoveVelStart(50*v_z);


                } else {
                    TranslateMessage(&msg);     // idk what this does
                    DispatchMessage(&msg);
                }

            }

            }

    } catch (mnErr& theErr) {
        cout << "Caught error: " << theErr.ErrorMsg << "\n";
    } catch (exception& e) {
        cout << "Caught error: " << e.what() << endl;
        printf("Node did not complete homing:  \n\t -Ensure Homing settings have been defined through ClearView. \n\t -Check for alerts/Shutdowns \n\t -Ensure timeout is longer than the longest possible homing move.\n");

    }

    // Safe disable. May want wrapper for this
    if (motorX) motorX->EnableReq(false);
    if (motorZ) motorZ->EnableReq(false);
    if (Mgr) Mgr->PortsClose();

    

    // Unload the hook
    UnhookWindowsHookEx(g_keyboardHook);
    DeleteCriticalSection(&cs);
    cout << "exiting..." << endl;

    return 0;
}



// Callback function to handle keyboard functionality (different thread)
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {

        KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
        KeyEvent evt{ pKeyInfo->vkCode, wParam == WM_KEYDOWN };

        // Emergency stop on 'Esc'
        if (evt.pressed && evt.vkCode == VK_ESCAPE) {
            printf("E-stop called");
            if (Port) {
                Port->GrpShutdown.ShutdownInitiate();
            }
            g_running = false;
            PostQuitMessage(0);
        }

        // Store the event in the shared queue
        EnterCriticalSection(&cs);
        keyQueue.push(evt);
        LeaveCriticalSection(&cs);

        // wake main thread
        PostThreadMessage(g_mainThreadId, WM_KEY_EVENT, 0, 0);

        // Exit if 'q' is pressed
        if (evt.pressed && evt.vkCode == 'Q') {
            g_running = false;
            PostQuitMessage(0);
        }

        // Suppress only arrow keys and Escape
        if (evt.vkCode == VK_LEFT || evt.vkCode == VK_RIGHT || evt.vkCode == VK_UP || evt.vkCode == VK_DOWN || evt.vkCode == 'Q') {
            return 1; // Block this key from reaching other apps
        }
        
    }

    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}


// Send message and wait for newline
void msgUser(const char* msg) {
    cout << msg;
    getchar(); //pauses code execution.
}


void home(SysManager* manager, INode* motor, const char* name, const int timeout) {
    // requires Sysmanager, motor, motor name, and timeout in ms.
    
    // Check the Node to see if it has already been homed, 
    if (motor->Motion.Homing.HomingValid()) {
        // Print state of node
        if (motor->Motion.Homing.WasHomed()) {
            printf("%s has already been homed, Rehoming...\n", name);
        } else {
            printf("%s has not been homed.  Homing Node now...\n", name);
        }

        //home the Node
        motor->Motion.Homing.Initiate();

        int home_timestamp = manager->TimeStampMsec() + timeout;    //define a timeout in case the node is unable to enable
        
        // Basic mode - Poll until disabled
        while (!motor->Motion.Homing.WasHomed()) {
            // Process any posted messages (keyboard input, etc.)
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

        motor->Motion.PosnMeasured.Refresh();      //Refresh our current measured position
        printf("%s completed homing with soft limits now active, current position:\t%8.0f\tsoft limits:\t[%d,%d]\n", name, motor->Motion.PosnMeasured.Value(), motor->Limits.SoftLimit1.Value(), motor->Limits.SoftLimit2.Value());
    }
}

void print_motor_info(INode* motor) {
    printf("   Node[%d]: type=%d\n", 1, motor->Info.NodeType());
    printf("            userID: %s\n", motor->Info.UserID.Value());
    printf("        FW version: %s\n", motor->Info.FirmwareVersion.Value());
    printf("          Serial #: %d\n", motor->Info.SerialNumber.Value());
    printf("             Model: %s\n", motor->Info.Model.Value());
}