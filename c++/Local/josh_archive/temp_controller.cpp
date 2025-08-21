#include "Oven5R6900.h"

#include <iostream>

using namespace std;


int main() {

    float number;
    int32_t mode;

    Oven5R6900 temp_controller = Oven5R6900("COM3");

    temp_controller.get_mode(mode);
    cout << "Mode: " << mode << endl;

    temp_controller.get_setpoint(number);
    cout << "Current setpoint: " << fixed << setprecision(2) << number << " C" << endl;

    temp_controller.get_temp(number);
    cout << "Current temperature: " << fixed << setprecision(2) << number << " C" << endl;

    temp_controller.set_mode(mode);
    cout << "Mode set to " << mode << endl;

    number = 20;
    temp_controller.set_setpoint(number);
    cout << "Setpoint set to " << number << " C" << endl;



    return 0;
}