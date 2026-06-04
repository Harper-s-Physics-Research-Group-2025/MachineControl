#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port, float temp
    // Output: bool was operation successful

    float temp;
    RTE7 bath = RTE7(argv[1]);

    if (!bath.get_temp(temp)) {
        cout << -999;
        return 0;
    } else {
        cout << temp;
        return 1;
    }

}