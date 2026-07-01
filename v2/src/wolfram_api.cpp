/*
wolfram_api.cpp

Exposes lab equipment functionality (functions) to be able to be called by Wolfram code.

Author: Joshua Darrow and Samuel Ntadom
Date: 06.09.2026 1330


*/


#include "WolframLibrary.h"
#include "controls/lab.h"




extern "C" {

    /*****************************************************
     * Function name: WolframLibrary_getVersion
     * Input: None
     * Side effect: None
     * Output: mint (Returns engine layout version indices)
     *****************************************************/
    DLLEXPORT mint WolframLibrary_getVersion() { return WolframLibraryVersion; }
    
    /*****************************************************
     * Function name: WolframLibrary_initialize
     * Input: lp (WolframLibraryData)
     * Side effect: Performs low-level runtime integration checks
     * Output: int (0 on initialization tracking)
     *****************************************************/
    DLLEXPORT int WolframLibrary_initialize(WolframLibraryData lp) {
        Lab::log("\n\n");
        Lab::initialize_servos();
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: WolframLibrary_uninitialize
     * Input: lp (WolframLibraryData)
     * Side effect: None
     * Output: None
     *****************************************************/
    DLLEXPORT void WolframLibrary_uninitialize(WolframLibraryData lp) {
        Lab::delete_bath();
        Lab::delete_temp_controller();
        Lab::shutdown_servos();
    }



    /* ==========================================================================
       WOLFRAM INTERFACE FUNCTIONS
       ========================================================================== */



    // Get logging status. 0 = Off; 1 = On
    DLLEXPORT int wget_logging_status(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        int return_code;
        bool verbose;
        std::string file;
        
        return_code = Lab::get_log_settings(verbose, file);
        MArgument_setInteger(Res, static_cast<int>(verbose));
        return return_code;
    }


    // Get log file path
    DLLEXPORT int wget_log_file(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        int return_code;
        bool verbose;
        std::string file;
        return_code = Lab::get_log_settings(verbose, file);
        MArgument_setUTF8String(Res, const_cast<char*>(file.c_str()));
        return return_code;
    }


    // Set logging status and log file path.
    DLLEXPORT int wset_log_settings(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        int return_code;
        bool verbose = MArgument_getInteger(Args[0]);
        std::string file = MArgument_getUTF8String(Args[1]);

        if (!Lab::logfile_valid(file)) {
            std::string err_msg = "File path " + file = " is not a valid path.";
            lp->Message(err_msg.c_str());
            return 1;
        }

        return_code = Lab::set_log_settings(verbose, file);
        MArgument_setInteger(Res, static_cast<int>(verbose));

        return return_code;
    }





    // Initialize RTE7 bath api object in memory (open the COM port)
    DLLEXPORT int winitialize_bath(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);

        return Lab::init_bath(comm_str); 
    }


    // Release bath object memory and set bath pointer to nullptr.
    DLLEXPORT int wdelete_bath(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::delete_bath(); 
    }


    /*****************************************************
     * Function name: bath_on
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Turns on bath system equipment
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::bath_on(); 
    }


    // /*****************************************************
    //  * Function name: bath_off
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Powers down the fluid bath safely
    //  * Output: int (LIBRARY_NO_ERROR)
    //  *****************************************************/
    DLLEXPORT int wbath_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::bath_off(); 
    }


    /*****************************************************
     * Function name: wbath_manual
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Puts bath in manual mode (enables buttons on the display)
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_manual(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::bath_manual(); 
    }


    /*****************************************************
     * Function name: bath_get_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Places evaluated loop status inside returning variables
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_get_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){

        float temp;
        int return_code = Lab::bath_get_temp(temp);
        MArgument_setReal(Res, static_cast<double>(temp));

        return return_code; 
    }


    /*****************************************************
     * Function name: bath_read_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Places evaluated loop status inside returning variables
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_get_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){

        float temp;
        int return_code = Lab::bath_get_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));

        return return_code; 
    }


    /*****************************************************
     * Function name: wbath_set_setpoint
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Set desired bath temperature
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_set_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){

        float temp = static_cast<float>MArgument_getReal(Args[0]);
        int return_code = Lab::bath_set_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));
        Lab::log("bath setpoint: " + std::to_string(temp) + "return code: " + std::to_string(return_code));

        return return_code; 
    }






    // Initialize temp controller hardware api object (open the COM port, etc.)
    DLLEXPORT int winitialize_temperature_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        return Lab::init_temp_controller(comm_str); 
    }

    
    // Release temp controller object memory and set pointer to nullptr.
    DLLEXPORT int wdelete_temperature_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::delete_temp_controller(); 
    }


    /*****************************************************
     * Function name: temperature_control_on
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Turns on H-bridge transistor output
     * Output: int return code
     *****************************************************/
    DLLEXPORT int wtemperature_control_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){        
        return Lab::temperature_control_on(); 
    }
    
    
    /*****************************************************
     * Function name: temperature_control_off
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Turns off H-bridge transistor output
     * Output: int return code
     *****************************************************/
    DLLEXPORT int wtemperature_control_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        return Lab::temperature_control_off(); 
    }

    
    /*****************************************************
     * Function name: temperature_control_get_mode
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Polls underlying control loop structures directly
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wtemperature_control_get_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){

        int mode;
        int return_code = Lab::temperature_control_get_mode(mode);
        MArgument_setInteger(Res, static_cast<mint>(mode));

        return return_code; 
    }

    /*****************************************************
     * Function name: temperature_control_set_mode
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Commits updated state definitions to thermoelectric elements
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wtemperature_control_set_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){       
        
        int mode = static_cast<int>(MArgument_getInteger(Args[0]));
        int return_code = Lab::temperature_control_set_mode(mode);
        MArgument_setInteger(Res, static_cast<mint>(mode));

        return return_code;  
    }


    /*****************************************************
     * Function name: temperature_control_read_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Collects localized thermocouple instrumentation reading items
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wtemperature_control_get_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){

        float temp;
        int return_code = Lab::temperature_control_get_temp(temp);
        MArgument_setReal(Res, static_cast<double>(temp));

        return return_code; 
    }


    /*****************************************************
     * Function name: temperature_control_read_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Collects localized thermocouple instrumentation reading items
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wtemperature_control_get_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){

        float temp;
        int return_code = Lab::temperature_control_get_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));

        return return_code; 
    }


    /*****************************************************
     * Function name: temperature_control_set_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Prints set temperatures
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wtemperature_control_set_setpoint(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        
        float temp = static_cast<float>MArgument_getReal(Args[0]);
        int return_code = Lab::temperature_control_set_setpoint(temp);
        MArgument_setReal(Res, static_cast<double>(temp));

        return return_code; 
    }


    // /*****************************************************
    //  * Function name: temperature_control_dump
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Spits complete matrix overview tables out to terminal streams
    //  * Output: int (LIBRARY_NO_ERROR)
    //  *****************************************************/
    // DLLEXPORT int temperature_control_dump(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
    //     char* comm_c_str = MArgument_getUTF8String(Args[0]); 
    //     std::string comm_str(comm_c_str);
        
    //     equipment.temperature_control_dump(comm_str);
    //     return LIBRARY_NO_ERROR; 
    // }
    










    /*****************************************************
     * Function name: read_labjack_ain0
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Triggers analog acquisition over USB lines
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wread_labjack_ain(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        long channel = static_cast<long>(MArgument_getInteger(Args[0]));
        double voltage;
        int return_code = Lab::read_labjack_ain(channel, voltage);
        MArgument_setReal(Res, voltage);        
        return return_code; 
    }

    // /*****************************************************
    //  * Function name: record
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Fixed shadowing initialization issues across multiple port handles
    //  * Output: int (LIBRARY_NO_ERROR or structure argument bounds match checks)
    //  *****************************************************/
    // DLLEXPORT int record(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
    //     if (Argc != 3) {
    //         return LIBRARY_FUNCTION_ERROR;
    //     }
      
    //     char* csv_file = MArgument_getUTF8String(Args[0]); 
    //     std::string file_str(csv_file);  

    //     char* bath_port = MArgument_getUTF8String(Args[1]); 
    //     std::string b_port_str(bath_port); 

    //     char* temperature_port = MArgument_getUTF8String(Args[2]); 
    //     std::string t_port_str(temperature_port); 
        
    //     equipment.record(file_str, b_port_str, t_port_str);
    //     return LIBRARY_NO_ERROR; 
    // }

    /*****************************************************
     * Function name: winitialize_servos
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Sets up communication with servo motors.
     * Output: int (LIBRARY_NO_ERROR or parameter function issues checking limits)
     *****************************************************/
    DLLEXPORT int winitialize_servos(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return Lab::initialize_servos();
    }


    /*****************************************************
     * Function name: wshutdown_servos
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Stop communication with the servo motors
     * Output: int (LIBRARY_NO_ERROR or parameter function issues checking limits)
     *****************************************************/
    DLLEXPORT int wshutdown_servos(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return Lab::shutdown_servos();
    }


    // Simple function to pass the servo motor boolean state back to Mathematica
    DLLEXPORT int wservo_hardware_online(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        MArgument_setInteger(Res, Lab::servos_ready());
        return LIBRARY_NO_ERROR;
    }


    DLLEXPORT int wget_servo_alerts(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        char alertX[256] = {0};
        char alertZ[256] = {0};

        // Call your hardware function to fill the buffers
        int return_code = Lab::get_servo_alerts(alertX, alertZ);

        // Combine them into a clean message string
        std::string combined_alerts = "X: " + std::string(alertX) + " | Z: " + std::string(alertZ);

        // Set the result back to Mathematica as a string
        MArgument_setUTF8String(Res, const_cast<char*>(combined_alerts.c_str()));
        return return_code;
    }


    /*****************************************************
     * Function name: servo_motor_home
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Resets staging platforms back to index boundaries
     * Output: int (LIBRARY_NO_ERROR or parameter function issues checking limits)
     *****************************************************/
    DLLEXPORT int wservos_home(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        if (Argc == 1) { 
            int milliseconds = static_cast<int>(MArgument_getInteger(Args[0])); 
            int return_code = Lab::servo_motor_home(milliseconds);
            return return_code;
        } 
        else {
            return LIBRARY_FUNCTION_ERROR;
        }
    }


    /*****************************************************
     * Function name: servo_motor_read_position
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Evaluates positional feedback frames from connected hardware axes
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wservos_get_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){    
        
        // 1. set tensor dimensions
        mint dims[1] = {2}; 
        MTensor position;

        // 2. Allocate memory for the tensor
        int err = lp->MTensor_new(MType_Real, 1, dims, &position);
        if (err) return LIBRARY_MEMORY_ERROR;

        // 3. Extract the underlying C data pointer
        double* data = lp->MTensor_getRealData(position);


        float x_mm;
        float z_mm;

        int return_code = Lab::servos_get_position(x_mm, z_mm);

        data[0] = static_cast<double>(x_mm);
        data[1] = static_cast<double>(z_mm);
        
        MArgument_setMTensor(Res, position);

        return return_code;
    }

    /*****************************************************
     * Function name: servo_motor_set_position
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Passes absolute microstepping dimensional target positions down to node controllers
     * Output: int (LIBRARY_NO_ERROR or error values bounds tracking check)
     *****************************************************/
    DLLEXPORT int wservos_set_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        
        // 1. set tensor dimensions
        mint dims[1] = {3}; 
        MTensor position;

        // 2. Allocate memory for the tensor
        int err = lp->MTensor_new(MType_Real, 1, dims, &position);
        if (err) return LIBRARY_MEMORY_ERROR;

        // 3. Extract the underlying C data pointer
        double* data = lp->MTensor_getRealData(position);


        float x_mm = static_cast<float>(MArgument_getReal(Args[0]));
        float z_mm = static_cast<float>(MArgument_getReal(Args[1]));
        float vel_rms = static_cast<float>(MArgument_getReal(Args[2]));
        // int timeout_ms = static_cast<int>(MArgument_getReal(Args[3]));

        int return_code = Lab::servos_set_position(x_mm, z_mm, vel_rms);

        data[0] = static_cast<double>(x_mm);        
        data[1] = static_cast<double>(z_mm);
        data[2] = static_cast<double>(vel_rms);        
        // data[3] = static_cast<double>(timeout_ms);

        MArgument_setMTensor(Res, position);

        return return_code; 
    }

    

    // /*****************************************************
    //  * Function name: temperature_control_ramp_soak
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Triggers stepped heating/cooling configuration programs
    //  * Output: int (LIBRARY_NO_ERROR or error checks bounds verification checking)
    //  *****************************************************/
    // DLLEXPORT int temperature_control_ramp_soak(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
    //     if (Argc != 6) {
    //         return LIBRARY_FUNCTION_ERROR;
    //     }

    //     char* comm = MArgument_getUTF8String(Args[0]); 
    //     std::string comm_str(comm);
    //     double seq_num = MArgument_getReal(Args[1]);
    //     int soak_temp = static_cast<int>(MArgument_getReal(Args[2]));
    //     int ramp_dur = static_cast<int>(MArgument_getReal(Args[3]));
    //     double soak_dur = MArgument_getReal(Args[4]);
    //     int deviation = static_cast<int>(MArgument_getReal(Args[5]));

    //     equipment.temperature_control_ramp_soak(comm_str, seq_num, soak_temp, ramp_dur, soak_dur, deviation);
    //     return LIBRARY_NO_ERROR;
    // }


    DLLEXPORT int wmotors_ready(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        
        int return_code = Lab::servos_homed();
        MArgument_setInteger(Res, static_cast<mint>(return_code));
        return !return_code;
    }


    // Check if motors are homed.
     DLLEXPORT int wservos_homed(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
                
        int return_code = Lab::servos_homed();
        MArgument_setInteger(Res, static_cast<mint>(return_code));
        return !return_code;

    }


    // Manual control of servo motors
    DLLEXPORT int wmanual_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {

        int return_code = Lab::servo_motor_manual_control();
        MArgument_setInteger(Res, static_cast<mint>(return_code));
        return return_code;

    }


    DLLEXPORT int wreturn_success(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        return LIBRARY_NO_ERROR;
    }


    
    DLLEXPORT int wreturn_error(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
            return LIBRARY_FUNCTION_ERROR;
        }

}