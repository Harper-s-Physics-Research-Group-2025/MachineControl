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

(* LabJack Data-Collection Suite -- pure Wolfram Language, built on top of ReadLabjack/TempCtrlGetTemp *)
WolframMachineControl`LabJackRecordData::usage = "LabJackRecordData[filename, finalTemp, interval] turns on the temp controller and sets its setpoint to finalTemp, then repeatedly waits `interval` seconds and reads LabJack channels 0-7 plus the temp controller's actual temperature (TempCtrlGetTemp[]), stopping once it reaches finalTemp (works whether finalTemp is above or below the starting temperature). Saves the results to <filename>.csv in v2/data and returns the full path."
WolframMachineControl`LabJackListCSVs::usage = "LabJackListCSVs[] lists every CSV file in v2/data as a formatted grid of file name and creation date, most recent first."
WolframMachineControl`LabJackPlotData::usage = "LabJackPlotData[filename, channel] plots the given LabJack channel column from <filename>.csv (as saved by LabJackRecordData) against the temp controller temperature column."
WolframMachineControl`LabJackPlotData::nofile = "No CSV file found at `1`."
WolframMachineControl`LabJackPlotData::nochannel = "No column for channel `1` found in `2`."
WolframMachineControl`LabJackRecordData::tempctrlfail = "Couldn't read the temp controller's temperature -- is TempCtrlInit[] connected?"

WolframMachineControl`TempCtrlPlotTemp::usage = "TempCtrlPlotTemp[targetTemp, interval:1] turns on the temp controller and sets its setpoint to targetTemp, then samples its actual temperature (TempCtrlGetTemp[]) every `interval` seconds (default 1) until it reaches targetTemp, and plots temperature vs. time."
WolframMachineControl`TempCtrlPlotTemp::tempctrlfail = "Couldn't read the temp controller's temperature -- is TempCtrlInit[] connected?"

WolframMachineControl`LabJackTempSweep::usage = "LabJackTempSweep[startTemp, temps, lipidName, waterConcentration, interval:1] repeatedly returns the temp controller to startTemp and drives it to each temperature in temps, recording LabJack data (via LabJackRecordData) for every leg of the trip -- both the move out to each temps[[i]] and the move back to startTemp -- so a list of N temperatures produces 2N CSVs. Each is named lipidName_waterConcentration_rise-or-fall_from_to_to.csv (e.g. monopalmatin_60_rise_10_to_45.csv), where rise/fall and the two temperatures reflect that leg's actual direction and endpoints. Returns the list of CSV paths written."
WolframMachineControl`LabJackTempSweep::tempctrlfail = "Couldn't read the temp controller's temperature -- is TempCtrlInit[] connected?"
WolframMachineControl`LabJackTempSweep::legfailed = "Measurement `1` (target `2`\[Degree]C) failed -- aborting the rest of the sweep."

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
   4. DATA-COLLECTION/PLOTTING SUITE (pure Wolfram Language -- no LibraryLink,
      just orchestrates ReadLabjack/TempCtrlGetTemp and file I/O)
   ========================================================================== *)

(* v2/data, resolved from $dllPath so this works regardless of where the paclet is installed *)
$labJackDataDir = FileNameJoin[{DirectoryName[$dllPath, 4], "data"}];
If[!DirectoryQ[$labJackDataDir], CreateDirectory[$labJackDataDir]];

normalizeCSVName[name_String] := If[StringEndsQ[name, ".csv", IgnoreCase -> True], name, name <> ".csv"];

(* Sets the temp controller's setpoint and blocks until it actually arrives -- same wait shape as
   LabJackRecordData/TempCtrlPlotTemp, but no recording/plotting. Used to position the temp
   controller at a sweep's starting temperature before any measurement begins. *)
waitForTempCtrl[target_?NumericQ, interval_?NumericQ] := Module[{currentTemp, direction},
    currentTemp = TempCtrlGetTemp[];
    TempCtrlOn[];
    TempCtrlSetSetpoint[target];
    direction = Sign[target - currentTemp];
    While[True,
        Pause[interval];
        currentTemp = TempCtrlGetTemp[];
        If[(direction >= 0 && currentTemp >= target) || (direction < 0 && currentTemp <= target), Break[]];
    ];
];

WolframMachineControl`LabJackRecordData[filename_String, finalTemp_?NumericQ, interval_?NumericQ] := Module[
    {startTime, currentTemp, direction, header, csvPath, data},

    currentTemp = TempCtrlGetTemp[];
    If[!NumericQ[currentTemp],
        Message[WolframMachineControl`LabJackRecordData::tempctrlfail];
        Return[$Failed]
    ];

    TempCtrlOn[];
    TempCtrlSetSetpoint[finalTemp];

    direction = Sign[finalTemp - currentTemp]; (* +1 = wait for it to rise, -1 = wait for it to fall, 0 = already there *)
    startTime = AbsoluteTime[];
    header = Join[{"Time (s)", "TempCtrlTemp (C)"}, Table["Channel" <> ToString[ch] <> " (V)", {ch, 0, 7}]];

    data = Reap[
        While[True,
            Pause[interval];
            currentTemp = TempCtrlGetTemp[];
            Sow[Join[{AbsoluteTime[] - startTime, currentTemp}, Table[ReadLabjack[ch], {ch, 0, 7}]]];
            If[(direction >= 0 && currentTemp >= finalTemp) || (direction < 0 && currentTemp <= finalTemp), Break[]];
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

WolframMachineControl`LabJackPlotData[filename_String, channel_Integer] := Module[
    {csvPath, raw, header, data, tempCol, columnName, columnIndex},

    csvPath = FileNameJoin[{$labJackDataDir, normalizeCSVName[filename]}];
    If[!FileExistsQ[csvPath],
        Message[WolframMachineControl`LabJackPlotData::nofile, csvPath];
        Return[$Failed]
    ];

    raw = Import[csvPath, "CSV"];
    header = First[raw];
    data = Rest[raw];
    tempCol = data[[All, 2]]; (* TempCtrlTemp -- the plot's x-axis *)

    columnName = "Channel" <> ToString[channel] <> " (V)";
    columnIndex = FirstPosition[header, columnName];
    If[MissingQ[columnIndex],
        Message[WolframMachineControl`LabJackPlotData::nochannel, channel, normalizeCSVName[filename]];
        Return[$Failed]
    ];
    columnIndex = First[columnIndex];

    ListLinePlot[Transpose[{tempCol, data[[All, columnIndex]]}],
        PlotLegends -> {columnName},
        AxesLabel -> {"Temp Controller Temperature (\[Degree]C)", "Voltage (V)"},
        PlotLabel -> normalizeCSVName[filename] <> " -- " <> columnName,
        PlotMarkers -> Automatic,
        GridLines -> Automatic,
        ImageSize -> Large
    ]
];

WolframMachineControl`TempCtrlPlotTemp[targetTemp_?NumericQ] := WolframMachineControl`TempCtrlPlotTemp[targetTemp, 1];

WolframMachineControl`TempCtrlPlotTemp[targetTemp_?NumericQ, interval_?NumericQ] := Module[
    {startTime, currentTemp, direction, data},

    currentTemp = TempCtrlGetTemp[];
    If[!NumericQ[currentTemp],
        Message[WolframMachineControl`TempCtrlPlotTemp::tempctrlfail];
        Return[$Failed]
    ];

    TempCtrlOn[];
    TempCtrlSetSetpoint[targetTemp];

    direction = Sign[targetTemp - currentTemp]; (* +1 = wait for it to rise, -1 = wait for it to fall, 0 = already there *)
    startTime = AbsoluteTime[];
    data = Reap[
        While[True,
            Pause[interval];
            currentTemp = TempCtrlGetTemp[];
            Sow[{AbsoluteTime[] - startTime, currentTemp}];
            If[(direction >= 0 && currentTemp >= targetTemp) || (direction < 0 && currentTemp <= targetTemp), Break[]];
        ]
    ][[2, 1]];

    ListLinePlot[data,
        AxesLabel -> {"Time (every " <> ToString[interval] <> "s)", "Temp Controller Temperature (\[Degree]C)"},
        PlotLabel -> "Temp Controller Temperature vs. Time (target: " <> ToString[targetTemp] <> "\[Degree]C)",
        PlotMarkers -> Automatic,
        GridLines -> Automatic,
        ImageSize -> Large
    ]
];

WolframMachineControl`LabJackTempSweep[startTemp_?NumericQ, temps_List, lipidName_String, waterConcentration_] :=
    WolframMachineControl`LabJackTempSweep[startTemp, temps, lipidName, waterConcentration, 1];

WolframMachineControl`LabJackTempSweep[startTemp_?NumericQ, temps_List, lipidName_String, waterConcentration_, interval_?NumericQ] := Module[
    {measurementCount = 0, currentNominalTemp, direction, filename, csvPath, csvPaths = {}},

    If[!NumericQ[TempCtrlGetTemp[]],
        Message[WolframMachineControl`LabJackTempSweep::tempctrlfail];
        Return[$Failed]
    ];

    (* Get to the starting temperature first -- not recorded, just positioning *)
    waitForTempCtrl[startTemp, interval];
    currentNominalTemp = startTemp; (* the *intended* temp, not a live sensor reading -- keeps filenames clean *)

    Do[
        Do[
            direction = If[target >= currentNominalTemp, "rise", "fall"];
            measurementCount++;
            filename = lipidName <> "_" <> ToString[waterConcentration] <> "_" <> direction <> "_" <>
                ToString[currentNominalTemp] <> "_to_" <> ToString[target];
            csvPath = LabJackRecordData[filename, target, interval];
            If[csvPath === $Failed,
                Message[WolframMachineControl`LabJackTempSweep::legfailed, measurementCount, target];
                Return[$Failed]
            ];
            AppendTo[csvPaths, csvPath];
            currentNominalTemp = target;
            ,
            {target, {temps[[i]], startTemp}}
        ];
        ,
        {i, Length[temps]}
    ];

    csvPaths
];

End[]
EndPackage[]