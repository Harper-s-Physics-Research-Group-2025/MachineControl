(* ::Package:: *)

VerificationTest[
	PacletDirectoryLoad["C:\\Users\\Student\\Desktop\\machine_controller\\MachineControl\\v2"];
    Needs["WolframMachineControl`"];
    NameQ["WolframMachineControl`*"],
    True,
    TestID -> "Package-Load-Verification"
]


(* ==========================================================================
	0. Global Variables
   ========================================================================*)
LogFile = "C:\\Users\\Student\\Desktop\\machine_controller\\MachineControl\\v2\\log.txt"




(* ==========================================================================
   1. LOGGING INFRASTRUCTURE TESTS
   ========================================================================== *)

VerificationTest[
    WolframMachineControl`SetLogSettings[1, LogFile],
    1,
    TestID -> "WolframMachineControl`SetLogSettings[1, " <> LogFile <> "]"
]

VerificationTest[
    WolframMachineControl`GetLogStatus[],
    1,
    TestID -> "WolframMachineControl`GetLogStatus[]"
]

VerificationTest[
    WolframMachineControl`GetLogFile[],
    LogFile,
    TestID -> "WolframMachineControl`GetLogFile[]"
]


(* ==========================================================================
   2. RECIRCULATING FLUID BATH TESTS
   ========================================================================== *)

VerificationTest[
    WolframMachineControl`BathInit[],
    0,
    TestID -> "BathInit[]"
]

VerificationTest[
    WolframMachineControl`BathOn[],
    0,
    TestID -> "BathOn[]"
]

VerificationTest[
    MatchQ[WolframMachineControl`BathGetTemp[], _Real],
    True,
    TestID -> "MatchQ[BathGetTemp[], _Real]"
]

VerificationTest[
    WolframMachineControl`BathSetSetpoint[25.0],
    25.0,
    TestID -> "BathSetSetpoint[25.0]"
]

VerificationTest[
    WolframMachineControl`BathGetSetpoint[],
    25.0,
    TestID -> "BathGetSetpoint[]"
]

(* Hard to test this without the next test failing.
VerificationTest[
    WolframMachineControl`BathManual[],
    0,
    TestID -> "BathManual[]"
] *)

Pause[3]   (* Pause 2 seconds before turning off *)

VerificationTest[
    WolframMachineControl`BathOff[],
    0,
    TestID -> "BathOff[]"
]

VerificationTest[
    WolframMachineControl`DeleteBath[],
    0,
    TestID -> "DeleteBath[]"
]



(* ==========================================================================
   3. THERMOELECTRIC/TRANSISTOR H-BRIDGE TESTS
   ========================================================================== *)

VerificationTest[
    WolframMachineControl`TempCtrlInit[],
    0,
    TestID -> "TempCtrlInit[]"
]

VerificationTest[
    WolframMachineControl`TempCtrlOn[],
    0,
    TestID -> "TempCtrlOn[]"
]

VerificationTest[
    WolframMachineControl`TempCtrlSetMode[0],
    0,
    TestID -> "TempCtrlSetMode[0]"
]

VerificationTest[
    WolframMachineControl`TempCtrlGetMode[],
    0,
    TestID -> "TempCtrlGetMode[]"
]

VerificationTest[
    MatchQ[WolframMachineControl`TempCtrlGetTemp[], _Real],
    True,
    TestID -> "MatchQ[TempCtrlGetTemp[], _Real]"
]

VerificationTest[
    WolframMachineControl`TempCtrlSetSetpoint[37.5],
    37.5,
    TestID -> "TempCtrlSetSetpoint[37.5]"
]

VerificationTest[
    WolframMachineControl`TempCtrlGetSetpoint[],
    37.5,
    TestID -> "TempCtrlSetSetpoint[37.5]"
]



VerificationTest[
    WolframMachineControl`TempCtrlOff[],
    0,
    TestID -> "TempCtrlOff[]"
]

VerificationTest[
    WolframMachineControl`DeleteTempCtrl[],
    0,
    TestID -> "DeleteTempCtrl[]"
]



(* ==========================================================================
   4. LABJACK USB DATA ACQUISITION TESTS
   ========================================================================== *)

VerificationTest[
    MatchQ[WolframMachineControl`ReadLabjack[0], _Real],
    True,
    TestID -> "LabJack-Read-AIN0"
]


(* ==========================================================================
   5. TEKNIC CLEARPATH SERVO TESTS
   ========================================================================== *)

VerificationTest[
    WolframMachineControl`ServoEnable[],
    0,
    TestID -> "ServoEnable[]"
]

VerificationTest[
    WolframMachineControl`ServoReady[],
    1,
    TestID -> "ServosReady[]"
]

VerificationTest[
    WolframMachineControl`ServoHome[30000], (* 10 second timeout limit *)
    0,
    TestID -> "ServoHome[30000]"
]

VerificationTest[
    WolframMachineControl`ServoHomed[], 
    1,
    TestID -> "ServoHomed[]"
]

VerificationTest[
    Round[WolframMachineControl`ServoSetPos[10.0, -20.0, 100]],
    {10, -20, 100},
    TestID -> "ServoSetPos[10, -20, 100]"
]

VerificationTest[
    Round[WolframMachineControl`ServoGetPos[]],
    {10, -20},
    TestID -> "Round[WolframMachineControl`ServoGetPos[]]"
]

VerificationTest[
    MatchQ[WolframMachineControl`ServoGetAlerts[], _String], 
    True,
    TestID -> "MatchQ[WolframMachineControl`ServoGetAlerts[], _UTF8String[String]]"
]

VerificationTest[
    WolframMachineControl`ServoDisable[],
    0,
    TestID -> "ServoDisable[]"
]
