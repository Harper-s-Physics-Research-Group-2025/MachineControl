(* ::Package:: *)

(* ::Title:: *)
(*Functions*)


(* ::Text:: *)
(*This file serves as a package and docummentation*)


(* ::Subtitle:: *)
(*1. Move sample holder to position (x,y)*)


BeginPackage["XRayLabFunctions"];

(*
MoveToPosition: Home's the sample holder, then sets its position.
Arguments: X an int, Y an int
Return: None
Print Out: Final X and Y positions of the Sample holder 
*)
MoveToPosition[x_,y_] := RunProcess[{"C:\\Users\\sin2\\source\\repos\\MovePosition\\x64\\Debug\\MovePosition.exe",ToString[x],ToString[y]}]


(* ::Subtitle:: *)
(*2. Manage Temperature*)


(*
SetSetPointTemperature: sets the temperature the system should rise or fall to
Arguments: desiredTemp, float
Return: None
Output: The new set point temperature
Note: Temperature is set to the nearest whole number
*)
SetSetPointTemperature[desiredTemp_]:= RunProcess[{"C:\\Users\\sin2\\source\\repos\\temperature_controller\\x64\\Debug\\temperature_controller.exe","setSetPointTemp",ToString[desiredTemp]}]

(*
GetSetPointTemperature: prints out the current set point temperature
Arguments: None
Return: None
Output: The current set point temperature
*)
GetSetPointTemperature[]:= RunProcess[{"C:\\Users\\sin2\\source\\repos\\temperature_controller\\x64\\Debug\\temperature_controller.exe","getSetPointTemp"}]

(*
GetCurrentTemperature: gets the system's immediate temperature
Arguments: None
Return: None
Output: The system's current temperature 
*)
GetCurrentTemperature[]:= RunProcess[{"C:\\Users\\sin2\\source\\repos\\temperature_controller\\x64\\Debug\\temperature_controller.exe","getTemp"}]

(*
GetMode: gets the current mode of the oven industries' device
Arguments: None
Return: None
Print Out: the device's current mode
*)
GetMode[]:= RunProcess[{"C:\\Users\\sin2\\source\\repos\\temperature_controller\\x64\\Debug\\temperature_controller.exe","getMode"}]
(*
SetMode: sets the current mode of the oven industries' device
Arguments: mode, int_32
Return: None
Print Out: the device's new mode 
*)
SetMode[mode_]:= RunProcess[{"C:\\Users\\sin2\\source\\repos\\temperature_controller\\x64\\Debug\\temperature_controller.exe","setMode",ToString[mode]}]
(*
SetRamp: Home's the sample holder, then sets its position.
Arguments: X an int, Y an int
Return: None
Print Out: Final X and Y positions of the Sample holder 
Note: Keeping mode at zero (0) is important
*)
SetRamp[initialTemperature_,finalTemperature_,time_] :=RunProcess[{"C:/msys64/ucrt64/bin/python.exe","c:/Users/sin2/OneDrive - Calvin University/Desktop/ManageTemperature/main.py","r",ToString[initialTemperature],ToString[finalTemperature],ToString[time]}];
(*
StartCirculation: Home's the sample holder, then sets its position.
Arguments: X an int, Y an int
Return: None
Print Out: Final X and Y positions of the Sample holder 
*)
StartCirculation[initialTemperature_,finalTemperature_,interval_,cycles_]:=RunProcess[{"C:/msys64/ucrt64/bin/python.exe","c:/Users/sin2/OneDrive - Calvin University/Desktop/ManageTemperature/main.py","r",ToString[initialTemperature],ToString[finalTemperature],ToString[interval],ToString[cycles]}];





(* ::Subtitle:: *)
(*3. Get Position*)


(*
StartCirculation: Home's the sample holder, then sets its position.
Arguments: X an int, Y an int
Return: None
Print Out: Final X and Y positions of the Sample holder 
*)
DisplaySampleHolderPosition[]:=RunProcess[{"C:\\Users\\sin2\\source\\repos\\Get Position\\x64\\Debug\\Get Position.exe"}]



(* ::Subtitle:: *)
(*4. Manual Control*)


(*
StartCirculation: Home's the sample holder, then sets its position.
Arguments: X an int, Y an int
Return: None
Print Out: Final X and Y positions of the Sample holder 
*)
InitiateManualControl[]:=Run["\"C:\\Users\\sin2\\source\\repos\\Manual_Control\\x64\\Debug\\Manual_Control.exe\""]

End[];
EndPackage[];
