(* ::Package:: *)

(* ::Title:: *)
(*Functions*)


(* ::Subtitle:: *)
(*1. Move sample holder to position (x,y)*)


BeginPackage["XRayLabFunctions"];

(*The functions here allow you to 
 (1) Set the temperature you need the temperature controller to change to.
(2) Get the current temperature the temperature controller is reading.
(3) Ramp the temperature controller.
(4) Circulate the temperature controller.
 *)
MoveToPosition[x_,y_] := RunProcess[{"C:\\Users\\sin2\\source\\repos\\Test\\x64\\Debug\\Test.exe",ToString[x],ToString[y]}]


(* ::Subtitle:: *)
(*2. Manage Temperature*)


(*The functions here allow you to 
 (1) Set the temperature you need the temperature controller to change to.
(2) Get the current temperature the temperature controller is reading.
(3) Ramp the temperature controller.
(4) Circulate the temperature controller.
 *)
 SetTemperature[desiredTemp_]:= Module[{result},
result=RunProcess[{"C:/msys64/ucrt64/bin/python.exe","c:/Users/sin2/OneDrive - Calvin University/Desktop/ManageTemperature/main.py","s",ToString[desiredTemp]}];
result["StandardOutput"];
]
SetRamp[initialTemperature_,finalTemperature_,time_] :=RunProcess[{"C:/msys64/ucrt64/bin/python.exe","c:/Users/sin2/OneDrive - Calvin University/Desktop/ManageTemperature/main.py","r",ToString[initialTemperature],ToString[finalTemperature],ToString[time]}];
StartCirculation[initialTemperature_,finalTemperature_,interval_,cycles_]:=RunProcess[{"C:/msys64/ucrt64/bin/python.exe","c:/Users/sin2/OneDrive - Calvin University/Desktop/ManageTemperature/main.py","r",ToString[initialTemperature],ToString[finalTemperature],ToString[interval],ToString[cycles]}];


(* ::Subtitle:: *)
(*3. Get Position*)


(*The functions here allow you to 
 (1) Set the temperature you need the temperature controller to change to.
(2) Get the current temperature the temperature controller is reading.
(3) Ramp the temperature controller.
(4) Circulate the temperature controller.
 *) 
DisplaySampleHolderPosition[]:=RunProcess[{"C:\\Users\\sin2\\source\\repos\\Get Position\\x64\\Debug\\Get Position.exe"}]

End[];
EndPackage[];
