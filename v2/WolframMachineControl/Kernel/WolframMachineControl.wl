(* ::Package:: *)

BeginPackage["WolframMachineControl`"]

(* ==========================================================================
   1. PUBLIC USAGE DECLARATIONS
   ========================================================================== *)

(* Fluid Bath Suite *)
BathOn::usage = "BathOn[port] activates the fluid bath equipment at the specified COM port string."
BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."
BathManual::usage = "BathManual[port] Puts bath into manual mode which means it responds to button presses rather than the computer."
BathGetTemp::usage = "BathGetTemp[port] gracefully powers down the fluid bath equipment."
BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."
BathOff::usage = "BathOff[port] gracefully powers down the fluid bath equipment."


(*WAdd::usage = "WAdd[a, b] adds two integers.";
WSub::usage = "WSub[a, b] subtracts two integers.";*)


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


End[]
EndPackage[]
