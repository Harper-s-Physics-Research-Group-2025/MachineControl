#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port, float temp
    // Output: Current bath temperature
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 3) {
        cerr << "incorrect number " << argc-1 << " of command line arguments passed" << endl;
        return 2;
    }

    float temp;
    RTE7 bath = RTE7(argv[1]);

    if (!bath.get_temp(temp)) {
        cout << -999;
        return 1;
    } else {
        cout << temp;
        return 0;
    }

}