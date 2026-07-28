# CLAUDE.md — MachineControl v2

## What this is

Code to control the **Josh 2.0** machine: an automated system for X-ray lipid
analysis built for the Harper lab (Calvin University). The machine has a
temperature-controlled sample holder, servo-driven positioning, and optical
sensors.

**v1** (in `../v1`) exposed control as separate standalone executables.
**v2 (this directory, in progress)** replaces those executables with a single
C++ **LibraryLink DLL** that Mathematica loads and calls directly, so the whole
machine can be driven from a Wolfram notebook.

## Architecture (how a call flows)

```
Mathematica notebook / .wl paclet   ← user-facing Wolfram functions
        │  LibraryFunctionLoad
        ▼
src/wolfram_api.cpp                  ← extern "C" DLLEXPORT wrappers (the DLL entry points)
        │  calls
        ▼
Lab:: namespace  (include/controls/lab.h, src/lab.cpp)   ← business logic, owns device state
        │  drives
        ▼
Per-device classes: RTE7, Oven5R6900, Homer, ManualController, Recorder
        │
        ▼
Vendor SDKs: Teknic sFoundation (servos), LabJack UD (ADC), Win32 COM ports
```

## File & folder guide

### `src/` — C++ implementation
- `wolfram_api.cpp` — **the DLL boundary.** `extern "C"` functions (`WolframLibrary_initialize`, `wget_logging_status`, etc.) that Mathematica calls. Thin wrappers that unpack `MArgument`s and forward to `Lab::`.
- `wolfram_api.new.cpp` — work-in-progress/alternate version of the above.
- `lab.cpp` — implements the `Lab::` namespace: the coordinating logic that holds global device handles and calls into the device classes.
- `RTE7.cpp` — Neslab RTE7 water-bath controller (serial COM).
- `Oven5R6900.cpp` — Oven Industries 5R6-900 thermoelectric temperature controller (serial COM, incl. ramp/soak modes).

### `include/controls/` — headers (one per device + the coordinator)
- `lab.h` — declares the whole `Lab::` API surface: logging, bath, temp controller, LabJack read, and servo functions. Start here to see everything the DLL can do.
- `RTE7.h` — `RTE7` class: bath on/off/manual, get/set temp & setpoint.
- `Oven5R6900.h` — `Oven5R6900` class: enable H-bridge output, get/set temp, mode, setpoint.
- `recorder.h` — `Recorder` class: logs timestamped bath temp / controller temp / LabJack voltage to CSV; also inits Teknic motors and LabJack.
- `sm_homer.h` — `Homer` class: homes the Teknic servo motors (X and Z) with a timeout.
- `sm_manual_controller.h` — `ManualController` class: keyboard-driven manual jogging of the servos (Win32 keyboard hook, emergency stop).

### `WolframMachineControl/` — the Wolfram paclet (Mathematica-side package)
- `Kernel/WolframMachineControl.wl` — public Wolfram functions (`BathOn`, `BathSetTemp`, `TempCtrlOn`, `ServoHome`, `ServoSetPos`, `ReadLabjack`, …). Locates the DLL via `FindLibrary`, loads the Teknic `sFoundation20.dll`, and binds each Wolfram symbol to a DLL export. This is the interface a user actually calls.
- `PacletInfo.wl` — paclet manifest (name, version 0.1.0, contexts).
- `Tests/Test_WolframMachineControl.wlt` — Wolfram unit test file.

### `WolframNotebooks/` — interactive notebooks
- `TestMachineControl.nb` — manual/exploratory testing of the loaded library.
- `Record.nb` — driving/using the data recorder.

### `algorithms/tests/` — C++ test scaffolding
- `test_main.cpp` — GoogleTest suite skeleton (tests are stubs, mostly TODO).
- `TESTS_TODO.md` — planned test coverage.
- `CMakeLists.txt` — test build config.
- `notebooks/library/test_interface.nb` — notebook for exercising the library.

### Build & docs (repo/build plumbing)
- `CMakeLists.txt` — root build config. Finds Wolfram/LabJack/Teknic headers, sets C++17, builds the DLL.
- `build.ps1` — direct MSVC (`cl.exe`) build script; the practical way the DLL currently gets compiled on Windows. Note: hardcoded paths (Wolfram 14.2, `C:\Projects\...`).
- `README.md` — fuller project docs: dependencies, build steps, full API list. Some paths/names are aspirational and drift from the current tree.
- `structure.md` — an earlier idealized layout sketch (uses placeholder names like `my_project`; **not** the current real structure — trust this file instead).
- `BUILD_FAILURE_ANALYSIS.md` — log of compile errors hit while getting the DLL to build and how each was fixed (C++17, linking, etc.). Useful when the build breaks.
- `notes` — dated dev journal (CMake setup, dependency linking progress).
- `log.txt` — runtime log output written by the `Lab::log` logging subsystem.

## Naming note

Docs/paths use several names for the same DLL: **"Josh 2.0" / Josh_Controller /
LabController.dll / wolfram_machine_controller / WolframMachineControl**. They
refer to the same LibraryLink library.

## Platform

Windows-only (Win32 serial + `windows.h`, `conio.h`), C++17, MSVC, Mathematica
13.0+. Depends on Teknic sFoundation, LabJack UD driver, and the Wolfram LibraryLink headers.
