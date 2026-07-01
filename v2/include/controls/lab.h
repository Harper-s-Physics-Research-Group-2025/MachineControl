/*
Title: lab.h
Description: Namespace that contains functions for operating the sample holder, sensors, and other related equipment
Version: 0
Authors: Josh Darrow and Samuel Ntadom
Date: 06.09.2026
 
*/
//TODO: make a get postion and temerature functions
//TODO: In new docummentation, ask users to install Wolfram, Labjack, and Clear view libraries
//TODO: Review sm_manual_controller
//Make sure the CMakeLists.txt is configured propely

#pragma once // Prevent double-inclusion compiler errors
#include <vector>
#include <numeric>
#include <string>
#include <iostream>
#include <iomanip>
#include <variant>
#include <queue>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <stdexcept>


// Equipment classes
#include "RTE7.h" // For bath controller
#include "Oven5R6900.h" // For temperature control functions

#include "LabJackUD.h"   // Header for LabJack UD library

#include "pubSysCls.h"          // For servo motor functions




//#include "recorder.h" // For recorder

// For servo_motor_set_position
//#include <windows.h>
//#include <algorithm>

// Convention: return 0 for success
//                   other integer for error
//             pass values and results via referenced variables
namespace Lab {

    // Persistent pointers to bath, temp controller
    extern RTE7* bath;
    extern Oven5R6900* tc;


    // Persistent hardware pointers to servo motors
    extern sFnd::SysManager* Mgr;
    extern sFnd::IPort* Port;
    extern sFnd::INode* motorX;
    extern sFnd::INode* motorZ;

    extern bool LOG;
    extern std::string LOG_FILE;

    void log(const std::string& msg);
    int get_log_settings(bool& verbose, std::string& file);
    int set_log_settings(bool& verbose, std::string& file);
    bool logfile_valid(std::string& filepath);


    // RTE7 bath suite
    int init_bath(std::string COMM);
    int delete_bath();
    int bath_on();
    int bath_off();
    int bath_manual();
    int bath_get_temp(float& temp);
    int bath_get_setpoint(float& temp);
    int bath_set_setpoint(float& temp); 


    // Oven industries 5R6-900 temperature control suite
    int init_temp_controller(std::string COMM);
    int delete_temp_controller();
    int temperature_control_on();
    int temperature_control_off();
    int temperature_control_get_mode(int& mode);
    int temperature_control_set_mode(int& mode); 
    int temperature_control_get_temp(float& temp);
    int temperature_control_get_setpoint(float& temp);
    int temperature_control_set_setpoint(float& temp);

    // int temperature_control_ramp_soak(std::string COMM, double seq_num, int soak_temp, int ramp_dur, double soak_dur, int deviation);
    // void record(std::string CSV_FILENAME, std::string BATH_PORT, std::string TEMERATURE_PORT) const;
    // // bool servo_motor_is_home();

    int read_labjack_ain(const long channel, double& voltage);
    
    // Servo motors
    int initialize_servos();
    int shutdown_servos();
    bool servos_ready();
    bool servos_homed();
    int get_servo_alerts(char* alertX, char* alertZ);
    int servo_motor_home(int milliseconds);  // Returns homing status code
    int servos_get_position(float& x_mm, float& y_mm);
    int servos_set_position(float& x_mm, float& y_mm, float& vel_rms);
    int servo_motor_manual_control(); 
    
    
    
    // Keyboard hook callback for manual control
    LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);     // keyboard callback
    
}