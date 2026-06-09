#include "Oven5R6900.h"

#include <iostream>
#include <string>

using namespace std;


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Not enough arguments. Usage: program.exe <action> || <action> <value>" << std::endl;
        return 1;
    }

    string action = argv[1];  // Convert first argument to int
    float value;
    int32_t mode;
    bool isTrue = argc == 3;
    
    if(action == "setMode" ) mode =  isTrue? atoi(argv[2]) : 0;  
    if (action == "setSetPointTemp") value = isTrue ? stof(argv[2]) : 0.0;
    

    Oven5R6900 temp_controller = Oven5R6900("COM10");

    if (action == "getMode") {
        temp_controller.get_mode(mode);
        std::cout << "Mode: " << mode << endl;
    }
    else if (action == "setMode") {
        temp_controller.set_mode(mode);
        std::cout << "Mode set to " << mode << endl;

    }
    else if (action == "getSetPointTemp") {
        temp_controller.get_setpoint(value);
        cout << "Current setpoint: " << fixed << setprecision(2) << value << " C" << endl;

    }
    else if (action == "setSetPointTemp") {
        temp_controller.set_setpoint(value);
        cout << "Setpoint set to " << value << " C" << endl;


    } else if (action == "getTemp") {
        temp_controller.get_temp(value);
        cout << "Current temperature: " << fixed << setprecision(2) << value << " C" << endl;
    }
    else {
        std::cout << "Unknown action: " << action << std::endl;
        return 1;
    }

   

   


    return 0;
}
