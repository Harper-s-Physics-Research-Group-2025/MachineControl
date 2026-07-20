# MachineControl — Project Notes

C++ Wolfram LibraryLink DLL that gives Mathematica direct control over the Josh 2.0 automated
X-ray lipid analysis system (Harper lab, Calvin University): a fluid bath, a thermoelectric temp
controller, Teknic ClearPath servo motors, and a LabJack ADC. Windows-only. Lives under `v2/`.

**Status as of 2026-07-17: fully working.** The full test suite
(`v2/paclet/Tests/Test_WolframMachineControl.wlt`) passes **29/29** against real hardware — bath,
temp controller, servos, LabJack, logging, all of it. Got there after a long debugging session;
see `v2/docs/BUGS.md` for the full bug list (14 entries, all fixed) and `v2/specs/` for design
rationale on the trickier pieces (port auto-detection, the LibraryLink integration, portable
notebook paths).

## Where things live (post-reorg)

- `v2/src/`, `v2/include/controls/` — the C++ (`lab.cpp` is the big one; `wolfram_api.cpp` is the
  thin LibraryLink boundary; `RTE7.cpp`/`Oven5R6900.cpp` are the two serial device classes).
- `v2/paclet/` — the Wolfram paclet (`Kernel/WolframMachineControl.wl`, `PacletInfo.wl`,
  `Tests/Test_WolframMachineControl.wlt`). Renamed from `WolframMachineControl/` — the actual
  Wolfram context name (`WolframMachineControl\``) and the `.wl`/`.wlt` filenames were
  deliberately left unchanged, only the containing folder was renamed.
- `v2/notebooks/` — `TestMachineControl.nb`, `Record.nb`. Renamed from `WolframNotebooks/`.
- `v2/docs/` — `BUGS.md`, `HARDWARE_COMMS.md`. Renamed from a folder called `md_files/`.
- `v2/specs/` — design-rationale docs (`port-autodetect.md`, `librarylink.md`,
  `portable-paths.md`). This is where new "why we built it this way" writeups belong.
- `v2/tests/` — `test_protocol_parsing.cpp` (doctest-based C++ unit tests, no hardware needed —
  covers checksum/parsing logic only, not actual serial I/O).

## Build & test

```bash
cd v2/build
cmake ..
cmake --build . --config Release --target wolfram_machine_controller   # the DLL
cmake --build . --config Release --target protocol_tests               # C++ unit tests (no hardware)
./Release/protocol_tests.exe
```

To run the real `.wlt` suite against hardware without needing the Mathematica GUI:
```bash
"/c/Program Files/Wolfram Research/WolframScript/wolframscript.exe" -file <script.wl>
```
where the script does `PacletDirectoryLoad[...]; Needs["WolframMachineControl\`"]; report =
TestReport["...Test_WolframMachineControl.wlt"]`. Useful for independently verifying a fix
without relying on a possibly-stale notebook session — this is how several hangs in the history
below were actually confirmed fixed.

## Sharp edges (learned the hard way — read before touching servo/serial code)

- **The DLL file-locks while a Mathematica kernel has it loaded.** If a rebuild fails with
  "Permission denied" copying into `paclet/LibraryResources/`, a kernel (or a leftover
  `WolframKernel`/`wolfram` process) still has it open. Kill it (Task Manager, or
  `Stop-Process -Force` by PID if you have it) and rebuild.
- **`initialize_servos()` (src/lab.cpp) must be called directly, on the calling thread — do not
  wrap it in a timeout via a background thread.** This was tried (matching the pattern that works
  fine for the bath/temp-controller probes) and it **crashed the process** — the Teknic SDK
  (`sFnd::SysManager`) appears to have thread-affinity requirements. See `docs/BUGS.md` #11 for
  the full story. If `ServoEnable[]` hangs, that's a known, currently-accepted risk; recover by
  killing the kernel, not by re-adding a timeout wrapper here.
- **`std::async`'s returned `std::future` blocks in its own destructor** until the task finishes,
  if you never called `.get()` on it — this silently defeated an earlier timeout fix (the timeout
  logic fired correctly, then the function hung anyway one line later when the future went out of
  scope). The fix used elsewhere (`probe_with_timeout()` in `src/lab.cpp`) is a detached
  `std::thread` + `std::promise`/`std::future` pair instead, which has no such behavior. Use that
  pattern, not `std::async`, for any future "bound a blocking Win32/SDK call" needs.
- **Reopening a COM port immediately after closing it can fail.** `find_bath_port()`/
  `find_temp_controller_port()` open a temporary probe object per candidate port, close it on
  match, then `init_bath()`/`init_temp_controller()` immediately reopen the same port persistently
  — the OS/FTDI driver needs a moment to release the handle. Both now `Sleep(150)` before
  reopening (`docs/BUGS.md` #14). This is a known-fragile fixed delay, not a real retry loop — see
  today's code-review findings below.
- **The bath and temp controller currently use FTDI FT232 adapters** (`VID_0403&PID_6001`),
  matched in `find_serial_adapter_ports()` (`src/lab.cpp`). They used to be Prolific
  (`VID_067B&PID_2303`) until that adapter's Windows 11 driver support was dropped
  (`PL2303TA DO NOT SUPPORT WINDOWS 11 OR LATER`). If the hardware ever changes chipset again,
  that VID/PID string is the one line to update — see `docs/HARDWARE_COMMS.md`.
- **This repo briefly existed as two out-of-sync copies** (one under `Desktop/machine_controller/`,
  one under `Documents/JuxtapositionOfSampleHolders/`) — a real source of confusion for a while
  (fixes made in one, tested from the other). Resolved: **`Desktop/machine_controller/MachineControl`
  is the canonical copy.** The notebooks' `PacletDirectoryLoad[...]` calls now resolve dynamically
  via `NotebookDirectory[]` instead of a hardcoded path, specifically so this can't recur silently
  (see `specs/portable-paths.md`).

## Session history (condensed — see `git log` and `docs/BUGS.md` for full detail)

- **2026-07-13** — Initial cleanup pass: removed stale files, rewrote README, documented known
  bugs (the original static-review `BUGS.md`, bugs #1-#9).
- **2026-07-16** — Implemented COM port auto-detection for the bath/temp controller (protocol
  probing to disambiguate two identical-looking serial adapters). Found and fixed bug #10 (a
  checksum-validation bug that let garbled replies pass as "success," undermining the new
  auto-detect probe) via live hardware testing.
- **2026-07-17** — Long hardware-debugging session: new physical cables required switching the
  hardware-ID match from Prolific to FTDI; found and fixed a `Needs[...]`-triggered kernel hang
  (bug #11, automatic servo init on package load with no timeout); found and fixed a port
  auto-detect hang (bug #12); discovered `std::async`'s blocking-destructor gotcha was silently
  defeating both of those fixes (bug #13) and switched to detached-thread+promise; tried
  timeout-wrapping the Teknic SDK call and it **crashed** the process instead of hanging, so that
  one got reverted to direct/un-timed (documented in bug #11's update); found and fixed a
  reopen-after-probe race (bug #14, the `Sleep(150)` fix). Ended with the full suite passing
  29/29 against real hardware, confirmed via `wolframscript` runs independent of the GUI.
  Reorganized the project layout (`paclet/`, `notebooks/`, `docs/`, `specs/`, `tests/`) and added
  a small C++ unit test suite (doctest) for the checksum/parsing logic.

## Open items for next session

A high-effort code review was run against the full session's diff right after hitting 29/29.
Eight findings survived verification and are recorded as `docs/BUGS.md` #15-22 — **not yet
fixed**, that's the natural next step. Three are real crash risks worth fixing before doing much
more in this area: #15 (`servos_homed()` null-deref crashes instead of returning `False`), #16
(an uncaught exception in a background probe thread can crash the whole process on a garbled
reply), #17 (a timed-out probe can permanently block a working port from being reopened). The
rest (#18-22) are lower-priority robustness/efficiency/doc-cleanup items.

A ninth issue (#23) turned up through direct usage the next day: most DLL wrappers in
`src/wolfram_api.cpp` return their `Lab::` function's raw `1`-for-failure straight through as the
LibraryLink status code, which collides with `LIBRARY_TYPE_ERROR` — so ordinary failures (e.g.
calling `BathOn[]` before `BathInit[]`) print a misleading "inconsistent types" error instead of
a real message. Documented in both `docs/BUGS.md` #23 and `README.md`'s new "Known Confusing
Error Messages" section, listing exactly which functions are affected and which already return
proper status. Not fixed yet either.

Full detail for all nine in `docs/BUGS.md`.
