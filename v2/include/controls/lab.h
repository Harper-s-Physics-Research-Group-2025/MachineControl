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


// Equipment classes
#include "RTE7.h" // For bath controller
#include "Oven5R6900.h" // For temperature control functions

#include "LabJackUD.h"   // Header for LabJack UD library

//#include "recorder.h" // For recorder

#include "pubSysCls.h" // For servo motor functions

// For servo_motor_set_position
//#include <windows.h>
//#include <algorithm>

// Convention: return 0 for success
//                   other integer for error
//             pass values and results via referenced variables
namespace Lab {

    // RTE7 bath suite
    int bath_on(std::string COMM);
    int bath_off(std::string COMM);
    int bath_manual(std::string COMM);
    int bath_get_temp(std::string COMM, float& temp);
    int bath_get_setpoint(std::string COMM, float& temp);
    int bath_set_setpoint(std::string COMM, float& temp); 


    // Oven industries 5R6-900 temperature control suite
    int temperature_control_on(std::string COMM);
    int temperature_control_off(std::string COMM);
    int temperature_control_get_mode(std::string COMM, int& mode);
    int temperature_control_set_mode(std::string COMM, int& mode); 
    int temperature_control_get_temp(std::string COMM, float& temp);
    int temperature_control_get_setpoint(std::string COMM, float& temp);
    int temperature_control_set_setpoint(std::string COMM, float& temp);

    // int temperature_control_ramp_soak(std::string COMM, double seq_num, int soak_temp, int ramp_dur, double soak_dur, int deviation);

    int read_labjack_ain(const long channel, double& voltage);
    // void record(std::string CSV_FILENAME, std::string BATH_PORT, std::string TEMERATURE_PORT) const;
    
    // Overloaded servo homing functions
    // int servo_motor_home() const;
    int servo_motor_home(int milliseconds);  // Returns homing status code
    
    // bool servo_motor_is_home();
    
    int servo_motor_get_position(float& x_mm, float& y_mm);
    
    // Overloaded servo positioning functions
    int servo_motor_set_position(double& x_mm, double& y_mm, double& vel_rms);
    int servo_motor_manual_control();  
    
    
    // function definitions
    LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);     // keyboard callback
    
}