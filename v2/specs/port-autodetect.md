# Spec: Auto-Detecting the Bath and Temp Controller COM Ports

**What problem this solves:** `BathInit`/`TempCtrlInit` used to require a hardcoded COM port
string (e.g. `BathInit["COM5"]`). Windows reassigns COM numbers across replugs/reboots, so a
hardcoded string quietly goes stale — the observed port for one adapter has moved between COM3,
COM7, COM9, and COM5 across sessions. Worse, both the bath and the temp controller connect through
the *identical* Prolific USB-to-serial adapter (`VID_067B&PID_2303`), so even matching by hardware
ID or device name can only narrow candidates down, never uniquely identify either device.

**The fix, in one sentence:** find every COM port that *could* be one of these two adapters, then
ask each candidate a real question in each device's own protocol — whichever port gives a correct,
checksummed answer is that device.

## Where it's implemented

All in `src/lab.cpp` (declared in `include/controls/lab.h`):

| Function | Job |
|---|---|
| `find_prolific_ports()` | Enumerates every Windows COM port whose USB hardware ID is `VID_067B&PID_2303`. Returns a list of COM port names (e.g. `["COM5", "COM9"]`) — candidates, not yet identified. |
| `find_bath_port()` | Opens each candidate as an `RTE7` and sends a read-only "get setpoint" query. The first port that replies with a valid, checksummed response is the bath. |
| `find_temp_controller_port()` | Same idea, opening each candidate as an `Oven5R6900` instead and probing with *its* protocol. |
| `init_bath()` | Calls `find_bath_port()`, then connects (was previously `init_bath(std::string COMM)`, taking a hardcoded port). |
| `init_temp_controller()` | Same, using `find_temp_controller_port()`. |

For a full line-by-line walkthrough of `find_prolific_ports()` and where each Windows type/function
comes from, see [`../SETUPAPI.md`](../SETUPAPI.md).

## How the disambiguation actually works

1. **Narrow the search.** `find_prolific_ports()` uses the Windows `SetupAPI` to list every
   currently-connected COM port device, and filters to only those reporting hardware ID
   `VID_067B&PID_2303` (Prolific). This step alone can return 0, 1, or 2 ports — it cannot tell the
   bath and temp controller apart, since they're the same physical adapter chip.
2. **Ask each candidate a question only the real device can answer correctly.** Both `RTE7` and
   `Oven5R6900` already have a `get_setpoint()` method that sends a real command and validates the
   reply's checksum. Opening a candidate port as the *wrong* device type and sending it the *wrong*
   protocol's bytes will simply fail the checksum check (or time out) — it does not risk actuating
   the hardware, because the probe is a read-only query, never a write/set command.
3. **First correct answer wins.** `find_bath_port()`/`find_temp_controller_port()` return the first
   candidate port that answers correctly, or `""` if none did (ambiguous or nothing connected).

> **Correction:** step 2 above wasn't actually true until it was caught by live hardware testing.
> `RTE7`/`Oven5R6900`'s response parsers detected a checksum mismatch but returned a `-999`
> sentinel instead of surfacing it as a failure, so `get_setpoint()` reported success as long as
> *any* bytes came back — checksum-valid or not. That let the probe falsely "confirm" the wrong
> port. Fixed as `BUGS.md` #10; the checksum is now actually enforced.

This approach is robust to the exact failure mode that motivated it: even if the two Prolific
adapters get unplugged and replugged into different physical USB ports (changing their COM
numbers, and potentially even which one Windows lists first), the protocol probe still identifies
each one correctly by what it *is*, not by what port number it happens to have right now.

## What this does **not** solve

- **It's Windows-only.** The port enumeration uses the Win32 `SetupAPI`
  (`SetupDiGetClassDevsA` and friends) — see [`../COMMUNICATION.md`](../COMMUNICATION.md#does-any-of-this-work-on-mac-or-linux)
  for what a Mac/Linux port would need instead.
- **It doesn't fix a bad connection, only finds it.** If a Prolific adapter is plugged in but the
  actual device behind it is powered off or miswired, its probe will simply fail like any other
  non-matching candidate — `find_bath_port()`/`find_temp_controller_port()` return `""`, and
  `BathInit[]`/`TempCtrlInit[]` report a normal failure code.
- **Startup latency scales with candidate count.** Each candidate probe is a real serial
  round-trip (send + wait for a timeout or a reply). With only two Prolific adapters ever expected
  on this system, this is on the order of a few hundred milliseconds — but it's not instant, and
  it would grow if more identical adapters were ever added to the same machine.

## Also changed to support this

- `CMakeLists.txt` — links `Setupapi.lib` (needed for `SetupDiGetClassDevsA` and friends).
- `include/controls/lab.h` — `init_bath()`/`init_temp_controller()` dropped their `std::string COMM`
  parameter.
- `src/wolfram_api.cpp` — `winitialize_bath`/`winitialize_temperature_control` dropped the
  `MArgument_getUTF8String` argument read, since there's no port string to unpack anymore.
- `WolframMachineControl/Kernel/WolframMachineControl.wl` — `BathInit`/`TempCtrlInit`'s
  `LibraryFunctionLoad` argument template changed from `{UTF8String}` to `{}` to match.
- `README.md` and `WolframMachineControl/Tests/Test_WolframMachineControl.wlt` — updated to call
  `BathInit[]`/`TempCtrlInit[]` with no arguments.

Verified with `cmake --build . --config Release` — compiles clean, DLL copied into the paclet
directory as expected (see [`librarylink.md`](librarylink.md) for how that copy step works).
