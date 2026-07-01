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

    // RTE7 Bath
    RTE7* bath = nullptr;

    // Temp controller
    Oven5R6900* tc = nullptr;


    // servo motor hardware
    sFnd::SysManager* Mgr = nullptr;
    sFnd::IPort* Port = nullptr;
    sFnd::INode* motorX = nullptr; 
    sFnd::INode* motorZ = nullptr; 

    // logging
    bool LOG = false;
    std::string LOG_FILE = "C:\\Users\\Student\\Documents\\JuxtapositionOfSampleHolders\\MachineControl\\v2\\log.txt";


    
    
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
    DWORD g_mainThreadId;       // for posting messages?
    
    
    
    
    
    
    // logging functions
    void log(const std::string& msg) {                      // Write to file if desired.
        if (LOG) {
            std::ofstream f(LOG_FILE, std::ios::app);
            f << msg << std::endl;
        }
    }


    int get_log_settings(bool& verbose, std::string& file) {        // Get current logging settings.
        verbose = LOG;
        file = LOG_FILE;
        log("log settings accessed by get_log_settings().");
        return 0;
    }


    int set_log_settings(bool& verbose, std::string& file) {        // Turn logging on/off. Specify log location.
        log("log settings modified by set_log_settings(). LOGFILE: " + file);
        LOG = verbose;
        LOG_FILE = file;
        return 0;
    }


    bool logfile_valid(std::string& filepath) {
        return std::filesystem::exists(std::filesystem::path(filepath).parent_path());
    }




    // RTE7 Bath functions
    int init_bath(std::string COMM) {
        // 1. Clean up an existing connection if called a second time
        if (bath != nullptr) {
            delete bath;
        }
        
        // 2. Instantiate a fresh connection on the heap
        bath = new RTE7(COMM);
        
        // 3. Return a success token (e.g., check if the pointer is valid)
        return (bath == nullptr);
    }


    int delete_bath() {
        log("\nin int delete_bath()");

        delete bath;
        bath = nullptr;

        log("Bath object successfully deleted");

        return 0;
    }


    int bath_on() {
        return !bath->turn_on();
    }


    int bath_off() {
        return !bath->turn_off();
    }


    int bath_manual() {
        return !bath->manual();
    }


    int bath_get_temp(float& temp) {
        return !bath->get_temp(temp);
    }


    int bath_get_setpoint(float& temp) {
        return !bath->get_setpoint(temp);
    }


    int bath_set_setpoint(float& temp) {
        return !bath->set_setpoint(temp);
    }




    // Oven Industries 5R6-900 Temperature Controller
    int init_temp_controller(std::string COMM) {
        // 1. Clean up an existing connection if called a second time
        if (tc != nullptr) {
            delete tc;
        }
        
        // 2. Instantiate a fresh connection on the heap
        tc = new Oven5R6900(COMM);
        
        // 3. Return a success token (e.g., check if the pointer is valid)
        return (tc == nullptr);
    }


    int delete_temp_controller() {
        log("\nIn delete_temp_controller().");
        
        delete tc;
        tc = nullptr;

        log("temp controller successfully deleted");
        return 0;
    }


    int temperature_control_on() {         // Enable H-bridge output
        bool state = 1;
        return !tc->enable(state);
    }


    int temperature_control_off() {        // Disable H-bridge output
        bool state = 0;
        return !tc->enable(state);
    }


    int temperature_control_get_mode(int& mode) {
        return !tc->get_mode(mode);
    }


    int temperature_control_set_mode(int& mode) {
        return !tc->set_mode(mode);
    }


    int temperature_control_get_temp(float& temp) {
        return !tc->get_temp(temp);
    }


    int temperature_control_get_setpoint(float& temp) {
        return !tc->get_setpoint(temp);
    }


    int temperature_control_set_setpoint(float& temp) {
        return !tc->set_setpoint(temp);
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


    // TODO:
    


    int initialize_servos() {

        std::vector<std::string> comHubPorts;


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

            motorX->Status.AlertsClear();                   
            motorZ->Status.AlertsClear();

            motorX->Motion.NodeStopClear();                  
            motorZ->Motion.NodeStopClear();
            
            motorX->EnableReq(true);
            motorZ->EnableReq(true);
            Sleep(200); 

            return 0;
        
        } catch (sFnd::mnErr& theErr) {
            // If an SDK error occurs (e.g., motor has a hard fault), shut down
            shutdown_servos(); 
            return -1;
        } catch (...) {
            shutdown_servos();
            return -1;
        }

    }


    int shutdown_servos() {

        // Safely kill power to the motor coils before severing communication
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);

        if (Mgr) {
            Mgr->PortsClose();
            Mgr = nullptr;
        }
        Port = nullptr;
        motorX = nullptr;
        motorZ = nullptr;

        return 0;

    }


    bool servos_ready() {
        
        log("\nin bool servos_ready()");

        // return false if motors are nullptr
        if (!motorX || !motorZ) {
            log("One or more motors are nullptr. (!motorX || !motorZ): " + std::to_string((!motorX || !motorZ)));   
            return false;
        }


        // Check enabling and alerts
        try {
            log("\tRefreshing status and alerts.");
            // 1. Force a hardware sync
            motorX->Status.RT.Refresh();
            motorZ->Status.RT.Refresh();
            motorX->Status.Alerts.Refresh();
            motorZ->Status.Alerts.Refresh();

            log("\tChecking enabling and alert status");


            // Read safety flags
            int IsEnabled = motorX->Motion.IsReady() && motorZ->Motion.IsReady(); // Checks if enabled and fully operational
            int Alerts = (motorX->Status.RT.Value().cpm.AlertPresent) || 
                            (motorZ->Status.RT.Value().cpm.AlertPresent);      // Checks for active faults
            
            log("\tIsEnabled: " + std::to_string(IsEnabled));
            log("\tAlerts: " + std::to_string(Alerts));
            log("Returning (IsEnabled && !Alerts) : " + std::to_string(IsEnabled && !Alerts));

            return IsEnabled && !Alerts;

        } catch (...) {
            // log("unknown error caught, returning 1.");
            log("\tCaught some kind of generic error.");
            log("Returning false");
            return false; // Communication dropped or node became invalid
        }
    }



    bool servos_homed() {


        try {

            return (motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) && 
                (motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid());

        } catch (...) {
            log("\tCaught some kind of generic error.");
            log("Returning false");
            return false;
        }
    }


    int get_servo_alerts(char* alertX, char* alertZ) {

        log("\nin int get_servo_alerts()");

        // return false if motors are nullptr
        if (!motorX || !motorZ) {
            log("One or more motors are nullptr. (!motorX || !motorZ): " + std::to_string((!motorX || !motorZ)));   
            return 1;
        }

        log("Checking Alert bits");

        // 1. make sure registers are up to date
        motorX->Status.RT.Refresh();
        motorZ->Status.RT.Refresh();
        motorX->Status.Alerts.Refresh();
        motorZ->Status.Alerts.Refresh();


        // 2. Fetch the raw integer bitmasks correctly
        motorX->Status.Alerts.Value().StateStr(alertX, 256);
        motorZ->Status.Alerts.Value().StateStr(alertZ, 256);
        log("MotorX: " + std::string(alertX));
        log("MotorZ: " + std::string(alertZ));

        return 0;

    }


    int servo_motor_home(int milliseconds) {
        
        log("\nIn servo_motor_home()");
        log("\tchecking servos_ready()");

        if (!servos_ready()) {      // servos uninitialized
            return 1;
        }      
        
        log("\tservos check complete.");

        try {

            log("Initiating homing sequence.");
            motorX->Motion.Homing.Initiate();
            motorZ->Motion.Homing.Initiate();
            int home_timestamp = Mgr->TimeStampMsec() + milliseconds;    
            
            while (!motorX->Motion.Homing.WasHomed() || !motorZ->Motion.Homing.WasHomed()) {
                if (Mgr->TimeStampMsec() > home_timestamp) {
                    log("\tHoming timed out after " + std::to_string(milliseconds) + " milliseconds.");
                    return 1;
                }
                Sleep(10);
            }

            log("\tRefreshing current position buffer");
            motorX->Motion.PosnMeasured.Refresh();
            motorZ->Motion.PosnMeasured.Refresh();
            log("\tBuffer refreshed.");
            log("\tMeasuring motor position");
            float x_mm = static_cast<float>(motorX->Motion.PosnMeasured.Value()) / 800;
            float z_mm = static_cast<float>(motorZ->Motion.PosnMeasured.Value()) / 800;
            log("\tMeasure position complete.");
            log("\tCurrent parameters (x_mm, z_mm): " + std::to_string(x_mm) + ", " + std::to_string(z_mm));


        } catch (sFnd::mnErr& theErr) {
            log("\tsFoundation error: "); // + std::to_string(theErr.ErrorMsg));
            return 1;
        } catch (std::exception& e) {
            log("generic error occured");
            return 1;
        }

        return 0;

    }


    int servos_get_position(float& x_mm, float& z_mm) { 

        log("\nIn servos_get_position()");
        log("\tCurrent parameters (x_mm, z_mm): " + std::to_string(x_mm) + ", " + std::to_string(z_mm));
        log("\tchecking servos_ready()");

        if (!servos_ready()) return 1;      // servos uninitialized

        log("\n\tservos_ready() check complete");

        try {
            log("\tRefreshing current position buffer");
            motorX->Motion.PosnMeasured.Refresh();
            motorZ->Motion.PosnMeasured.Refresh();
            log("\tBuffer refreshed.");
            log("\tMeasuring motor position");
            x_mm = static_cast<float>(motorX->Motion.PosnMeasured.Value()) / 800;
            z_mm = static_cast<float>(motorZ->Motion.PosnMeasured.Value()) / 800;
            log("\tMeasure position complete.");
            log("\tCurrent parameters (x_mm, z_mm): " + std::to_string(x_mm) + ", " + std::to_string(z_mm));

        } catch (sFnd::mnErr& theErr) {
            log("sFoundation error");
            return 1;
        } catch (std::exception& e) {
            log("generic error occured"); // + std::to_string(e));
            return 1;
        }

        return 0;

    } 


    int servos_set_position(float& x_mm, float& z_mm, float& vel_rms) {
        
        log("\nIn servos_set_position()");
        log("\tchecking servos_ready()");

        if (!servos_ready()) return 1;      // check servo status

        log("\tservo check complete.");

        const int vel_limit = 1000;         // hardcoded velocity limit
    
        if (vel_rms > vel_limit) {
            log("Desired RPM exceeds limit, exiting...");
            // std::cerr << "Desired velocity exceeds limit of " << std::to_string(vel_limit) << std::endl;
            return 1;
        }

        try {

            if (!(motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) ||       
                !(motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid())) {
                    log("\tMotors are not homed. Please home before continuing");
                    return 1;
            }

            // Set max RPM
            log("\tSetting max RPM.");
            motorX->VelUnit(sFnd::INode::RPM);                        
            motorZ->VelUnit(sFnd::INode::RPM);                        
            motorX->Motion.VelLimit = vel_rms;              
            motorZ->Motion.VelLimit = vel_rms;              
            log("\tMax RPM set to: " + std::to_string(vel_rms));

            log("\tStarting Movement");
            motorX->Motion.MovePosnStart(x_mm*800, true);
            motorZ->Motion.MovePosnStart(z_mm*800, true);

            int timeout_timestamp = Mgr->TimeStampMsec() + 100 + max(motorX->Motion.MovePosnDurationMsec(x_mm*800, true), motorZ->Motion.MovePosnDurationMsec(z_mm*800, true));

            while (!motorX->Motion.MoveIsDone() || !motorZ->Motion.MoveIsDone()) {
                if (Mgr->TimeStampMsec() > timeout_timestamp) {
                    log("Movement timed out.");
                    return 1;
                }
            }
            
            log("\tFinished movement.");
            
            log("\tRefreshing current position buffer.");
            motorX->Motion.PosnMeasured.Refresh();
            motorZ->Motion.PosnMeasured.Refresh();
            log("\tBuffer refreshed.");
            
            log("\tMeasuring motor position.");
            x_mm = static_cast<float>(motorX->Motion.PosnMeasured.Value()) / 800;
            z_mm = static_cast<float>(motorZ->Motion.PosnMeasured.Value()) / 800;
            log("\tMeasure position complete.");
            log("\tCurrent parameters (x_mm, z_mm): " + std::to_string(x_mm) + ", " + std::to_string(z_mm));

        } catch (sFnd::mnErr& theErr) {
            log("Caught error: " + std::string(theErr.ErrorMsg));
            return 1;
        } catch (std::exception& e) {
            log("Global error.");          // std::string(e.what())
            return 1;
        }


        return 0;
    
    }




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

        log("In int servo_motor_manual_control()");

        // Initialize variables
        std::unordered_set<DWORD> keys;
        int v_x;                            // possibly should be double
        int v_z;
        g_running = true;


        log("\tInitializing critical section");
        // Initialize synchronization
        InitializeCriticalSection(&cs);
        g_mainThreadId = GetCurrentThreadId();      // get for PostThreadMessage
        
        log("\tCritical section initialized, installing keyboard hook...");
        // Install keyboard hook
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
        if (g_keyboardHook == NULL) {
            log("Failed to install keyboard hook.");
            // std::cerr << "Failed to install keyboard hook. Error: " << GetLastError() << std::endl;
            DeleteCriticalSection(&cs);
            return 1;
        }

        log("\tKeyboard hook installed, starting message queue...");
        // Run Program
        // Ensure message queue exists
        MSG msg;
        PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

        try {

            log("Use arrow keys to move motors. Press 'q' to quit.");
            log("Entering main message loop...");

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

            log("\tExited main loop");

        } catch (sFnd::mnErr& theErr) {
            log("\tsFoundation error.");
            // std::cout << "Caught error: " << theErr.ErrorMsg << "\n";
        } catch (std::exception& e) {
            log("\tGeneric error.");
            // std::cout << "Caught error: " << e.what() << std::endl;
            // printf("Node did not complete homing:  \n\t -Ensure Homing settings have been defined through ClearView. \n\t -Check for alerts/Shutdowns \n\t -Ensure timeout is longer than the longest possible homing move.\n");

        }

        //shutdown_servos();

        log("\tUnloading keyboard hook and deleting critical section");
        // Unload the hook
        UnhookWindowsHookEx(g_keyboardHook);
        DeleteCriticalSection(&cs);
        log("exiting...");

        return 0;
    }



}





















    



