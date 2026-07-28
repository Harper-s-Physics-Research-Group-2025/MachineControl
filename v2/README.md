# Josh 2.0 Lab Equipment Controller — Wolfram LibraryLink DLL

A C++ LibraryLink DLL, loaded directly by Mathematica, for controlling the Josh 2.0
automated X-ray lipid analysis system (Harper lab, Calvin University).

## Project Overview

This library gives Mathematica direct control over the Josh 2.0 hardware:
- **Fluid bath**: Neslab RTE7 water bath (serial COM)
- **Temperature controller**: Oven Industries 5R6-900 thermoelectric controller, incl. ramp/soak modes (serial COM)
- **Servo motors**: Teknic ClearPath SC motors (X/Z sample positioning), via the Teknic sFoundation SDK
- **Data acquisition**: LabJack analog-to-digital converter

## Project Structure

```
v2/
├── CMakeLists.txt              # Build config: finds Wolfram/LabJack/Teknic, builds the DLL
├── src/                        # C++ implementation
│   ├── wolfram_api.cpp         # DLL boundary — extern "C" functions Mathematica calls
│   ├── lab.cpp                 # Lab:: namespace — coordinating logic, owns device state
│   ├── RTE7.cpp                # Neslab RTE7 water bath
│   └── Oven5R6900.cpp          # Oven Industries 5R6-900 temp controller
├── include/controls/            # Headers — one per device + the coordinator
│   ├── lab.h                    # Full Lab:: API surface
│   ├── RTE7.h
│   └── Oven5R6900.h
├── paclet/                      # The Wolfram paclet (Mathematica-facing package)
│   ├── Kernel/WolframMachineControl.wl   # Public Wolfram functions, binds to DLL exports
│   └── PacletInfo.wl
├── notebooks/            # Interactive notebooks for manual testing
│   ├── Experimentation.nb
│   ├── TestMachineControl.nb
│   └── Record.nb
└── data/                 # CSVs written by LabJackRecordData[] / read by LabJackPlotData[]
```

## Getting Started (New User)

This walks through everything needed to go from a fresh clone to controlling hardware from a
Mathematica notebook.

### Step 1: Install the required software (Windows)

1. **Mathematica 13.0 or later** — this gives you the LibraryLink headers the DLL needs to compile.
2. **Visual Studio** — install with the "Desktop development with C++" workload. This gives you
   the MSVC compiler (with C++17 support), which is what actually builds the DLL.
3. **CMake 3.15 or later** — https://cmake.org/download/ — this is what drives the build.
4. **LabJack UD Driver** — https://labjack.com/support/software/installers/ud
   - After installing, confirm `LabJackUD.lib` exists under `Drivers/64bit/`. There's also a 32-bit
     copy in the parent `Drivers/` folder — make sure you're pointed at the 64-bit one, or the
     build will link but fail at runtime.
5. **Teknic ClearView SDK** — provides `sFoundation20.lib` and `sFoundation20.dll`, needed for the
   servo motors.

### Step 2: Get the code and build the DLL

```bash
git clone <repo-url>
cd MachineControl/v2
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

What this does:
- `cmake ..` reads `CMakeLists.txt`, which automatically searches your machine for the Wolfram,
  LabJack, and Teknic install locations.
- `cmake --build .` compiles the four `.cpp` files in `src/` into one DLL.
- If everything is found correctly, CMake **automatically copies** the finished DLL and
  `sFoundation20.dll` into `paclet/LibraryResources/<platform>/` for you — you do
  not need to move any files by hand.

If CMake can't find Wolfram, LabJack, or Teknic on its own (for example, if you installed one of
them to a non-default location), open `CMakeLists.txt` and add your install path to the `HINTS`
list for the relevant `find_path`/`find_library` call.

### Step 3: Load the library in Mathematica

If your notebook is saved somewhere under `v2/` (e.g. in `notebooks/`, like the provided test
notebook), it can find the paclet on its own — no hardcoded path needed:

```mathematica
PacletDirectoryLoad[ParentDirectory[NotebookDirectory[]]]
Needs["WolframMachineControl`"]
```

This resolves relative to wherever the notebook file itself lives, so it keeps working even if
this whole `v2/` folder gets moved or copied elsewhere (see `specs/portable-paths.md` for why that
matters). If you're working from a brand-new, not-yet-saved notebook, `NotebookDirectory[]` has
nothing to resolve yet — use an explicit path instead:

```mathematica
PacletDirectoryLoad["<full path to>/v2"]
Needs["WolframMachineControl`"]
```

Either way, this tells Mathematica where the paclet lives and loads its functions into your
session. The package finds the DLL you just built automatically — there is nothing else to
configure.

### Step 4: Try the test notebook first

Before writing your own code, open `notebooks/TestMachineControl.nb` and run through it.
It exercises every function against the real hardware, so it's the fastest way to confirm your
COM ports and device connections are correct before you build anything on top.

### Step 5: Control the hardware

Once the test notebook works, you can call the same functions from your own notebook:

```mathematica
(* Fluid bath *)
BathInit[];                       (* auto-detects and connects to the bath's COM port *)
BathOn[];
BathSetSetpoint[37.0];
temp = BathGetTemp[];
BathOff[];

(* Temperature controller *)
TempCtrlInit[];                   (* auto-detects and connects to the temp controller's COM port *)
TempCtrlOn[];
TempCtrlSetSetpoint[25.0];

(* Servo motors *)
ServoEnable[];
ServoHome[5000];                  (* home, with a 5-second timeout *)
ServoSetPos[10.0, 5.0, 100.0];    (* move to x=10mm, z=5mm, at 100 RPM *)
pos = ServoGetPos[];              (* returns {x_mm, z_mm} *)

(* LabJack *)
voltage = ReadLabjack[0];         (* read channel 0 *)

(* LabJack data-collection suite -- records channels 0-7 + temp controller temperature
   every 5s until the temp controller actually reaches 50C, then saves/lists/plots the CSV *)
LabJackRecordData["run1", 50.0, 5];
LabJackListCSVs[]
LabJackPlotData["run1", 0]        (* plot channel 0 *)
```

Every function above is documented in more detail in the [API Functions](#api-functions) section below.

## Dependencies

### System Requirements
- Windows
- Mathematica 13.0 or later
- MSVC (Visual Studio) with C++17 support
- CMake 3.15 or later

### External Libraries
- **Wolfram LibraryLink headers**: included with Mathematica (`SystemFiles/IncludeFiles/C`)
- **LabJack UD Driver**: https://labjack.com/support/software/installers/ud (use the 64-bit lib under `Drivers/64bit/`)
- **Teknic ClearView SDK**: `sFoundation20.lib`/`.dll` (servo motor control)

## API Functions

The C++ DLL exports low-level `w*`-prefixed functions (see `src/wolfram_api.cpp`); the paclet
(`paclet/Kernel/WolframMachineControl.wl`) wraps each one in a public Wolfram symbol. For exactly
what each function returns on success and on failure (including a few sharp edges — see
[Known Confusing Error Messages](#known-confusing-error-messages) below), see
[docs/API_REFERENCE.md](docs/API_REFERENCE.md).

### Logging
- `GetLogStatus[]`, `GetLogFile[]`, `SetLogSettings[status, logfile]`

### Fluid Bath
- `BathInit[]` — auto-detects and connects to the bath's COM port (see [Auto-Detected COM Ports](#auto-detected-com-ports))
- `BathOn[]`, `BathOff[]`, `BathManual[]`
- `BathGetTemp[]`, `BathGetSetpoint[]`, `BathSetSetpoint[temp]`

### Temperature Controller
- `TempCtrlInit[]` — auto-detects and connects to the temp controller's COM port (see [Auto-Detected COM Ports](#auto-detected-com-ports))
- `TempCtrlOn[]`, `TempCtrlOff[]`
- `TempCtrlGetMode[]`, `TempCtrlSetMode[mode]` (0 = normal, 2 = ramp/soak)
- `TempCtrlGetTemp[]`, `TempCtrlGetSetpoint[]`, `TempCtrlSetSetpoint[temp]`
- `TempCtrlPlotTemp[targetTemp, interval:1]` — sets the temp controller's setpoint to `targetTemp`,
  samples its temperature every `interval` seconds (default 1) until it arrives, then plots
  temperature vs. time.

### Data Acquisition
- `ReadLabjack[channel]`
- `LabJackRecordData[filename, finalTemp, interval]` — heats/cools the temp controller to
  `finalTemp`, logging channels 0-7 and its temperature every `interval` seconds until it arrives.
  Saves to `v2/data/<filename>.csv` and returns the path. Argument meanings in
  [docs/API_REFERENCE.md](docs/API_REFERENCE.md).
- `LabJackListCSVs[]` — lists every CSV in `v2/data`, newest first.
- `LabJackPlotData[filename, channel]` — plots the given channel from `<filename>.csv` against
  the temp controller temperature.
- `LabJackTempSweep[startTemp, temps, lipidName, waterConcentration, interval:1]` — repeatedly
  returns to `startTemp` and drives out to each temperature in `temps`, recording (via
  `LabJackRecordData`) both legs of every round trip, so `N` temperatures produce `2N` CSVs.

### Servo Motors
- `ServoEnable[]`, `ServoDisable[]`, `ServoGetAlerts[]`
- `ServoHome[milliseconds]`, `ServoHomed[]`, `ServoReady[]`
- `ServoGetPos[]` — returns `{x_mm, z_mm}`
- `ServoSetPos[x_mm, z_mm, rpm]` — returns updated `{x_mm, z_mm}`
- `ServoManualControl[]` — keyboard-driven manual jogging
- `ServoFindMaxIntensity[channel, xSpec, zSpec, opts]` — **finds the point of highest light
  intensity.** Rasters the sample holder over the region given by `xSpec`/`zSpec` (each either
  `{start, end, step}` in mm, or a plain number to hold that axis fixed), averages several
  `ReadLabjack[channel]` readings at every stop, and returns the `{x_mm, z_mm}` where the
  detector voltage peaked — then re-scans a tighter window around it to sharpen the answer.
  Requires `ServoEnable[]` + `ServoHome[...]` first. Options and caveats in
  [docs/API_REFERENCE.md](docs/API_REFERENCE.md#finding-the-point-of-highest-light-intensity).

  ```wolfram
  ServoEnable[]; ServoHome[30000];
  peak = ServoFindMaxIntensity[0, {0, 10, 1}, {-20, -10, 1}];
  peak["Position"]    (* -> {x_mm, z_mm} of maximum intensity *)
  ```

## Known Confusing Error Messages

Two error patterns look scary but usually mean something mundane. Both are `LibraryFunctionError`
formatting quirks, not real crashes — check these first before assuming something is broken.

### `LibraryFunctionError[LIBRARY_TYPE_ERROR, 1]` / "inconsistent types was encountered"

This almost always means **a function failed for an ordinary reason** (not initialized, hardware
didn't respond, bad input) — it does *not* mean a real type mismatch. Most low-level DLL wrappers
in `src/wolfram_api.cpp` return their `Lab::` function's result directly as the LibraryLink status
code:

```cpp
DLLEXPORT int wbath_on(...) { return Lab::bath_on(); }
```

`Lab::bath_on()` (and most of its siblings) return `1` for "didn't work" — but `1` is also the
numeric value of Wolfram's own `LIBRARY_TYPE_ERROR` (see `WolframLibrary.h`). Mathematica has no
way to tell those apart, so it prints "inconsistent types" for what is actually just a plain
failure. The most common cause by far: **calling a device function before its `*Init[]`/
`ServoEnable[]` call**, e.g. `BathOn[]` before `BathInit[]` — the underlying pointer is still
null, the function correctly detects that and returns failure, and that failure gets mislabeled.

**Affected** (share this exact pattern — check that you called the right `Init`/`Enable` first):
all `Bath*`/`TempCtrl*` functions, `SetLogSettings`, `ServoGetAlerts[]`, `ServoHome[]`,
`ServoGetPos[]`, `ServoSetPos[]`.

**Not affected** (already return a proper `True`/`False` without this collision):
`ServoReady[]`, `ServoHomed[]`, `DeleteBath[]`, `DeleteTempCtrl[]`, `ServoDisable[]`,
`GetLogStatus[]`, `GetLogFile[]`.

**One extra oddity:** `ServoEnable[]` (`initialize_servos()`) can also return `-1` on an SDK
exception — a negative number that doesn't correspond to *any* named `LIBRARY_*_ERROR` constant,
so its on-screen presentation is unpredictable. Same underlying issue, worth knowing about
separately if `ServoEnable[]` fails in a way that looks different from the usual "inconsistent
types" message.

This is tracked as `docs/BUGS.md` #23 (not yet fixed) — the real fix is mapping these functions'
failures to `LIBRARY_FUNCTION_ERROR` instead of a raw `1`, so the message actually says "function
failed" instead of "inconsistent types."

### `LibraryFunction::cfct` / "the number of arguments N does not match the length M of the argument template"

A completely different, unrelated cause: you called a function with the wrong number of
arguments for what it's currently bound to in `paclet/Kernel/WolframMachineControl.wl`. This is
Wolfram checking your call *before* it ever reaches our C++ code, not something our error-handling
can affect. Check the function's actual signature in the [API Functions](#api-functions) section
above — e.g. `BathInit[]`/`TempCtrlInit[]` take **zero** arguments (they auto-detect their own
port now), so `BathInit["COM5"]` throws exactly this error.

## Auto-Detected COM Ports

`BathInit[]` and `TempCtrlInit[]` take no COM port argument — both devices auto-detect their
own port, the same way the servo hub and LabJack ADC always have. See
[docs/HARDWARE_COMMS.md](docs/HARDWARE_COMMS.md) for how every device talks to this app, or
[specs/port-autodetect.md](specs/port-autodetect.md) for exactly how the auto-detection works.

## Testing

- `paclet/Tests/Test_WolframMachineControl.wlt` — Wolfram-side tests. Exercises the
  real DLL end-to-end and needs actual hardware connected.
- `notebooks/TestMachineControl.nb`, `notebooks/Record.nb` — interactive/manual testing notebooks
- `tests/test_protocol_parsing.cpp` — C++ unit tests (using [doctest](https://github.com/doctest/doctest))
  for the pure checksum/parsing logic in `RTE7`/`Oven5R6900`. No hardware needed — build and run with:
  ```bash
  cmake --build build --config Release --target protocol_tests
  build/Release/protocol_tests.exe
  ```
  This only covers logic that doesn't touch the serial port (checksums, byte/response parsing) —
  the actual serial I/O still needs either real hardware or the `.wlt` suite above.

## Development Notes

- All DLL functions return integer error codes unless otherwise noted (see `LIBRARY_NO_ERROR` / `LIBRARY_FUNCTION_ERROR` in `wolfram_api.cpp`)
- Bath and temp controller COM ports are auto-detected — see [Auto-Detected COM Ports](#auto-detected-com-ports)
- Positions are in millimeters, temperatures in Celsius
- This project is Windows-only today — see [docs/HARDWARE_COMMS.md](docs/HARDWARE_COMMS.md#does-any-of-this-work-on-mac-or-linux)
  for exactly what a Mac/Linux port would require

## Documentation

Beyond this README:

| Doc | What it covers |
|---|---|
| [docs/HARDWARE_COMMS.md](docs/HARDWARE_COMMS.md) | How each device (bath, temp controller, servos, LabJack) actually talks to this app, and Mac/Linux support |
| [docs/API_REFERENCE.md](docs/API_REFERENCE.md) | What every public function returns, on both success and failure |
| [docs/BUGS.md](docs/BUGS.md) | Known bugs found by code review, with file/line references |
| [specs/librarylink.md](specs/librarylink.md) | How Mathematica calls into the C++ DLL, and what to touch when adding a new function |
| [specs/port-autodetect.md](specs/port-autodetect.md) | How `BathInit[]`/`TempCtrlInit[]` find their own COM port (includes a line-by-line walkthrough of the Windows API calls involved) |
| [specs/portable-paths.md](specs/portable-paths.md) | How the notebooks find their own file paths automatically instead of hardcoding them |


## Authors

- Josh Darrow
- Samuel Ntadom
