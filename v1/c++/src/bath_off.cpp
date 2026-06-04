#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Turn bath off
    // Input: String com port
    // Returns: errorcode, 0 = noerror, 1 = error


    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage <command> <COM port>" << endl;
        return 2;
    }

    RTE7 bath = RTE7(argv[1]);

    if (!bath.turn_off()) {
        cerr << "shutdown failed" << endl;
        return 1;
    }
    
    cout << "Success!" << endl;
    return 0;
}