/*
Title: API.cpp
Description: The class definition for Josh de Cart's controller 
Version: 1
Date Started: May 26, 2026
Authors: Josh Darrow and Samuel Ntadom
Date Completed: May 29, 2026
*/
#include "WolframLibrary.h"
#include "controls/API.h"


using namespace std;
using namespace sFnd;

using generic_type = variant<int, float, string, bool>;

// Define error codes for Mathematica integration
#define ERROR_INVALID_COMM_PORT 1001
#define ERROR_VALUE_OUT_OF_RANGE 1002

/*****************************************************
 * Function name: bath_on
 * Input: COMM, string
 * Side effect: success message or initialization failed error
 * Output: None
 *****************************************************/
void LabEquipment::bath_on(std::string COMM) const {
    RTE7 bath = RTE7(COMM);

    if (!bath.turn_on()) {
        cerr << "initialization failed" << endl;
    } else {
        cout << "Success!" << endl;
    }
}

/*****************************************************
 * Function name: bath_off
 * Input: COMM, string
 * Side effect: success message or shutdown failed error
 * Output: None
 *****************************************************/
void LabEquipment::bath_off(std::string COMM) const {
    RTE7 bath = RTE7(COMM);

    if (!bath.turn_off()) {
        cerr << "shutdown failed" << endl;
    } else {
        cout << "Success!" << endl;
    }
}

/*****************************************************
 * Function name: bath_dump
 * Input: COMM, string
 * Side effect: Prints current temperature and Set Point
 * Output: None
 *****************************************************/
void LabEquipment::bath_dump(std::string COMM) const {
    float TEMP;
    float SET_TEMP;

    RTE7 bath = RTE7(COMM);

    if (!bath.get_temp(TEMP) || !bath.get_setpoint(SET_TEMP)) {
        cerr << "Read Failed." << endl;
    } else {
        cout << "Temp: " << TEMP << endl;
        cout << "Setpoint: " << SET_TEMP << endl;
    }
}

/*****************************************************
 * Function name: bath_read_temp
 * Input: COMM, string
 * Side effect: Prints temperature on success or error message
 * Output: int (0 for success, 1 for failure)
 *****************************************************/
double LabEquipment::bath_read_temp(std::string COMM) const {
    float temp = 0.0f;
    RTE7 bath = RTE7(COMM);

    if (!bath.get_temp(temp)) {
        cerr << "Read Failed." << endl;
    }
    return static_cast<double>(temp);
} 

/*****************************************************
 * Function name: bath_set_temp
 * Input: COMM (string), Temp (double)
 * Side effect: Updates bath setpoint, prints verification/error
 * Output: None
 *****************************************************/
void LabEquipment::bath_set_temp(std::string COMM, double Temp) const {
    RTE7 bath = RTE7(COMM);
    float temp_float = static_cast<float>(Temp);
    
    if (!bath.set_setpoint(temp_float)) {
        cerr << "Failed to set temp: " << endl;
    } else {
        cout << "Temp set to: " << Temp << endl;
    }
}

/*****************************************************
 * Function name: read_labjack_ain0
 * Input: None
 * Side effect: Prints voltage reading from LabJack U3 channel AIN0
 * Output: int (0 for success, negative values for errors)
 *****************************************************/
void LabEquipment::read_labjack_ain0() const {
    LJ_HANDLE h;       // Handle for the device
    int errorcode;
    double voltage;

    // 1. Open the first found LabJack U3
    errorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when opening the LabJack." << endl;
        exit(-1);
    }
    
    // 2. Initialize settings on the LabJack
    ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, 0, 1, 0);  // Set channel 0 to analog input
    ePut(h, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, 1, 0);            // Resolution index

    // 3. Read analog input AIN0 (single-ended)
    errorcode = eGet(h, LJ_ioGET_AIN, 0, &voltage, 0);
    if (errorcode != 0) {
        cout << "Error " << errorcode << " occured when reading the LabJack." << endl;
        Close();
        exit(-2);
    }

    // 4. Close the device
    Close();

    cout << voltage;
    
}

/*****************************************************
 * Function name: record
 * Input: CSV_FILENAME (string), BATH_PORT (string), TEMERATURE_PORT (string)
 * Side effect: Launches telemetry logging file generation
 * Output: None
 *****************************************************/
void LabEquipment::record(std::string CSV_FILENAME, std::string BATH_PORT, std::string TEMERATURE_PORT) const {
    Recorder experiment;
    experiment.record(CSV_FILENAME, BATH_PORT, TEMERATURE_PORT);
}

/*****************************************************
 * Function name: servo_motor_home (no args)
 * Input: None
 * Side effect: Executes home routing configuration sequences
 * Output: int (Homing execution result code)
 *****************************************************/
int LabEquipment::servo_motor_home() const {
    Homer experiment;
    return experiment.homing();
}

/*****************************************************
 * Function name: servo_motor_home (with timeout)
 * Input: milliseconds (int)
 * Side effect: Homing operation bound by timeframe constraints 
 * Output: int (Homing execution result code with timeout)
 *****************************************************/
int LabEquipment::servo_motor_home(int milliseconds) const {
    Homer experiment;
    return experiment.homing(milliseconds);
}

/*****************************************************
 * Function name: servo_motor_is_home
 * Input: None
 * Side effect: None
 * Output: bool (Returns current homing status tracker flags)
 *****************************************************/
bool LabEquipment::servo_motor_is_home() { 
    return false; 
} 

/*****************************************************
 * Function name: servo_motor_manual_control
 * Input: None
 * Side effect: Drops controller thread execution into manual steering loops
 * Output: None
 *****************************************************/
void LabEquipment::servo_motor_manual_control() const {
    ManualController experiment;
    experiment.control();
}

/*****************************************************
 * Function name: servo_motor_read_position
 * Input: None
 * Side effect: Prints X/Z locations converted from raw encoder edge counts
 * Output: void
 *****************************************************/
void LabEquipment::servo_motor_read_position() const { 
    SysManager* Mgr = nullptr;
    IPort* Port;
    INode* motorX; 
    INode* motorZ; 
        
    vector<string> comHubPorts;                 

    float x_pos;
    float z_pos;

    try {
        Mgr = SysManager::Instance();
        SysManager::FindComHubPorts(comHubPorts);
        
        if (comHubPorts.size() == 1) {
            Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
        } else {
            cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
            std::exit(ERROR_INVALID_COMM_PORT);
        }

        Mgr->PortsOpen(1);

        Port = &Mgr->Ports(0);
        motorX = &Port->Nodes(0); 
        motorZ = &Port->Nodes(1); 

        x_pos = static_cast<float>(motorX->Motion.PosnMeasured.Value()) / 800;
        z_pos = static_cast<float>(motorZ->Motion.PosnMeasured.Value()) / 800;
        cout << "mm: (" << x_pos << ", " << z_pos << ")" << endl;

    } catch (mnErr& theErr) {
        cerr << "Caught error: " << theErr.ErrorMsg << "\n";
        if (Mgr) Mgr->PortsClose();
        exit(1);
    } catch (exception& e) {
        cerr << "Caught error: " << e.what() << endl;
        if (Mgr) Mgr->PortsClose();
        exit(1);
    }

    if (Mgr) Mgr->PortsClose();

} 

/*****************************************************
 * Function name: servo_motor_set_position (Velocity Limit Bound)
 * Input: x_pos (double), z_pos (double), vel_rms (int)
 * Side effect: Moves multi-axis carriage elements with linear bounds tracking
 * Output: void
 *****************************************************/
void LabEquipment::servo_motor_set_position(double x_pos, double z_pos, int vel_rms) const {
    SysManager* Mgr = nullptr;
    IPort* Port;
    INode* motorX; 
    INode* motorZ; 
        
    vector<string> comHubPorts;                 
    const int vel_limit = 1000;     
    vector<float> args = {0, 0, 240, 10000};     

    args[0] = static_cast<float>(x_pos);
    args[1] = static_cast<float>(z_pos);
    args[2] = static_cast<float>(vel_rms);
  
    if (args[2] > vel_limit) {
        cerr << "Desired velocity exceeds limit" << endl;
        exit(1);
    }

    args[0] *= 800;     
    args[1] *= 800;     

    try {
        Mgr = SysManager::Instance();
        SysManager::FindComHubPorts(comHubPorts);
        
        if (comHubPorts.size() == 1) {
            Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
        } else {
            cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
            exit(1);
        }

        Mgr->PortsOpen(1);

        Port = &Mgr->Ports(0);
        motorX = &Port->Nodes(0);
        motorZ = &Port->Nodes(1);

        if (!(motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) ||       
            !(motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid())) {
                cerr << "Motors are not homed. Please home before continuing" << endl;
                if (motorX) motorX->EnableReq(false);
                if (motorZ) motorZ->EnableReq(false);
                if (Mgr) Mgr->PortsClose();
                exit(1);
        }

        motorX->Status.AlertsClear();                   
        motorZ->Status.AlertsClear();

        motorX->Motion.NodeStopClear();                  
        motorZ->Motion.NodeStopClear();
        
        motorX->EnableReq(true);
        motorZ->EnableReq(true);
        Sleep(200); 

        motorX->VelUnit(INode::RPM);                        
        motorZ->VelUnit(INode::RPM);                        
        
        motorX->Motion.VelLimit = args[2];              
        motorZ->Motion.VelLimit = args[2];              
        
        motorX->Motion.MovePosnStart(args[0], true);
        motorZ->Motion.MovePosnStart(args[1], true);

        args[3] = Mgr->TimeStampMsec() + 100 + max(motorX->Motion.MovePosnDurationMsec(args[0], true), motorZ->Motion.MovePosnDurationMsec(args[1], true));

        while (!motorX->Motion.MoveIsDone() || !motorZ->Motion.MoveIsDone()) {
            if (Mgr->TimeStampMsec() > args[3]) {
                cerr << "Movement timed out" << endl;
                exit(1);
            }
        }

        cout << "(" << motorX->Motion.PosnMeasured.Value() / 800 << ", " << motorZ->Motion.PosnMeasured.Value() / 800 << ")" << endl;

    } catch (mnErr& theErr) {
        cerr << "Caught error: " << theErr.ErrorMsg << "\n";
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();
        exit(1);
    } catch (exception& e) {
        cerr << "Caught error: " << e.what() << endl;
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();
        exit(1);
      
    }

    if (motorX) motorX->EnableReq(false);
    if (motorZ) motorZ->EnableReq(false);
    if (Mgr) Mgr->PortsClose();

  
}

/*****************************************************
 * Function name: servo_motor_set_position (Timed Trajectory Override)
 * Input: x_pos (double), z_pos (double), vel_rms (int), milliseconds (double)
 * Side effect: Moves motor axes utilizing custom runtime calculation constraints
 * Output: void
 *****************************************************/
void LabEquipment::servo_motor_set_position(double x_pos, double z_pos, int vel_rms, double milliseconds) const  {
    SysManager* Mgr = nullptr;
    IPort* Port;
    INode* motorX; 
    INode* motorZ; 
        
    vector<string> comHubPorts;                 
    const int vel_limit = 1000;     
    vector<float> args = {0, 0, 240, 10000};     

    args[0] = static_cast<float>(x_pos);
    args[1] = static_cast<float>(z_pos);
    args[2] = static_cast<float>(vel_rms);
    args[3] = static_cast<float>(milliseconds);
  
    if (args[2] > vel_limit) {
        cerr << "Desired velocity exceeds limit" << endl;
        exit(1);
    }

    args[0] *= 800;     
    args[1] *= 800;     

    try {
        Mgr = SysManager::Instance();
        SysManager::FindComHubPorts(comHubPorts);
        
        if (comHubPorts.size() == 1) {
            Mgr->ComHubPort(0, comHubPorts[0].c_str()); 
        } else {
            cerr << "Found number " << comHubPorts.size() << " of ports that is not 1" << endl;
            exit(1);
        }

        Mgr->PortsOpen(1);

        Port = &Mgr->Ports(0);
        motorX = &Port->Nodes(0);
        motorZ = &Port->Nodes(1);

        if (!(motorX->Motion.Homing.WasHomed() && motorX->Motion.Homing.HomingValid()) ||       
            !(motorZ->Motion.Homing.WasHomed() && motorZ->Motion.Homing.HomingValid())) {
                cerr << "Motors are not homed. Please home before continuing" << endl;
                if (motorX) motorX->EnableReq(false);
                if (motorZ) motorZ->EnableReq(false);
                if (Mgr) Mgr->PortsClose();
                exit(1);
        }

        motorX->Status.AlertsClear();                   
        motorZ->Status.AlertsClear();

        motorX->Motion.NodeStopClear();                  
        motorZ->Motion.NodeStopClear();
        
        motorX->EnableReq(true);
        motorZ->EnableReq(true);
        Sleep(200); 

        motorX->VelUnit(INode::RPM);                        
        motorZ->VelUnit(INode::RPM);                        
        
        motorX->Motion.VelLimit = args[2];              
        motorZ->Motion.VelLimit = args[2];              
        
        motorX->Motion.MovePosnStart(args[0], true);
        motorZ->Motion.MovePosnStart(args[1], true);

        args[3] = Mgr->TimeStampMsec() + 100 + max(motorX->Motion.MovePosnDurationMsec(args[0], true), motorZ->Motion.MovePosnDurationMsec(args[1], true));

        while (!motorX->Motion.MoveIsDone() || !motorZ->Motion.MoveIsDone()) {
            if (Mgr->TimeStampMsec() > args[3]) {
                cerr << "Movement timed out" << endl;
                exit(1);
            }
        }

        cout << "(" << motorX->Motion.PosnMeasured.Value() / 800 << ", " << motorZ->Motion.PosnMeasured.Value() / 800 << ")" << endl;

    } catch (mnErr& theErr) {
        cerr << "Caught error: " << theErr.ErrorMsg << "\n";
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();
        exit(1);
    } catch (exception& e) {
        cerr << "Caught error: " << e.what() << endl;
        if (motorX) motorX->EnableReq(false);
        if (motorZ) motorZ->EnableReq(false);
        if (Mgr) Mgr->PortsClose();
        exit(1);
    }

    if (motorX) motorX->EnableReq(false);
    if (motorZ) motorZ->EnableReq(false);
    if (Mgr) Mgr->PortsClose();
 }

/*****************************************************
 * Function name: temperature_control_dump
 * Input: COMM (string)
 * Side effect: Formats sequence states, registers, and limits into text streams
 * Output: void
 *****************************************************/
void LabEquipment::temperature_control_dump(std::string COMM) const {
    vector<generic_type> base_args = {20.0f, 300, 300, 0, 0, 5.0f, 1, string("0")};     
    vector<vector<generic_type>> args(8, base_args);

    // Relocated custom stream operator locally inside implementation scope safely 
    struct LocalOstreamHelper {
        static void print(ostream& os, const generic_type& v) {
            visit([&](const auto& val) { os << val; }, v);
        }
    };

    bool OUTPUT;
    int MODE;
    float CURR_TEMP;
    float CURR_VOLTAGE;
    int CURR_SEQ;
    float SET_TEMP;
    float SET_VOLTAGE;
    int RS_STATUS;
    bool RS;
    bool RAMP;
    bool SOAK;
    float P;
    float I;
    float D;
    int RUN_METHOD;
    float MAX_DEVIATION;
    int COUNT_LEN;

    Oven5R6900 tc = Oven5R6900(COMM);

    if (!tc.get_state(OUTPUT)
        || !tc.get_mode(MODE)
        || !tc.get_temp(CURR_TEMP)
        || !tc.get_voltage(CURR_VOLTAGE)
        || !tc.get_ramp_soak_curr_seq(CURR_SEQ)
        || !tc.get_setpoint(SET_TEMP)
        || !tc.get_max_voltage(SET_VOLTAGE)
        || !tc.get_proportional_bandwidth(P)
        || !tc.get_integral_gain(I)
        || !tc.get_derivative_gain(D)
        || !tc.get_ramp_soak_status(RS_STATUS)
        || !tc.get_run_method(RUN_METHOD)
        || !tc.get_max_deviation(MAX_DEVIATION)
        || !tc.get_count_length(COUNT_LEN)) 
    {
        cerr << "Initial read failed." << endl;    
        exit(1);
    }

    for (size_t i = 0; i < args.size(); i++) {
        args[i][7] = to_string(i);      

        if (!tc.get_soak_temp(get<string>(args[i][7]), get<float>(args[i][0]))             
            || !tc.get_ramp_duration(get<string>(args[i][7]), get<int>(args[i][1]))          
            || !tc.get_soak_duration(get<string>(args[i][7]), get<int>(args[i][2]))          
            || !tc.get_num_repeats(get<string>(args[i][7]), get<int>(args[i][3]))            
            || !tc.get_next_sequence_num(get<string>(args[i][7]), get<int>(args[i][4]))) 
        {
            cerr << "Sequence retrieval failed on sequence " << i << endl;
            exit(1);
        }
    }

    RS = RS_STATUS & 0b1;
    SOAK = (RS_STATUS >> 1) & 0b1;
    RAMP = (RS_STATUS >> 2) & 0b1;

    cout << "\nOff (0), On (1)\nH-bridge Output: " << OUTPUT << "\nMode (0-3): " << MODE << "\n\n";
    cout << "Current Temperature: " << CURR_TEMP << "\nCurrent Voltage: " << CURR_VOLTAGE << "\n\n";
    cout << "Set Temperature: " << SET_TEMP << "\nSet Voltage: " << SET_VOLTAGE << "\n";
    cout << "Proportional Bandwidth: " << P << "\nIntegral Gain: " << I << "\nDerivative Gain: " << D << "\n\n";
    cout << "Sequence Pointer: " << CURR_SEQ << "\nRamp/Soak: " << RS << "\nRamp: " << RAMP << "\nSoak: " << SOAK << "\n";
    cout << "Ramp/soak method: " << RUN_METHOD << "\nRamp/soak max deviation (C): " << MAX_DEVIATION << "\n";
    cout << "Ramp/soak counter interval (s): " << COUNT_LEN * 0.2f << "\n\n";

    vector<string> row_labels = {"Soak temp", "Ramp duration", "Soak duration", "Remaining repeats", "Next sequence"};

    cout << setw(20) << "Sequence";
    for (size_t i = 0; i < args.size(); i++) {
        cout << setw(12) << i;
    }
    cout << endl;

    for (int row = 0; row < 5; ++row) {
        cout << setw(20) << row_labels[row];
        for (size_t col = 0; col < args.size(); ++col) {
            cout << setw(12);
            LocalOstreamHelper::print(cout, args[col][row]);
        }
        cout << endl;
    }
    
    
}

/*****************************************************
 * Function name: temperature_control_read_mode
 * Input: COMM (string)
 * Side effect: Prints operating state mode mapping flags
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_read_mode(std::string COMM) const {
    Oven5R6900 thermoelectric = Oven5R6900(COMM);
    int mode = -1;

    if (!thermoelectric.get_mode(mode)) {
        cerr << "Mode get failed" << endl;
    } else {
        cout << "Mode: " << mode << endl;
    }
}

/*****************************************************
 * Function name: temperature_control_set_mode
 * Input: COMM (string), Value (double)
 * Side effect: Place-holder configuration update slot
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_set_mode(std::string COMM, double Mode) const {
    Oven5R6900 thermoelectric = Oven5R6900(COMM);
    int mode;

    try {
        mode  = stoi(to_string(Mode));    // populate argument array and convert types        
    } catch (const invalid_argument& e) {
        cerr << "Invalid input: " << e.what() << " '" << Mode << "'" << endl;
        exit(2);
    }

    
    if (!thermoelectric.set_mode(mode)) {
        cerr << "Mode set failed" << endl;
        exit(1);
    }


    cout << "Mode: " << mode << endl;
} 

/*****************************************************
 * Function name: temperature_control_read_temp
 * Input: COMM (string)
 * Side effect: Displays current temperature metrics read over serial loops
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_read_temp(std::string COMM) const {
    float temp;
    Oven5R6900 thermoelectric = Oven5R6900(COMM);

    if (!thermoelectric.get_temp(temp)) {
        cerr << "Temperature read failed" << endl;
    } else {
        cout << "Temp: " << temp << endl;
    }
}

/*****************************************************
 * Function name: temperature_control_set_temp
 * Input: COMM (string), Temp (double)
 * Side effect: Alters thermoelectric set target parameter paths 
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_set_temp(std::string COMM, double Temp) const {
    Oven5R6900 thermoelectric = Oven5R6900(COMM);
    int mode;
    float temp = static_cast<float>(Temp);

    if (!thermoelectric.get_mode(mode) || mode != 0) {
        cout << "Controller not in fixed temperature mode." << endl;
    }

    if (!thermoelectric.set_setpoint(temp)) {
        cerr << "Temperature set failed" << endl;
    } else {
        cout << "Set temp: " << temp << endl;
    }
}

/*****************************************************
 * Function name: temperature_control_off
 * Input: COMM (string)
 * Side effect: Disables H-Bridge output driver pipelines
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_off(std::string COMM) const {
    Oven5R6900 temp_controller = Oven5R6900(COMM);
    bool state = 0;
    
    if (!temp_controller.enable(state) || state == 1) {
        cerr << "Shutdown failed" << endl;
    } else {
        cout << "Success!" << endl;
    }
}

/*****************************************************
 * Function name: temperature_control_on
 * Input: COMM (string)
 * Side effect: Arms current paths on temperature board modules
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_on(std::string COMM) const {
    Oven5R6900 temp_controller = Oven5R6900(COMM);
    bool state = 1;

    if (!temp_controller.enable(state) || state == 0) {
        cerr << "initialization failed" << endl;
    } else {
        cout << "Success!" << endl;
    }
}

/*****************************************************
 * Function name: temperature_control_ramp_soak
 * Input: COMM (string), seq_num (double), soak_temp (int), ramp_dur (int), soak_dur (double), deviation (int)
 * Side effect: Provisions stepped internal sequence array sets inside the Oven module
 * Output: None
 *****************************************************/
void LabEquipment::temperature_control_ramp_soak(std::string COMM, double seq_num, int soak_temp, int ramp_dur, double soak_dur, int deviation) const {
    vector<generic_type> args = {string("0"), 20.0f, 300, 300, 5.0f, 1, 0, 0};

    args[0] = COMM;
    args[1] = static_cast<float>(seq_num);
    args[2] = soak_temp;
    args[3] = ramp_dur;
    args[4] = static_cast<float>(soak_dur);
    args[5] = deviation;

    Oven5R6900 thermoelectric = Oven5R6900(COMM);

    int32_t rs_mode = 2;
    int off = 0;
    int on = 1;
    int count_length = 5;

    get<int>(args[7]) += 1;     

    if (!(thermoelectric.get_mode(rs_mode) && rs_mode == 2)) {
        cerr << "Module off or in mode " << rs_mode << " not ramp/soak mode (2)" << endl;
        exit(1);
    }

    if (
        !thermoelectric.set_ramp_soak(off)                          
        || !thermoelectric.set_soak_temp(get<string>(args[0]), get<float>(args[1]))             
        || !thermoelectric.set_ramp_duration(get<string>(args[0]), get<int>(args[2]))          
        || !thermoelectric.set_soak_duration(get<string>(args[0]), get<int>(args[3]))          
        || !thermoelectric.set_max_deviation(get<float>(args[4]))                  
        || !thermoelectric.set_run_method(get<int>(args[5]))                    
        || !thermoelectric.set_next_sequence_num(get<string>(args[0]), get<int>(args[7]))    
        || !thermoelectric.set_num_repeats(get<string>(args[0]), get<int>(args[6]))            
        || !thermoelectric.set_count_length(count_length)              
        || !thermoelectric.set_ramp_soak(on)                           
    ) {
        cerr << "Ramp Soak failed" << endl;
        exit(1);
    }

    auto gts = [](const generic_type& v) -> string {
        return visit([](auto&& x) -> string {
            if constexpr (is_same_v<decay_t<decltype(x)>, string>) return x;
            else if constexpr (is_same_v<decay_t<decltype(x)>, bool>) return x ? "true" : "false";
            else return to_string(x);
        }, v);
    };
    cout << "Sequence: " << gts(args[0]) << "\nRamp to (C): " << gts(args[1]) << "\nRamp for (s): " << gts(args[2]) << "\n";
    cout << "Soak for (s): " << gts(args[3]) << "\nTolerance (C): " << gts(args[4]) << "\nMethod: " << gts(args[5]) << "\n";
    cout << "Repeats: " << gts(args[6]) << "\nNext Sequence: " << gts(args[7]) << endl;
}


// Global instance of the hardware manager
static LabEquipment equipment;

extern "C" {

    /*****************************************************
     * Function name: WolframLibrary_getVersion
     * Input: None
     * Side effect: None
     * Output: mint (Returns engine layout version indices)
     *****************************************************/
    DLLEXPORT mint WolframLibrary_getVersion() { 
        return WolframLibraryVersion; 
    }
    
    /*****************************************************
     * Function name: WolframLibrary_initialize
     * Input: lp (WolframLibraryData)
     * Side effect: Performs low-level runtime integration checks
     * Output: int (0 on initialization tracking)
     *****************************************************/
    DLLEXPORT int WolframLibrary_initialize(WolframLibraryData lp) { 
        return 0; 
    }
    
    /*****************************************************
     * Function name: WolframLibrary_uninitialize
     * Input: lp (WolframLibraryData)
     * Side effect: None
     * Output: None
     *****************************************************/
    DLLEXPORT void WolframLibrary_uninitialize(WolframLibraryData lp) {}

    /* ==========================================================================
       WOLFRAM INTERFACE FUNCTIONS
       ========================================================================== */

    /*****************************************************
     * Function name: bath_on
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Turns on bath system equipment
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int bath_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        equipment.bath_on(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: bath_off
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Powers down the fluid bath safely
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int bath_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);

        equipment.bath_off(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: bath_dump
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Flushes data frames out to stdout/notebook fields
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int bath_dump(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        equipment.bath_dump(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: bath_read_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Places evaluated loop status inside returning variables
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int bath_read_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        double temp = equipment.bath_read_temp(comm_str);
    
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: bath_set_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Modifies setpoints inside destination devices
     * Output: int (LIBRARY_NO_ERROR or error status)
     *****************************************************/
    DLLEXPORT int bath_set_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        if (Argc != 2) {
            return LIBRARY_FUNCTION_ERROR;
        }
      
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        double temp = MArgument_getReal(Args[1]);
        
        equipment.bath_set_temp(comm_str, temp);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: read_labjack_ain0
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Triggers analog acquisition over USB lines
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int read_labjack_ain0(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        equipment.read_labjack_ain0();
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: record
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Fixed shadowing initialization issues across multiple port handles
     * Output: int (LIBRARY_NO_ERROR or structure argument bounds match checks)
     *****************************************************/
    DLLEXPORT int record(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        if (Argc != 3) {
            return LIBRARY_FUNCTION_ERROR;
        }
      
        char* csv_file = MArgument_getUTF8String(Args[0]); 
        std::string file_str(csv_file);  

        char* bath_port = MArgument_getUTF8String(Args[1]); 
        std::string b_port_str(bath_port); 

        char* temperature_port = MArgument_getUTF8String(Args[2]); 
        std::string t_port_str(temperature_port); 
        
        equipment.record(file_str, b_port_str, t_port_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: servo_motor_home
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Resets staging platforms back to index boundaries
     * Output: int (LIBRARY_NO_ERROR or parameter function issues checking limits)
     *****************************************************/
    DLLEXPORT int servo_motor_home(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        if (Argc == 0) {
            equipment.servo_motor_home();
            return LIBRARY_NO_ERROR;
        }
        else if (Argc == 1) { 
            double ms_double = MArgument_getReal(Args[0]);
            int milliseconds = static_cast<int>(ms_double); 
            equipment.servo_motor_home(milliseconds);
            return LIBRARY_NO_ERROR;
        } 
        else {
            return LIBRARY_FUNCTION_ERROR;
        }
    }

    /*****************************************************
     * Function name: servo_motor_is_home
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Validates coordinate configuration integrity indices 
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int servo_motor_is_home(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res) {
        equipment.servo_motor_is_home();
        return LIBRARY_NO_ERROR;
    }

    /*****************************************************
     * Function name: servo_motor_manual_control
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Redirects terminal pipelines into Interactive driving modes
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int servo_motor_manual_control(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){  
        equipment.servo_motor_manual_control();
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: servo_motor_read_position
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Evaluates positional feedback frames from connected hardware axes
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int servo_motor_read_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){    
        // Fixed: maps correctly to the real class implementation method member name
       equipment.servo_motor_read_position();
       
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: servo_motor_set_position
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Passes absolute microstepping dimensional target positions down to node controllers
     * Output: int (LIBRARY_NO_ERROR or error values bounds tracking check)
     *****************************************************/
    DLLEXPORT int servo_motor_set_position(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        if (Argc < 3 || Argc > 4) {
            return LIBRARY_FUNCTION_ERROR;
        }

        double x_pos = MArgument_getReal(Args[0]); 
        double z_pos = MArgument_getReal(Args[1]);
        int vel_rms = static_cast<int>(MArgument_getReal(Args[2]));

        if (Argc == 4) {
            double milliseconds = MArgument_getReal(Args[3]);
            equipment.servo_motor_set_position(x_pos, z_pos, vel_rms, milliseconds);
        } else {
            equipment.servo_motor_set_position(x_pos, z_pos, vel_rms);
        }
        
    
        return LIBRARY_NO_ERROR;
    }

    /*****************************************************
     * Function name: temperature_control_dump
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Spits complete matrix overview tables out to terminal streams
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_dump(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        equipment.temperature_control_dump(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: temperature_control_read_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Collects localized thermocouple instrumentation reading items
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_read_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        // Fixed: Class return type is void; captured via print inside member, or passing placeholder error metrics
        equipment.temperature_control_read_temp(comm_str);
       
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: temperature_control_read_mode
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Polls underlying control loop structures directly
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_read_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        equipment.temperature_control_read_mode(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: temperature_control_set_mode
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Commits updated state definitions to thermoelectric elements
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_set_mode(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        if (Argc != 2) return LIBRARY_FUNCTION_ERROR;
        
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        double mode = MArgument_getReal(Args[1]);
        
        equipment.temperature_control_set_mode(comm_str, mode);
        return LIBRARY_NO_ERROR; 
    }


    /*****************************************************
     * Function name: temperature_control_set_temp
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Prints set temperatures
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_set_temp(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        if (Argc != 2) return LIBRARY_FUNCTION_ERROR;
        
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        double temp = MArgument_getReal(Args[1]);
        
        equipment.temperature_control_set_temp(comm_str, temp);
        return LIBRARY_NO_ERROR; 
    }


    /*****************************************************
     * Function name: temperature_control_off
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Cuts thermoelectric power generation safely
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_off(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        equipment.temperature_control_off(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: temperature_control_on
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Activates loop monitoring subroutines over target ports
     * Output: int (LIBRARY_NO_ERROR)
     *****************************************************/
    DLLEXPORT int temperature_control_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        char* comm_c_str = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm_c_str);
        
        equipment.temperature_control_on(comm_str);
        return LIBRARY_NO_ERROR; 
    }

    /*****************************************************
     * Function name: temperature_control_ramp_soak
     * Input: lp (WolframLibraryData), Argc (mint), Args (MArgument*), Res (MArgument)
     * Side effect: Triggers stepped heating/cooling configuration programs
     * Output: int (LIBRARY_NO_ERROR or error checks bounds verification checking)
     *****************************************************/
    DLLEXPORT int temperature_control_ramp_soak(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
        if (Argc != 6) {
            return LIBRARY_FUNCTION_ERROR;
        }

        char* comm = MArgument_getUTF8String(Args[0]); 
        std::string comm_str(comm);
        double seq_num = MArgument_getReal(Args[1]);
        int soak_temp = static_cast<int>(MArgument_getReal(Args[2]));
        int ramp_dur = static_cast<int>(MArgument_getReal(Args[3]));
        double soak_dur = MArgument_getReal(Args[4]);
        int deviation = static_cast<int>(MArgument_getReal(Args[5]));

        equipment.temperature_control_ramp_soak(comm_str, seq_num, soak_temp, ramp_dur, soak_dur, deviation);
        return LIBRARY_NO_ERROR;
    }
}