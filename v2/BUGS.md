# Known Bugs

Originally found via static read-through of the C++ source. All bugs below have since been
fixed and the DLL rebuilds clean (`cmake --build . --config Release`) after each fix, but none
of these fixes have been exercised against the real hardware yet — only compiled.

## High severity

### 1. ~~Null-pointer crash if a device function is called before its init function~~ — Fixed
`Lab::bath_*()`/`Lab::temperature_control_*()` (`src/lab.cpp`) now check `bath`/`tc` for null
before dereferencing and return `1` instead of crashing.

### 2. ~~`ServoReady[]` checks the wrong thing~~ — Fixed
`ServoReady[]` (`WolframMachineControl.wl`) is now bound to `wservo_hardware_online`
(`Lab::servos_ready()` — enabled + alert-free), not the homing check. The redundant
`wmotors_ready` wrapper was removed from `src/wolfram_api.cpp`.

### 3. ~~`ServoHomed[]` / `ServoReady[]` throw instead of returning `False`~~ — Fixed
`wservos_homed` (`src/wolfram_api.cpp`) now always returns `LIBRARY_NO_ERROR`; the actual
true/false answer only goes through `Res`, never the status code. `wservo_hardware_online`
already did this correctly.

### 4. ~~Device "init" can silently report success when the COM port never opened~~ — Fixed
`init_bath()`/`init_temp_controller()` (`src/lab.cpp`) now check `is_connected()` after
constructing the device object (the constructor only logs to `cerr` on failure, it doesn't
throw) and return failure — cleaning up the half-open object — instead of reporting success.

## Lower severity

### 5. ~~Partial re-init leaves inconsistent global state~~ — Fixed
`initialize_servos()` (`src/lab.cpp`) now calls `shutdown_servos()` before returning on the
"found zero or more than one COM hub" path, matching the cleanup already done in the `catch`
blocks below it.

### 6. `read_labjack_ain` closes *all* LabJack handles, not just its own — partially fixed
`src/lab.cpp`, `read_labjack_ain` — the LabJack UD API's `Close()` genuinely takes no handle
argument (confirmed in the vendor's `LabJackUD.h`) and closes every open device process-wide;
there is no per-handle close in this legacy API, so this half of the bug isn't fixable at the
call site — just documented in a comment. **Fixed:** the `ePut` configuration calls' return
codes are now checked and propagated instead of silently ignored.

### 7. ~~Uninitialized read in a log line~~ — Fixed
`Lab::servos_get_position` (`src/lab.cpp`) logged `x_mm`/`z_mm` as "current parameters" on entry,
before they were ever assigned — `wservos_get_position` (`src/wolfram_api.cpp`) declares them with
no initializer, since they're pure out-parameters. Removed the premature log line; the values are
still logged later, after being measured.

### 8. ~~Ramp/soak getter and setter command IDs disagree~~ — Fixed
`src/Oven5R6900.cpp` — the `set_*` counterparts (`set_soak_temp`, `set_ramp_duration`,
`set_soak_duration`, `set_num_repeats`, `set_next_sequence_num`) now compute their command ID
the same way their `get_*` counterparts do (`hex(8 + seq)`), so the same logical sequence index
hits the same command byte on the device for both reads and writes. This surface is still
commented out in `wolfram_api.cpp` (not exposed to Mathematica), so this was fixed without a way
to test it against real hardware.

### 9. ~~Dead declarations~~ — Fixed
`include/controls/Oven5R6900.h` declared private methods `read_temp`, `read_setpoint`, and
`read_ack` that were never defined in `Oven5R6900.cpp` (the getters call `dispatch_message`
directly instead). Confirmed unused and removed.

## Found via live testing (not in the original static review)

### 10. ~~Checksum failures were silently reported as success~~ — Fixed
`RTE7::read_temp`/`read_setpoint` and `Oven5R6900::dispatch_message` all call a response parser
that returns a `-999` sentinel on a checksum mismatch — but none of them checked for it, so a
garbled or wrong-protocol reply was reported as a successful call. This directly undermined the
port-autodetect probe (see `specs/port-autodetect.md`): any port that echoed back *any* bytes at
all, correct protocol or not, could pass as a "valid, checksummed response." Both files now check
for the `-999` sentinel and return `false` instead of silently succeeding with garbage data.
Found by watching `WolframMachineControl/Tests/Test_WolframMachineControl.wlt` fail against real
hardware: `BathInit[]`/`TempCtrlInit[]` succeeded, but every real command right after
(`BathOn[]`, `TempCtrlOn[]`, ...) failed — a pattern consistent with the probe having matched the
wrong port.

## Not yet checked
- No run against real hardware was performed for the fixes above — only build-verified.

## Removed dead code
`sm_homer.h`, `sm_manual_controller.h`, and `recorder.h`/`recorder.cpp` were confirmed unused
(not in `CMakeLists.txt`'s build list, not `#include`d anywhere, and `sm_homer.cpp`/
`sm_manual_controller.cpp` didn't even exist) and have been deleted. `lab.cpp` already
reimplements equivalent homing and manual-control logic directly.
