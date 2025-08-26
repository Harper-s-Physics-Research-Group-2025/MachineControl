#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;


// function declarations
HANDLE init_serial(string &port_name);                         // initialize serial port
bool write(HANDLE hSerial, vector<uint8_t>& message);          // read and write to the port
bool read(HANDLE hSerial, vector<uint8_t>& response);
vector<uint8_t> hex_string_to_bytes(const string& hex);        // convert str to numerical format
uint8_t chiller_checksum(const vector<uint8_t>& data);         // compute checksum for device
void print_bytes(const vector<uint8_t>& data, const string& label);    // printing stuff



int main() {
    string port_name = "COM3";
    string hex_input;

    cout << "Enter hex message (e.g. CA0001810801...): ";
    cin >> hex_input;

    vector<uint8_t> message = hex_string_to_bytes(hex_input);  // convert to bytes
    message.push_back(chiller_checksum(message));  // Compute and append checksum

    print_bytes(message, "Sending");

    HANDLE hSerial = init_serial(port_name);    // create serial connection

    if (hSerial == INVALID_HANDLE_VALUE) {    // check connection
        cout << "Initialization of serial connection failed." << endl;
        return 1;
    }

    if (!write(hSerial, message)) {    // write message
        CloseHandle(hSerial);
        return 1;
    }

    // Read response
    vector<uint8_t> response(12);

    if (!read(hSerial, response)) {
        cout << "no response from serial connection." << endl;
    }
    print_bytes(response, "Received");

    CloseHandle(hSerial);    // Close
    return 0;
}





// Initialize a serial connection
HANDLE init_serial(string &port_name) {
    port_name = "\\\\.\\" + port_name;   // full port name

    HANDLE hSerial_connection = CreateFileA(    // treat serial connection as a file
        port_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hSerial_connection == INVALID_HANDLE_VALUE) {
        cerr << "Error opening port." << endl;
        return INVALID_HANDLE_VALUE;
    }

    // Configure port
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hSerial_connection, &dcb)) {
        cerr << "Failed to get serial state." << endl;
        CloseHandle(hSerial_connection);
        return INVALID_HANDLE_VALUE;
    }

    dcb.BaudRate = CBR_19200;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(hSerial_connection, &dcb)) {
        cerr << "Failed to set serial state." << endl;
        CloseHandle(hSerial_connection);
        return INVALID_HANDLE_VALUE;
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial_connection, &timeouts)) {
        cerr << "Failed to set timeouts." << endl;
        CloseHandle(hSerial_connection);
        return INVALID_HANDLE_VALUE;
    };

    return hSerial_connection;
}


bool write(HANDLE hSerial, vector<uint8_t>& message) {
    // Write message to serial port

    DWORD bytes_written;
    if (!WriteFile(hSerial, message.data(), message.size(), &bytes_written, nullptr)) {    // try write
        cerr << "Failed to write to serial port." << endl;
        return false;
    }

    if (bytes_written != message.size()) {    // check if entire message sent
        cerr << "Partial write: only " << bytes_written << " of "
                  << message.size() << " bytes sent." << endl;
        return false;
    }

    return true;
}


bool read(HANDLE hSerial, vector<uint8_t>& response) {
    // Read message from serial port

    DWORD bytes_read;
    if (!ReadFile(hSerial, response.data(), response.size(), &bytes_read, nullptr)) {    // try read
        cerr << "Failed to read from serial port." << endl;
        return false;
    } else {
        response.resize(bytes_read);  // Trim to actual bytes read
    }

    return true;
}


// Convert hex string to byte vector
vector<uint8_t> hex_string_to_bytes(const string& hex) {

    // Remove spaces
    string cleaned;   // doesn't work yet
    for (int i = 0; i < hex.length(); i++) {
        if (hex[i] != ' ') {
            cleaned += hex[i];
        }
    }

    cout << cleaned << endl;

    vector<uint8_t> bytes;
    for (size_t i = 0; i < cleaned.length(); i += 2) {
        string byte_str = cleaned.substr(i, 2);     // substring of two hex numbers
        uint8_t byte = static_cast<uint8_t>(stoi(byte_str, nullptr, 16));       // convert ascii to int_32, then cast to uint_8 which is just a byte
        bytes.push_back(byte);
    }
    return bytes;
}


// Compute checksum for NESLAB RTE7: bitwise invert of LSB of sum
uint8_t chiller_checksum(const vector<uint8_t>& data) {
    uint32_t sum = 0;
    for (size_t i = 1; i < data.size(); ++i)
        sum += data[i];
    uint8_t lsb = sum & 0xFF;
    return lsb ^ 0xFF;
}


// Print byte vector
void print_bytes(const vector<uint8_t>& data, const string& label) {
    cout << label << ": ";
    for (uint8_t b : data)
        cout << hex << setw(2) << setfill('0') << (int)b << " ";
    cout << dec << endl;
}

// bool get_temp_chiller(HANDLE* hSerial_ptr, float* temp_ptr) {

//     // write();
//     // read();
//     // process();

//     // return 1;
// }

// bool set_temp_chiller(HANDLE* hSerial_ptr, float* temp_ptr) {
//     // write();
//     // read();
//     // process();

//     // return 1;
// }
