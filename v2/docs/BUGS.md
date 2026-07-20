# Known Bugs

Originally found via static read-through of the C++ source. Bugs #1-14 have all been fixed, the
DLL rebuilds clean (`cmake --build . --config Release`), and as of bug #14, the full
`Test_WolframMachineControl.wlt` suite passes 29/29 against real hardware (bath, temp controller,
servos, LabJack). Bugs #15-22 were found by a code review afterward and are **not yet fixed** —
see that section below.

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

## Found via code review after hitting 29/29 (not yet fixed)

A high-effort code review was run against this whole session's diff right after the suite first
passed 29/29. These survived verification but haven't been fixed yet.

### 15. `servos_homed()` crashes instead of returning `False` if servos were never initialized
`src/lab.cpp` — dereferences `motorX`/`motorZ` with no null check, and its `catch (...)` cannot
catch the resulting access violation, since nothing in `CMakeLists.txt` sets `/EHa` (MSVC's
default `/EHsc` only catches C++ exceptions, not structured/SEH ones like a null-pointer fault).
Now that `WolframLibrary_initialize` no longer auto-calls `initialize_servos()` on package load
(bug #11), a user calling `ServoHomed[]` before ever calling `ServoEnable[]` is a realistic, easy
mistake — and it crashes the kernel instead of returning `False`.

### 16. A garbled reply while probing the wrong device can crash the process
`Lab::probe_with_timeout()` (`src/lab.cpp`) only wraps `promise::set_value(...)` in `try`/`catch`
on its background thread — not the `candidate.get_setpoint(temp)` call itself. `Oven5R6900::
parse_response()` (`src/Oven5R6900.cpp`) does `response.substr(response.size()-3, 2)` with no
length check; a reply under 3 bytes (plausible when probing the *wrong* device with the *wrong*
protocol — e.g. the bath receiving an Oven5R6900-formatted query) underflows `size()-3` and
`substr` throws `std::out_of_range`. That exception is never caught anywhere in the chain, so it
escapes the thread entry point — `std::terminate()`, crashing the whole kernel process, not just
failing that one probe.

### 17. A timed-out probe can permanently block a working port from being reopened
`RTE7`'s (and `Oven5R6900`'s) `CreateFileA` call (`src/RTE7.cpp`) opens the serial port with
`dwShareMode = 0` (exclusive). If a `probe_with_timeout()` candidate genuinely times out while
still in-flight rather than failing fast, the abandoned background thread's `RTE7`/`Oven5R6900`
object stays alive holding that exclusive handle for as long as the thread keeps running. Any
later attempt to open that same port — a retry, or the same candidate being probed again in a
later `BathInit[]`/`TempCtrlInit[]` call in the same session — fails, misreporting a genuinely
working port as unreachable.

### 18. The `Sleep(150)` fix for bug #14 is a magic-number guess, not a bounded retry
`src/lab.cpp` (`init_bath()` and `init_temp_controller()`, ~line 242 and ~289) — 150ms was tuned
against one test run's OS/FTDI driver behavior; there's no guarantee it's enough on a different
machine, USB hub, or driver version, and no adaptive fallback if it isn't — a regression would
look exactly like bug #14 did. The same fix (with a near-identical comment) is also duplicated at
both call sites instead of living in one shared helper. `probe_with_timeout()`, two functions
earlier in the same file, already establishes an early-return-on-success pattern for this exact
class of problem ("a Win32 serial call might stall longer than expected") that this fix doesn't
reuse.

### 19. Candidate port probes run sequentially, not concurrently, despite already being threaded
`find_bath_port()`/`find_temp_controller_port()` (`src/lab.cpp`) loop over candidates, calling
`probe_with_timeout()` for one and waiting up to its full 1.5s timeout before starting the next —
even though each probe already runs on its own independent background thread. With both expected
FTDI adapters present and one stalled, worst-case latency is `candidateCount * 1.5s` instead of
~1.5s if all candidates were probed concurrently and awaited together.

### 20. Stale doc: `specs/librarylink.md` still says servos auto-start on package load
Left over from before bug #11's fix removed that automatic call — could mislead a future
maintainer into reintroducing it (and bug #11's hang) while "fixing" something unrelated.

### 21. `initialize_servos()` is a pointless one-line wrapper left over from a reverted fix
`src/lab.cpp` — the split into `initialize_servos()` / `initialize_servos_impl()` only ever
existed to support the background-thread timeout wrapper that was tried for bug #11 and then
fully reverted. The wrapper now does nothing; a reader has to open both functions and the
cross-referencing comments to confirm that.

### 22. `GetLogFile[]` test became tautological
`paclet/Tests/Test_WolframMachineControl.wlt` — its expected value was changed from an
independent hardcoded path literal to the same `LogFile` variable used to configure logging in
the first place, so the assertion can no longer catch a real path-configuration bug (both sides
of the comparison now derive from the same variable and would drift together).

## Removed dead code
`sm_homer.h`, `sm_manual_controller.h`, and `recorder.h`/`recorder.cpp` were confirmed unused
(not in `CMakeLists.txt`'s build list, not `#include`d anywhere, and `sm_homer.cpp`/
`sm_manual_controller.cpp` didn't even exist) and have been deleted. `lab.cpp` already
reimplements equivalent homing and manual-control logic directly.
