#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Turn bath off
    // Input: String com port
    // Output: bool was operation successful (opposite of conventional 0 = no error)

    RTE7 bath = RTE7(argv[1]);

    if (!bath.turn_off()) {
        return 0;
    } else {
        return 1;
    }

}