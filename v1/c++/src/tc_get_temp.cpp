#include "Oven5R6900.h"


using namespace std;


int main(int argc, char** argv) {
    // Get temperature controller current temperature
    // Input: String com port
    // Output: Current temperature in Celcius
    // Returns: Errorcode, 0 = success, 1 = failure

    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage: <COM port>" << endl;
        return 2;
    }



    float temp;
    Oven5R6900 thermoelectric = Oven5R6900(argv[1]);


    if (!thermoelectric.get_temp(temp)) {
        cerr << "Temperature read failed" << endl;
        return 1;
    }
    
    cout << "Temp: " << temp << endl;
    return 0;
}