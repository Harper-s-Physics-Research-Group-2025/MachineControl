#include "../include/controls/lab.h"
#include <iostream>


int main() {
    double voltage;
    Lab::read_labjack_ain(0, voltage);
    std::cout << voltage << std::endl;
    return 0;
}