/*
src/lab.cpp
Description: Namespace that contains functions for operating the sample holder, sensors, and other related equipment
Version: 0
Authors: Josh Darrow and Samuel Ntadom
Date: 06.09.2026 1400
*/

#include "controls/lab.h"
#define WM_KEY_EVENT (WM_USER + 1)      // custom message channel


namespace Lab {


    // RTE7 Bath functions
    int bath_on(std::string COMM) {
        RTE7 bath = RTE7(COMM);
        return !bath.turn_on();
    }


    int bath_off(std::string COMM) {
        RTE7 bath = RTE7(COMM);
        return !bath.turn_off();
    }


    int bath_manual(std::string COMM) {
        RTE7 bath = RTE7(COMM);
        return !bath.manual();
    }


    int bath_get_temp(std::string COMM, float& temp) {
        RTE7 bath = RTE7(COMM);
        return !bath.get_temp(temp);
    }


    int bath_get_setpoint(std::string COMM, float& temp) {
        RTE7 bath = RTE7(COMM);
        return !bath.get_setpoint(temp);
    }


    int bath_set_setpoint(std::string COMM, float& temp) {
        RTE7 bath = RTE7(COMM);
        return !bath.set_setpoint(temp);
    }




    // Oven Industries 5R6-900 Temperature Controller
    int temperature_control_on(std::string COMM) {         // Enable H-bridge output
        Oven5R6900 tc = Oven5R6900(COMM);
        bool state = 1;
        return !tc.enable(state);
    }


    int temperature_control_off(std::string COMM) {        // Disable H-bridge output
        Oven5R6900 tc = Oven5R6900(COMM);
        bool state = 0;
        return !tc.enable(state);
    }


    int temperature_control_get_mode(std::string COMM, int& mode) {
        Oven5R6900 tc = Oven5R6900(COMM);
        return !tc.get_mode(mode);
    }


    int temperature_control_set_mode(std::string COMM, int& mode) {
        Oven5R6900 tc = Oven5R6900(COMM);
        return !tc.set_mode(mode);
    }


    int temperature_control_get_temp(std::string COMM, float& temp) {
        Oven5R6900 tc = Oven5R6900(COMM);
        return !tc.get_temp(temp);
    }


    int temperature_control_get_setpoint(std::string COMM, float& temp) {
        Oven5R6900 tc = Oven5R6900(COMM);
        return !tc.get_setpoint(temp);
    }


    int temperature_control_set_setpoint(std::string COMM, float& temp) {
        Oven5R6900 tc = Oven5R6900(COMM);
        return !tc.set_setpoint(temp);
    }





    // Labjack read channel
    int read_labjack_ain(const long channel, double& voltage) {
        LJ_HANDLE h;       // Handle for the device
        int errorcode;

        // 1. Open the first found LabJack U3
        errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h);
        if (errorcode != 0) return errorcode;

        // 2. Initialize settings on the LabJack
        ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, channel, 1, 0);  // Set channel 0 to analog input
        ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, 1, 0);            // Resolution index

        // 3. Read analog input AIN0 (single-ended)
        errorcode = eGet(h, LJ_ioGET_AIN, channel, &voltage, 0);

        // 4. Close the device
        Close();

        return errorcode;
    }





    // Teknic Servo Motors

    





    int servo_motor_home(int milliseconds) {

        sFnd::SysManager* Mgr = nullptr;
        sFnd::IPort* Port = nullptr;
        sFnd::INode* motorX = nullptr; 
        sFnd::INode* motorZ = nullptr; 
            
        std::vector<std::string> comHubPorts;                 
        std::vector<int> args = {milliseconds};    

        try {
            Mgr = sFnd::SysManager::Instance();
            sFnd::SysManager::FindComHubPorts(comHubPorts);
            
            if (comHubPorts.size() == 1) {
                Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
            } else {
                // cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
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

            // home(Mgr, motorX, "X-axis", args[0]);
            // home(Mgr, motorZ, "Z-axis", args[0]);

            motorX->Motion.Homing.Initiate();
            motorZ->Motion.Homing.Initiate();
            int home_timestamp = Mgr->TimeStampMsec() + args[0];    
            
            while (!motorX->Motion.Homing.WasHomed() || !motorZ->Motion.Homing.WasHomed()) {
                if (Mgr->TimeStampMsec() > home_timestamp) {
                    // cerr << "Homing timed out after "<< timeout << " milliseconds" << endl;
                    return 1;
                }
                Sleep(10);
            }

            motorX->Motion.PosnMeasured.Refresh();
            motorZ->Motion.PosnMeasured.Refresh();


            // cout << "Final position (mm):  (" << motorX->Motion.PosnMeasured.Value() / 800 << ", " << motorZ->Motion.PosnMeasured.Value() / 800 << ")" << endl;
        } catch (sFnd::mnErr& theErr) {
            // cerr << "Caught error: " << theErr.ErrorMsg << "\n";
            if (motorX) motorX->EnableReq(false);
            if (motorZ) motorZ->EnableReq(false);
            if (Mgr) Mgr->PortsClose();
            return 1;
        } catch (std::exception& e) {
            // cerr << "Caught error: " << e.what() << endl;
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


    int servo_motor_get_position(float& x_mm, float& z_mm) { 
        sFnd::SysManager* Mgr = nullptr;
        sFnd::IPort* Port;
        sFnd::INode* motorX; 
        sFnd::INode* motorZ; 
            
        std::vector<std::string> comHubPorts;

        try {
            Mgr = sFnd::SysManager::Instance();
            sFnd::SysManager::FindComHubPorts(comHubPorts);
            
            if (comHubPorts.size() == 1) {
                Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
            } else {
                // cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
                // std::exit(ERROR_INVALID_COMM_PORT);
                return 1;
            }

            Mgr->PortsOpen(1);

            Port = &Mgr->Ports(0);
            motorX = &Port->Nodes(0); 
            motorZ = &Port->Nodes(1); 

            x_mm = static_cast<float>(motorX->Motion.PosnMeasured.Value()) / 800;
            z_mm = static_cast<float>(motorZ->Motion.PosnMeasured.Value()) / 800;
            // cout << "mm: (" << x_pos << ", " << z_pos << ")" << endl;

        } catch (sFnd::mnErr& theErr) {
            // cerr << "Caught error: " << theErr.ErrorMsg << "\n";
            if (Mgr) Mgr->PortsClose();
            return 1;
        } catch (std::exception& e) {
            // cerr << "Caught error: " << e.what() << endl;
            if (Mgr) Mgr->PortsClose();
            return 1;
        }

        if (Mgr) Mgr->PortsClose();
        return 0;

    } 


    int servo_motor_set_position(double& x_mm, double& z_mm, double& vel_rms) {
        sFnd::SysManager* Mgr = nullptr;
        sFnd::IPort* Port;
        sFnd::INode* motorX; 
        sFnd::INode* motorZ; 
            
        std::vector<std::string> comHubPorts;                 
        const int vel_limit = 1000;     
        std::vector<double> args = {0, 0, 240, 10000};     

        args[0] = x_mm;
        args[1] = z_mm;
        args[2] = vel_rms;
    
        if (args[2] > vel_limit) {
            std::cerr << "Desired velocity exceeds limit" << std::endl;
            return 1;
        }

        args[0] *= 800;     
        args[1] *= 800;     

        try {
            Mgr = sFnd::SysManager::Instance();
            sFnd::SysManager::FindComHubPorts(comHubPorts);
            
            if (comHubPorts.size() == 1) {
                Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
            } else {
                std::cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << std::endl;
                return 1;
            }

            Mgr->PortsOpen(1);

            Port = &Mgr->Ports(0);
            motorX = &Port->Nodes(0);
            motorZ = &Port->Nodes(1);

            if (!(motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) ||       
                !(motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid())) {
                    std::cerr << "Motors are not homed. Please home before continuing" << std::endl;
                    if (motorX) motorX->EnableReq(false);
                    if (motorZ) motorZ->EnableReq(false);
                    if (Mgr) Mgr->PortsClose();
                    return 1;
            }

            motorX->Status.AlertsClear();                   
            motorZ->Status.AlertsClear();

            motorX->Motion.NodeStopClear();                  
            motorZ->Motion.NodeStopClear();
            
            motorX->EnableReq(true);
            motorZ->EnableReq(true);
            Sleep(200); 

            motorX->VelUnit(sFnd::INode::RPM);                        
            motorZ->VelUnit(sFnd::INode::RPM);                        
            
            motorX->Motion.VelLimit = args[2];              
            motorZ->Motion.VelLimit = args[2];              
            
            motorX->Motion.MovePosnStart(args[0], true);
            motorZ->Motion.MovePosnStart(args[1], true);

            args[3] = Mgr->TimeStampMsec() + 100 + max(motorX->Motion.MovePosnDurationMsec(args[0], true), motorZ->Motion.MovePosnDurationMsec(args[1], true));

            while (!motorX->Motion.MoveIsDone() || !motorZ->Motion.MoveIsDone()) {
                if (Mgr->TimeStampMsec() > args[3]) {
                    std::cerr << "Movement timed out" << std::endl;
                    return 1;
                }
            }

            std::cout << "(" << motorX->Motion.PosnMeasured.Value() / 800 << ", " << motorZ->Motion.PosnMeasured.Value() / 800 << ")" << std::endl;

        } catch (sFnd::mnErr& theErr) {
            std::cerr << "Caught error: " << theErr.ErrorMsg << "\n";
            if (motorX) motorX->EnableReq(false);
            if (motorZ) motorZ->EnableReq(false);
            if (Mgr) Mgr->PortsClose();
            return 1;
        } catch (std::exception& e) {
            std::cerr << "Caught error: " << e.what() << std::endl;
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




    // Initialize global variables for manual control

    // Event structure to hold key events
    struct KeyEvent {
        DWORD vkCode;
        bool pressed;  // true = down, false = up
    };


    // Shared queue and synchronization
    std::queue<KeyEvent> keyQueue;
    CRITICAL_SECTION cs;

    HHOOK g_keyboardHook;
    volatile bool g_running = true;
    bool g_initialized_motors = false;
    volatile bool g_emergency_stop = false;
    DWORD g_mainThreadId;       // for posting messages?
        
        
    // Teknic motors
    sFnd::SysManager* Mgr = nullptr;
    sFnd::IPort* Port;
    sFnd::INode* motorX; // Controlled with Left/Right
    sFnd::INode* motorZ; // Controlled with Up/Down




    // Callback function to handle keyboard functionality (presumably called by the keyboard handler)
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


    int servo_motor_manual_control() {

        // Initialize variables
        std::vector<std::string> comHubPorts;                 // usually only one port (board) and up to 4 motors per port
        std::unordered_set<DWORD> keys;
        int v_x;                            // possibly should be double
        int v_z;


        // Initialize synchronization
        InitializeCriticalSection(&cs);
        g_mainThreadId = GetCurrentThreadId();      // get for PostThreadMessage
        
        
        // Install keyboard hook
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
        if (g_keyboardHook == NULL) {
            std::cerr << "Failed to install keyboard hook. Error: " << GetLastError() << std::endl;
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
                    Mgr = sFnd::SysManager::Instance();

                    // Find Ports
                    sFnd::SysManager::FindComHubPorts(comHubPorts);
                    printf("Found %llu SC Hubs\n", comHubPorts.size());

                    if (comHubPorts.size() == 1) {
                        // assign ports
                        Mgr->ComHubPort(0, comHubPorts[0].c_str()); // for our use case we will only ever be using one com port (circuit board controller)
                    } else {
                        printf("Found number (%llu) ports that is not 1\n", comHubPorts.size()); // handle case with more than one port found
                        // msgUser("Press any key to confirm and exit");
                        return 1;
                    }

                    // Open the port(s)
                    Mgr->PortsOpen(1);

                    Port = &Mgr->Ports(0);
                    motorX = &Port->Nodes(0); // Controlled with Left/Right
                    motorZ = &Port->Nodes(1); // Controlled with Up/Down
                    
                    // print motor (Node) information

                    printf("Motor X  Serial #: %d\n", motorX->Info.SerialNumber.Value());
                    printf("Motor Z  Serial #: %d\n", motorZ->Info.SerialNumber.Value());
                    // printf("Press enter to continue, any other key to exit...");
                    // char c = _getch();
                    // if (c != '\n' && c != '\r') break;
                    // printf("Enabling and Homing Motors\n");


                    motorX->Status.AlertsClear();                   //Clear Alerts on node 
                    motorZ->Status.AlertsClear();

                    motorX->Motion.NodeStopClear();                  // Clear Nodestops ?
                    motorZ->Motion.NodeStopClear();
                    
                    motorX->EnableReq(true);
                    motorZ->EnableReq(true);
                    Sleep(200); // wait for enabling

                    // Homing
                    printf("current position (X, Z): (%.0f,%.0f)\n", motorX->Motion.PosnMeasured.Value(), motorZ->Motion.PosnMeasured.Value());
                    // home(Mgr, motorX, "X-axis", 20000);
                    // home(Mgr, motorZ, "Z-axis", 20000);
                    printf("current position (X, Z): (%.0f,%.0f)\n", motorX->Motion.PosnMeasured.Value(), motorZ->Motion.PosnMeasured.Value());

                    g_initialized_motors = true;
                    std::cout << "Use arrow keys to move motors. Press 'q' to quit.\n";
                }

                    

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
                        v_x = (keys.find(39) != keys.end()) - (keys.find(37) != keys.end());
                        v_z = (keys.find(38) != keys.end()) - (keys.find(40) != keys.end());

                        motorX->Motion.MoveVelStart(500*v_x);
                        motorZ->Motion.MoveVelStart(500*v_z);


                    } else {
                        TranslateMessage(&msg);     // idk what this does
                        DispatchMessage(&msg);
                    }

                }

                }

        } catch (sFnd::mnErr& theErr) {
            std::cout << "Caught error: " << theErr.ErrorMsg << "\n";
        } catch (std::exception& e) {
            std::cout << "Caught error: " << e.what() << std::endl;
            printf("Node did not complete homing:  \n\t -Ensure Homing settings have been defined through ClearView. \n\t -Check for alerts/Shutdowns \n\t -Ensure timeout is longer than the longest possible homing move.\n");

        }

        // Safe disable. May want wrapper for this
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();

        

        // Unload the hook
        UnhookWindowsHookEx(g_keyboardHook);
        DeleteCriticalSection(&cs);
        std::cout << "exiting..." << std::endl;

        return 0;
    }



}





















    



