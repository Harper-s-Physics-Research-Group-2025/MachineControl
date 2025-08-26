#pragma once    // tells compiler to only #include once when included from other files
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>
#include <cctype>
#include <sstream>
#include <functional>




class Oven5R6900 {
public:
    explicit Oven5R6900(const std::string& port_name);
    ~Oven5R6900();

    bool is_connected() const;

    bool send_command(const std::vector<uint8_t>& message);
    bool read_response(std::vector<uint8_t>& response);

    bool enable(bool& state);        // enable/disable H-bridge output

    // getters
    bool get_state(bool& state);
    bool get_temp(float& out_temp);
    bool get_mode(int32_t& mode);
    bool get_setpoint(float& temp);
    bool get_voltage(float& voltage);
    bool get_max_voltage(float& voltage);
    bool get_proportional_bandwidth(float& p);
    bool get_integral_gain(float& i);
    bool get_derivative_gain(float& d);
    bool get_ramp_soak_status(int& status);
    bool get_ramp_soak_curr_seq(int& seq);

    bool get_soak_temp(const std::string& sequence, float& temp);
    bool get_ramp_duration(const std::string& sequence, int& counts);        // ramp duration in counts
    bool get_soak_duration(const std::string& sequence, int& counts);        // soak duration in counts (count duration determined by set_count_length)
    bool get_num_repeats(const std::string& sequence, int& repeats);         // number of times to run a ramp/soak sequence
    bool get_next_sequence_num(const std::string& sequence, int& next_sequence);  // next ramp/soak sequence to run if multiple are stored in memory
    bool get_run_method(int& method);             // Not sure (let mode = 1 for now)
    bool get_max_deviation(float& temp);
    bool get_count_length(int& periods);

    // setters
    bool set_mode(int32_t& mode);
    bool set_setpoint(float& temp);
    bool set_voltage(float& voltage);
    bool set_proportional_bandwidth(float& p);
    bool set_integral_gain(float& i);
    bool set_derivative_gain(float& d);
    
    bool set_ramp_soak(int& status);            // turn ramp/soak on/off. 0 = off, 1 = on.
    bool set_soak_temp(const std::string& sequence, float& temp);
    bool set_ramp_duration(const std::string& sequence, int& counts);        // ramp duration in counts
    bool set_soak_duration(const std::string& sequence, int& counts);        // soak duration in counts (count duration determined by set_count_length)
    bool set_num_repeats(const std::string& sequence, int& repeats);         // number of times to run a ramp/soak sequence
    bool set_next_sequence_num(const std::string& sequence, int& next_sequence);  // next ramp/soak sequence to run if multiple are stored in memory
    bool set_run_method(int& mode);             // Not sure (let mode = 1 for now)
    bool set_max_deviation(float& temp);        // maximum temperature deviation for the ramp soak procedure to continue
    bool set_count_length(int& periods);        // set base timer period as a multiple of .2 seconds



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
    bool dispatch_message(const std::string& command_id,
        float& value, 
        float scale_factor, 
        bool hex_format = true, 
        std::function<void(float)> log_callback = nullptr
    );

};