#include "RTE7.h"


using namespace std;


// Constants
float TEMP;
float SET_TEMP;


int main(int argc, char** argv) {
    // Get bath parameters
    // Input: String com port
    // Output: Current bath parameters
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage <command> <COM port>" << endl;
        return 2;
    }

    
    RTE7 bath = RTE7(argv[1]);

    if (!bath.get_temp(TEMP)
    || !bath.get_setpoint(SET_TEMP)) {
        cout << "Read Failed." << endl;
        return 1;
    }

    cout << "Temp: " << TEMP << endl;
    cout << "Setpoint: " << SET_TEMP << endl;
    return 0;

}