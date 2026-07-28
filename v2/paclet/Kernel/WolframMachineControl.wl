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

(* Intensity Peak-Finding -- pure Wolfram Language, built on top of ServoSetPos/ReadLabjack *)
WolframMachineControl`ServoFindMaxIntensity::usage = "ServoFindMaxIntensity[channel, xSpec, zSpec] scans the sample holder over the given positions and returns the (X, Z) position where LabJack `channel` reads the highest voltage -- i.e. the point of highest light intensity. Each of xSpec/zSpec is either {start, end, step} (in mm) to scan that axis, or a plain number to hold that axis fixed at that position. Returns an Association with \"Position\", \"Intensity\", \"Scan\", \"Plot\" and \"Passes\". Options: \"RPM\" (move speed, default 100), \"SettleTime\" (seconds to wait after each move before reading, default 0.25), \"SamplesPerPoint\" (readings averaged per point, default 5), \"RefinePasses\" (extra finer scans centred on the running best point, default 1), \"RefineFactor\" (step shrink per refine pass, default 5), \"ReturnToPeak\" (move back to the winning position when done, default True), \"ShowPlot\" (default True)."
WolframMachineControl`ServoFindMaxIntensity::notready = "Servos aren't ready -- call ServoEnable[] first (and check ServoGetAlerts[])."
WolframMachineControl`ServoFindMaxIntensity::nothomed = "Servos aren't homed -- call ServoHome[milliseconds] first."
WolframMachineControl`ServoFindMaxIntensity::badspec = "`1` must be either a number (axis held fixed) or {start, end, step} in mm; got `2`."
WolframMachineControl`ServoFindMaxIntensity::movefail = "Move to (`1`, `2`) mm failed or landed more than `3` mm away -- aborting the scan."
WolframMachineControl`ServoFindMaxIntensity::readfail = "Couldn't read LabJack channel `1` at (`2`, `3`) mm -- aborting the scan."

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

(* ==========================================================================
   5. INTENSITY PEAK-FINDING (pure Wolfram Language -- no LibraryLink, just
      orchestrates ServoSetPos/ServoGetPos and ReadLabjack)
   ========================================================================== *)

(* An axis spec is either a plain number (hold this axis fixed there) or {start, end, step}
   in mm. Returns the list of positions to visit, or $Failed if the spec is malformed.
   The endpoint is always included: the step is rounded to the nearest value that divides
   the span evenly, so a 0-to-10 scan with step 3 gives 4 evenly spaced points, not 0/3/6/9/10. *)
axisPositions[spec_?NumericQ] := {N[spec]};
axisPositions[{start_?NumericQ, end_?NumericQ, step_?NumericQ}] := Module[{n},
    If[step <= 0 || start == end, Return[{N[start]}]];
    n = Max[1, Round[Abs[end - start]/step]];
    Table[N[start + (end - start) i/n], {i, 0, n}]
];
axisPositions[_] := $Failed;

(* The actual step size a spec resolves to -- used to size each refinement pass's window.
   0 for a fixed axis, which is what keeps refinement from ever moving a held axis. *)
axisStep[spec_?NumericQ] := 0;
axisStep[{start_?NumericQ, end_?NumericQ, step_?NumericQ}] := Module[{n},
    If[step <= 0 || start == end, Return[0]];
    n = Max[1, Round[Abs[end - start]/step]];
    Abs[end - start]/n
];

(* Move, then confirm we actually landed there. ServoSetPos returns the *measured* position on
   success but echoes the request back on failure (API_REFERENCE.md / BUGS.md #23), so neither the
   status nor the returned value alone can be trusted -- compare it against what we asked for
   instead. A failed LibraryLink call comes back non-numeric, which the MatchQ below also
   catches; Quiet only suppresses the noise of that message repeating at every grid point. *)
moveAndVerify[x_, z_, rpm_, tol_] := Module[{result},
    result = Quiet[ServoSetPos[N[x], N[z], N[rpm]]];
    If[!MatchQ[result, {_?NumericQ, _?NumericQ, _?NumericQ}], Return[$Failed]];
    If[Abs[result[[1]] - x] > tol || Abs[result[[2]] - z] > tol, Return[$Failed]];
    N[result[[1 ;; 2]]]
];

(* Average several reads to pull a weak optical signal out of ADC noise. A failed ReadLabjack
   returns a non-numeric LibraryFunctionError, so every sample is checked before it's averaged. *)
readIntensity[channel_, samples_, settle_] := Module[{values},
    If[settle > 0, Pause[settle]];
    values = Quiet[Table[ReadLabjack[channel], {samples}]];
    If[!AllTrue[values, NumericQ], Return[$Failed]];
    Mean[values]
];

(* One serpentine raster over xs * zs. Serpentine (alternating Z direction per X column) rather
   than raster-scan-and-fly-back, so the holder never makes a long empty return move.
   Returns the list of {x, z, volts} measured, or $Failed if any move or read failed. *)
scanGrid[channel_, xs_, zs_, rpm_, settle_, samples_, tol_] := Module[{points},
    Catch[
        points = Reap[
            Do[
                Module[{voltage, landed},
                    landed = moveAndVerify[xs[[i]], z, rpm, tol];
                    If[landed === $Failed,
                        Message[WolframMachineControl`ServoFindMaxIntensity::movefail, xs[[i]], z, tol];
                        Throw[$Failed, "scanGrid"]
                    ];
                    voltage = readIntensity[channel, samples, settle];
                    If[voltage === $Failed,
                        Message[WolframMachineControl`ServoFindMaxIntensity::readfail, channel, xs[[i]], z];
                        Throw[$Failed, "scanGrid"]
                    ];
                    Sow[{landed[[1]], landed[[2]], voltage}]
                ],
                {i, Length[xs]}, {z, If[OddQ[i], zs, Reverse[zs]]}
            ]
        ][[2]];
        If[points === {}, {}, First[points]]
        ,
        "scanGrid"
    ]
];

Options[WolframMachineControl`ServoFindMaxIntensity] = {
    "RPM" -> 100, "SettleTime" -> 0.25, "SamplesPerPoint" -> 5,
    "RefinePasses" -> 1, "RefineFactor" -> 5, "PositionTolerance" -> 0.1,
    "ReturnToPeak" -> True, "ShowPlot" -> True
};

WolframMachineControl`ServoFindMaxIntensity[channel_Integer, xSpec_, zSpec_, OptionsPattern[]] := Module[
    {rpm, settle, samples, refinePasses, refineFactor, tol, xs, zs, xStep, zStep,
     xWindow, zWindow, xBounds, zBounds, allPoints = {}, passPoints, passCounts = {}, best, plot},

    rpm = OptionValue["RPM"];
    settle = OptionValue["SettleTime"];
    samples = OptionValue["SamplesPerPoint"];
    refinePasses = OptionValue["RefinePasses"];
    refineFactor = OptionValue["RefineFactor"];
    tol = OptionValue["PositionTolerance"];

    If[ServoReady[] =!= 1,
        Message[WolframMachineControl`ServoFindMaxIntensity::notready];
        Return[$Failed]
    ];
    If[ServoHomed[] =!= 1,
        Message[WolframMachineControl`ServoFindMaxIntensity::nothomed];
        Return[$Failed]
    ];

    xs = axisPositions[xSpec];
    zs = axisPositions[zSpec];
    If[xs === $Failed, Message[WolframMachineControl`ServoFindMaxIntensity::badspec, "xSpec", xSpec]; Return[$Failed]];
    If[zs === $Failed, Message[WolframMachineControl`ServoFindMaxIntensity::badspec, "zSpec", zSpec]; Return[$Failed]];

    (* Refinement never leaves the region the caller asked for -- these clamp every later pass *)
    xBounds = MinMax[xs];
    zBounds = MinMax[zs];
    xStep = axisStep[xSpec];
    zStep = axisStep[zSpec];

    (* Coarse pass, then successively finer passes over a +/- one-step window around the best
       point found so far. A fixed axis has step 0, so its window collapses and it stays put. *)
    If[Catch[Do[
        If[pass > 1,
            (* Re-scan +/- one previous step either side of the best point, at 1/refineFactor
               the previous resolution. A fixed axis has step 0, so its window collapses to a
               single position and it never moves. *)
            xWindow = xStep; zWindow = zStep;
            xStep = xStep/refineFactor;
            zStep = zStep/refineFactor;
            xs = axisPositions[{Max[xBounds[[1]], best[[1]] - xWindow],
                                Min[xBounds[[2]], best[[1]] + xWindow], xStep}];
            zs = axisPositions[{Max[zBounds[[1]], best[[2]] - zWindow],
                                Min[zBounds[[2]], best[[2]] + zWindow], zStep}];
        ];

        Print["Pass ", pass, "/", refinePasses + 1, ": ", Length[xs] Length[zs],
              " points (X step ", NumberForm[N[xStep], {4, 3}], " mm, Z step ", NumberForm[N[zStep], {4, 3}], " mm)"];

        passPoints = scanGrid[channel, xs, zs, rpm, settle, samples, tol];
        If[passPoints === $Failed, Throw[$Failed, "findMax"]];

        allPoints = Join[allPoints, passPoints];
        AppendTo[passCounts, Length[passPoints]];
        best = First[MaximalBy[allPoints, Last]];
        ,
        {pass, refinePasses + 1}
    ], "findMax"] === $Failed,
        Return[$Failed]  (* scanGrid already issued the specific ::movefail/::readfail message *)
    ];

    plot = intensityPlot[allPoints, best, channel];

    If[TrueQ[OptionValue["ReturnToPeak"]],
        If[moveAndVerify[best[[1]], best[[2]], rpm, tol] === $Failed,
            Message[WolframMachineControl`ServoFindMaxIntensity::movefail, best[[1]], best[[2]], tol]
        ]
    ];

    Print["Peak intensity ", NumberForm[best[[3]], {6, 4}], " V at (X, Z) = (",
          NumberForm[best[[1]], {6, 3}], ", ", NumberForm[best[[2]], {6, 3}], ") mm, from ",
          Length[allPoints], " points"];
    If[TrueQ[OptionValue["ShowPlot"]], Print[plot]];

    <|"Position" -> best[[1 ;; 2]], "Intensity" -> best[[3]], "Scan" -> allPoints,
      "Passes" -> passCounts, "Plot" -> plot|>
];

(* A scan that only moved along one axis plots as a curve; a real 2D raster plots as a surface.
   The peak is marked on both. *)
intensityPlot[points_, best_, channel_] := Module[{xVaries, zVaries, curve},
    xVaries = Length[Union[points[[All, 1]]]] > 1;
    zVaries = Length[Union[points[[All, 2]]]] > 1;

    If[xVaries && zVaries,
        Return[ListDensityPlot[points,
            InterpolationOrder -> 0, ColorFunction -> "SunsetColors",
            PlotLegends -> Automatic, FrameLabel -> {"X (mm)", "Z (mm)"},
            PlotLabel -> "LabJack channel " <> ToString[channel] <> " intensity (V)",
            Epilog -> {White, PointSize[0.02], Point[best[[1 ;; 2]]]},
            ImageSize -> Large
        ]]
    ];

    (* Single-axis scan: plot against whichever axis actually moved (X if neither did) *)
    curve = SortBy[points[[All, {If[zVaries && !xVaries, 2, 1], 3}]], First];
    ListLinePlot[curve,
        AxesLabel -> {If[zVaries && !xVaries, "Z (mm)", "X (mm)"], "Voltage (V)"},
        PlotLabel -> "LabJack channel " <> ToString[channel] <> " intensity vs. position",
        PlotMarkers -> Automatic, GridLines -> Automatic,
        Epilog -> {Red, PointSize[0.02],
                   Point[{best[[If[zVaries && !xVaries, 2, 1]]], best[[3]]}]},
        ImageSize -> Large
    ]
];

End[]
EndPackage[]