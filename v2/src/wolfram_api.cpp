/*
wolfram_api.cpp

Wolfram LibraryLink API wrapper for the Lab namespace.
Acts as the translation and safety layer between the Wolfram Kernel and low-level hardware.

Author: Joshua Darrow and Samuel Ntadom
Date: 06.09.2026 1330
*/

#include "WolframLibrary.h"
#include "controls/lab.h"

extern "C" {

    /* ==========================================================================
       WOLFRAM LIFECYCLE MANAGEMENT
       ========================================================================== */

    // Returns LibraryLink compatibility framework version indices.
    DLLEXPORT mint WolframLibrary_getVersion() { 
        return WolframLibraryVersion; 
    }
    
    // Automatically invoked by Wolfram when the DLL is first loaded into memory.
    DLLEXPORT int WolframLibrary_initialize(WolframLibraryData lp) {
        Lab::log("\n\n--- Initializing Wolfram API Layer ---");
        Lab::initialize_servos();
        return LIBRARY_NO_ERROR; 
    }

    // Automatically invoked by Wolfram when the library is explicitly unloaded.
    // Cleanly tears down global memory vectors and safely closes active COM port handles.
    DLLEXPORT void WolframLibrary_uninitialize(WolframLibraryData lp) {
        Lab::log("--- Uninitializing Wolfram API Layer ---");
        Lab::delete_bath();
        Lab::delete_temp_controller();
        Lab::shutdown_servos();
    }


    /* ==========================================================================
       LOGGING SUBSYSTEM
       ========================================================================== */

    // Returns logging status (0 = Disabled, 1 = Enabled) to the Kernel payload.
    DLLEXPORT int wget_logging_status(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        bool verbose;
        std::string file;
        
        int return_code = Lab::get_log_settings(verbose, file);
        MArgument_setInteger(Res, static_cast<mint>(verbose));
        return return_code;
    }

    // Returns the active target path of the physical log file as a string.
    DLLEXPORT int wget_log_file(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        bool verbose;
        std::string file;
        
        int return_code = Lab::get_log_settings(verbose, file);
        MArgument_setUTF8String(Res, const_cast<char*>(file.c_str()));
        return return_code;
    }

    // Commits dynamic runtime logging path parameters into the core system log streams.
    DLLEXPORT int wset_log_settings(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        bool verbose = MArgument_getInteger(Args[0]);
        std::string file = MArgument_getUTF8String(Args[1]);

        int return_code = Lab::set_log_settings(verbose, file);
        MArgument_setInteger(Res, static_cast<mint>(verbose));
        return return_code;
    }


    /* ==========================================================================
       FLUID BATH WRAPPERS (RTE7)
       ========================================================================== */

    // Auto-detects the bath's COM port and instantiates the RTE7 object on the heap.
    DLLEXPORT int winitialize_bath(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::init_bath();
    }

    // Destroys the heap bath structure, forcing an internal destructor port release sequence.
    DLLEXPORT int wdelete_bath(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::delete_bath(); 
    }

    // Commits hardware activation signals to the heater/pump circuits.
    DLLEXPORT int wbath_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::bath_on(); 
    }

    // Disables active loop elements and steps the bath down to a safe standby state.
    DLLEXPORT int wbath_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::bath_off(); 
    }

    // Decouples serial tracking loops, shifting interface control back to manual physical buttons.
    DLLEXPORT int wbath_manual(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::bath_manual(); 
    }

    // Extracts structural real-time temperature reads from the inner bath RTD probe.
    DLLEXPORT int wbath_get_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        float temp;
        int return_code = Lab::bath_get_temp(temp);
        MArgument_setReal(Res, static_cast<double>(temp));
        return return_code; 
    }

    // Collects the target reference setpoint configured inside the controller logic bounds.
    DLLEXPORT int wbath_get_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        float temp;
        int return_code = Lab::bath_get_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));
        return return_code; 
    }

    // Pushes an updated absolute target thermal point down the transmission matrix lines.
    DLLEXPORT int wbath_set_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        float temp = static_cast<float>(MArgument_getReal(Args[0]));
        int return_code = Lab::bath_set_setpoint(temp);
        
        MArgument_setReal(Res, static_cast<double>(temp));
        Lab::log("Bath setpoint written: " + std::to_string(temp) + " | Status: " + std::to_string(return_code));
        return return_code; 
    }



    /* ==========================================================================
       TEMPERATURE CONTROLLER WRAPPERS (H-BRIDGE/THERMISTOR)
       ========================================================================== */

    // Auto-detects the temp controller's COM port, allocates controller memory structures,
    // and locks the target serial loop communications.
    DLLEXPORT int winitialize_temperature_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::init_temp_controller();
    }

    // Releases controller tracking contexts and updates internal structures back to nullptr.
    DLLEXPORT int wdelete_temperature_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::delete_temp_controller(); 
    }

    // Energizes the high-power H-bridge transistor driver blocks.
    DLLEXPORT int wtemperature_control_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){        
        return Lab::temperature_control_on(); 
    }
    
    // De-energizes H-bridge gates to instantly kill current to the thermoelectric elements.
    DLLEXPORT int wtemperature_control_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::temperature_control_off(); 
    }

    // Reads operational mode state indices (e.g., 0 = Normal, 2 = Ramp/Soak profiles).
    DLLEXPORT int wtemperature_control_get_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        int mode;
        int return_code = Lab::temperature_control_get_mode(mode);
        MArgument_setInteger(Res, static_cast<mint>(mode));
        return return_code; 
    }

    // Commits updated functional cycle profiles to internal device registers.
    DLLEXPORT int wtemperature_control_set_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){       
        int mode = static_cast<int>(MArgument_getInteger(Args[0]));
        int return_code = Lab::temperature_control_set_mode(mode);
        MArgument_setInteger(Res, static_cast<mint>(mode));
        return return_code;  
    }

    // Queries current thermistor probe diagnostic arrays for local environmental calculations.
    DLLEXPORT int wtemperature_control_get_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        float temp;
        int return_code = Lab::temperature_control_get_temp(temp);
        MArgument_setReal(Res, static_cast<double>(temp));
        return return_code; 
    }

    // Extracts the active target reference setpoint tracking value.
    DLLEXPORT int wtemperature_control_get_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        float temp;
        int return_code = Lab::temperature_control_get_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));
        return return_code; 
    }

    // Updates target reference limits across active thermoelectric control registers.
    DLLEXPORT int wtemperature_control_set_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        float temp = static_cast<float>(MArgument_getReal(Args[0]));
        int return_code = Lab::temperature_control_set_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));
        return return_code; 
    }


    /* ==========================================================================
       DATA ACQUISITION (LABJACK)
       ========================================================================== */

    // Commands an single analog voltage execution read across target physical pins via USB.
    DLLEXPORT int wread_labjack_ain(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        long channel = static_cast<long>(MArgument_getInteger(Args[0]));
        double voltage;
        
        int return_code = Lab::read_labjack_ain(channel, voltage);
        MArgument_setReal(Res, voltage);        
        return return_code; 
    }

    
    /* ==========================================================================
       SERVO MOTOR WRAPPERS (TEKNIC)
       ========================================================================== */

    // Connects to the global sFoundation framework and resets peripheral safety loops.
    DLLEXPORT int winitialize_servos(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return Lab::initialize_servos();
    }

    // Commands all axis channels to spin down, unlinking active communication links.
    DLLEXPORT int wshutdown_servos(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return Lab::shutdown_servos();
    }

    // Returns a raw status evaluation checking if physical motor channels are awake.
    DLLEXPORT int wservo_hardware_online(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        MArgument_setInteger(Res, Lab::servos_ready());
        return LIBRARY_NO_ERROR;
    }

    // Unpacks raw dual-axis state safety logs into a joined string token payload.
    DLLEXPORT int wget_servo_alerts(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        char alertX[256] = {0};
        char alertZ[256] = {0};

        int return_code = Lab::get_servo_alerts(alertX, alertZ);
        std::string combined_alerts = "X: " + std::string(alertX) + " | Z: " + std::string(alertZ);

        MArgument_setUTF8String(Res, const_cast<char*>(combined_alerts.c_str()));
        return return_code;
    }

    // Triggers absolute mechanical axis homing limits until a timeout cutoff is reached.
    DLLEXPORT int wservos_home(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        if (Argc != 1) return LIBRARY_FUNCTION_ERROR;
        
        int milliseconds = static_cast<int>(MArgument_getInteger(Args[0])); 
        int return_code = Lab::servo_motor_home(milliseconds);
        return return_code;
    }

    // Returns structural {X, Z} positioning scales packed inside a 1D Wolfram MTensor block.
    DLLEXPORT int wservos_get_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){    
        mint dims[1] = {2}; 
        MTensor position;

        int err = lp->MTensor_new(MType_Real, 1, dims, &position);
        if (err) return LIBRARY_MEMORY_ERROR;

        double* data = lp->MTensor_getRealData(position);
        float x_mm, z_mm;

        int return_code = Lab::servos_get_position(x_mm, z_mm);
        data[0] = static_cast<double>(x_mm);
        data[1] = static_cast<double>(z_mm);
        
        MArgument_setMTensor(Res, position);
        return return_code;
    }

    // Drives linear platform coordinates to absolute {X, Z} locations at fixed feedrates.
    DLLEXPORT int wservos_set_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        mint dims[1] = {3}; 
        MTensor position;

        int err = lp->MTensor_new(MType_Real, 1, dims, &position);
        if (err) return LIBRARY_MEMORY_ERROR;

        double* data = lp->MTensor_getRealData(position);
        float x_mm = static_cast<float>(MArgument_getReal(Args[0]));
        float z_mm = static_cast<float>(MArgument_getReal(Args[1]));
        float vel_rms = static_cast<float>(MArgument_getReal(Args[2]));

        int return_code = Lab::servos_set_position(x_mm, z_mm, vel_rms);
        data[0] = static_cast<double>(x_mm);        
        data[1] = static_cast<double>(z_mm);
        data[2] = static_cast<double>(vel_rms);        

        MArgument_setMTensor(Res, position);
        return return_code; 
    }

    // Confirms if tracking limits match initialized status fields.
    DLLEXPORT int wmotors_ready(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        int return_code = Lab::servos_homed();
        MArgument_setInteger(Res, static_cast<mint>(return_code));
        return !return_code;
    }

    // Evaluates local flags to assert whether internal home data coordinate scaling is active.
    DLLEXPORT int wservos_homed(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        int return_code = Lab::servos_homed();
        MArgument_setInteger(Res, static_cast<mint>(return_code));
        return !return_code;
    }

    // Drops motor safety overrides, exposing individual axis configurations to free physical tuning.
    DLLEXPORT int wmanual_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        int return_code = Lab::servo_motor_manual_control();
        MArgument_setInteger(Res, static_cast<mint>(return_code));
        return return_code;
    }

    /* ==========================================================================
       DEBUGGING & UNIT TEST FALLBACKS
       ========================================================================== */

    // Debug tracking token forcing a flat validation success pathway.
    DLLEXPORT int wreturn_success(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return LIBRARY_NO_ERROR;
    }

    // Debug tracking token forcing an unhandled evaluation runtime failure response.
    DLLEXPORT int wreturn_error(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return LIBRARY_FUNCTION_ERROR;
    }
}