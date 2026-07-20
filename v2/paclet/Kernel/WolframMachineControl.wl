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

(* LabJack Data-Collection Suite -- pure Wolfram Language, built on top of ReadLabjack/BathGetTemp *)
WolframMachineControl`LabJackRecordData::usage = "LabJackRecordData[filename, finalTemp, interval] turns on both the bath and the temp controller and sets both setpoints to finalTemp, then repeatedly waits `interval` seconds and reads LabJack channels 0-7 plus both devices' actual temperatures (BathGetTemp[], TempCtrlGetTemp[]), stopping once BOTH have reached finalTemp (works whether finalTemp is above or below the starting temperature). Saves the results to <filename>.csv in v2/data and returns the full path."
WolframMachineControl`LabJackListCSVs::usage = "LabJackListCSVs[] lists every CSV file in v2/data as a formatted grid of file name and creation date, most recent first."
WolframMachineControl`LabJackPlotData::usage = "LabJackPlotData[filename] plots every LabJack channel column from <filename>.csv (as saved by LabJackRecordData) against the bath temperature column."
WolframMachineControl`LabJackPlotData::nofile = "No CSV file found at `1`."
WolframMachineControl`LabJackRecordData::bathfail = "Couldn't read the bath's temperature -- is BathInit[] connected?"
WolframMachineControl`LabJackRecordData::tempctrlfail = "Couldn't read the temp controller's temperature -- is TempCtrlInit[] connected?"
WolframMachineControl`LabJackRecordData::clamped = "Requested setpoint `1` was clamped by the bath to `2` -- recording will wait for `2`, not `1`."

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
WolframMachineControl`ServoReady = LibraryFunctionLoad[$dllPath, "wservo_hardware_online", {}, Integer];
WolframMachineControl`ServoManualControl = LibraryFunctionLoad[$dllPath, "wmanual_control", {}, Integer];

WolframMachineControl`ReturnSuccess = LibraryFunctionLoad[$dllPath, "wreturn_success", {}, Integer];
WolframMachineControl`ReturnError = LibraryFunctionLoad[$dllPath, "wreturn_error", {}, Integer];



(* ==========================================================================
   4. LABJACK DATA-COLLECTION SUITE (pure Wolfram Language -- no LibraryLink,
      just orchestrates ReadLabjack/BathGetTemp and file I/O)
   ========================================================================== *)

(* v2/data, resolved from $dllPath so this works regardless of where the paclet is installed *)
$labJackDataDir = FileNameJoin[{DirectoryName[$dllPath, 4], "data"}];
If[!DirectoryQ[$labJackDataDir], CreateDirectory[$labJackDataDir]];

normalizeCSVName[name_String] := If[StringEndsQ[name, ".csv", IgnoreCase -> True], name, name <> ".csv"];

WolframMachineControl`LabJackRecordData[filename_String, finalTemp_?NumericQ, interval_?NumericQ] := Module[
    {startTime, currentBathTemp, currentTempCtrlTemp, bathDirection, tempCtrlDirection, bathTarget,
     tempCtrlTarget, bathReached, tempCtrlReached, header, csvPath, data, confirmedSetpoint},

    currentBathTemp = BathGetTemp[];
    If[!NumericQ[currentBathTemp],
        Message[WolframMachineControl`LabJackRecordData::bathfail];
        Return[$Failed]
    ];
    currentTempCtrlTemp = TempCtrlGetTemp[];
    If[!NumericQ[currentTempCtrlTemp],
        Message[WolframMachineControl`LabJackRecordData::tempctrlfail];
        Return[$Failed]
    ];

    BathOn[];
    confirmedSetpoint = BathSetSetpoint[finalTemp];
    (* Use the bath's *confirmed* setpoint as its actual target -- if the bath silently clamped
       the request to its own Hi/Lo Temperature Limit, waiting for the unclamped finalTemp would
       loop forever, since the bath will never actually reach it. *)
    bathTarget = If[NumericQ[confirmedSetpoint], confirmedSetpoint, finalTemp];
    If[NumericQ[confirmedSetpoint] && confirmedSetpoint != finalTemp,
        Message[WolframMachineControl`LabJackRecordData::clamped, finalTemp, confirmedSetpoint]
    ];

    TempCtrlOn[];
    TempCtrlSetSetpoint[finalTemp];
    (* Unlike BathSetSetpoint, TempCtrlSetSetpoint just echoes the request back -- there's no
       confirmed value to fall back on, so no clamp detection is possible on this side. *)
    tempCtrlTarget = finalTemp;

    bathDirection = Sign[bathTarget - currentBathTemp]; (* +1 = wait for it to rise, -1 = fall, 0 = already there *)
    tempCtrlDirection = Sign[tempCtrlTarget - currentTempCtrlTemp];
    startTime = AbsoluteTime[];
    header = Join[{"Time (s)", "BathTemp (C)", "TempCtrlTemp (C)"}, Table["Channel" <> ToString[ch] <> " (V)", {ch, 0, 7}]];

    data = Reap[
        While[True,
            Pause[interval];
            currentBathTemp = BathGetTemp[];
            currentTempCtrlTemp = TempCtrlGetTemp[];
            Sow[Join[{AbsoluteTime[] - startTime, currentBathTemp, currentTempCtrlTemp}, Table[ReadLabjack[ch], {ch, 0, 7}]]];
            bathReached = (bathDirection >= 0 && currentBathTemp >= bathTarget) || (bathDirection < 0 && currentBathTemp <= bathTarget);
            tempCtrlReached = (tempCtrlDirection >= 0 && currentTempCtrlTemp >= tempCtrlTarget) || (tempCtrlDirection < 0 && currentTempCtrlTemp <= tempCtrlTarget);
            If[bathReached && tempCtrlReached, Break[]];
        ]
    ][[2, 1]];

    csvPath = FileNameJoin[{$labJackDataDir, normalizeCSVName[filename]}];
    Export[csvPath, Prepend[data, header], "CSV"];
    Print["Recorded ", Length[data], " readings over ", Round[Last[data][[1]]], " s -- saved to ", csvPath];
    csvPath
];

WolframMachineControl`LabJackListCSVs[] := Module[{files, info},
    files = FileNames["*.csv", $labJackDataDir];
    If[files === {},
        Print["No CSV files found in ", $labJackDataDir];
        Return[{}]
    ];
    info = SortBy[{FileNameTake[#], FileDate[#, "Creation"]} & /@ files, -AbsoluteTime[Last[#]] &];
    Grid[
        Prepend[
            {First[#], DateString[Last[#], {"Year", "-", "Month", "-", "Day", "  ", "Hour24", ":", "Minute", ":", "Second"}]} & /@ info,
            Style[#, Bold] & /@ {"File Name", "Date Created"}
        ],
        Frame -> All, Dividers -> All, Alignment -> Left, Spacings -> {2, 1},
        Background -> {None, {LightGray, White}}
    ]
];

WolframMachineControl`LabJackPlotData[filename_String] := Module[
    {csvPath, raw, header, data, tempCol, channelNames, series},

    csvPath = FileNameJoin[{$labJackDataDir, normalizeCSVName[filename]}];
    If[!FileExistsQ[csvPath],
        Message[WolframMachineControl`LabJackPlotData::nofile, csvPath];
        Return[$Failed]
    ];

    raw = Import[csvPath, "CSV"];
    header = First[raw];
    data = Rest[raw];
    tempCol = data[[All, 2]]; (* BathTemp -- kept as the plot's x-axis *)
    channelNames = header[[4 ;;]];
    series = Table[Transpose[{tempCol, data[[All, i]]}], {i, 4, Length[header]}];

    ListLinePlot[series,
        PlotLegends -> channelNames,
        AxesLabel -> {"Bath Temperature (\[Degree]C)", "Voltage (V)"},
        PlotLabel -> normalizeCSVName[filename],
        PlotMarkers -> Automatic,
        GridLines -> Automatic,
        ImageSize -> Large
    ]
];

End[]
EndPackage[]
