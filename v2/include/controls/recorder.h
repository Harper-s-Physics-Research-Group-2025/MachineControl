#pragma once
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>

#include "pubSysCls.h"
#include "Oven5R6900.h"
#include "RTE7.h"
#include "LabJackUD.h"   // Header for UD library
#pragma comment(lib, "LabJackUD.lib")  // Link UD lib (Windows)

using namespace std;
using namespace sFnd;

class Recorder {
public: // Crucial: make methods accessible to your API wrapper

    int record(std::string CSV_FILENAME, std::string BATH_PORT, std::string TEMERATURE_PORT) const;

    bool init_teknic_motors(SysManager* Mgr, INode*& X, INode*& Z) const;

    string currentTimestamp() const;

    bool init_labjack(LJ_HANDLE& h) const;

    double poll_labjack(LJ_HANDLE h) const;
};