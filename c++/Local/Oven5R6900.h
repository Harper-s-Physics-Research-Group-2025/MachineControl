#pragma once    // tells compiler to only #include once when included from other files
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>
#include <cctype>
#include <sstream>




class Oven5R6900 {
public:
    explicit Oven5R6900(const std::string& port_name);
    ~Oven5R6900();

    bool is_connected() const;

    bool send_command(const std::vector<uint8_t>& message);
    bool read_response(std::vector<uint8_t>& response);

    // bool turn_on();        // enable H-bridge output
    // bool turn_off();

    // getters
    bool get_temp(float& out_temp);
    bool get_mode(int32_t& mode);
    bool get_setpoint(float& temp);
    bool get_voltage(float& voltage);
    bool get_proportional_bandwidth(float& p);
    bool get_integral_gain(float& i);
    bool get_derivative_gain(float& i);

    // setters
    bool set_mode(int32_t& mode);
    bool set_setpoint(float& temp);
    bool set_voltage(float& voltage);
    bool set_proportional_bandwidth(float& p);
    bool set_integral_gain(float& i);
    bool set_derivative_gain(float& i);


private:
    HANDLE hSerial;

    bool initSerial(const std::string& port_name);
    std::string checksum(const std::string& message) const;
    std::vector<uint8_t> hexStringToBytes(const std::string& hex) const;
    void print_bytes(const std::vector<uint8_t>& data) const;

    bool read_temp(float& out_temp);
    bool read_setpoint(float& setpoint);
    bool read_ack();

    int32_t parse_response(std::string& response) const;

};