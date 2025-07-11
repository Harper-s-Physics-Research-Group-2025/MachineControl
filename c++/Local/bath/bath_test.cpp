#include "RTE7.h"
#include <iostream>

using namespace std;

int main() {

    RTE7 neslab = RTE7("COM4");
    float temp;
    float setpoint;
    
    neslab.turn_on();
    Sleep(2450); // wait 2.45 seconds before turning off
    neslab.get_temp(temp);

    cout << "The current bath temperature is " << temp << endl;

    neslab.set_setpoint(30.0);
    neslab.get_setpoint(setpoint);

    cout << "The current bath setpoint is " << setpoint << endl;

    neslab.turn_off();

    return 0;
}