/*
src/lab.cpp
Description: Namespace that contains functions for operating the sample holder, sensors, and other related equipment
Version: 0
Authors: Josh Darrow and Samuel Ntadom
Date: 06.09.2026 1400
*/

#include "controls/lab.h"


// RTE7 Bath functions
int Lab::bath_on(std::string COMM) {
    RTE7 bath = RTE7(COMM);
    return !bath.turn_on();
}


int Lab::bath_off(std::string COMM) {
    RTE7 bath = RTE7(COMM);
    return !bath.turn_off();
}


int Lab::bath_manual(std::string COMM) {
    RTE7 bath = RTE7(COMM);
    return !bath.manual();
}


int Lab::bath_get_temp(std::string COMM, float& temp) {
    RTE7 bath = RTE7(COMM);
    return !bath.get_temp(temp);
}


int Lab::bath_get_setpoint(std::string COMM, float& temp) {
    RTE7 bath = RTE7(COMM);
    return !bath.get_setpoint(temp);
}


int Lab::bath_set_setpoint(std::string COMM, float& temp) {
    RTE7 bath = RTE7(COMM);
    return !bath.set_setpoint(temp);
}




// Oven Industries 5R6-900 Temperature Controller
int Lab::temperature_control_on(std::string COMM) {         // Enable H-bridge output
    Oven5R6900 tc = Oven5R6900(COMM);
    bool state = 1;
    return !tc.enable(state);
}


int Lab::temperature_control_off(std::string COMM) {        // Disable H-bridge output
    Oven5R6900 tc = Oven5R6900(COMM);
    bool state = 0;
    return !tc.enable(state);
}


int Lab::temperature_control_get_mode(std::string COMM, int& mode) {
    Oven5R6900 tc = Oven5R6900(COMM);
    return !tc.get_mode(mode);
}


int Lab::temperature_control_set_mode(std::string COMM, int& mode) {
    Oven5R6900 tc = Oven5R6900(COMM);
    return !tc.set_mode(mode);
}


int Lab::temperature_control_get_temp(std::string COMM, float& temp) {
    Oven5R6900 tc = Oven5R6900(COMM);
    return !tc.get_temp(temp);
}


int Lab::temperature_control_get_setpoint(std::string COMM, float& temp) {
    Oven5R6900 tc = Oven5R6900(COMM);
    return !tc.get_setpoint(temp);
}


int Lab::temperature_control_set_setpoint(std::string COMM, float& temp) {
    Oven5R6900 tc = Oven5R6900(COMM);
    return !tc.set_setpoint(temp);
}





// Labjack read channel
int Lab::read_labjack_ain(const long channel, double& voltage) {
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