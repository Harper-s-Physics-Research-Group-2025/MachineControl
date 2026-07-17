/*
src/lab.cpp
Description: Namespace that contains functions for operating the sample holder, sensors, and other related equipment
Version: 0
Authors: Josh Darrow and Samuel Ntadom
Date: 06.09.2026 1400
*/

#include "controls/lab.h"
#include <setupapi.h>
#include <devguid.h>

#define WM_KEY_EVENT (WM_USER + 1)      // custom message channel


namespace Lab {

    // Create namespace instances of the hardware managers
    // Avoid reinitializing and uninitializing them every function call.

    // pointer to RTE7 bath object
    RTE7* bath = nullptr;

    // pointer to temp controller object
    Oven5R6900* tc = nullptr;


    // servo motor hardware
    sFnd::SysManager* Mgr = nullptr;
    sFnd::IPort* Port = nullptr;
    sFnd::INode* motorX = nullptr; 
    sFnd::INode* motorZ = nullptr; 

    // logging defaults
    bool LOG = false;
    std::string LOG_FILE = "C:\\Users\\Student\\Desktop\\machine_controller\\MachineControl\\v2\\log.txt";


    
    
    // A few global variables for manual control
    // Event structure to hold key events
    struct KeyEvent {
        DWORD vkCode;   // unique code for each keyboard key
        bool pressed;   // true = down, false = up
    };


    // Shared queue and synchronization
    std::queue<KeyEvent> keyQueue;      // queue for passing KeyEvent structs
    CRITICAL_SECTION cs;                // critical section for multithread safe queue. (probably not completely necessary in this case)

    HHOOK g_keyboardHook;               // operational status of keyboard callback function
    volatile bool g_running = true;     // whether the manual control for motors is running
    DWORD g_mainThreadId;       // for posting messages?
    
    
    
    
    
    // -------------------------------------------------------------------------
    // Logging
    // -------------------------------------------------------------------------

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


    int set_log_settings(bool& verbose, std::string& file) {        // Turn logging on/off. Specify log file location.
        
        if (!logfile_valid(file)) {         // For invalid file path, do nothing.
            log("Invalid file path: " + file);
            return 1;
        }

        log("log settings modified by set_log_settings(). LOGFILE: " + file);
        LOG = verbose;
        LOG_FILE = file;
        return 0;
    }


    bool logfile_valid(std::string& filepath) {                     // file path validity check
        return std::filesystem::exists(std::filesystem::path(filepath).parent_path());
    }




    // -------------------------------------------------------------------------
    // RTE7 Fluid Bath
    // -------------------------------------------------------------------------

    // Enumerates the Windows "Ports (COM & LPT)" device class and returns every
    // COM port whose USB hardware ID matches VID_0403&PID_6001 (FTDI FT232
    // USB-to-Serial -- the chip used by the bath's and temp controller's
    // current cables). The bath and the temp controller both use identical
    // adapters, so the hardware ID alone can only narrow candidates down --
    // it can't tell the two apart. Disambiguating which candidate is which
    // device happens by protocol probing below.
    std::vector<std::string> find_serial_adapter_ports() {
        std::vector<std::string> found_ports;

        HDEVINFO device_info = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
        if (device_info == INVALID_HANDLE_VALUE) return found_ports;

        SP_DEVINFO_DATA device_data = { 0 };
        device_data.cbSize = sizeof(SP_DEVINFO_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_data); ++i) {
            char hardware_id[256] = { 0 };
            if (!SetupDiGetDeviceRegistryPropertyA(device_info, &device_data, SPDRP_HARDWAREID,
                    nullptr, reinterpret_cast<PBYTE>(hardware_id), sizeof(hardware_id), nullptr)) {
                continue;
            }

            std::string id(hardware_id);
            if (id.find("VID_0403&PID_6001") == std::string::npos) continue;   // not an FTDI adapter

            char friendly_name[256] = { 0 };
            if (!SetupDiGetDeviceRegistryPropertyA(device_info, &device_data, SPDRP_FRIENDLYNAME,
                    nullptr, reinterpret_cast<PBYTE>(friendly_name), sizeof(friendly_name), nullptr)) {
                continue;
            }

            // Friendly names look like "USB Serial Port (COM10)" -- pull the COMx token out.
            std::string name(friendly_name);
            size_t open_paren = name.rfind("(COM");
            size_t close_paren = name.rfind(")");
            if (open_paren == std::string::npos || close_paren == std::string::npos) continue;

            found_ports.push_back(name.substr(open_paren + 1, close_paren - open_paren - 1));  // e.g. "COM10"
        }

        SetupDiDestroyDeviceInfoList(device_info);
        return found_ports;
    }


    // Probes a candidate port as the given Device type (RTE7 or Oven5R6900), with a hard
    // wall-clock timeout so a stuck serial exchange can't hang the whole kernel. The
    // real hardware's read/write timeouts (COMMTIMEOUTS, set in each device's initSerial)
    // are supposed to bound this on their own, but can't always be trusted -- a device or
    // driver quirk can stall a synchronous ReadFile/WriteFile longer than configured.
    //
    // Both constructing the Device (which opens the port via CreateFileA -- itself
    // capable of blocking if the OS/driver hasn't fully released a just-closed handle
    // on that same COM port yet) and the get_setpoint() call happen on the background
    // thread, so the timeout covers the whole probe, not just the read/write.
    //
    // Deliberately uses a detached std::thread + std::promise here, NOT std::async:
    // a std::future returned by std::async blocks in ITS OWN DESTRUCTOR until the task
    // finishes, even if you never call .get() on it -- so "give up after `timeout` and
    // return false" would silently re-block the caller anyway the moment the abandoned
    // std::async future went out of scope, defeating the entire point of the timeout.
    // A future obtained from a std::promise has no such blocking-destructor behavior,
    // so it's safe to just walk away from once wait_for() reports a timeout.
    //
    // If the probe doesn't finish within `timeout`, this returns false and moves on --
    // but the Device object (owned entirely by the detached thread's lambda) is kept
    // alive for as long as that thread keeps running, rather than being destroyed while
    // it might still be blocked inside a synchronous Win32 call on its handle.
    // Destroying it out from under a pending synchronous ReadFile/WriteFile/CreateFileA
    // on another thread is undefined behavior on Windows (can crash), so a timed-out
    // probe deliberately leaks its handle/thread rather than risk that -- accepted here
    // since it's a rare edge case, and the alternative is hanging the entire Mathematica
    // kernel with no recovery but killing the process.
    template <typename Device>
    bool probe_with_timeout(const std::string& port, std::chrono::milliseconds timeout) {
        auto result = std::make_shared<std::promise<bool>>();
        std::future<bool> fut = result->get_future();

        std::thread([port, result]() {
            Device candidate(port);
            float temp;
            bool ok = candidate.get_setpoint(temp);
            try { result->set_value(ok); } catch (...) {}   // no one may be listening anymore -- fine
        }).detach();

        if (fut.wait_for(timeout) == std::future_status::ready) {
            return fut.get();
        }
        return false;
    }


    // Finds the bath among the candidate serial-adapter ports by opening each one
    // and sending a harmless, read-only "get setpoint" query. Only the port
    // actually wired to the RTE7 will complete that handshake with a valid,
    // checksummed reply, so this works even after the two adapters get
    // swapped into different physical USB ports.
    std::string find_bath_port() {
        std::vector<std::string> candidates = find_serial_adapter_ports();
        log("find_bath_port: found " + std::to_string(candidates.size()) + " candidate serial adapter port(s).");
        for (const std::string& port : candidates) {
            bool ok = probe_with_timeout<RTE7>(port, std::chrono::milliseconds(1500));
            log("find_bath_port: probed " + port + " -> " + (ok ? "matched (this is the bath)" : "no valid reply (or timed out)"));
            if (ok) return port;
        }
        log("find_bath_port: no candidate serial adapter port answered the RTE7 protocol probe.");
        return "";
    }


    // Same idea as find_bath_port(), but probing with the Oven5R6900's own
    // read-only "get setpoint" query.
    std::string find_temp_controller_port() {
        std::vector<std::string> candidates = find_serial_adapter_ports();
        log("find_temp_controller_port: found " + std::to_string(candidates.size()) + " candidate serial adapter port(s).");
        for (const std::string& port : candidates) {
            bool ok = probe_with_timeout<Oven5R6900>(port, std::chrono::milliseconds(1500));
            log("find_temp_controller_port: probed " + port + " -> " + (ok ? "matched (this is the temp controller)" : "no valid reply (or timed out)"));
            if (ok) return port;
        }
        log("find_temp_controller_port: no candidate serial adapter port answered the Oven5R6900 protocol probe.");
        return "";
    }


    int init_bath() {

        std::string COMM = find_bath_port();
        if (COMM.empty()) return 1;            // no unambiguous match found

        if (bath != nullptr) { delete bath;}   // 1. Clean up an existing connection if called a second time

        // find_bath_port()'s temporary probe object just closed this exact port; give the
        // OS/FTDI driver a moment to fully release the handle before reopening it, or the
        // reopen below can fail immediately even though the probe just succeeded on it.
        Sleep(150);

        bath = new RTE7(COMM);          // 2. Instantiate a fresh connection on the heap
        if (!bath->is_connected()) {    // 3. Constructor logs a failure but doesn't throw -- check explicitly
            delete bath;
            bath = nullptr;
            return 1;
        }
        return 0;

    }


    int delete_bath() {
        log("\nin int delete_bath()");

        delete bath;
        bath = nullptr;

        log("Bath object successfully deleted");

        return 0;
    }


    int bath_on()                       { if (!bath) return 1; return !bath->turn_on(); }
    int bath_off()                      { if (!bath) return 1; return !bath->turn_off(); }
    int bath_manual()                   { if (!bath) return 1; return !bath->manual(); }
    int bath_get_temp(float& temp)      { if (!bath) return 1; return !bath->get_temp(temp); }
    int bath_get_setpoint(float& temp)  { if (!bath) return 1; return !bath->get_setpoint(temp); }
    int bath_set_setpoint(float& temp)  { if (!bath) return 1; return !bath->set_setpoint(temp); }




    // -------------------------------------------------------------------------
    // Oven Industries 5R6-900 Temperature Controller
    // -------------------------------------------------------------------------

    int init_temp_controller() {
        std::string COMM = find_temp_controller_port();
        if (COMM.empty()) return 1;         // no unambiguous match found

        if (tc != nullptr) { delete tc;}    // 1. Clean up an existing connection if called a second time

        // Same reasoning as init_bath(): let the OS/FTDI driver fully release the probe's
        // just-closed handle on this port before reopening it.
        Sleep(150);

        tc = new Oven5R6900(COMM);     // 2. Instantiate a fresh connection on the heap
        if (!tc->is_connected()) {     // 3. Constructor logs a failure but doesn't throw -- check explicitly
            delete tc;
            tc = nullptr;
            return 1;
        }
        return 0;
    }


    int delete_temp_controller() {
        log("\nIn delete_temp_controller().");
        
        delete tc;
        tc = nullptr;

        log("temp controller successfully deleted");
        return 0;
    }


    int temperature_control_on()                        { if (!tc) return 1; bool s = 1; return !tc->enable(s); }      // Enable H-bridge output
    int temperature_control_off()                       { if (!tc) return 1; bool s = 0; return !tc->enable(s); }      // Disable H-bridge output
    int temperature_control_get_mode(int& mode)         { if (!tc) return 1; return !tc->get_mode(mode); }
    int temperature_control_set_mode(int& mode)         { if (!tc) return 1; return !tc->set_mode(mode); }
    int temperature_control_get_temp(float& temp)       { if (!tc) return 1; return !tc->get_temp(temp); }
    int temperature_control_get_setpoint(float& temp)   { if (!tc) return 1; return !tc->get_setpoint(temp); }
    int temperature_control_set_setpoint(float& temp)   { if (!tc) return 1; return !tc->set_setpoint(temp); }



    // -------------------------------------------------------------------------
    // LabJack U3 Analog Input
    // -------------------------------------------------------------------------

    int read_labjack_ain(const long channel, double& voltage) {
        LJ_HANDLE h;       // Handle for the device
        int errorcode;

        // 1. Open the first found LabJack U3
        errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h);
        if (errorcode != 0) return errorcode;

        // 2. Initialize settings on the LabJack
        errorcode = ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, channel, 1, 0);  // Set channel 0 to analog input
        if (errorcode == 0) errorcode = ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, 1, 0);  // Resolution index

        // 3. Read analog input AIN0 (single-ended)
        if (errorcode == 0) errorcode = eGet(h, LJ_ioGET_AIN, channel, &voltage, 0);

        // 4. Close the device. NOTE: the UD API's Close() takes no handle -- it closes every open
        // LabJack device process-wide (there is no per-handle close in this legacy API). Harmless
        // with a single device connected; a landmine if a second LabJack is ever added.
        Close();

        return errorcode;
    }





    // -------------------------------------------------------------------------
    // Teknic ClearPath Servo Motors
    // -------------------------------------------------------------------------

    // Create managers to control the servo motors
    // set the namespace pointers to the locations of these objects.
    // Split out from initialize_servos() below -- see that function's comment for why
    // it's a thin, direct, un-timed wrapper around this rather than running it on a
    // background thread.
    int initialize_servos_impl() {

        std::vector<std::string> comHubPorts;

        try {
            Mgr = sFnd::SysManager::Instance();
            sFnd::SysManager::FindComHubPorts(comHubPorts);

            if (comHubPorts.size() == 1) {
                Mgr->ComHubPort(0, comHubPorts[0].c_str());
            } else {
                std::cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << std::endl;
                shutdown_servos();   // Mgr was already assigned above -- reset it rather than leaving a partial state
                return 1;
            }

            // Connect to motors
            Mgr->PortsOpen(1);
            Port = &Mgr->Ports(0);
            motorX = &Port->Nodes(0);
            motorZ = &Port->Nodes(1);

            // Clear alerts and faults. Enable motors.
            motorX->Status.AlertsClear();
            motorZ->Status.AlertsClear();
            motorX->Motion.NodeStopClear();
            motorZ->Motion.NodeStopClear();
            motorX->EnableReq(true);
            motorZ->EnableReq(true);
            Sleep(200);     // wait for enable to complete

            return 0;

        } catch (sFnd::mnErr& theErr) {    // If an SDK error occurs (e.g., motor has a hard fault), shut down
            shutdown_servos();
            return -1;
        } catch (...) {
            shutdown_servos();
            return -1;
        }

    }


    // Calls initialize_servos_impl() directly, on the calling thread, with no timeout.
    //
    // A background-thread timeout wrapper was tried here (matching probe_with_timeout()'s
    // pattern) to guard against BUGS.md #11's hang. It was reverted: running the Teknic
    // SDK calls (SysManager::Instance(), FindComHubPorts, PortsOpen) from a thread other
    // than the one that originally calls into this DLL caused the whole process to crash
    // (segfault) during hub setup, not just hang -- consistent with hardware SDKs like
    // this frequently having thread-affinity requirements that a generic std::thread
    // wrapper silently violates. A crash is worse than a hang (no recovery is possible by
    // killing a *different*, still-hung process), so until there's a safer way to bound
    // this call, the hang risk from #11 is the accepted tradeoff -- call ServoEnable[]
    // knowing it can block if the Teknic hub doesn't respond, and if it does hang, recover
    // by killing the kernel process, same as before the timeout was ever attempted.
    int initialize_servos() {
        return initialize_servos_impl();
    }


    // Release controller memory back to the system
    // set pointers to null
    int shutdown_servos() {

        // Safely kill power to the motor coils before closing communication
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


    // Servo motor status check
    bool servos_ready() {
        
        log("\nin bool servos_ready()");

        if (!motorX || !motorZ) {       // return false if motors are nullptr
            log("One or more motors are nullptr.");   
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
            log("\tGeneric error in servos_ready(), Returning false");
            return false;
        }
    }


    // Returns true if both motors have completed a valid homing sequence
    bool servos_homed() {

        try {
            return (motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) && 
                (motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid());
        } catch (...) {
            log("\tGeneric error in servos_homed(), Returning false");
            return false;
        }
    }

    
    // Writes human-readable alert strings for each axis into caller-provided buffers (256 bytes each)
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

        // 2. Fetch alert bits and descriptions.
        motorX->Status.Alerts.Value().StateStr(alertX, 256);
        motorZ->Status.Alerts.Value().StateStr(alertZ, 256);
        log("MotorX: " + std::string(alertX));
        log("MotorZ: " + std::string(alertZ));

        return 0;
    }


    // home the system
    int servo_motor_home(int milliseconds) {
        
        log("\nIn servo_motor_home()");
        log("\tchecking servos_ready()");

        if (!servos_ready()) { return 1; }     // servos uninitialized 
        
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
            log("\tsFoundation error: ");
            return 1;
        } catch (std::exception& e) {
            log("generic error occured");
            return 1;
        }

        return 0;

    }


    // Get current apparatus position (mm)
    int servos_get_position(float& x_mm, float& z_mm) {

        log("\nIn servos_get_position()");
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


    // move apparatus to new position (mm)
    int servos_set_position(float& x_mm, float& z_mm, float& vel_rms) {
        
        log("\nIn servos_set_position()");
        log("\tchecking servos_ready()");

        if (!servos_ready()) return 1;      // check servo status

        log("\tservo check complete.");

        const int vel_limit = 1000;         // hardcoded velocity limit
    
        if (vel_rms > vel_limit) {
            log("Desired RPM exceeds limit, exiting...");
            return 1;
        }

        try {

            if (!servos_homed()) {
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
            log("Generic error: " + std::string(e.what()));
            return 1;
        }

        return 0;
    }



    // -------------------------------------------------------------------------
    // Manual Motor Control (keyboard-driven)
    // -------------------------------------------------------------------------

    // Callback function to handle keyboard functionality (low level keyboard hook called by the keyboard handler)
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


    // Runs a blocking manual control loop: arrow keys drive motors, Esc = E-stop, Q = quit
    int servo_motor_manual_control() {

        log("In int servo_motor_manual_control()");

        // Initialize variables
        std::unordered_set<DWORD> keys;     // currently held keys (of interest)
        int v_x, v_z;
        g_running = true;


        log("\tInitializing critical section");
        // Initialize synchronization
        InitializeCriticalSection(&cs);
        g_mainThreadId = GetCurrentThreadId();      // get for PostThreadMessage

        // Install keyboard hook
        log("\tCritical section initialized, installing keyboard hook...");
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
        if (g_keyboardHook == NULL) {
            log("Failed to install keyboard hook.");
            DeleteCriticalSection(&cs);
            return 1;
        }


        log("\tKeyboard hook installed, starting message queue...");
        MSG msg;// Run Program
        PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);  // Ensure message queue exists

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
        }

        log("\tUnloading keyboard hook and deleting critical section");
        UnhookWindowsHookEx(g_keyboardHook);        // Unload the hook
        DeleteCriticalSection(&cs);
        log("exiting...");

        return 0;
    }

}





















    



