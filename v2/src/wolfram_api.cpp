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
    DLLEXPORT int WolframLibrary_initialize(WolframLibraryData lp) {  return 0; }
    
    /*****************************************************
     * Function name: WolframLibrary_uninitialize
     * Input: lp (WolframLibraryData)
     * Side effect: None
     * Output: None
     *****************************************************/
    DLLEXPORT void WolframLibrary_uninitialize(WolframLibraryData lp) {}



    /* ==========================================================================
       WOLFRAM INTERFACE FUNCTIONS
       ========================================================================== */


    /*****************************************************
     * Function name: bath_on
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Turns on bath system equipment
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);

        return Lab::bath_on(comm_str); 
    }


    // /*****************************************************
    //  * Function name: bath_off
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Powers down the fluid bath safely
    //  * Output: int (LIBRARY_NO_ERROR)
    //  *****************************************************/
    DLLEXPORT int wbath_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        return Lab::bath_off(comm_str); 
    }


    /*****************************************************
     * Function name: wbath_manual
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Puts bath in manual mode (enables buttons on the display)
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_manual(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        return Lab::bath_manual(comm_str); 
    }


    /*****************************************************
     * Function name: bath_get_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Places evaluated loop status inside returning variables
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wbath_get_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        float temp;

        int return_code = Lab::bath_get_temp(comm_str, temp);
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
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        float temp;

        int return_code = Lab::bath_get_setpoint(comm_str, temp);
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
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        float temp = static_cast<float>MArgument_getReal(Args[1]);

        int return_code = Lab::bath_set_setpoint(comm_str, temp);
        MArgument_setReal(Res, static_cast<double>(temp));

        return return_code; 
    }







    /*****************************************************
     * Function name: temperature_control_on
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Turns on H-bridge transistor output
     * Output: int return code
     *****************************************************/
    DLLEXPORT int wtemperature_control_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        return Lab::temperature_control_on(comm_str); 
    }
    
    
    /*****************************************************
     * Function name: temperature_control_off
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Effect: Turns off H-bridge transistor output
     * Output: int return code
     *****************************************************/
    DLLEXPORT int wtemperature_control_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        return Lab::temperature_control_off(comm_str); 
    }

    
    /*****************************************************
     * Function name: temperature_control_get_mode
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Polls underlying control loop structures directly
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int wtemperature_control_get_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        int mode;

        int return_code = Lab::temperature_control_get_mode(comm_str, mode);
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
        if (Argc != 2) return LIBRARY_FUNCTION_ERROR;
        
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        int mode = static_cast<int>(MArgument_getInteger(Args[1]));

        int return_code = Lab::temperature_control_set_mode(comm_str, mode);
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
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        float temp;

        int return_code = Lab::temperature_control_get_temp(comm_str, temp);
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
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        float temp;

        int return_code = Lab::temperature_control_get_setpoint(comm_str, temp);
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
        if (Argc != 2) return LIBRARY_FUNCTION_ERROR;
        
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        float temp = static_cast<float>MArgument_getReal(Args[1]);

        int return_code = Lab::temperature_control_set_setpoint(comm_str, temp);
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
     * Function name: servo_motor_home
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Resets staging platforms back to index boundaries
     * Output: int (LIBRARY_NO_ERROR or parameter function issues checking limits)
     *****************************************************/
    DLLEXPORT int servo_motor_home(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        if (Argc == 0) {
            int return_code = Lab::servo_motor_home(20000);
            return return_code;
        }
        else if (Argc == 1) { 
            int milliseconds = static_cast<int>(MArgument_getReal(Args[0])); 
            int return_code = Lab::servo_motor_home(milliseconds);
            return return_code;
        } 
        else {
            return LIBRARY_FUNCTION_ERROR;
        }
    }

    // /*****************************************************
    //  * Function name: servo_motor_is_home
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Validates coordinate configuration integrity indices 
    //  * Output: int (LIBRARY_NO_ERROR)
    //  *****************************************************/
    // DLLEXPORT int servo_motor_is_home(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
    //     equipment.servo_motor_is_home();
    //     return LIBRARY_NO_ERROR;
    // }

    // /*****************************************************
    //  * Function name: servo_motor_manual_control
    //  * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
    //  * Side effect: Redirects terminal pipelines into Interactive driving modes
    //  * Output: int (LIBRARY_NO_ERROR)
    //  *****************************************************/
    // DLLEXPORT int servo_motor_manual_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){  
    //     equipment.servo_motor_manual_control();
    //     return LIBRARY_NO_ERROR; 
    // }

    /*****************************************************
     * Function name: servo_motor_read_position
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Evaluates positional feedback frames from connected hardware axes
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int servo_motor_read_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){    
        
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

        int return_code = Lab::servo_motor_get_position(x_mm, z_mm);

        data[0] = x_mm;
        data[1] = z_mm;
        
        MArgument_setMTensor(Res, position);

        return LIBRARY_NO_ERROR;
    }

    /*****************************************************
     * Function name: servo_motor_set_position
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Passes absolute microstepping dimensional target positions down to node controllers
     * Output: int (LIBRARY_NO_ERROR or error values bounds tracking check)
     *****************************************************/
    DLLEXPORT int servo_motor_set_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        
        // 1. set tensor dimensions
        mint dims[1] = {3}; 
        MTensor position;

        // 2. Allocate memory for the tensor
        int err = lp->MTensor_new(MType_Real, 1, dims, &position);
        if (err) return LIBRARY_MEMORY_ERROR;

        // 3. Extract the underlying C data pointer
        double* data = lp->MTensor_getRealData(position);


        data[0] = MArgument_getReal(Args[0]);
        data[1] = MArgument_getReal(Args[1]);
        data[2] = MArgument_getReal(Args[2]);

        int return_code = Lab::servo_motor_set_position(data[0], data[1], data[2]);

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
}