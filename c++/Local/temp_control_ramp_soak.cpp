#include "Oven5R6900.h"
#include <variant>


using namespace std;
using generic_type = variant<int, float, string, bool>;


// for working with vectors of generic types
vector<generic_type> args = {string("0"), 20.0f, 300, 300, 5.0f, 1, 0, 0};
ostream& operator<<(ostream& os, const generic_type& v) {
    visit([&](const auto& val) {
        os << val;
    }, v);
    return os;
}


int main(int argc, char** argv) {
    // Initiate ramp soak procedure
    // Input: String com port, soak temp, ramp duration (s), soak duration (s), max deviation for procedure to continue, run_method, sequence number, next sequence number, # repeats
    // Output: Soak temp, ramp duration (s), soak duration (s), max deviation for procedure to continue, run_method, sequence number, next sequence number, # repeats
    // Returns: Errorcode, 0 = success, 1 = failure


    // check for correct number of command line arguments
    if (argc < 3 || argc > 10) {
        cerr << "Usage: <program> <COM port> <seq_num> <soak_temp> <ramp_dur> <soak_dur> <deviation> <method> <repeats> <next_seq>" << endl;
        return 1;
    } 
    
    // populate array with auto detecting types
    for (int i = 2; i < argc; i++) {

        try {
            visit([&](auto&& current_val) {
                using T = decay_t<decltype(current_val)>;

                if constexpr (is_same_v<T, int>) {
                    args[i - 2] = stoi(argv[i]);
                }
                else if constexpr (is_same_v<T, float>) {
                    args[i - 2] = stof(argv[i]);
                }
                else if constexpr (is_same_v<T, string>) {
                    args[i - 2] = string(argv[i]);
                }
            }, args[i - 2]);
        
        } catch (const exception& e) {
            cerr << "Error parsing argument " << i - 2
                    << " as " << args[i - 2].index() << ": " << e.what() << " (input was '" << argv[i] << "')\n";
            return 2;
        }
    }


    

    Oven5R6900 thermoelectric = Oven5R6900(argv[1]);

    int32_t rs_mode = 2;
    int off = 0;
    int on = 1;
    int count_length = 5;

    get<int>(args[7]) += 1;     // next sequence is 1-indexed

    // Check if in ramp soak mode
    if (!(thermoelectric.get_mode(rs_mode) && rs_mode == 2)) {
        cerr << "Module off or in mode " << rs_mode << " not ramp/soak mode (2)" << endl;
        return 1;
    }

    if (
        !thermoelectric.set_ramp_soak(off)                          // cancel previous ramp/soak procedures
        || !thermoelectric.set_soak_temp(get<string>(args[0]), get<float>(args[1]))             // set soak temp
        || !thermoelectric.set_ramp_duration(get<string>(args[0]), get<int>(args[2]))          // set ramp duration
        || !thermoelectric.set_soak_duration(get<string>(args[0]), get<int>(args[3]))          // set soak duration
        || !thermoelectric.set_max_deviation(get<float>(args[4]))                  // max deviation from calculated ramp/soak profile at any point in the process for procedure to continue
        || !thermoelectric.set_run_method(get<int>(args[5]))                    // set to 1 (wait for soak temp to be reached?)
        || !thermoelectric.set_next_sequence_num(get<string>(args[0]), get<int>(args[7]))    // next sequence
        || !thermoelectric.set_num_repeats(get<string>(args[0]), get<int>(args[6]))            // repeats
        || !thermoelectric.set_count_length(count_length)              // set to 5 (5*.2 = 1 sec)
        || !thermoelectric.set_ramp_soak(on)                           // start procedure
    ) {
        cerr << "Ramp Soak failed" << endl;
        return 1;
    }

    cout << "Sequence: " << args[0] << endl;
    cout << "Ramp to (C): " << args[1] << endl;
    cout << "Ramp for (s): " << args[2] << endl;
    cout << "Soak for (s): " << args[3] << endl;
    cout << "Tolerance (C): " << args[4] << endl;
    cout << "Method: " << args[5] << endl;
    cout << "Repeats: " << args[6] << endl;
    cout << "Next Sequence: " << args[7] << endl;
    return 0;
}