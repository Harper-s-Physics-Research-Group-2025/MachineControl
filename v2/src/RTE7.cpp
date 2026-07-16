#include "controls/RTE7.h"


using namespace std;



RTE7::RTE7(const string& port_name) {
    hSerial = INVALID_HANDLE_VALUE;
    if (!initSerial(port_name)) {
        cerr << "Failed to open serial port." << endl;
    }
}

RTE7::~RTE7() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
    }
}

bool RTE7::is_connected() const {
    return hSerial != INVALID_HANDLE_VALUE;
}

bool RTE7::initSerial(const string& port_name_raw) {
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
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

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

bool RTE7::send_command(const vector<uint8_t>& message) {
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

    // printBytes(message, "Sent");
    return true;
}

bool RTE7::read_response(vector<uint8_t>& response) {

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
    // printBytes(response, "Received");
    return true;
}

uint8_t RTE7::chillerChecksum(const vector<uint8_t>& data) const {
    uint32_t sum = 0;
    for (size_t i = 1; i < data.size(); ++i)
        sum += data[i];
    uint8_t lsb = sum & 0xFF;
    return lsb ^ 0xFF;
}

vector<uint8_t> RTE7::hexStringToBytes(const string& hex) const {
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

void RTE7::printBytes(const vector<uint8_t>& data, const string& label) const {
    cout << label << ": ";
    for (uint8_t b : data) {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(b) << " ";
    }
    cout << dec << endl;
}

// process responses and extract float values from get requests
float RTE7::parse_float_response(vector<uint8_t>& response) const {

    float out_temp;

    // validate checksum
    uint8_t res_check = response.back(); // last byte of response
    response.pop_back();                 // remove checksum to recompute for data
    uint8_t expected_check = chillerChecksum(response); 
    if (res_check != expected_check) return -999;    // checksum failed

    // extract parameters
    response.erase(response.begin()); // erase first byte "CA"
    uint8_t command = response[2];
    uint8_t data_len = response[3];
    uint8_t qualifier = response[4];

    float precision = pow(10, ((qualifier & 0xF0) >> 4));

    // extract data
    vector<uint8_t> data(response.begin()+5, response.end());
    uint16_t temp_raw = 0;
    for (int i = 0; i < data.size(); i++) {   // convert
        temp_raw = (temp_raw << 8) | data[i];     // shift two bytes and OR with next two bytes of byte list
    }
    
    out_temp = temp_raw / precision;
    return out_temp;
}


bool RTE7::turn_on() {
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0x81, 0x08, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x66};
    vector<uint8_t> res(16);
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive
    return true;
}


// turn bath off
bool RTE7::turn_off() {
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0x81, 0x08, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x67};
    vector<uint8_t> res(16);
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive
    return true;
}


// revert to manual control
bool RTE7::manual() {
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0x81, 0x08, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x67};
    vector<uint8_t> res(16);
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive
    return true;
}


// Send Read Internal Temperature command (CA 00 01 20 00 DE)
bool RTE7::read_temp(float& out_temp) {
    
    // construct message
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0x20, 0x00, 0xDE};
    vector<uint8_t> res(16);
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    out_temp = parse_float_response(res);

    return out_temp != -999;   // parse_float_response signals a checksum failure with -999
}

// Send Read Internal Temperature command (CA 00 01 70 00 8E)
bool RTE7::read_setpoint(float& out_temp) {

    // construct message
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0x70, 0x00, 0x8E};
    vector<uint8_t> res(16);
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive

    out_temp = parse_float_response(res);

    return out_temp != -999;   // parse_float_response signals a checksum failure with -999

}

// not sure if necessary
bool RTE7::read_acknowledge() {
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0x00, 0x00, 0xFE};
    vector<uint8_t> res(16);
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive
    return true;
}


bool RTE7::get_temp(float& out_temp) {

    if (!read_temp(out_temp)) {
        cerr << "Read Temp Failed" << endl;
        return false;
    };

    if(!read_acknowledge()) {
        cerr << "Read Acknowedge Failed" << endl;
        return false;
    }

    return true;
}



bool RTE7::get_setpoint(float& temp) {

    if (!read_setpoint(temp)) {
        cerr << "Read Setpoint Failed" << endl;
        return false;
    };

    if(!read_acknowledge()) {
        cerr << "Read Acknowedge Failed" << endl;
        return false;
    }

    return true;
}


bool RTE7::set_setpoint(const float& temp) {
    vector<uint8_t> msg = {0xCA, 0x00, 0x01, 0xF0, 0x02};
    vector<uint8_t> res(16);

    uint16_t temp_in = (int)(temp * 10);  // cast to int
    msg.push_back((temp_in >> 8) & 0xFF);
    msg.push_back(temp_in & 0xFF);
    msg.push_back(chillerChecksum(msg));
    if (!send_command(msg)) return false;    // send
    if (!read_response(res)) return false;   // receive
    return true;
}