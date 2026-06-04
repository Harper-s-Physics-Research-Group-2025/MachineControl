#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port, float temp
    // Output: bool was operation successful

    RTE7 bath = RTE7(argv[1]);

    if (!bath.set_setpoint(stof(argv[2]))) {
            cout << -999;
        return 0;
    } else {
        cout << argv[2];
        return 1;
    }

}