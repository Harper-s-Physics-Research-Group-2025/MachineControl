(* ::Package:: *)

BeginPackage["WolframMachineControl`"]

(* ==========================================================================
   1. PUBLIC USAGE DECLARATIONS
   ========================================================================== *)

(* Fluid Bath Suite *)
BathOn::usage = "BathOn[port] activates the fluid bath equipment at the specified COM port string."
BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."
BathDump::usage = "BathDump[port] flushes underlying fluid bath data frames out to console fields."
BathReadTemp::usage = "BathReadTemp[port] evaluates loop status variables and extracts current temperature data."
BathSetTemp::usage = "BathSetTemp[port, temp Real] overrides target temperature setpoints inside destination systems."

(* LabJack Acquisition *)
ReadLabjackAin0::usage = "ReadLabjackAin0[] triggers a direct single-ended analog acquisition capture sequence over active USB lines."

(* Process Recording Automation *)
RecordHardwareSuite::usage = "RecordHardwareSuite[csvFilename, bathPort, temperaturePort] initiates persistent background file stream logging across target infrastructure handles."

(* Servo Axis Mechanics *)
ServoMotorHome::usage = "ServoMotorHome[] or ServoMotorHome[timeoutMs] sends structural index homing trajectories down to microstepping staging nodes."
ServoMotorIsHome::usage = "ServoMotorIsHome[] queries global state metrics to validate coordinate configuration boundary markers."
ServoMotorManualControl::usage = "ServoMotorManualControl[] blocks execution contexts, shifting control pipelines over to local interactive terminal driving modes."
ServoMotorReadPosition::usage = "ServoMotorReadPosition[] parses real-time physical microstepping feedback coordinate locations."
ServoMotorSetPosition::usage = "ServoMotorSetPosition[xPos, zPos, velRms] or ServoMotorSetPosition[xPos, zPos, velRms, durationMs] commands absolute multi-axis positioning profiles."

(* Thermal Enclosure Controller Suite *)
TemperatureControlDump::usage = "TemperatureControlDump[port] exports complete internal runtime parameter overview configurations to console targets."
TemperatureControlReadTemp::usage = "TemperatureControlReadTemp[port] extracts localized sensor instrumentation thermocouple inputs."
TemperatureControlReadMode::usage = "TemperatureControlReadMode[port] probes underlying control loop processing registers directly."
TemperatureControlSetMode::usage = "TemperatureControlSetMode[port, mode] updates algorithmic state metrics across heating element pathways."
TemperatureControlSetTemp::usage = "TemperatureControlSetTemp[port, temp] overrides direct setpoints targeting thermostatic processing hardware."
TemperatureControlOff::usage = "TemperatureControlOff[port] isolates and safelands active thermoelectric power lines."
TemperatureControlOn::usage = "TemperatureControlOn[port] registers and spins up localized hardware control thread loops."
TemperatureControlRampSoak::usage = "TemperatureControlRampSoak[port, sequence, soakTemp, rampDuration, soakDuration, deviation] pushes a complete automated thermal profiling segment down to hardware controllers."

Begin["`Private`"]

(* 2. Find and track the library path automatically *)
$dllPath = FindLibrary["wolfram_machine_controller"];
(*dllPath = FileNameJoin[{
    PacletObject["WolframMachineControl"]["Location"],
    "LibraryResources",
    $SystemID,
    "wolfram_machine_controller"   (* no extension \[LongDash] LibraryFunctionLoad appends .dll/.so/.dylib *)
}];*)

If[$dllPath === $Failed || !FileExistsQ[$dllPath],
    Message[General::error, "Critical Fault: Failed to locate 'wolfram_machine_controller' binary inside paclet structure runtime paths."];
    Abort[];
];



(* ==========================================================================
   3. LOW-LEVEL LIBRARYLINK LINKAGE BINDINGS
   ========================================================================== *)

libBathOn = LibraryFunctionLoad[$dllPath, "bath_on", {"UTF8String"}, "Integer"];
libBathOff = LibraryFunctionLoad[$dllPath, "bath_off", {"UTF8String"}, "Void"];
libBathDump = LibraryFunctionLoad[$dllPath, "bath_dump", {"UTF8String"}, "Void"];

(* Note: Your C++ signature for bath_read_temp takes args but currently discards 
   the return on 'Res' inside the C layer. If you fix it to pass via MArgument_setReal, 
   change the "Void" declaration here to "Real" *)
libBathReadTemp = LibraryFunctionLoad[$dllPath, "bath_read_temp", {"UTF8String"}, "Void"];
libBathSetTemp = LibraryFunctionLoad[$dllPath, "bath_set_temp", {"UTF8String", "Real"}, "Void"];

libReadLabjack = LibraryFunctionLoad[$dllPath, "read_labjack_ain0", {}, "Void"];
libRecord = LibraryFunctionLoad[$dllPath, "record", {"UTF8String", "UTF8String", "UTF8String"}, "Void"];

libServoHome = LibraryFunctionLoad[$dllPath, "servo_motor_home", {"Real"}, "Void"];
libServoIsHome = LibraryFunctionLoad[$dllPath, "servo_motor_is_home", {}, "Void"];
libServoManual = LibraryFunctionLoad[$dllPath, "servo_motor_manual_control", {}, "Void"];
libServoReadPos = LibraryFunctionLoad[$dllPath, "servo_motor_read_position", {}, "Void"];
libServoSetPos = LibraryFunctionLoad[$dllPath, "servo_motor_set_position", {"Real", "Real", "Real", "Real"}, "Void"];

libTempDump = LibraryFunctionLoad[$dllPath, "temperature_control_dump", {"UTF8String"}, "Void"];
libTempReadTemp = LibraryFunctionLoad[$dllPath, "temperature_control_read_temp", {"UTF8String"}, "Void"];
libTempReadMode = LibraryFunctionLoad[$dllPath, "temperature_control_read_mode", {"UTF8String"}, "Void"];
libTempSetMode = LibraryFunctionLoad[$dllPath, "temperature_control_set_mode", {"UTF8String", "Real"}, "Void"];
libTempSetTemp = LibraryFunctionLoad[$dllPath, "temperature_control_set_temp", {"UTF8String", "Real"}, "Void"];
libTempOff = LibraryFunctionLoad[$dllPath, "temperature_control_off", {"UTF8String"}, "Void"];
libTempOn = LibraryFunctionLoad[$dllPath, "temperature_control_on", {"UTF8String"}, "Void"];

libTempRampSoak = LibraryFunctionLoad[$dllPath, "temperature_control_ramp_soak", 
    {"UTF8String", "Real", "Real", "Real", "Real", "Real"}, 
    "Void"
];


(* ==========================================================================
   4. HIGH-LEVEL WOLFRAM METHOD EMULATION & INTERFACES (extra)
   ========================================================================== *)

(* Fluid Bath *)
BathOn[port_String] := libBathOn[port]
BathOff[port_String] := libBathOff[port]
BathDump[port_String] := libBathDump[port]
BathReadTemp[port_String] := libBathReadTemp[port]
BathSetTemp[port_String, temp_?NumericQ] := libBathSetTemp[port, N[temp]]

(* Instrumentation *)
ReadLabjackAin0[] := libReadLabjack[]
RecordHardwareSuite[csv_String, bath_String, temp_String] := libRecord[csv, bath, temp]

(* Motion Controls & Overload Emulation *)
ServoMotorHome[] := libServoHome[-1.0] (* Uses flag value to signal default execution branch context *)
ServoMotorHome[timeout_?NumericQ] := libServoHome[N[timeout]]

ServoMotorIsHome[] := libServoIsHome[]
ServoMotorManualControl[] := libServoManual[]
ServoMotorReadPosition[] := libServoReadPos[]

ServoMotorSetPosition[x_?NumericQ, z_?NumericQ, vel_?NumericQ] := libServoSetPos[N[x], N[z], N[vel], -1.0]
ServoMotorSetPosition[x_?NumericQ, z_?NumericQ, vel_?NumericQ, ms_?NumericQ] := libServoSetPos[N[x], N[z], N[vel], N[ms]]

(* Enclosure Management *)
TemperatureControlDump[port_String] := libTempDump[port]
TemperatureControlReadTemp[port_String] := libTempReadTemp[port]
TemperatureControlReadMode[port_String] := libTempReadMode[port]
TemperatureControlSetMode[port_String, mode_?NumericQ] := libTempSetMode[port, N[mode]]
TemperatureControlSetTemp[port_String, temp_?NumericQ] := libTempSetTemp[port, N[temp]]
TemperatureControlOff[port_String] := libTempOff[port]
TemperatureControlOn[port_String] := libTempOn[port]

TemperatureControlRampSoak[port_String, seq_?NumericQ, soakTemp_?NumericQ, rampDur_?NumericQ, soakDur_?NumericQ, dev_?NumericQ] := 
    libTempRampSoak[port, N[seq], N[soakTemp], N[rampDur], N[soakDur], N[dev]]



End[]
EndPackage[]
