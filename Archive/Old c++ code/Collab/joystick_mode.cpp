#include "pubSysCls.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
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
DWORD g_mainThreadId;       // for posting messages?

// Teknic motors
SysManager* myMgr = nullptr;
vector<string> comHubPorts;                 // usually only one port (board) and up to 4 motors per port
unordered_set<DWORD> keys;
int v_x;                            // possibly should be double
int v_z;

// function definitions
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);     // keyboard callback
void msgUser(const char* msg);  // message user

// DWORD g_fg_proc_id;     // only care about key presses when window is active
// DWORD g_console_proc_id;
// DWORD  g_curr_proc_id;

int main() {

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


    try {
        myMgr = SysManager::Instance();

        // Find Ports
        SysManager::FindComHubPorts(comHubPorts);
        printf("Found %llu SC Hubs\n", comHubPorts.size());

        if (comHubPorts.size() == 1) {
            // assign ports
            myMgr->ComHubPort(0, comHubPorts[0].c_str()); // for our use case we will only ever be using one com port (circuit board controller)
        } else {
            printf("Found number (%llu) ports that is not 1", comHubPorts.size()); // handle case with more than one port found
            msgUser("Press any key to confirm and exit");
            return -1;
        }

        

        // Open the port(s)
        myMgr->PortsOpen(1);

        IPort &myPort = myMgr->Ports(0);

        INode &motorX = myPort.Nodes(0); // Controlled with Left/Right
        INode &motorZ = myPort.Nodes(1); // Controlled with Up/Down
        
        // print motor (Node) information
        printf("   Node[%d]: type=%d\n", 0, motorX.Info.NodeType());
        printf("            userID: %s\n", motorX.Info.UserID.Value());
        printf("        FW version: %s\n", motorX.Info.FirmwareVersion.Value());
        printf("          Serial #: %d\n", motorX.Info.SerialNumber.Value());
        printf("             Model: %s\n", motorX.Info.Model.Value());

        printf("   Node[%d]: type=%d\n", 1, motorZ.Info.NodeType());
        printf("            userID: %s\n", motorZ.Info.UserID.Value());
        printf("        FW version: %s\n", motorZ.Info.FirmwareVersion.Value());
        printf("          Serial #: %d\n", motorZ.Info.SerialNumber.Value());
        printf("             Model: %s\n", motorZ.Info.Model.Value());

        motorX.Status.AlertsClear();                   //Clear Alerts on node 
        motorZ.Status.AlertsClear();

        motorX.Motion.NodeStopClear();                  // Clear Nodestops ?
        motorZ.Motion.NodeStopClear();
        
        motorX.EnableReq(true);
        motorZ.EnableReq(true);
        Sleep(200); // wait for enabling

        cout << "Use arrow keys to move motors. Press 'q' to quit.\n";

        // Run Program
        // Ensure message queue exists
        MSG msg;
        PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

        // Main message + event handling loop
        while (g_running) {

            if(GetMessage(&msg, NULL, 0, 0)) {
                if (msg.message == WM_KEY_EVENT) {
                    
                    // Process key events
                    EnterCriticalSection(&cs);

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
                    v_x = (keys.find(37) != keys.end()) - (keys.find(39) != keys.end());
                    v_z = (keys.find(38) != keys.end()) - (keys.find(40) != keys.end());

                    motorX.Motion.MoveVelStart(100*v_x);
                    motorZ.Motion.MoveVelStart(100*v_z);


                } else {
                    TranslateMessage(&msg);     // idk what this does
                    DispatchMessage(&msg);
                }

            }

        }

        motorX.EnableReq(false);
        motorZ.EnableReq(false);
        myMgr->PortsClose();


        

    } catch (mnErr& theErr) {
        cout << "Caught error: " << theErr.ErrorMsg << "\n";
    }

    // Unload the hook
    UnhookWindowsHookEx(g_keyboardHook);
    DeleteCriticalSection(&cs);
    cout << "exiting..." << endl;

    return 0;
}



// Callback function to handle keyboard functionality (different thread)
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {

        // GetWindowThreadProcessId(GetForegroundWindow(), &g_fg_proc_id);      // get for active window check
        // GetWindowThreadProcessId(GetConsoleWindow(), &g_console_proc_id);
        // g_curr_proc_id = GetCurrentProcessId();

        // cout << "Foreground PID: " << g_fg_proc_id << endl;
        // cout << "Console PID: " << g_console_proc_id << endl;
        // cout << "Current PID: " << g_curr_proc_id << endl;

        // if (g_fg_proc_id == g_console_proc_id) {        // process key strokes if console window is active

            KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;
            KeyEvent evt{ pKeyInfo->vkCode, wParam == WM_KEYDOWN };

            // Store the event in the shared queue
            EnterCriticalSection(&cs);
            keyQueue.push(evt);
            LeaveCriticalSection(&cs);

            // wake main thread
            PostThreadMessage(g_mainThreadId, WM_KEY_EVENT, 0, 0);

            // Exit if Q is pressed
            if (evt.pressed && evt.vkCode == 'Q') {
                g_running = false;
                PostQuitMessage(0);
            }

        // }
        
    }

    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}


// Send message and wait for newline
void msgUser(const char* msg) {
    cout << msg;
    getchar(); //pauses code execution.
}