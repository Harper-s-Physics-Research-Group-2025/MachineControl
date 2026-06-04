#include "Oven5R6900.h"


using namespace std;



int main(int argc, char** argv) {
    // Set temperature controller setpoint
    // Input: String com port, desired set temperature
    // Output: New setpoint
    // Returns: Errorcode, 0 = success, 1 = failure

    // check for correct number of command line arguments
    if (argc != 3) {
        cerr << "Usage: <COM port> <temp>" << endl;
        return 2;
    }

    
    Oven5R6900 thermoelectric = Oven5R6900(argv[1]);
    int mode;
    float temp;
    try {
        temp = stof(argv[2]);
    } catch (const invalid_argument& e) {
        cerr << "Invalid input: " << e.what() << " '" << argv[2] << "'" << endl;
        return 2;
    }

    if (!thermoelectric.get_mode(mode) || mode != 0) {
        cout << "Controller not in fixed temperature mode." << endl;
    }


    if (!thermoelectric.set_setpoint(temp)) {
        cerr << "Temperature set failed" << endl;
        return 1;
    }

    cout << "Set temp: " << temp << endl;
    return 0;

}