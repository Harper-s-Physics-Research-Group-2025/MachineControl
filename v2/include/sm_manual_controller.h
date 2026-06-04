#pragma once
#include "pubSysCls.h"
#include <windows.h>
#include <conio.h>
#include <iostream>
#include <sstream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <string>


#pragma comment(lib, "user32.lib")

#define WM_KEY_EVENT (WM_USER + 1)      

class ManualController {
    private: 
        struct KeyEvent {
            DWORD vkCode;
            bool pressed;  
        };

        std::queue<KeyEvent> keyQueue;
        CRITICAL_SECTION cs;

        HHOOK g_keyboardHook;
        volatile bool g_running = true;
        bool g_initialized_motors = false;
        volatile bool g_emergency_stop = false;
        DWORD g_mainThreadId;

        sFnd::SysManager* Mgr = nullptr;
        sFnd::IPort* Port = nullptr;
        sFnd::INode* motorX = nullptr;
        sFnd::INode* motorZ = nullptr;

        std::vector<std::string> comHubPorts;
        std::unordered_set<DWORD> keys;
        int v_x;                            
        int v_z;

        // Track instance globally for the Windows Hook Callback routing context
        static ManualController* pInstance;


    public:
        int control();
        static LRESULT CALLBACK StaticKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam); 
        LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam); 
        void msgUser(const char* msg) const;
        void home(sFnd::SysManager* manager, sFnd::INode* motor, const char* name, const int timeout);
        void print_motor_info(sFnd::INode* motor) const;
};