#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port, float temp
    // Output: Echos desired setpoint
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 3) {
        cerr << "incorrect number " << argc-1 << " of command line arguments passed" << endl;
        return 1;
    }

    RTE7 bath = RTE7(argv[1]);

    if (!bath.set_setpoint(stof(argv[2]))) {
            cout << -999;
        return 2;
    } else {
        cout << argv[2];
        return 0;
    }

}