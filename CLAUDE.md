# MachineControl — Project Notes

C++ Wolfram LibraryLink DLL that gives Mathematica direct control over the Josh 2.0 automated
X-ray lipid analysis system (Harper lab, Calvin University): a fluid bath, a thermoelectric temp
controller, Teknic ClearPath servo motors, and a LabJack ADC. Windows-only. Lives under `v2/`.

**Status as of 2026-08-07: fully working, and the post-29/29 code-review backlog is now clear.**
The full test suite (`v2/paclet/Tests/Test_WolframMachineControl.wlt`) passes **29/29** against
real hardware — bath, temp controller, servos, LabJack, logging, all of it. Got there after a long
debugging session; see `v2/docs/BUGS.md` for the full bug list (27 entries, all fixed) and
`v2/specs/` for design rationale on the trickier pieces (port auto-detection, the LibraryLink
integration, portable notebook paths).

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
- `v2/data/` — CSVs written by `LabJackRecordData[]` / read by `LabJackPlotData[]` (see below).

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
  scope). The fix used elsewhere (`start_probe()`/`find_port_by_probe()` in `src/lab.cpp`, the
  rewrite of the original `probe_with_timeout()`) is a `std::thread` + `std::promise`/`std::future`
  pair instead, which has no such behavior. Use that pattern, not `std::async`, for any future
  "bound a blocking Win32/SDK call" needs.
- **Reopening a COM port immediately after closing it can fail.** `find_bath_port()`/
  `find_temp_controller_port()` open a temporary probe object per candidate port, close it on
  match, then `init_bath()`/`init_temp_controller()` immediately reopen the same port persistently
  — the OS/FTDI driver needs a moment to release the handle. Both now go through
  `open_persistent_connection<Device>()`, which retries the reopen a few times with a 150ms delay
  between attempts rather than a single fixed-delay guess (`docs/BUGS.md` #14, #18).
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
- **2026-07-20** — Live report of `BathGetSetpoint[]` reading stuck at 25 regardless of what
  `BathSetSetpoint[...]` was called with. Traced `RTE7.cpp` byte-for-byte against Thermo NESLAB's
  own NC serial protocol manual (checksum/packing turned out correct) and found two real,
  independent bugs instead: `parse_float_response` decoded negative readings as unsigned garbage,
  and never checked for the bath's "Bad Command"/"Bad Checksum" error frame (command byte `0x0F`),
  so a rejected command's error bytes got silently decoded as if they were a real reading;
  separately, `set_setpoint()` never parsed its own ack, so `BathSetSetpoint[]` blindly echoed the
  *requested* value instead of the bath's *confirmed* one — masking any silent clamp to the bath's
  own Hi/Lo Temperature Limit. Fixed both (`docs/BUGS.md` #25-26), covered by new doctest cases,
  and verified live via `wolframscript` that arbitrary setpoints (e.g. 40.0) now round-trip
  correctly. Also added a `LabJackRecordData[]`/`LabJackListCSVs[]`/`LabJackPlotData[]` suite —
  pure Wolfram Language in `WolframMachineControl.wl`, no DLL changes — for recording LabJack
  channels 0-7 plus bath temperature over time until a target temperature is reached, then
  listing/plotting the resulting CSVs (saved to the new `v2/data/`). Extended `LabJackRecordData[]`
  to drive and wait on both the bath and temp controller together, since it wasn't actually
  commanding either device toward `finalTemp` at first — it was only reading temperatures and
  waiting, so calling it while nothing was actively heating/cooling looked exactly like a hang.
- **2026-07-21** — Fixed bug #27: `BathInit[]`/`TempCtrlInit[]` returned a generic `1`-as-error when
  called on an already-connected device, because `init_bath()`/`init_temp_controller()` probed for
  the COM port *before* releasing the existing connection — and the probe opens each candidate port
  exclusively, so the already-open port (held by the live connection) always failed its own probe.
  Fixed by releasing the old connection before re-probing; verified live that repeated `Init[]`
  calls now all return `0` and the device keeps working afterward.
- Investigated a report that the LabJack wasn't reading the actual laser intensity. Found a real
  suspect in `Lab::read_labjack_ain` (`src/lab.cpp`): every read set `LJ_chAIN_RESOLUTION` to `1`,
  which per LabJack's own U3 docs enables "QuickSample" mode -- faster ADC conversions at the cost
  of more noise. Changed it to `0` (QuickSample off) so a subtle signal isn't masked by that noise;
  not yet confirmed whether this actually fixes the reported symptom, separate from a wiring check
  (which channel is physically connected) also still pending.
- Per request, removed all bath dependence from `LabJackRecordData[]` -- it now only drives/waits
  on the temp controller (`TempCtrlOn[]`/`TempCtrlSetSetpoint[]`/`TempCtrlGetTemp[]`), and its CSV
  no longer has a `BathTemp` column. `LabJackPlotData[]` updated to match (plots channels against
  `TempCtrlTemp` instead of `BathTemp`). Added `TempCtrlPlotTemp[targetTemp]` -- initially took a
  fixed sampling duration, but per follow-up request now takes a target temperature instead: sets
  the setpoint, samples `TempCtrlGetTemp[]` once per second until it arrives, then plots
  temperature vs. time (same drive-and-wait shape as `LabJackRecordData`, just without channels/CSV).
  Later given an optional `interval` argument (default `1`) too -- first attempt used the
  `interval_?NumericQ:1` pattern, which silently failed to bind (calls returned unevaluated
  instead of running); fixed with a plain two-clause definition
  (`TempCtrlPlotTemp[t_] := TempCtrlPlotTemp[t, 1]`) instead of a combined optional/PatternTest.
  `docs/API_REFERENCE.md` also reorganized per request: `TempCtrlPlotTemp[]` moved into the
  Temperature Controller section, and `ReadLabjack[]` (plus its channel-pinout explainer) merged
  into the "LabJack Data-Collection Suite" subsection under Data Acquisition, so that section is
  now the single home for everything LabJack-related.
- `TempCtrlPlotTemp[]`'s x-axis label changed from a hardcoded `"Time (s)"` to `"Time (every
  <interval>s)"`, so the graph reflects the actual wait time between measurements for that run
  instead of a generic label.
- `LabJackPlotData[filename]` changed to `LabJackPlotData[filename, channel]` -- now plots only
  the requested channel column (matched by header name, e.g. `"Channel3 (V)"`) instead of every
  channel at once. Fails cleanly with a new `LabJackPlotData::nochannel` message if that channel
  isn't a column in the CSV.
- Added `LabJackTempSweep[startTemp, temps, lipidName, waterConcentration, interval:1]` -- a full
  multi-temperature scan protocol, explicitly built *on top of* the existing, unmodified
  `LabJackRecordData[]` rather than changing it. Repeatedly returns the temp controller to
  `startTemp` and drives it out to each temperature in `temps` in turn, recording both legs of
  every round trip (`N` temperatures -> `2N` CSVs). Filenames went through two iterations: first
  `lipidName_waterConcentration_rise-or-fall_n.csv`, but per follow-up request changed to match
  the convention already used by real files sitting in `v2/data/` (e.g.
  `monopalmatin_60_rise_10_to_45.csv`, produced by the user's own earlier testing) --
  `lipidName_waterConcentration_rise-or-fall_from_to_to.csv`, with no trailing measurement counter
  at all (checked by `ls`-ing `v2/data/` directly rather than trusting the user's typed example,
  which had turned out to include a counter the real files didn't). Added a
  private `waitForTempCtrl` helper (same wait shape as `LabJackRecordData`/`TempCtrlPlotTemp`, no
  recording) to position at `startTemp` before the first (unrecorded) leg. Hit the *exact* same
  `interval_?NumericQ:1` silent-non-binding bug as `TempCtrlPlotTemp` earlier and caught it via a
  mocked-hardware test (`Block`-overriding `TempCtrlGetTemp`/`TempCtrlOn`/`TempCtrlSetSetpoint`/
  `LabJackRecordData` to avoid needing real hardware) before it shipped -- fixed the same way, with
  a two-clause definition instead of the combined optional/PatternTest. Also verified via the same
  mock harness that a mid-sweep `LabJackRecordData` failure aborts the rest of the sweep rather
  than continuing with bad data.
- **2026-08-07** — Cleared the whole post-29/29 code-review backlog (`docs/BUGS.md` #15-24, ten
  entries, all now fixed). Three were real crash risks: #15 `servos_homed()` null-deref (added the
  same null check `servos_ready()` already used); #16 an uncaught exception from a garbled reply
  while probing the wrong device type could `std::terminate()` the whole kernel process (wrapped
  the probe body in `try`/`catch`, and added length guards to both `Oven5R6900::parse_response`
  and `RTE7::parse_float_response` so a too-short reply returns the existing `-999` sentinel
  instead of reading out of bounds — two new doctest cases cover this); #17 a timed-out probe
  leaked its exclusive port handle forever (now calls `CancelSynchronousIo()` on the stuck worker
  thread before giving up, the Windows-documented way to unblock a pending synchronous read/write
  from another thread, with no risk of the calling thread ever blocking on it). Also fixed: #18/#19
  rewrote the port-probing code (`probe_with_timeout()` → `start_probe()` + `find_port_by_probe()`)
  so all candidates are probed concurrently against one shared deadline instead of sequentially at
  full timeout each, and `init_bath()`/`init_temp_controller()`'s reopen-after-probe delay is now a
  bounded 3-attempt retry (`open_persistent_connection()`) instead of one fixed `Sleep(150)` guess;
  #20 fixed a stale doc claiming servos still auto-start on package load; #21 merged the pointless
  `initialize_servos()`/`initialize_servos_impl()` split back into one function; #22 fixed a
  tautological `GetLogFile[]` test that compared against the same variable used to configure
  logging, so it could never actually fail; #23/#24 fixed the DLL wrapper layer in
  `src/wolfram_api.cpp` two ways — status codes are now properly mapped
  (`lab_status != 0 ? LIBRARY_FUNCTION_ERROR : LIBRARY_NO_ERROR`) instead of passing a raw `1`
  through as the LibraryLink status (which collided with `LIBRARY_TYPE_ERROR` and printed a
  misleading "inconsistent types" message), and every previously-uninitialized local that gets
  written to `Res` now starts from a `0`/`0.0` sentinel so a failure path can no longer return
  stack garbage disguised as a real reading. `ServoHome[]` specifically also gained a real `Res`
  write it never had before (it used to return undefined memory unconditionally) — had to match
  the existing `.wlt` test's expectation of `0` on success, not the `1`-for-true guess tried first.
  Verified via `wolframscript`: full suite still 29/29 against real hardware after every fix.
