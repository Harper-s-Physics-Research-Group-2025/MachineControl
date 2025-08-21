#include "Oven5R6900.h"


using namespace std;



int main(int argc, char** argv) {
    // Set temperature controller mode
    // Input: String com port, desired mode
    // Output: New mode
    // Returns: Errorcode, 0 = success, 1 = failure

    // check for correct number of command line arguments
    if (argc != 3) {
        cerr << "Usage: <COM port> <mode>" << endl;
        return 2;
    }



    
    Oven5R6900 thermoelectric = Oven5R6900(argv[1]);
    int mode;

    try {
        mode  = stoi(argv[2]);    // populate argument array and convert types        
    } catch (const invalid_argument& e) {
        cerr << "Invalid input: " << e.what() << " '" << argv[2] << "'" << endl;
        return 2;
    }

    
    if (!thermoelectric.set_mode(mode)) {
        cerr << "Mode set failed" << endl;
        return 1;
    }


    cout << "Mode: " << mode << endl;
    return 0;

}