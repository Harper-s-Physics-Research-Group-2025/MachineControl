/*
Title: API.h
Description: The class declaration for Josh de Cart's controller 
Version: 1
Date Started: May 26, 2026
Authors: Josh Darrow and Samuel Ntadom
Date Completed: 
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
#include "WolframLibrary.h"

// For bath controller
#include "RTE7.h"

// For LabJack
#include "LabJackUD.h"   // Header for UD library
#pragma comment(lib, "LabJackUD.lib")  // Link UD lib (Windows)

// For recorder
#include "recorder.h"

// For sm_home
#include "sm_homer.h"

// For servo_motor_manual_control
#include "sm_manual_controller.h"

// For servo motor functions
#include "pubSysCls.h"

// For servo_motor_set_position
#include <windows.h>
#include <algorithm>

// For temperature control functions
#include "Oven5R6900.h"
#include <variant>

namespace Lab {
    int bath_on(std::string COMM) const;
    int bath_off(std::string COMM) const;
}

class LabEquipment {
public:     
    int bath_on(std::string COMM) const;
    int bath_off(std::string COMM) const;
    void bath_dump(std::string COMM) const;
    double bath_read_temp(std::string COMM) const;
    void bath_set_temp(std::string COMM, double Temp) const; // Added const to match API footprint
    void read_labjack_ain0() const;
    void record(std::string CSV_FILENAME, std::string BATH_PORT, std::string TEMERATURE_PORT) const;
    
    // Overloaded servo homing functions
    int servo_motor_home() const;
    int servo_motor_home(int milliseconds) const;  // Returns homing status code
    
    bool servo_motor_is_home();
    
    void servo_motor_manual_control() const;
    void servo_motor_read_position() const;
    
    // Overloaded servo positioning functions
    void servo_motor_set_position(double x_pos, double z_pos, int vel_rms) const;
    void servo_motor_set_position(double x_pos, double z_pos, int vel_rms, double milliseconds) const;
    
    // Temperature control suite
    void temperature_control_dump(std::string COMM) const; // Changed to int to match API.cpp return statement
    void temperature_control_read_mode(std::string COMM) const;
    void temperature_control_set_mode(std::string COMM, double Value) const; 
    void temperature_control_read_temp(std::string COMM) const;
    void temperature_control_set_temp(std::string COMM, double Temp) const;
    void temperature_control_off(std::string COMM) const;
    void temperature_control_on(std::string COMM) const;
    void temperature_control_ramp_soak(std::string COMM, double seq_num, int soak_temp, int ramp_dur, double soak_dur, int deviation) const;
};