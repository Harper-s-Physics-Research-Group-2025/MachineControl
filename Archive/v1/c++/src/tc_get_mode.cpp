#include "Oven5R6900.h"


using namespace std;



int main(int argc, char** argv) {
    // Get temperature controller mode
    // Input: String com port
    // Output: mode
    // Returns: Errorcode, 0 = success, 1 = failure

    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage: <COM port>" << endl;
        return 2;
    }



    
    Oven5R6900 thermoelectric = Oven5R6900(argv[1]);
    int mode = -1;

    if (!thermoelectric.get_mode(mode)) {
        cerr << "Mode get failed" << endl;
        return 1;
    }
    
    cout << "Mode: " << mode << endl;
    return 0;
    
}