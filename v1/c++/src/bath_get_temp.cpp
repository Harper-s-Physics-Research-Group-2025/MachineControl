#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port
    // Output: Current bath temperature
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage <command> <COM port>" << endl;
        return 2;
    }

    float temp;
    RTE7 bath = RTE7(argv[1]);

    if (!bath.get_temp(temp)) {
        return 1;
    }

    cout << "Temp: " << temp << endl;
    return 0;


}