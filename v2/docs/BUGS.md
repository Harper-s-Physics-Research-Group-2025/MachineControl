# Known Bugs

Originally found via static read-through of the C++ source. Bugs #1-14 have all been fixed, the
DLL rebuilds clean (`cmake --build . --config Release`), and as of bug #14, the full
`Test_WolframMachineControl.wlt` suite passes 29/29 against real hardware (bath, temp controller,
servos, LabJack). Bugs #15-24 were found by a code review (and, for #23-24, direct usage plus a
return-value audit) afterward; all ten have now been fixed too, and the full suite still passes
29/29 against real hardware after the fixes (see each entry below for what changed). Bugs #25-26
were found while investigating a live "bath setpoint stuck at 25" report and have been fixed and
covered by new unit tests in `tests/test_protocol_parsing.cpp`. Bug #27 was found while
investigating a live report of `BathInit[]`/`TempCtrlInit[]` erroring when called on an
already-connected device, and has been fixed and verified against real hardware.

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
Found by watching `paclet/Tests/Test_WolframMachineControl.wlt` fail against real
hardware: `BathInit[]`/`TempCtrlInit[]` succeeded, but every real command right after
(`BathOn[]`, `TempCtrlOn[]`, ...) failed — a pattern consistent with the probe having matched the
wrong port.

### 11. ~~`Needs[...]` could hang the entire kernel with no timeout~~ — Fixed, then extended
`WolframLibrary_initialize` (`src/wolfram_api.cpp`) used to call `Lab::initialize_servos()`
automatically, synchronously, the moment the DLL first loads — which happens as a side effect of
`Needs["WolframMachineControl`"]`, not anything servo-specific. `initialize_servos()` makes
blocking Teknic SDK calls (`FindComHubPorts`, `PortsOpen`) with no timeout anywhere in the code
path, so if that handshake ever stalls, the entire kernel hangs on what looks like an unrelated
`Needs[...]` call, with the Teknic hub already connected and powered — Task Manager showed the
kernel process pinned at 0% CPU (blocked, not busy) with no way to recover except killing the
process. First fix: removed the automatic call entirely — it was also redundant, since the
documented workflow (`README.md` Step 5, and the `.wlt` test suite) already calls `ServoEnable[]`
explicitly before using the servos. Side benefit: the servo motors also no longer get energized
(`EnableReq(true)`) automatically on package load, only when a user explicitly calls
`ServoEnable[]`.

That first fix only removed the *automatic* trigger — it didn't add a timeout to
`initialize_servos()` itself, so the exact same stall was still fully reachable any time
`ServoEnable[]` is called explicitly, which the `.wlt` suite does (and which normal usage
requires). Confirmed still hanging there via `.wlt` runs and `log.txt` showing nothing past
`delete_temp_controller()` succeeding.

Tried the same fix as bug #12: renamed the existing logic to `initialize_servos_impl()` and ran it
via a background thread with a hard 5-second timeout. **This made things worse, not better** —
running the Teknic SDK calls (`SysManager::Instance()`, `FindComHubPorts`, `PortsOpen`) from a
thread other than the one that calls into the DLL crashed the whole process (segfault) during hub
setup, confirmed via a `wolframscript` run:
```
Found Teknic SC4-HUB
Friendly name: Teknic ClearPath 4-axis SC Hub (COM12)
Port number COM12
The product exited for an unknown reason.
Segmentation fault
```
Hardware SDKs wrapping USB/driver communication frequently have thread-affinity requirements —
they assume they're always called from the same thread (often the one that first touched them) —
and a generic `std::thread` wrapper silently violates that assumption. Unlike
`find_bath_port()`/`find_temp_controller_port()`'s fully self-contained `RTE7`/`Oven5R6900`
objects (which tolerated the same pattern fine), `initialize_servos_impl()` wraps a process-wide
Teknic singleton that apparently doesn't.

**Reverted.** `initialize_servos()` now just calls `initialize_servos_impl()` directly again, with
no timeout, no background thread. A crash is worse than a hang — a hung process can at least be
killed and recovered from; this was crashing mid-hardware-setup. The hang risk from the first half
of this bug is the accepted tradeoff until there's a safer way to bound this specific call — if
`ServoEnable[]` hangs, recover the same way as before any of this: kill the kernel process.

### 12. ~~Port auto-detect probe could hang the entire kernel~~ — Mitigated
Running the full `.wlt` suite against real hardware hung indefinitely right after `DeleteBath[]`
succeeds, at the very next test (`TempCtrlInit[]`) — `find_temp_controller_port()`'s serial probe
never returned. The configured `COMMTIMEOUTS` in `RTE7`/`Oven5R6900`'s `initSerial` are supposed to
bound every read/write, but evidently can't always be trusted — a device or driver-level stall can
block a synchronous `ReadFile`/`WriteFile` past its configured timeout.

`find_bath_port()`/`find_temp_controller_port()` now wrap each candidate probe in a hard
1.5-second wall-clock timeout (`Lab::probe_with_timeout`, `src/lab.cpp`), so a stuck exchange fails
that candidate and moves on instead of freezing the kernel. First attempt at this only wrapped the
`get_setpoint()` call itself in the timeout, leaving the `RTE7`/`Oven5R6900` *construction*
(`CreateFileA`, opening the port) running unprotected on the calling thread — and the hang turned
out to still reproduce after that fix, meaning construction itself is a plausible place for the
real stall (e.g. the OS/FTDI driver not having fully released a just-closed handle on that same
COM port yet). Both construction and the probe now happen entirely inside the timed async task,
so the timeout actually covers the whole probe. A timed-out probe's `RTE7`/`Oven5R6900` object
lives only inside that background task's lambda, so it stays alive for as long as that thread
keeps running rather than being destroyed while a synchronous Win32 call might still be blocked
on its handle — closing a handle out from under pending synchronous I/O on another thread is
undefined behavior on Windows. Root cause of the underlying stall is still not fully confirmed (no
way to reproduce without the physical rig); this is a mitigation, not a root-cause fix.

### 13. ~~Both timeout fixes above (#11, #12) still hung, silently, one line later~~ — Fixed
After shipping both timeout fixes, the `.wlt` suite still froze — but now *past* the point where
each timeout's log message correctly fired (`"...did not respond within timeout."`), with nothing
logged after. Confirmed via Task Manager: sampled the kernel process's CPU usage twice, 5 real
seconds apart — identical value both times, proving it was genuinely blocked, not just slow.

Root cause: both `probe_with_timeout()` and `initialize_servos()` used
`std::async(std::launch::async, ...)`, and a `std::future` returned by `std::async` has a
standard-mandated special rule — **its destructor blocks until the task finishes**, if the result
was never retrieved via `.get()`. So `wait_for(timeout)` would correctly report a timeout and the
code would log and `return false`/`-1` as intended — but the moment that local `future` went out
of scope one line later, its destructor silently re-blocked the calling thread until the abandoned
Teknic/serial call actually finished (which, per bug #11/#12, might be never). The timeout
appeared to work because the log message fired; it didn't actually protect anything, because
returning from the function was itself blocked.

Fixed by switching both functions from `std::async` to a detached `std::thread` paired with a
`std::promise`/`std::future`. A future obtained from a `std::promise` has no blocking-destructor
behavior — it's just an ordinary object — so giving up after `wait_for(timeout)` and returning is
actually safe. The detached thread keeps running independently; if it eventually finishes, it
calls `promise::set_value(...)`, which is a harmless no-op if nothing is listening anymore.

This fixed `probe_with_timeout()` (bug #12) for good. It did *not* end up fixing
`initialize_servos()` (bug #11) — see that entry's update: running the Teknic SDK on any thread
other than the caller's turned out to crash the process outright, so that one was reverted to a
direct, un-timed call rather than kept on a background thread.

### 14. ~~`BathInit[]`/`TempCtrlInit[]` could fail immediately after their own probe just succeeded~~ — Fixed
Running the full suite via `wolframscript` (independent of the Mathematica front end, to rule out
notebook-specific issues) surfaced this cleanly: the bath's protocol probe would report
`"matched (this is the bath)"` in `log.txt`, then the very next real command would fail
(`"Bath setpoint written: ... | Status: 1"`) — every bath test failing despite the probe that
identifies it as the bath succeeding moments earlier.

Cause: `find_bath_port()`/`find_temp_controller_port()` open a temporary `RTE7`/`Oven5R6900` to
probe each candidate port, closing it the instant a match is confirmed. `init_bath()`/
`init_temp_controller()` then immediately open a *second*, persistent connection on that exact
same port, milliseconds later. The OS/FTDI driver apparently needs a brief moment to fully
release a just-closed handle before a new one can succeed — reopening immediately could fail
outright, which reads as a clean, fast failure right after a successful probe, not a hang.

Fixed with a 150ms `Sleep()` in both `init_bath()` and `init_temp_controller()`, between closing
the probe and opening the persistent connection. Confirmed fixed via `wolframscript`: bath went
from failing all 6 of its tests to passing all 6, with the log showing `Status: 0` instead of
`Status: 1` on the exact same command.

## Found via code review after hitting 29/29 — all fixed

A high-effort code review was run against this whole session's diff right after the suite first
passed 29/29. These survived verification; all ten have now been fixed, and the full
`Test_WolframMachineControl.wlt` suite still passes 29/29 against real hardware afterward.

### 15. ~~`servos_homed()` crashes instead of returning `False` if servos were never initialized~~ — Fixed
`src/lab.cpp` — dereferenced `motorX`/`motorZ` with no null check, and its `catch (...)` couldn't
catch the resulting access violation, since nothing in `CMakeLists.txt` sets `/EHa` (MSVC's
default `/EHsc` only catches C++ exceptions, not structured/SEH ones like a null-pointer fault).
Now that `WolframLibrary_initialize` no longer auto-calls `initialize_servos()` on package load
(bug #11), a user calling `ServoHomed[]` before ever calling `ServoEnable[]` is a realistic, easy
mistake. Fixed: `servos_homed()` now checks `!motorX || !motorZ` up front and returns `false`,
matching the pattern `servos_ready()` already used.

### 16. ~~A garbled reply while probing the wrong device can crash the process~~ — Fixed
`Lab::probe_with_timeout()` (`src/lab.cpp`) only wrapped `promise::set_value(...)` in `try`/`catch`
on its background thread — not the `candidate.get_setpoint(temp)` call itself. `Oven5R6900::
parse_response()` (`src/Oven5R6900.cpp`) does `response.substr(response.size()-3, 2)` with no
length check; a reply under 3 bytes (plausible when probing the *wrong* device with the *wrong*
protocol — e.g. the bath receiving an Oven5R6900-formatted query) underflowed `size()-3` and
`substr` threw `std::out_of_range`, uncaught, escaping the thread entry point —
`std::terminate()`, crashing the whole kernel process, not just failing that one probe. Fixed two
ways: (1) the probe's whole body (device construction + `get_setpoint()`) is now wrapped in
`try`/`catch` (see bug #19's `start_probe()`/`find_port_by_probe()` rewrite), so any exception
from either device class just reports "no match" instead of escaping the thread; (2)
`Oven5R6900::parse_response()` and `RTE7::parse_float_response()` (`src/RTE7.cpp`, which has the
same unguarded-indexing shape via `response[2]`/`response[3]`/`response[4]`, undefined behavior
rather than even a catchable exception) now both return the existing `-999` failure sentinel on a
too-short reply instead of reading/erasing past the end of the buffer. Covered by two new cases in
`tests/test_protocol_parsing.cpp`.

### 17. ~~A timed-out probe can permanently block a working port from being reopened~~ — Fixed
`RTE7`'s (and `Oven5R6900`'s) `CreateFileA` call (`src/RTE7.cpp`) opens the serial port with
`dwShareMode = 0` (exclusive). If a probe genuinely timed out while still in-flight rather than
failing fast, the abandoned background thread's `RTE7`/`Oven5R6900` object stayed alive holding
that exclusive handle for as long as the thread kept running — any later attempt to open that same
port (a retry, or the same candidate probed again in a later `BathInit[]`/`TempCtrlInit[]` call in
the same session) would fail, misreporting a genuinely working port as unreachable. Fixed:
`find_port_by_probe()` (`src/lab.cpp`, the rewrite of the old `probe_with_timeout()`/candidate loop
— see bug #19) now calls `CancelSynchronousIo()` on a timed-out candidate's worker thread before
moving on. This is the Windows-documented way to unblock a pending synchronous `ReadFile`/
`WriteFile` call from another thread, letting the worker actually finish, close its handle, and
free the port instead of leaking it. If the stall is inside `CreateFileA` itself rather than a
cancellable read/write, cancellation is a harmless no-op and the worker is still just detached
exactly as before — the calling thread is never made to wait on it either way, so this can only
improve recovery, never regress it into a hang.

### 18. ~~The `Sleep(150)` fix for bug #14 is a magic-number guess, not a bounded retry~~ — Fixed
`src/lab.cpp` (`init_bath()` and `init_temp_controller()`) — 150ms was tuned against one test run's
OS/FTDI driver behavior, with no guarantee it's enough on a different machine, USB hub, or driver
version, and the same fix (with a near-identical comment) was duplicated at both call sites instead
of living in one shared helper. Fixed: both call sites now go through a new
`open_persistent_connection<Device>()` helper that retries the reopen up to three times with a
150ms delay between attempts (instead of one fixed-delay guess), reusing the same
early-return-on-success shape the probing code already established elsewhere in this file.
Verified against real hardware (`BathInit[]`/`TempCtrlInit[]` still succeed).

### 19. ~~Candidate port probes run sequentially, not concurrently, despite already being threaded~~ — Fixed
`find_bath_port()`/`find_temp_controller_port()` (`src/lab.cpp`) looped over candidates, calling
`probe_with_timeout()` for one and waiting up to its full 1.5s timeout before starting the next —
even though each probe already ran on its own independent background thread. With both expected
FTDI adapters present and one stalled, worst-case latency was `candidateCount * 1.5s` instead of
~1.5s if all candidates were probed concurrently and awaited together. Fixed: `probe_with_timeout()`
was split into `start_probe<Device>()` (launches one candidate's worker thread and returns
immediately with its future) and `find_port_by_probe<Device>()` (starts *all* candidates' workers
up front, then waits on each one's future against one shared absolute deadline instead of a fresh
per-candidate timeout), so total latency is bounded by `timeout` regardless of candidate count.
`find_bath_port()`/`find_temp_controller_port()` now call `find_port_by_probe()`. Verified against
real hardware: the suite still finds and matches both the bath and temp controller correctly.

### 20. ~~Stale doc: `specs/librarylink.md` still says servos auto-start on package load~~ — Fixed
Left over from before bug #11's fix removed that automatic call — could have misled a future
maintainer into reintroducing it (and bug #11's hang) while "fixing" something unrelated. Fixed:
the doc now explains servo init only happens via an explicit `ServoEnable[]` call, and why.

### 21. ~~`initialize_servos()` is a pointless one-line wrapper left over from a reverted fix~~ — Fixed
`src/lab.cpp` — the split into `initialize_servos()` / `initialize_servos_impl()` only ever
existed to support the background-thread timeout wrapper that was tried for bug #11 and then
fully reverted, leaving a wrapper that did nothing. Fixed: merged back into a single
`initialize_servos()`, carrying forward the explanatory comment about why it's called directly
rather than on a background thread.

### 22. ~~`GetLogFile[]` test became tautological~~ — Fixed
`paclet/Tests/Test_WolframMachineControl.wlt` — its expected value had been changed from an
independent hardcoded path literal to the same `LogFile` variable used to configure logging in
the first place, so the assertion could no longer catch a real path-configuration bug (both sides
of the comparison derived from the same variable and would drift together). Fixed: reverted to
comparing against an independent hardcoded literal.

### 23. ~~Most DLL wrappers return their `Lab::` function's `1`/failure code as the raw LibraryLink status, colliding with `LIBRARY_TYPE_ERROR`~~ — Fixed
`src/wolfram_api.cpp` — the majority of wrappers (`wbath_on`, `wbath_off`, `wbath_manual`,
`wbath_get_temp`, `wbath_get_setpoint`, `wbath_set_setpoint`, `winitialize_bath`,
`wtemperature_control_on`/`off`/`get_mode`/`set_mode`/`get_temp`/`get_setpoint`/`set_setpoint`,
`winitialize_temperature_control`, `wset_log_settings`, `wget_servo_alerts`, `wservos_home`,
`wservos_get_position`, `wservos_set_position`) do `return Lab::some_function();` directly,
passing the `Lab::` layer's own `0`-success/`1`-failure convention straight through as the
LibraryLink status code. `1` happens to be the numeric value of `LIBRARY_TYPE_ERROR` (see
`WolframLibrary.h`), so any ordinary failure (not initialized, hardware didn't respond, bad
input) gets misreported to Mathematica as `LibraryFunctionError[LIBRARY_TYPE_ERROR, 1]` /
"inconsistent types was encountered" instead of anything describing what actually went wrong.
`winitialize_servos` can additionally return `-1` on a Teknic SDK exception — a negative value
matching no named `LIBRARY_*_ERROR` constant at all, so its presentation is unpredictable.

First surfaced (and misdiagnosed as a real type error) the very first time `BathOn[]` was called
before `BathInit[]`, early in this session — noted at the time as a systemic issue but never
tracked or fixed. Resurfaced identically after bug #1's null-pointer-crash fix: that fix made
these functions return `1` *safely* instead of crashing, but never changed what `1` means to
LibraryLink, so the same confusing message persists.

Not affected — already return a proper status separately from the real answer, via `Res`:
`ServoReady[]` (`wservo_hardware_online`), `ServoHomed[]` (`wservos_homed`, fixed as bug #3),
`DeleteBath[]`, `DeleteTempCtrl[]`, `ServoDisable[]`, `GetLogStatus[]`, `GetLogFile[]` (all of
these either always succeed or already use `LIBRARY_NO_ERROR` correctly).

Fixed: every affected wrapper (plus `ServoEnable[]`/`winitialize_servos`, discussed above) now
captures the `Lab::` call's result as `lab_status` and returns
`lab_status != 0 ? LIBRARY_FUNCTION_ERROR : LIBRARY_NO_ERROR` instead of passing it through raw, so
an ordinary failure now surfaces as `LibraryFunctionError[LIBRARY_FUNCTION_ERROR, ...]` instead of
the misleading "inconsistent types" message. Verified against real hardware: the full test suite
still passes 29/29 (success paths are unaffected — `LIBRARY_NO_ERROR` is still `0`), and manually
calling e.g. `BathOn[]` before `BathInit[]` now reports a function error instead of a type error.

### 24. ~~Several "get" functions can hand back a garbage number, not just a confusing error, when the read fails~~ — Fixed
`src/wolfram_api.cpp` — a worse variant of bug #23: several wrappers declared an uninitialized
local, unconditionally handed it to `MArgument_set*(Res, ...)`, and only *then* checked whether the
`Lab::` call that was supposed to fill it in actually succeeded:
```cpp
DLLEXPORT int wbath_get_temp(...){
    float temp;                                       // uninitialized
    int return_code = Lab::bath_get_temp(temp);        // may return early without touching temp
    MArgument_setReal(Res, static_cast<double>(temp)); // sent to Mathematica regardless
    return return_code;
}
```
If `Lab::bath_get_temp` failed early (e.g. `bath` is null), `temp` was never assigned, so whatever
garbage was already on the stack got returned as if it were a real temperature reading — the
caller saw a `LibraryFunctionError` *and* a plausible-looking but meaningless number in the same
call, easy to accidentally use if the error isn't checked first.

**Affected the same way:** `BathGetTemp[]`, `BathGetSetpoint[]`, `TempCtrlGetMode[]`,
`TempCtrlGetTemp[]`, `TempCtrlGetSetpoint[]`, `ReadLabjack[]`, `ServoGetPos[]`.

**Worse case:** `wservos_home` (`ServoHome[milliseconds]`) never called any `MArgument_set*(Res,
...)` at all, on *any* path — success or failure. Its `.wl` binding still declares an `Integer`
return type, so `ServoHome[...]`'s returned value was undefined memory unconditionally; only the
status code (whether it threw `LibraryFunctionError` or not) carried any real signal.

**Different, milder case:** `wservos_set_position` (`ServoSetPos[...]`) initializes its output
from the function's own *input* arguments, not an uninitialized local — so on failure it echoes
back the *requested* `{x_mm, z_mm, rpm}` rather than the actual resulting position, which is
misleading but not undefined memory; left as-is (not required by the fix below).

Not affected: `SetLogSettings[...]`/`GetLogStatus[]`/`GetLogFile[]` (`Lab::get_log_settings`/
`set_log_settings` always assign their out-parameters, every path); `wget_servo_alerts` (its
buffers are explicitly zero-initialized, so a failure just yields empty-looking alert strings,
not garbage); `ServoHomed[]`/`ServoReady[]` (bug #3's fix already routes them correctly).

Documented in `docs/API_REFERENCE.md`. Fixed: every function in the "affected" list now
initializes its local to a `0`/`0.0` sentinel before the `Lab::` call, so a failure path returns a
deterministic, documented placeholder instead of stack garbage. `wservos_home` additionally now
calls `MArgument_setInteger(Res, static_cast<mint>(lab_status))` on every path, matching the
0-success/nonzero-failure convention every sibling status-only function in this file already
returns as its Integer value — verified against real hardware that `ServoHome[30000]` still
returns `0` on a successful home (matching the existing `.wlt` assertion) after this change.

Documented in `README.md`'s "Known Confusing Error Messages" section. Fixed as part of bug #23
above — see that entry for the actual `LIBRARY_FUNCTION_ERROR` mapping.

### 25. ~~`RTE7::parse_float_response` decoded negative readings as huge positive garbage~~ — Fixed
`src/RTE7.cpp` built the 16-bit temperature/setpoint value into a `uint16_t`, but the NC serial
protocol (confirmed against Thermo NESLAB's own manual, Appendix A) sends this value as a
**signed** 16-bit integer — e.g. -10.5°C is sent as `FF 97`. Decoding that as unsigned gave
6543.1 instead of -10.5. Fixed by assembling the raw bytes into a `uint16_t` bit pattern and then
`static_cast`-ing to `int16_t` before dividing by precision, so the two's-complement value is
interpreted correctly. Only affected negative temperatures/setpoints; positive values happened to
decode correctly before.

### 26. ~~`RTE7::set_setpoint` always reported success, even when the bath silently clamped or rejected the write~~ — Fixed
Found while investigating a live report of `BathSetSetpoint[...]` writes never actually changing
`BathGetSetpoint[]`'s reading. Two compounding issues in `src/RTE7.cpp`:
- `parse_float_response` extracted the response's `command` byte but never checked it. The bath
  replies to a malformed/rejected command with a "Bad Command"/"Bad Checksum" error frame
  (command byte `0x0F`, per Appendix A Table 1) whose data bytes are an error code, not a
  reading — that frame was being blindly decoded as if it were a real temperature. Fixed:
  `parse_float_response` now returns the existing `-999` failure sentinel when `command == 0x0F`.
- `set_setpoint(const float& temp)` never parsed its own ack response at all — it just checked
  that *some* bytes came back, then the caller (`wolfram_api.cpp`'s `wbath_set_setpoint`) echoed
  back the *requested* value regardless of what the bath actually did. Per the protocol, the
  bath's ack to a Set command echoes the value it actually stored — and the manual explicitly
  notes Set commands are "limited to the range of the bath," i.e. an out-of-range setpoint is
  **silently clamped**, not rejected. Fixed: `set_setpoint` is now `set_setpoint(float& temp)`
  (in/out) and writes the bath's confirmed value back into `temp` via `parse_float_response`, so
  `BathSetSetpoint[...]` now reports what the bath actually stored, surfacing a clamp instead of
  hiding it.

If the earlier live report turns out to be a real Hi/Lo Temperature Limit clamp (check `Hit`/`Lot`
in the bath's own physical Setup Loop), this fix will make `BathSetSetpoint[...]`'s return value
show the clamped number directly instead of silently echoing back whatever was requested.

### 27. ~~`BathInit[]`/`TempCtrlInit[]` failed with a generic error when called while already connected~~ — Fixed
`init_bath()`/`init_temp_controller()` (`src/lab.cpp`) probed for the device's COM port *before*
releasing any existing connection. `find_bath_port()`/`find_temp_controller_port()` find the
device by opening each candidate port themselves and testing it with a real protocol query
(`probe_with_timeout<...>`) — but `CreateFileA` opens serial ports exclusively (`dwShareMode=0`,
see `RTE7::initSerial`/`Oven5R6900::initSerial`), so if the device was already connected, its own
held-open port would fail that very probe. With no other candidate able to match, `find_*_port()`
correctly reported "no match," and `Init[]` returned the generic `1`-as-error — even though the
device was already connected and working fine. Fixed by moving the `delete bath`/`delete tc`
cleanup (plus the existing `Sleep(150)` settle delay, same reasoning as bug #14) to *before* the
probe instead of after, so a re-`Init[]` call actually reconnects and returns `0` like a fresh one.
Verified live: `BathInit[]`/`TempCtrlInit[]` called three times in a row all return `0`, and
`BathGetTemp[]`/`TempCtrlGetTemp[]` keep working correctly afterward.

## Removed dead code
`sm_homer.h`, `sm_manual_controller.h`, and `recorder.h`/`recorder.cpp` were confirmed unused
(not in `CMakeLists.txt`'s build list, not `#include`d anywhere, and `sm_homer.cpp`/
`sm_manual_controller.cpp` didn't even exist) and have been deleted. `lab.cpp` already
reimplements equivalent homing and manual-control logic directly.
