#include "Oven5R6900.h"
#include <iostream>


using namespace std;



int main(int argc, char** argv) {
    // Set temperature controller setpoint
    // Input: String com port, desired set temperature
    // Output: New setpoint
    // Returns: Errorcode, 0 = success, 1 = failure

    // check for correct number of command line arguments
    if (argc != 3) {
        cerr << "incorrect number " << argc-1 << " of command line arguments passed" << endl;
        return 2;
    }



    
    Oven5R6900 thermoelectric = Oven5R6900(argv[1]);
    float temp = stof(argv[2]);

    if (!thermoelectric.set_setpoint(temp)) {
        cout << -999;
        cerr << "Temperature set failed" << endl;
        return 1;
    } else {
        cout << temp;
        return 0;
    }

}