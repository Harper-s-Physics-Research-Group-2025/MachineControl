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
│   ├── Oven5R6900.h
│   ├── recorder.h               # CSV data logging + Teknic/LabJack init
│   ├── sm_homer.h                # Servo homing
│   └── sm_manual_controller.h    # Keyboard-driven manual jogging
├── WolframMachineControl/       # The Wolfram paclet (Mathematica-facing package)
│   ├── Kernel/WolframMachineControl.wl   # Public Wolfram functions, binds to DLL exports
│   └── PacletInfo.wl
└── WolframNotebooks/            # Interactive notebooks for manual testing
    ├── TestMachineControl.nb
    └── Record.nb
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
  `sFoundation20.dll` into `WolframMachineControl/LibraryResources/<platform>/` for you — you do
  not need to move any files by hand.

If CMake can't find Wolfram, LabJack, or Teknic on its own (for example, if you installed one of
them to a non-default location), open `CMakeLists.txt` and add your install path to the `HINTS`
list for the relevant `find_path`/`find_library` call.

### Step 3: Load the library in Mathematica

Open a new Mathematica notebook and run:

```mathematica
PacletDirectoryLoad["<full path to>/v2/WolframMachineControl"]
Needs["WolframMachineControl`"]
```

This tells Mathematica where the paclet lives and loads its functions into your session. The
package finds the DLL you just built automatically — there is nothing else to configure.

### Step 4: Try the test notebook first

Before writing your own code, open `WolframNotebooks/TestMachineControl.nb` and run through it.
It exercises every function against the real hardware, so it's the fastest way to confirm your
COM ports and device connections are correct before you build anything on top.

### Step 5: Control the hardware

Once the test notebook works, you can call the same functions from your own notebook:

```mathematica
(* Fluid bath *)
BathInit[];                       (* auto-detects and connects to the bath's COM port *)
BathOn[];
BathSetTemp[37.0];
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
(`WolframMachineControl/Kernel/WolframMachineControl.wl`) wraps each one in a public Wolfram symbol:

### Logging
- `GetLogStatus[]`, `GetLogFile[]`, `SetLogSettings[status, logfile]`

### Fluid Bath
- `BathInit[]` — auto-detects and connects to the bath's COM port (see [Auto-Detected COM Ports](#auto-detected-com-ports))
- `BathOn[]`, `BathOff[]`, `BathManual[]`
- `BathGetTemp[]`, `BathGetSetpoint[]`, `BathSetTemp[temp]`

### Temperature Controller
- `TempCtrlInit[]` — auto-detects and connects to the temp controller's COM port (see [Auto-Detected COM Ports](#auto-detected-com-ports))
- `TempCtrlOn[]`, `TempCtrlOff[]`
- `TempCtrlGetMode[]`, `TempCtrlSetMode[mode]` (0 = normal, 2 = ramp/soak)
- `TempCtrlGetTemp[]`, `TempCtrlGetSetpoint[]`, `TempCtrlSetSetpoint[temp]`

### Data Acquisition
- `ReadLabjack[channel]`

### Servo Motors
- `ServoEnable[]`, `ServoDisable[]`, `ServoGetAlerts[]`
- `ServoHome[milliseconds]`, `ServoHomed[]`, `ServoReady[]`
- `ServoGetPos[]` — returns `{x_mm, z_mm}`
- `ServoSetPos[x_mm, z_mm, rpm]` — returns updated `{x_mm, z_mm}`
- `ServoManualControl[]` — keyboard-driven manual jogging

## Auto-Detected COM Ports

`BathInit[]` and `TempCtrlInit[]` no longer take a COM port string — both devices connect through
identical Prolific USB-to-Serial adapters (`VID_067B&PID_2303`), so the COM number these get
assigned by Windows can (and does) drift across replugs/reboots.

Instead, `find_prolific_ports()` (in `src/lab.cpp`) enumerates every COM port backed by that
Prolific hardware ID. Since the hardware ID alone can't tell the bath and the temp controller
apart, `find_bath_port()` and `find_temp_controller_port()` disambiguate by protocol probing: each
candidate port is opened and sent a harmless, read-only "get setpoint" query in the target
device's own serial protocol (RTE7 for the bath, Oven5R6900 for the temp controller). Whichever
port replies with a valid, checksummed response is that device — this keeps working even if the
two adapters get swapped into different physical USB ports.

The Teknic servo hub and the LabJack ADC already auto-detect through their respective SDKs and
never required a COM port argument.

## Testing

- `WolframMachineControl/Tests/Test_WolframMachineControl.wlt` — Wolfram-side unit tests
- `WolframNotebooks/TestMachineControl.nb`, `WolframNotebooks/Record.nb` — interactive/manual testing notebooks

## Development Notes

- All DLL functions return integer error codes unless otherwise noted (see `LIBRARY_NO_ERROR` / `LIBRARY_FUNCTION_ERROR` in `wolfram_api.cpp`)
- Bath and temp controller COM ports are auto-detected — see [Auto-Detected COM Ports](#auto-detected-com-ports)
- Positions are in millimeters, temperatures in Celsius

## Authors

- Josh Darrow
- Samuel Ntadom
