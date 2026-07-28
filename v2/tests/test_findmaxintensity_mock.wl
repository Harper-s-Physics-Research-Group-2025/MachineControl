(* Mocked-hardware test for ServoFindMaxIntensity -- no servos, no LabJack, no laser needed.
   Block-overrides the hardware bindings with a simulated Gaussian beam and checks that the
   scan actually walks up to the peak.

   Run with:
     wolframscript.exe -file v2/tests/test_findmaxintensity_mock.wl

   The paclet still has to load (so the DLL must be present and loadable), but no call ever
   reaches real hardware. *)

PacletDirectoryLoad[FileNameJoin[{DirectoryName[$InputFileName], "..", "paclet"}]];
Needs["WolframMachineControl`"];

(* ---------- simulated hardware ---------- *)

$beamX = 3.7;      (* true peak, in mm *)
$beamZ = -12.3;
$beamWidth = 2.0;  (* Gaussian sigma, mm *)
$mockPos = {0., 0.};
$moveLog = {};
$failMoveAt = None;   (* set to {x, z} to simulate a move that fails there *)

mockSetPos[x_, z_, rpm_] := (
    If[$failMoveAt =!= None && Norm[{x, z} - $failMoveAt] < 10^-6, Return[$Failed]];
    $mockPos = {x, z};
    AppendTo[$moveLog, {x, z}];
    {x, z, rpm}
);

mockReadLabjack[channel_] := Module[{d2},
    d2 = (($mockPos[[1]] - $beamX)^2 + ($mockPos[[2]] - $beamZ)^2);
    2.5 Exp[-d2/(2 $beamWidth^2)] + RandomReal[{-0.0005, 0.0005}]  (* a little ADC noise *)
];

(* Every test runs inside this: hardware bindings swapped for the mocks above. *)
SetAttributes[withMockHardware, HoldAll];
withMockHardware[body_] := Block[
    {WolframMachineControl`ServoSetPos = mockSetPos,
     WolframMachineControl`ReadLabjack = mockReadLabjack,
     WolframMachineControl`ServoReady = (1 &),
     WolframMachineControl`ServoHomed = (1 &)},
    $mockPos = {0., 0.}; $moveLog = {};
    body
];

(* ---------- assertions ---------- *)

$failures = 0;
check[label_, cond_] := If[TrueQ[cond],
    Print["  PASS  ", label],
    $failures++; Print["  FAIL  ", label]
];

(* ---------- tests ---------- *)

Print["1D scan along X (Z held fixed at the beam's Z)"];
withMockHardware[
    Module[{r},
        r = WolframMachineControl`ServoFindMaxIntensity[0, {-5, 10, 0.5}, $beamZ,
            "SettleTime" -> 0, "SamplesPerPoint" -> 3, "RefinePasses" -> 0, "ShowPlot" -> False];
        check["returns an Association", AssociationQ[r]];
        check["X within one step of the true peak", Abs[r["Position"][[1]] - $beamX] <= 0.5];
        check["Z stayed fixed", r["Position"][[2]] == $beamZ];
        check["Z never moved during the scan", Union[$moveLog[[All, 2]]] == {$beamZ}];
        check["intensity is near the beam maximum", r["Intensity"] > 2.0];
    ]
];

Print["2D raster over X and Z"];
withMockHardware[
    Module[{r},
        r = WolframMachineControl`ServoFindMaxIntensity[0, {0, 8, 1}, {-16, -8, 1},
            "SettleTime" -> 0, "SamplesPerPoint" -> 3, "RefinePasses" -> 0, "ShowPlot" -> False];
        check["X within one step", Abs[r["Position"][[1]] - $beamX] <= 1.0];
        check["Z within one step", Abs[r["Position"][[2]] - $beamZ] <= 1.0];
        check["visited the whole 9x9 grid", Length[r["Scan"]] == 81];
    ]
];

Print["Refinement passes tighten the answer"];
withMockHardware[
    Module[{coarse, refined},
        coarse = WolframMachineControl`ServoFindMaxIntensity[0, {-5, 10, 1}, $beamZ,
            "SettleTime" -> 0, "SamplesPerPoint" -> 3, "RefinePasses" -> 0, "ShowPlot" -> False];
        refined = WolframMachineControl`ServoFindMaxIntensity[0, {-5, 10, 1}, $beamZ,
            "SettleTime" -> 0, "SamplesPerPoint" -> 3, "RefinePasses" -> 2, "ShowPlot" -> False];
        check["refined X is closer to the true peak",
            Abs[refined["Position"][[1]] - $beamX] <= Abs[coarse["Position"][[1]] - $beamX]];
        check["refined X is within 0.15 mm (vs. the 1 mm coarse step)",
            Abs[refined["Position"][[1]] - $beamX] <= 0.15];
        check["reports one count per pass", Length[refined["Passes"]] == 3];
        check["refined intensity >= coarse intensity", refined["Intensity"] >= coarse["Intensity"]];
    ]
];

Print["Both axes fixed -- degenerates to a single measurement"];
withMockHardware[
    Module[{r},
        r = WolframMachineControl`ServoFindMaxIntensity[0, 1.0, 2.0,
            "SettleTime" -> 0, "SamplesPerPoint" -> 1, "RefinePasses" -> 1, "ShowPlot" -> False];
        check["one point per pass, nothing moved", Union[$moveLog] == {{1.0, 2.0}}];
        check["position is the one we pinned", r["Position"] == {1.0, 2.0}];
    ]
];

Print["Malformed axis spec is rejected"];
withMockHardware[
    check["returns $Failed on a 2-element spec",
        Quiet[WolframMachineControl`ServoFindMaxIntensity[0, {0, 5}, 0.,
            "SettleTime" -> 0, "ShowPlot" -> False]] === $Failed]
];

Print["A failed move aborts the scan instead of recording a bogus reading"];
withMockHardware[
    Module[{r},
        $failMoveAt = {2., 0.};
        r = Quiet[WolframMachineControl`ServoFindMaxIntensity[0, {0, 6, 1}, 0.,
            "SettleTime" -> 0, "SamplesPerPoint" -> 1, "RefinePasses" -> 0, "ShowPlot" -> False]];
        $failMoveAt = None;
        check["returns $Failed", r === $Failed];
        check["stopped at the bad position", Length[$moveLog] == 2];
    ]
];

Print["Servos not homed is caught before any movement"];
Block[{WolframMachineControl`ServoReady = (1 &), WolframMachineControl`ServoHomed = (0 &),
       WolframMachineControl`ServoSetPos = mockSetPos, WolframMachineControl`ReadLabjack = mockReadLabjack},
    $moveLog = {};
    check["returns $Failed and never moves",
        Quiet[WolframMachineControl`ServoFindMaxIntensity[0, {0, 5, 1}, 0., "ShowPlot" -> False]] === $Failed
        && $moveLog === {}]
];

Print[""];
Print[If[$failures == 0, "All mock tests passed.", ToString[$failures] <> " mock test(s) FAILED."]];
