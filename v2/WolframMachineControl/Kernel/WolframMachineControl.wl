(* ::Package:: *)

BeginPackage["WolframMachineControl`"]

(* ==========================================================================
   1. PUBLIC USAGE DECLARATIONS
   ========================================================================== *)

(* Fluid Bath Suite *)
WolframMachineControl`BathOn::usage = "BathOn[port] activates the fluid bath equipment at the specified COM port string."
WolframMachineControl`BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."
WolframMachineControl`BathManual::usage = "BathManual[port] Puts bath into manual mode which means it responds to button presses rather than the computer."
WolframMachineControl`BathGetTemp::usage = "BathGetTemp[port] gracefully powers down the fluid bath equipment."
WolframMachineControl`BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."
WolframMachineControl`BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."


WolframMachineControl`TempCtrlOn::usage = "TempCtrlOn[string port] turns on the H-bridge transistor ouput of the temp controller"
WolframMachineControl`TempCtrlOff::usage = "TempCtrlOff[string port] turns on the H-bridge transistor ouput of the temp controller"
WolframMachineControl`TempCtrlGetMode::usage = "TempCtrlGetMode[string port] displays the mode of the controller (0-4) 0 = normal mode, 2 = ramp/soak"
WolframMachineControl`TempCtrlSetMode::usage = "TempCtrlSetMode[string port, int mode] sets the mode (0-4) of the controller, see prev."
WolframMachineControl`TempCtrlGetTemp::usage = "TempCtrlGetTemp[string port] returns the temperature of the controller's thermistor probe sensor"
WolframMachineControl`TempCtrlGetSetpoint::usage = "TempCtrlOn[string port] gets the current controller setpoint"
WolframMachineControl`TempCtrlSetSetpoint::usage = "TempCtrlOn[string port, float temp] changes the controller setpoint"

WolframMachineControl`ReadLabjack::usage = "ReadLabjack[int channel] reads voltage on specified channel"



Begin["`Private`"]

(* 2. Find and track the library path automatically *)
$dllPath = FindLibrary["wolfram_machine_controller"];

If[$dllPath === $Failed || !FileExistsQ[$dllPath],
    Message[General::error, "Critical Fault: Failed to locate 'wolfram_machine_controller' binary inside paclet structure runtime paths."];
    Abort[];
];



(* ==========================================================================
   3. LOW-LEVEL LIBRARYLINK LINKAGE BINDINGS
   ========================================================================== *)

WolframMachineControl`BathOn = LibraryFunctionLoad[$dllPath, "wbath_on", {UTF8String}, Integer];
WolframMachineControl`BathOff = LibraryFunctionLoad[$dllPath, "wbath_off", {UTF8String}, Integer];
WolframMachineControl`BathManual = LibraryFunctionLoad[$dllPath, "wbath_manual", {UTF8String}, Integer];
WolframMachineControl`BathGetTemp = LibraryFunctionLoad[$dllPath, "wbath_get_temp", {UTF8String}, Real];
WolframMachineControl`BathGetSetpoint = LibraryFunctionLoad[$dllPath, "wbath_get_setpoint", {UTF8String}, Real];
WolframMachineControl`BathSetSetpoint = LibraryFunctionLoad[$dllPath, "wbath_set_setpoint", {UTF8String, Real}, Real];

WolframMachineControl`TempCtrlOn = LibraryFunctionLoad[$dllPath, "wtemperature_control_on", {UTF8String}, Integer];
WolframMachineControl`TempCtrlOff = LibraryFunctionLoad[$dllPath, "wtemperature_control_off", {UTF8String}, Integer];
WolframMachineControl`TempCtrlGetMode = LibraryFunctionLoad[$dllPath, "wtemperature_control_get_mode", {UTF8String}, Integer];
WolframMachineControl`TempCtrlSetMode = LibraryFunctionLoad[$dllPath, "wtemperature_control_set_mode", {UTF8String, Integer}, Integer];
WolframMachineControl`TempCtrlGetTemp = LibraryFunctionLoad[$dllPath, "wtemperature_control_get_temp", {UTF8String}, Real];
WolframMachineControl`TempCtrlGetSetpoint = LibraryFunctionLoad[$dllPath, "wtemperature_control_get_setpoint", {UTF8String}, Real];
WolframMachineControl`TempCtrlSetSetpoint = LibraryFunctionLoad[$dllPath, "wtemperature_control_set_setpoint", {UTF8String, Real}, Real];

WolframMachineControl`ReadLabjack = LibraryFunctionLoad[$dllPath, "wread_labjack_ain", {Integer}, Real]

End[]
EndPackage[]
