#include <gtest/gtest.h>
#include "Josh_Controller/API.h"
#include "Josh_Controller/RTE7.h"
#include "Josh_Controller/Oven5R6900.h"
#include "Josh_Controller/recorder.h"
#include "Josh_Controller/sm_homer.h"
#include "Josh_Controller/sm_manual_controller.h"

// Water Bath Control Tests
TEST(BathControlTests, BathOnTest) {
    LabEquipment equipment;
    // Test bath_on() with COMM port
}

TEST(BathControlTests, BathOffTest) {
    LabEquipment equipment;
    // Test bath_off()
}

// Servo Motor Tests
TEST(ServoMotorTests, HomePositionTest) {
    LabEquipment equipment;
    // Test servo_motor_home()
}

// Temperature Control Tests
TEST(TemperatureControlTests, RampSoakTest) {
    LabEquipment equipment;
    // Test temperature_control_ramp_soak()
}

// Data Acquisition Tests
TEST(DataAcquisitionTests, LabJackReadTest) {
    LabEquipment equipment;
    // Test read_labjack_ain0()
}

// Add more test classes and cases based on TESTS_TODO.md

