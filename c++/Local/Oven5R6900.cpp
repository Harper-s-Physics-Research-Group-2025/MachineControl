#include "Oven5R6900.h"


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


// convert strings of hex numbers to 
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

    response.resize(bytes_read);
    // cout << "Received: ";
    // print_bytes(response);
    return true;
}





// Temp
bool Oven5R6900::get_temp(float& temp) {
    // construct message
    string message = "*000100000000";
    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    temp = parse_response(response) / 100.0f;

    return true;
}


// Get mode
bool Oven5R6900::get_mode(int32_t& mode) {
    // construct message
    string message = "*005100000000";
    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    mode = parse_response(response);

    if (mode == 0) {
        cout << "Using fixed set temperature" << endl;
    } else {
        cout << "Unknown mode " << mode << " check manual." << endl;
    }

    return true;
}


//
bool Oven5R6900::get_setpoint(float& temp){

    // construct message
    string message = "*004000000000";
    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    temp = parse_response(response) / 100.0f;

    return true;
}


// get voltage
bool Oven5R6900::get_voltage(float& voltage) {

    // construct message
    string message = "*004600000000";
    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    voltage = parse_response(response) / 1000.0f;

    return true;
}


// Within the bandwidth the controller adjusts how much power is sent to the thermoelectric (the closer the thermoelectric is to the desired temperature, the less power it sends)
bool Oven5R6900::get_proportional_bandwidth(float& p){
    
    // construct message
    string message = "*004100000000";
    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    p = parse_response(response) / 100.0f;

    return true;
}




// Setters
bool Oven5R6900::set_mode(int32_t& mode){

    // construct message
    string message = "*0021";

    ostringstream oss;
    oss << setw(8) << setfill('0') << mode;
    message += oss.str();  // "00000123"  convert int mode to string

    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    mode = static_cast<int32_t>(parse_response(response));

    if (mode == 0) {
        cout << "Using fixed set temperature" << endl;
    } else {
        cout << "Unknown mode " << mode << " check manual." << endl;
    }

    return true;
}


//
bool Oven5R6900::set_setpoint(float& temp) {

    // construct message
    string message = "*0010";

    ostringstream oss;
    oss << hex << setw(8) << setfill('0') << static_cast<uint32_t>(temp * 100);
    message += oss.str();  // "00000123"  convert int mode to string

    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    temp = parse_response(response) / 100.0f;

    return true;
}


// set voltage. Voltage is set to the voltage indicated by the response from the temperature controller
bool Oven5R6900::set_voltage(float& voltage) {
    
    // construct message
    string message = "*0016";

    ostringstream oss;
    oss << hex << setw(8) << setfill('0') << static_cast<uint32_t>(voltage * 1000);
    message += oss.str();  // "00000123"  convert int mode to string

    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    voltage = parse_response(response) / 100.0f;

    return true;
}

bool Oven5R6900::set_proportional_bandwidth(float& p){

    // construct message
    string message = "*0011";

    ostringstream oss;
    oss << hex << setw(8) << setfill('0') << static_cast<uint32_t>(p * 100);
    message += oss.str();  // "00000123"  convert int mode to string

    message += checksum(message);
    message += '\r';        // terminating character
    vector<uint8_t> msg(message.begin(), message.end());
    vector<uint8_t> res(16);

    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    string response(res.begin(), res.end());

    p = parse_response(response) / 100.0f;

    return true;
}

