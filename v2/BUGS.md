# Known Bugs

Found via static read-through of the C++ source (not build-verified — this is Windows-only
code requiring the Wolfram/LabJack/Teknic SDKs, none of which are available on this machine).

## High severity

### 1. Null-pointer crash if a device function is called before its init function
`src/lab.cpp:126-131` (bath) and `:158-164` (temp controller) dereference the `bath`/`tc`
pointers with no null check:
```cpp
int bath_on() { return !bath->turn_on(); }
```
If `BathOn["COM3"]` is called before `BathInit["COM3"]` (or after `DeleteBath[]`), this is a
null-pointer dereference that crashes the whole Mathematica kernel — not a catchable Wolfram
error. `BathInit`/`TempCtrlInit` exist in `WolframMachineControl.wl` but have no `::usage`
string, so they're easy to forget to call first.

### 2. `ServoReady[]` checks the wrong thing
`src/wolfram_api.cpp:302-306` — the function bound to `ServoReady[]` (`wmotors_ready`) calls
`Lab::servos_homed()`, not `Lab::servos_ready()`. It's an exact duplicate of `ServoHomed[]`.
The actual readiness check (motors enabled, no alerts — `Lab::servos_ready()`) is only wired
to `wservo_hardware_online`, which is never bound to any Wolfram symbol in the `.wl` file at
all. There is currently no way to query "are the motors enabled and alert-free" from
Mathematica.

### 3. `ServoHomed[]` / `ServoReady[]` throw instead of returning `False`
`src/wolfram_api.cpp:302-313` — both functions return `!return_code` as the LibraryLink status
code, where `return_code` is the actual boolean (0 = not homed, 1 = homed):
```cpp
int return_code = Lab::servos_homed();
MArgument_setInteger(Res, static_cast<mint>(return_code));
return !return_code;   // not homed (0) -> returns 1 = LIBRARY_FUNCTION_ERROR
```
Whenever the motors are NOT homed, this returns `LIBRARY_FUNCTION_ERROR`, so Mathematica
treats the call as a failure (`$Failed`) instead of delivering the boolean `False`. Checking
homing status before homing looks like a crash, not a clean check.

### 4. Device "init" can silently report success when the COM port never opened
`src/lab.cpp:104-111` and `:140-144`:
```cpp
int init_bath(std::string COMM) {
    if (bath != nullptr) { delete bath; }
    bath = new RTE7(COMM);
    return (bath == nullptr);   // always false — `new` throws on failure, never returns null
}
```
`RTE7`'s constructor only logs to `cerr` if the serial port fails to open; it doesn't throw.
So `BathInit["COM3"]` reports success (return code 0) even when the port never opened, and
every subsequent read/write just fails with a generic "Read failed" instead of ever surfacing
the real cause.

## Lower severity

### 5. Partial re-init leaves inconsistent global state
`src/lab.cpp:203-243`, `initialize_servos()` — if `FindComHubPorts` finds zero or more than one
COM hub, the function returns early without resetting `Mgr` (already set to the SysManager
singleton) or calling `shutdown_servos()`. `Mgr` ends up non-null while `Port`/`motorX`/`motorZ`
stay null — a partially-initialized state that isn't guarded consistently elsewhere.

### 6. `read_labjack_ain` closes *all* LabJack handles, not just its own
`src/lab.cpp:172-191` — calls the LabJack UD `Close()` function, which per the UD API closes
every open LabJack device handle process-wide (it takes no handle argument). Harmless with a
single device connected, but a landmine if multiple LabJack devices are ever used. The `ePut`
configuration calls' return codes are also unchecked.

### 7. Uninitialized read in a log line
`src/wolfram_api.cpp:268-271`, `wservos_get_position` — `float x_mm, z_mm;` are declared and
then logged (inside `Lab::servos_get_position`) before being assigned. Reads uninitialized
stack memory; cosmetic only (garbage values in the log), but technically undefined behavior.

### 8. Ramp/soak getter and setter command IDs disagree
`src/Oven5R6900.cpp` — `get_soak_temp` (and `get_ramp_duration`, `get_soak_duration`,
`get_num_repeats`, `get_next_sequence_num`) compute the command byte as `hex(8 + seq)` and then
prepend another literal `"8"`, giving two-character command IDs like `"88".."8f"`. Their setter
counterparts (`set_soak_temp`, etc.) just prepend `"8"` directly to the raw `sequence` string
with no `+8` offset. For the same logical sequence index, the getter and setter would hit
different command bytes on the device. **Currently unreachable** — the whole ramp/soak surface
is commented out in `wolfram_api.cpp`, so this isn't exposed to Mathematica today, but it will
bite whoever wires ramp/soak support back in.

### 9. Dead declarations
`include/controls/Oven5R6900.h` declares private methods `read_temp`, `read_setpoint`, and
`read_ack` that are never defined in `Oven5R6900.cpp` (the getters call `dispatch_message`
directly instead). Not a compile error since they're never referenced, just vestigial cruft
left over from an earlier design.

## Not yet checked
- `sm_homer.h`, `sm_manual_controller.h`, `recorder.h`/`recorder.cpp` — not part of the current
  build (`recorder.cpp` is present in `src/` but not listed in `CMakeLists.txt`'s `add_library`
  call, and `sm_homer.cpp`/`sm_manual_controller.cpp` don't exist at all despite being
  referenced, commented out, in `CMakeLists.txt`). `lab.cpp` reimplements equivalent homing and
  manual-control logic directly rather than using these classes, so they're effectively dead
  code in the current build.
- No actual compile/run was performed (Windows-only toolchain, SDKs not available here).
