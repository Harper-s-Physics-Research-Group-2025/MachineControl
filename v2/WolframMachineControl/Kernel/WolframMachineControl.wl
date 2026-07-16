(* ::Package:: *)

BeginPackage["WolframMachineControl`"]

(* ==========================================================================
   1. PUBLIC USAGE DECLARATIONS
   ========================================================================== *)

WolframMachineControl`GetLogStatus::usage = "GetLogStatus[] returns 'true' or 'false' whether logging is enabled."
WolframMachineControl`GetLogFile::usage = "GetLogFile[] returns the filename path of the log file."
WolframMachineControl`SetLogSettings::usage = "SetLogSettings[int status, string logfile] sets logging status and logfile path."

(* Fluid Bath Suite *)
WolframMachineControl`BathInit::usage = "BathInit[] auto-detects the bath's COM port (by probing candidate Prolific adapters with the RTE7 protocol) and connects to it."
WolframMachineControl`BathOn::usage = "BathOn[] activates the fluid bath equipment."
WolframMachineControl`BathOff::usage = "BathOff[] gracefully powers down the fluid bath equipment."
WolframMachineControl`BathManual::usage = "BathManual[] Puts bath into manual mode which means it responds to button presses rather than the computer."
WolframMachineControl`BathGetTemp::usage = "BathGetTemp[] gets current bath temperature."
WolframMachineControl`BathSetTemp::usage = "BathSetTemp[float temp] Sets bath temperature."

WolframMachineControl`TempCtrlInit::usage = "TempCtrlInit[] auto-detects the temp controller's COM port (by probing candidate Prolific adapters with the Oven5R6900 protocol) and connects to it."
WolframMachineControl`TempCtrlOn::usage = "TempCtrlOn[] turns on the H-bridge transistor ouput of the temp controller"
WolframMachineControl`TempCtrlOff::usage = "TempCtrlOff[] turns on the H-bridge transistor ouput of the temp controller"
WolframMachineControl`TempCtrlGetMode::usage = "TempCtrlGetMode[] displays the mode of the controller (0-4) 0 = normal mode, 2 = ramp/soak"
WolframMachineControl`TempCtrlSetMode::usage = "TempCtrlSetMode[int mode] sets the mode (0-4) of the controller, see prev."
WolframMachineControl`TempCtrlGetTemp::usage = "TempCtrlGetTemp[] returns the temperature of the controller's thermistor probe sensor"
WolframMachineControl`TempCtrlGetSetpoint::usage = "TempCtrlGetSetpoint[] gets the current controller setpoint"
WolframMachineControl`TempCtrlSetSetpoint::usage = "TempCtrlSetSetpoint[float temp] changes the controller setpoint"

WolframMachineControl`ReadLabjack::usage = "ReadLabjack[int channel] reads voltage on specified channel"

WolframMachineControl`ServoEnable::usage = "ServoOn[] Initializes the global servo communication structures and clears alerts."
WolframMachineControl`ServoDisable::usage = "ServoOff[] Uninitializes the global servo communication structures."
WolframMachineControl`ServoGetAlerts::usage = "ServoGetAlerts[] Returns the servo alert register contents."
WolframMachineControl`ServoHome::usage = "ServoHome[int milliseconds] homes teknic servo motors until timeout (milliseconds)"
WolframMachineControl`ServoGetPos::usage = "ServoGetPos[real x_mm, real z_mm] gets the float/real position of the sample holder (X, Z) in mm"
WolframMachineControl`ServoSetPos::usage = "ServoSetPos[real x_mm, real z_mm, real RPM] sets the float/real (X, Z) position of the sample holder in mm"
WolframMachineControl`ServoReady::usage = "ServoReady[] checks whether the servos are ready to receive commands."
WolframMachineControl`ServoHomed::usage = "ServoHomed[] checks whether the servos are homed with valid homing."
WolframMachineControl`ServoManualControl::usage = "ServoManualControl[] puts motors into manual mode."

Begin["`Private`"]



(* 2. Find and track the library path automatically *)
$dllPath = FindLibrary["wolfram_machine_controller"];

$teknicPath = FindLibrary["sFoundation20"];        (*point mathematica to the teknic .dll*)
LibraryLoad[$teknicPath];

If[$dllPath === $Failed || !FileExistsQ[$dllPath],
    Message[General::error, "Critical Fault: Failed to locate 'wolfram_machine_controller' binary inside paclet structure runtime paths."];
    Abort[];
];



(* ==========================================================================
   3. LOW-LEVEL LIBRARYLINK LINKAGE BINDINGS
   ========================================================================== *)

WolframMachineControl`GetLogStatus = LibraryFunctionLoad[$dllPath, "wget_logging_status", {}, Integer];
WolframMachineControl`GetLogFile = LibraryFunctionLoad[$dllPath, "wget_log_file", {}, UTF8String];
WolframMachineControl`SetLogSettings = LibraryFunctionLoad[$dllPath, "wset_log_settings", {Integer, UTF8String}, Integer];

WolframMachineControl`BathInit = LibraryFunctionLoad[$dllPath, "winitialize_bath", {}, Integer];
WolframMachineControl`DeleteBath = LibraryFunctionLoad[$dllPath, "wdelete_bath", {}, Integer]
WolframMachineControl`BathOn = LibraryFunctionLoad[$dllPath, "wbath_on", {}, Integer];
WolframMachineControl`BathOff = LibraryFunctionLoad[$dllPath, "wbath_off", {}, Integer];
WolframMachineControl`BathManual = LibraryFunctionLoad[$dllPath, "wbath_manual", {}, Integer];
WolframMachineControl`BathGetTemp = LibraryFunctionLoad[$dllPath, "wbath_get_temp", {}, Real];
WolframMachineControl`BathGetSetpoint = LibraryFunctionLoad[$dllPath, "wbath_get_setpoint", {}, Real];
WolframMachineControl`BathSetSetpoint = LibraryFunctionLoad[$dllPath, "wbath_set_setpoint", {Real}, Real];

WolframMachineControl`TempCtrlInit = LibraryFunctionLoad[$dllPath, "winitialize_temperature_control", {}, Integer];
WolframMachineControl`DeleteTempCtrl = LibraryFunctionLoad[$dllPath, "wdelete_temperature_control", {}, Integer];
WolframMachineControl`TempCtrlOn = LibraryFunctionLoad[$dllPath, "wtemperature_control_on", {}, Integer];
WolframMachineControl`TempCtrlOff = LibraryFunctionLoad[$dllPath, "wtemperature_control_off", {}, Integer];
WolframMachineControl`TempCtrlGetMode = LibraryFunctionLoad[$dllPath, "wtemperature_control_get_mode", {}, Integer];
WolframMachineControl`TempCtrlSetMode = LibraryFunctionLoad[$dllPath, "wtemperature_control_set_mode", {Integer}, Integer];
WolframMachineControl`TempCtrlGetTemp = LibraryFunctionLoad[$dllPath, "wtemperature_control_get_temp", {}, Real];
WolframMachineControl`TempCtrlGetSetpoint = LibraryFunctionLoad[$dllPath, "wtemperature_control_get_setpoint", {}, Real];
WolframMachineControl`TempCtrlSetSetpoint = LibraryFunctionLoad[$dllPath, "wtemperature_control_set_setpoint", {Real}, Real];

WolframMachineControl`ReadLabjack = LibraryFunctionLoad[$dllPath, "wread_labjack_ain", {Integer}, Real]

WolframMachineControl`ServoEnable  = LibraryFunctionLoad[$dllPath, "winitialize_servos", {}, Integer];
WolframMachineControl`ServoDisable = LibraryFunctionLoad[$dllPath, "wshutdown_servos", {}, Integer]; 
WolframMachineControl`ServoGetAlerts = LibraryFunctionLoad[$dllPath, "wget_servo_alerts", {}, UTF8String];
WolframMachineControl`ServoHome = LibraryFunctionLoad[$dllPath, "wservos_home", {Integer}, Integer]
WolframMachineControl`ServoHomed = LibraryFunctionLoad[$dllPath, "wservos_homed", {}, Integer];
WolframMachineControl`ServoGetPos = LibraryFunctionLoad[$dllPath, "wservos_get_position", {}, {Real, 1}]
WolframMachineControl`ServoSetPos = LibraryFunctionLoad[$dllPath, "wservos_set_position", {Real, Real, Real}, {Real, 1}]
WolframMachineControl`ServoReady = LibraryFunctionLoad[$dllPath, "wmotors_ready", {}, Integer];
WolframMachineControl`ServoManualControl = LibraryFunctionLoad[$dllPath, "wmanual_control", {}, Integer];

WolframMachineControl`ReturnSuccess = LibraryFunctionLoad[$dllPath, "wreturn_success", {}, Integer];
WolframMachineControl`ReturnError = LibraryFunctionLoad[$dllPath, "wreturn_error", {}, Integer];

End[]
EndPackage[]
