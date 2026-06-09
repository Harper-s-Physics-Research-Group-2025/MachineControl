#pragma once    // tells compiler to only #include once when included from other files
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>
#include <cctype>



class RTE7 {
public:
    explicit RTE7(const std::string& port_name);
    ~RTE7();

    bool is_connected() const;

    bool send_command(const std::vector<uint8_t>& message);
    bool read_response(std::vector<uint8_t>& response);

    bool turn_on();
    bool turn_off();

    // getters
    bool get_temp(float& out_temp);
    bool get_setpoint(float& temp);

    // setters
    bool set_setpoint(float temp);


private:
    HANDLE hSerial;

    bool initSerial(const std::string& port_name);
    uint8_t chillerChecksum(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> hexStringToBytes(const std::string& hex) const;
    void printBytes(const std::vector<uint8_t>& data, const std::string& label) const;

    bool read_temp(float& out_temp);
    bool read_setpoint(float& setpoint);
    bool read_acknowledge();

    float parse_float_response(std::vector<uint8_t>& response) const;

};
