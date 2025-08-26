#include "Oven5R6900.h"


using namespace std;


int main(int argc, char** argv) {
    // Turn temp controller off
    // Input: String com port
    // Returns: errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage: <COM port>" << endl;
        return 2;
    }
    
    
    Oven5R6900 temp_controller = Oven5R6900(argv[1]);
    bool state = 0;

    if (!temp_controller.enable(state) || state == 1) {
        cerr << "Shutdown failed" << endl;
        return 1;
    }
    
    cout << "Success!" << endl;
    
    return 0;
}