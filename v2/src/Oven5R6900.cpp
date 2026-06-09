#include "controls/Oven5R6900.h"


using namespace std;


// explicit constructor
Oven5R6900::Oven5R6900(const string& port_name) {
    hSerial = INVALID_HANDLE_VALUE;
    if (!initSerial(port_name)) {
        cerr << "Failed to open serial port." << endl;
    }
}


// Destructor
Oven5R6900::~Oven5R6900() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
    }
}


// Check connection
bool Oven5R6900::is_connected() const {
    return hSerial != INVALID_HANDLE_VALUE;
}


// Initialize Serial Connection
bool Oven5R6900::initSerial(const string& port_name_raw) {
    string port_name = "\\\\.\\" + port_name_raw;

    hSerial = CreateFileA(  // treat serial connection as file
        port_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hSerial == INVALID_HANDLE_VALUE) return false;  // validity checks

    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(hSerial, &dcb)) return false;

    dcb.BaudRate = CBR_19200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;
    // dcb.fDtrControl = DTR_CONTROL_ENABLE;

    // Optional control settings
    dcb.fDtrControl = DTR_CONTROL_DISABLE;   // not needed unless explicitly required
    dcb.fRtsControl = RTS_CONTROL_DISABLE;   // disable hardware flow control
    dcb.fOutxCtsFlow = FALSE;                // no CTS flow control
    dcb.fOutxDsrFlow = FALSE;                // no DSR flow control
    dcb.fOutX = FALSE;                       // no software flow control
    dcb.fInX = FALSE;

    if (!SetCommState(hSerial, &dcb)) return false;

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) return false;

    return true;
}


// convert strings of hex numbers to an integer of the same number
vector<uint8_t> Oven5R6900::hexStringToBytes(const string& hex) const {
    string cleaned;
    for (char c : hex) {
        if (!isspace(static_cast<unsigned char>(c))) {
            cleaned += c;
        }
    }

    vector<uint8_t> bytes;
    for (size_t i = 0; i < cleaned.length(); i += 2) {
        string byte_str = cleaned.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(stoi(byte_str, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}


// print hex bytes
void Oven5R6900::print_bytes(const vector<uint8_t>& data) const {
    for (uint8_t b : data) {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(b) << " ";
    }
    cout << dec << endl;
}


// Calculate checksum
string Oven5R6900::checksum(const string& message) const {

    // Sum address, command, and data blocks (exclude start and end characters) 
    // assume end char not included
    uint32_t sum = 0;
    for (size_t i = 1; i < message.size(); ++i)
        sum += static_cast<uint8_t>(message[i]);
    uint8_t lsb = sum & 0xFF;

    // convert to ascii
    string ascii = "0123456789abcdef";
    string chk;
    chk += ascii[(lsb >> 4) & 0x0F];
    chk += ascii[lsb & 0x0F];
    
    return chk;        // return string (list of two chars (bytes))
}


// send
bool Oven5R6900::send_command(const vector<uint8_t>& message) {
    if (hSerial == INVALID_HANDLE_VALUE) return false;

    DWORD bytes_written;
    if (!WriteFile(hSerial, message.data(), message.size(), &bytes_written, nullptr)) {
        cerr << "Write failed." << endl;
        return false;
    }

    if (bytes_written != message.size()) {
        cerr << "Partial write." << endl;
        return false;
    }

    // cout << "Sent: ";
    // print_bytes(message);
    return true;
}


// Read
bool Oven5R6900::read_response(vector<uint8_t>& response) {

    DWORD bytes_read = 0;

    if (!ReadFile(hSerial, response.data(), response.size(), &bytes_read, nullptr)) {
        cerr << "Read failed." << endl;
        return false;
    }

    if (bytes_read == 0) {
        cerr << "No response received." << endl;
        return false;
    }
    
    response.resize(bytes_read);
    // cout << "Received: ";
    // print_bytes(response);
    return true;
}


// process responses and extract float values from get requests
int32_t Oven5R6900::parse_response(string& response) const {

    int32_t data;

    // validate checksum
    string res_check = response.substr(response.size()-3, 2); // last two bytes of response before terminating char
    response = response.substr(0, response.size()-3);                 // slice string to recompute checksum
    string expected_check = checksum(response);

    if (res_check != expected_check) return -999;    // checksum failed


    // extract parameters
    response = response.substr(1, 8); // erase first byte '*'
    

    // Convert 2's complement hex to signed int
    data = static_cast<int32_t>(stoul(response, nullptr, 16));    // stoul() converts ascii to bytes, then cast to int which is stored as 2's C

    return data;
}


bool Oven5R6900::dispatch_message(
    const string& command_id,
    float& value,
    float scale_factor,
    bool hex_format,
    function<void(float)> log_callback) {
    
    // Build message
    // Format value
    uint32_t scaled_val = static_cast<uint32_t>(value * scale_factor);
    
    ostringstream oss;
    if (hex_format) oss << hex;
    oss << setw(8) << setfill('0') << scaled_val;

    string message = "*00" + command_id + oss.str();
    message += checksum(message);
    message += '\r';

    // convert string to list of bytes
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);
    
    // send and receive
    if (!send_command(msg) || !read_response(res)) return false;

    string response(res.begin(), res.end());
    value = parse_response(response) / scale_factor;

    if (log_callback) {
        log_callback(value);
    }

    return true;
}









// Turn controller output on (1) and off (0)
bool Oven5R6900::enable(bool& state) {
    float s = static_cast<float>(state);
    return dispatch_message("1d", s, 1, true,
        [&](float result) {
            state = static_cast<bool>(result);
        }
    );
}


// read state of controller output
bool Oven5R6900::get_state(bool& state) {
    float s = static_cast<float>(state);
    return dispatch_message("4d", s, 1, true,
        [&](float result) {
            state = static_cast<bool>(result);
        }
    );
}

// Temp
bool Oven5R6900::get_temp(float& temp) {
    temp = 0;
    return dispatch_message("01", temp, 100, true);
}


// Get mode
bool Oven5R6900::get_mode(int32_t& mode) {
    float m = 0;
    return dispatch_message("51", 
        m, 
        1, 
        true,
        [&](float result) {         // callback to convert mode to int
            mode = static_cast<int32_t>(result);
        }
    );
    
}


// get fixed temperature setpoint
bool Oven5R6900::get_setpoint(float& temp){
    temp = 0;
    return dispatch_message("40", temp, 100, true);
}


// get voltage
bool Oven5R6900::get_voltage(float& voltage) {
    voltage = 0;
    return dispatch_message("02", voltage, 1000, true);
}

// get voltage
bool Oven5R6900::get_max_voltage(float& voltage) {
    voltage = 0;
    return dispatch_message("46", voltage, 1000, true);
}


// Within the bandwidth the controller adjusts how much power is sent to the thermoelectric (the closer the thermoelectric is to the desired temperature, the less power it sends)
bool Oven5R6900::get_proportional_bandwidth(float& p){
    p = 0;
    return dispatch_message("41", p, 100, true);
}


// integral gain
bool Oven5R6900::get_integral_gain(float& i){
    i = 0;
    return dispatch_message("42", i, 100, true);
}


// derivative gain
bool Oven5R6900::get_derivative_gain(float& d){
    d = 0;
    return dispatch_message("43", d, 100, true);
}


// get ramp soak sequence status
bool Oven5R6900::get_ramp_soak_status(int& status) {
    float mode = 0;
    return dispatch_message("e9", 
        mode, 
        1, 
        true,
        [&](float result) {
            status = static_cast<int32_t>(result);      // back to int
        }
    );
}


// get ramp soak current sequence
bool Oven5R6900::get_ramp_soak_curr_seq(int& seq) {
    float sequence = 0;
    return dispatch_message("ea",
        sequence,
        1,
        true,
        [&](float result) {
            seq = static_cast<int32_t>(result);
        }
    );
}


// Get soak temperature
bool Oven5R6900::get_soak_temp(const std::string& sequence, float& temp) {
    int seq = stoi(sequence);
    stringstream ss;
    ss << hex << nouppercase << (8 + seq);
    temp = 0;
    return dispatch_message("8" + ss.str(), temp, 100, true);
}


// Get ramp duration in counts
bool Oven5R6900::get_ramp_duration(const std::string& sequence, int& counts) {
    int seq = stoi(sequence);
    stringstream ss;
    ss << hex << nouppercase << (8 + seq);
    float placeholder = 0;
    return dispatch_message("9" + ss.str(), placeholder, 1, true,
        [&] (float result) {
            counts = static_cast<int>(result);
        }
    );
}


// Get soak duration in counts
bool Oven5R6900::get_soak_duration(const std::string& sequence, int& counts) {
    int seq = stoi(sequence);
    stringstream ss;
    ss << hex << nouppercase << (8 + seq);
    float placeholder = 0;
    return dispatch_message("a" + ss.str(), placeholder, 1, true,
        [&] (float result) {
            counts = static_cast<int>(result);
        }
    );
}


// Get number of repeats
bool Oven5R6900::get_num_repeats(const std::string& sequence, int& repeats) {
    int seq = stoi(sequence);
    stringstream ss;
    ss << hex << nouppercase << (8 + seq);
    float r = 0;
    return dispatch_message("b" + ss.str(), r, 1, true,
        [&] (float result) {
            repeats = static_cast<int>(result);
        }
    );
}


// Get next sequence number
bool Oven5R6900::get_next_sequence_num(const std::string& sequence, int& next_sequence) {
    int seq = stoi(sequence);
    stringstream ss;
    ss << hex << nouppercase << (8 + seq);
    float placeholder = 0.0f;
    return dispatch_message("c" + ss.str(), placeholder, 1, true,
        [&] (float result) {
            next_sequence = static_cast<int>(result);
        }
    );
}


// Get run method
bool Oven5R6900::get_run_method(int& method) {
    float value = 0.0f;
    return dispatch_message("d1", value, 1, true,
        [&] (float result) {
            method = static_cast<int>(result);
        }
    );
}


// Get max deviation
bool Oven5R6900::get_max_deviation(float& degrees) {
    degrees = 0;
    return dispatch_message("d3", degrees, 100, true);
}


// set base timer period as a multiple of .2 seconds
bool Oven5R6900::get_count_length(int& periods) {
    float placeholder = 0;
    return dispatch_message("d5", placeholder, 1, true,
        [&] (float result) {
            periods = static_cast<int>(result);
        }
    );
}       












// Setters
bool Oven5R6900::set_mode(int32_t& mode){
    float m = static_cast<float>(mode);
    return dispatch_message("21", 
        m, 
        1, 
        true,
        [&](float result) {         // callback to convert mode to int
            mode = static_cast<int32_t>(result);
        }
    );

}


// temperature for "fixed set temperature mode"
bool Oven5R6900::set_setpoint(float& temp) {
    return dispatch_message("10", temp, 100, true);
}


// set voltage. Response from the temperature controller indicates new set voltage.
bool Oven5R6900::set_voltage(float& voltage) {
    return dispatch_message("16", voltage, 1000, true);
}


// adjust how responsive the controller is (too reponsive --> temperature oscillations)
bool Oven5R6900::set_proportional_bandwidth(float& p){
    return dispatch_message("11", p, 100, true);
}


// integral gain
bool Oven5R6900::set_integral_gain(float& i){
    return dispatch_message("12", i, 100, true);
}


// derivative gain
bool Oven5R6900::set_derivative_gain(float& i){
    return dispatch_message("13", i, 100, true);
}


// start/stop ramp soak sequence. 0 = stop, 1 = start
bool Oven5R6900::set_ramp_soak(int& status) {
    float mode = static_cast<float>(status);        // to float
    return dispatch_message("e8", 
        mode, 
        1, 
        true,
        [&](float result) {
            status = static_cast<int32_t>(result);      // back to int
        }
    );
}


// set soak temperature
bool Oven5R6900::set_soak_temp(const std::string& sequence, float& temp) {
    return dispatch_message("8" + sequence, temp, 100, true);
}


// ramp duration in counts
bool Oven5R6900::set_ramp_duration(const std::string& sequence, int& counts){
    float c = static_cast<float>(counts);
    return dispatch_message("9" + sequence, c, 1, true,
        [&](float result) {
            counts = static_cast<int>(result);      // back to int
        }
    );
}        


// soak duration in counts (count duration determined by set_count_length)
bool Oven5R6900::set_soak_duration(const std::string& sequence, int& counts) {
    float c = static_cast<float>(counts);
    return dispatch_message("a" + sequence, c, 1, true,
        [&](float result) {
            counts = static_cast<int>(result);      // back to int
        }
    );
}


// number of times to run a ramp/soak sequence
bool Oven5R6900::set_num_repeats(const std::string& sequence, int& repeats) {
    float num_repeats = static_cast<float>(repeats);
    return dispatch_message("b" + sequence, num_repeats, 1, true,
        [&](float result) {
            repeats = static_cast<int>(result);      // back to int
        }
    );
}    


// next ramp/soak sequence to run if multiple are stored in memory
bool Oven5R6900::set_next_sequence_num(const std::string& sequence, int& next_sequence) {
    float next_seq = static_cast<float>(next_sequence);
    return dispatch_message("c" + sequence, next_seq, 1, true,
        [&](float result) {
            next_sequence = static_cast<int>(result);      // back to int
        }
    );
}


// I think this changes the behavior of the ramp soak timer
bool Oven5R6900::set_run_method(int& mode) {
    float m = static_cast<float>(mode);
    return dispatch_message("d0", m, 1, true,
        [&](float result) {
            mode = static_cast<int>(result);      // back to int
        }
    );
}             


// maximum temperature deviation for the ramp soak procedure to continue
bool Oven5R6900::set_max_deviation(float& temp) {
    return dispatch_message("d2", temp, 100, true);
}


// set base timer period as a multiple of .2 seconds
bool Oven5R6900::set_count_length(int& periods) {
    float p = static_cast<float>(periods);
    return dispatch_message("d4", p, 1, true,
        [&](float result) {
            periods = static_cast<int>(result);      // back to int
        }
    );
}        