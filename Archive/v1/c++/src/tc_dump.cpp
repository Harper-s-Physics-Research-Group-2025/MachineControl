#include "Oven5R6900.h"
#include <variant>


using namespace std;
using generic_type = variant<int, float, string, bool>;


// for working with vectors of generic types
vector<generic_type> base_args = {20.0f, 300, 300, 0, 0, 5.0f, 1, string("0")};     // soak temp, ramp duration, soak duration, # repeats, next seq #, run_method, sequence
vector<vector<generic_type>> args(8, base_args);
ostream& operator<<(ostream& os, const generic_type& v) {
    visit([&](const auto& val) {
        os << val;
    }, v);
    return os;
}


// Constants
bool OUTPUT;
int MODE;
float CURR_TEMP;
float CURR_VOLTAGE;
int CURR_SEQ;

float SET_TEMP;
float SET_VOLTAGE;
int RS_STATUS;
bool RS;
bool RAMP;
bool SOAK;
float P;
float I;
float D;
int RUN_METHOD;
float MAX_DEVIATION;
int COUNT_LEN;



int main(int argc, char** argv) {
    // Set bath temperature
    // Input: String com port
    // Output: Most settable parameters
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc != 2) {
        cerr << "Usage: <command> <COM port>" << endl;
        return 2;
    }


    Oven5R6900 tc = Oven5R6900(argv[1]);

    if (!tc.get_state(OUTPUT)
        || !tc.get_mode(MODE)
        || !tc.get_temp(CURR_TEMP)
        || !tc.get_voltage(CURR_VOLTAGE)
        || !tc.get_ramp_soak_curr_seq(CURR_SEQ)
        || !tc.get_setpoint(SET_TEMP)
        || !tc.get_max_voltage(SET_VOLTAGE)
        || !tc.get_proportional_bandwidth(P)
        || !tc.get_integral_gain(I)
        || !tc.get_derivative_gain(D)
        || !tc.get_ramp_soak_status(RS_STATUS)
        || !tc.get_run_method(RUN_METHOD)
        || !tc.get_max_deviation(MAX_DEVIATION)
        || !tc.get_count_length(COUNT_LEN)) 
    {
        cerr << "Initial read failed." << endl;    
        return 1;
    }


    for (int i = 0; i < args.size(); i++) {

        args[i][7] = to_string(i);      // correct sequence number

        if (
        !tc.get_soak_temp(get<string>(args[i][7]), get<float>(args[i][0]))             // set soak temp
        || !tc.get_ramp_duration(get<string>(args[i][7]), get<int>(args[i][1]))          // set ramp duration
        || !tc.get_soak_duration(get<string>(args[i][7]), get<int>(args[i][2]))          // set soak duration
        || !tc.get_num_repeats(get<string>(args[i][7]), get<int>(args[i][3]))            // repeats
        || !tc.get_next_sequence_num(get<string>(args[i][7]), get<int>(args[i][4]))    // next sequence

        ) {
            cerr << "Sequence retrieval failed on sequence " << i << endl;
            return 1;
        }

    }



    // Parse RS_STATUS response
    RS = RS_STATUS & 0b1;
    SOAK = (RS_STATUS >> 1) & 0b1;
    RAMP = (RS_STATUS >> 2)& 0b1;

    // print info
    cout << endl;
    cout << "Off (0), On (1)" << endl;
    cout << "H-bridge Output: " << OUTPUT << endl;
    cout << "Mode (0-3): " << MODE << "\n" << endl;
    cout << "Current Temperature: " << CURR_TEMP << endl;
    cout << "Current Voltage: " << CURR_VOLTAGE << "\n" << endl;

    cout << "Set Temperature: " << SET_TEMP << endl;
    cout << "Set Voltage: " << SET_VOLTAGE << endl;
    cout << "Proportional Bandwidth: " << P << endl;
    cout << "Integral Gain: " << I << endl;
    cout << "Derivative Gain: " << D << "\n" << endl;

    cout << "Sequence Pointer: " << CURR_SEQ << endl;
    cout << "Ramp/Soak: " << RS << endl;
    cout << "Ramp: " << RAMP << endl;
    cout << "Soak: " << SOAK << endl;
    cout << "Ramp/soak method: " << RUN_METHOD << endl;
    cout << "Ramp/soak max deviation (C): " << MAX_DEVIATION << endl;
    cout << "Ramp/soak counter interval (s): " << COUNT_LEN * 0.2f << "\n" << endl;


    vector<string> row_labels = {"Soak temp", "Ramp duration", "Soak duration", "Remaining repeats", "Next sequence"};

    cout << setw(20) << "Sequence";

    // Print column headers
    for (int i = 0; i < args.size(); i++) {
        cout << setw(12) << i;
    }
    cout << endl;

    // Print each row with a row header ("Soak temp", "Ramp duration", etc.)
    for (int row = 0; row < 5; ++row) {
        cout << setw(20) << row_labels[row];
        for (int col = 0; col < args.size(); ++col) {
            cout << setw(12) << args[col][row];
        }
        cout << endl;
    }
    
    return 0;


}