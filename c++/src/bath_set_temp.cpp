#include "RTE7.h"

using namespace std;

int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port, float temp
    // Output: Echos desired setpoint
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 3) {
        cerr << "Usage: <command> <COM port> <temp>" << endl;
        return 1;
    }

    RTE7 bath = RTE7(argv[1]);

    float temp;
    try {
        temp = stof(argv[2]);
    } catch (const invalid_argument& e) {
        cerr << "Invalid input: " << e.what() << " '" << argv[2] << "'" << endl;
        return 2;
    }

    
    if (!bath.set_setpoint(temp)) {
        return 2;
    } 
    
    cout << "Set temp: " << temp << endl;
    return 0;

}