# Spec: Auto-Detecting the Bath and Temp Controller COM Ports

**What problem this solves:** `BathInit`/`TempCtrlInit` used to require a hardcoded COM port
string (e.g. `BathInit["COM5"]`). Windows reassigns COM numbers across replugs/reboots, so a
hardcoded string quietly goes stale — the observed port for one adapter has moved between COM3,
COM7, COM9, and COM5 across sessions. Worse, both the bath and the temp controller connect through
an *identical* model of USB-to-serial adapter (currently FTDI FT232, `VID_0403&PID_6001`; formerly
Prolific `VID_067B&PID_2303` before that adapter was retired — see `docs/HARDWARE_COMMS.md`), so even
matching by hardware ID or device name can only narrow candidates down, never uniquely identify
either device.

**The fix, in one sentence:** find every COM port that *could* be one of these two adapters, then
ask each candidate a real question in each device's own protocol — whichever port gives a correct,
checksummed answer is that device.

## Where it's implemented

All in `src/lab.cpp` (declared in `include/controls/lab.h`):

| Function | Job |
|---|---|
| `find_serial_adapter_ports()` | Enumerates every Windows COM port whose USB hardware ID is `VID_0403&PID_6001` (FTDI FT232). Returns a list of COM port names (e.g. `["COM10", "COM11"]`) — candidates, not yet identified. |
| `find_bath_port()` | Opens each candidate as an `RTE7` and sends a read-only "get setpoint" query. The first port that replies with a valid, checksummed response is the bath. |
| `find_temp_controller_port()` | Same idea, opening each candidate as an `Oven5R6900` instead and probing with *its* protocol. |
| `init_bath()` | Calls `find_bath_port()`, then connects (was previously `init_bath(std::string COMM)`, taking a hardcoded port). |
| `init_temp_controller()` | Same, using `find_temp_controller_port()`. |

A full line-by-line walkthrough of `find_serial_adapter_ports()` itself, with where each Windows
type/function comes from, is in the "Line-by-Line" section further down this file.

## How the disambiguation actually works

1. **Narrow the search.** `find_serial_adapter_ports()` uses the Windows `SetupAPI` to list every
   currently-connected COM port device, and filters to only those reporting hardware ID
   `VID_0403&PID_6001` (FTDI). This step alone can return 0, 1, or 2 ports — it cannot tell the
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
> port. Fixed as `docs/BUGS.md` #10; the checksum is now actually enforced.

This approach is robust to the exact failure mode that motivated it: even if the two adapters get
unplugged and replugged into different physical USB ports (changing their COM numbers, and
potentially even which one Windows lists first), the protocol probe still identifies each one
correctly by what it *is*, not by what port number it happens to have right now. It's also robust
to the adapters being replaced with a different chipset entirely — see the note below.

## What this does **not** solve

- **It's Windows-only.** The port enumeration uses the Win32 `SetupAPI`
  (`SetupDiGetClassDevsA` and friends) — see [`../docs/HARDWARE_COMMS.md`](../docs/HARDWARE_COMMS.md#does-any-of-this-work-on-mac-or-linux)
  for what a Mac/Linux port would need instead.
- **It doesn't fix a bad connection, only finds it.** If a matching adapter is plugged in but the
  actual device behind it is powered off or miswired, its probe will simply fail like any other
  non-matching candidate — `find_bath_port()`/`find_temp_controller_port()` return `""`, and
  `BathInit[]`/`TempCtrlInit[]` report a normal failure code.
- **Startup latency scales with candidate count.** Each candidate probe is a real serial
  round-trip (send + wait for a timeout or a reply), now hard-capped at 1.5 seconds per candidate
  (see `BUGS.md` #12) since the device's own configured timeouts turned out not to always be
  trustworthy. With only two matching adapters ever expected on this system, worst case (both
  candidates stall) is ~3 seconds — not instant, and it would grow if more identical adapters were
  ever added to the same machine.
- **The hardware ID is hardcoded.** `find_serial_adapter_ports()` matches one specific VID/PID
  string. If the adapters are ever swapped for a different chipset again, that string is the one
  line that needs updating — it happened once already (Prolific → FTDI), see `docs/HARDWARE_COMMS.md`.

## Line-by-Line: `find_serial_adapter_ports()`

Everything Windows-specific here is from **SetupAPI** (`setupapi.h`) and **device class GUIDs**
(`devguid.h`) — Windows SDK headers, not the C++ standard library.

```cpp
std::vector<std::string> find_serial_adapter_ports() {
    std::vector<std::string> found_ports;
```
`std::vector`/`std::string` — standard library (`<vector>`/`<string>`, pulled in via `lab.h`). This
will hold every COM port name that matches the FTDI hardware ID.

```cpp
    HDEVINFO device_info = SetupDiGetClassDevsA(&GUID_DEVCLASS_PORTS, nullptr, nullptr, DIGCF_PRESENT);
    if (device_info == INVALID_HANDLE_VALUE) return found_ports;
```
- `HDEVINFO` — `setupapi.h`. An opaque handle type (really just a typedef'd pointer) representing a
  "device information set" — Windows' in-memory list of devices matching some criteria.
- `SetupDiGetClassDevsA` — `setupapi.h`. The `A` suffix means the ANSI (narrow `char`, not
  `wchar_t`) variant. This call builds that device list.
  - `&GUID_DEVCLASS_PORTS` — `devguid.h`. A predefined `GUID` constant identifying the "Ports (COM
    & LPT)" device setup class — i.e. "give me devices in this category," not a specific device.
  - The two `nullptr`s are optional filters (`Enumerator`, `hwndParent`) this call doesn't use.
  - `DIGCF_PRESENT` — `setupapi.h`, a flag constant meaning "only devices currently plugged
    in/present," not every device Windows has ever seen.
- `INVALID_HANDLE_VALUE` — `winbase.h` (pulled in via `windows.h`), a generic sentinel
  (`(HANDLE)-1`) reused here as the failure return value for `HDEVINFO` too. If the call fails,
  bail out with an empty result.

```cpp
    SP_DEVINFO_DATA device_data = { 0 };
    device_data.cbSize = sizeof(SP_DEVINFO_DATA);
```
`SP_DEVINFO_DATA` — `setupapi.h`. A struct that identifies one specific device *within* that set
(analogous to how `HDEVINFO` represents the whole list). Every SetupAPI struct like this needs its
`cbSize` field manually set to its own size before use — this is how the Win32 API does
struct-versioning: the function checks `cbSize` to know which struct layout you're using.

```cpp
    for (DWORD i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_data); ++i) {
```
- `DWORD` — `windef.h` (via `windows.h`), an unsigned 32-bit integer — Win32's standard
  "index/count" type.
- `SetupDiEnumDeviceInfo` — `setupapi.h`. Walks the device set one entry at a time: pass index `i`,
  get `device_data` filled in for that device. Returns `FALSE` once `i` runs past the last device,
  which naturally ends the loop.

```cpp
        char hardware_id[256] = { 0 };
        if (!SetupDiGetDeviceRegistryPropertyA(device_info, &device_data, SPDRP_HARDWAREID,
                nullptr, reinterpret_cast<PBYTE>(hardware_id), sizeof(hardware_id), nullptr)) {
            continue;
        }
```
- `SetupDiGetDeviceRegistryPropertyA` — `setupapi.h`. Reads one property of the current device
  into a caller-supplied buffer. Its full signature is:
  ```cpp
  BOOL SetupDiGetDeviceRegistryPropertyA(
      HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, DWORD Property,
      PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize,
      PDWORD RequiredSize);
  ```
- `SPDRP_HARDWAREID` — `setupapi.h`, one of the `SPDRP_*` enum constants identifying *which*
  property to fetch — here, the device's USB hardware ID string(s) (e.g.
  `USB\VID_0403&PID_6001`).
- The first `nullptr` is `PropertyRegDataType` — we don't care what registry value type it reports
  back (`REG_SZ`, `REG_MULTI_SZ`, etc.), so it's skipped.
- `PBYTE` — `minwindef.h` (via `windows.h`), just `unsigned char*`. The API wants a raw byte
  buffer, so the `char[256]` gets `reinterpret_cast` to that.
- The last `nullptr` is `RequiredSize` — we don't ask how many bytes it actually needed; if our
  fixed 256-byte buffer is too small, the call just fails and `continue` skips that device rather
  than trying to resize and retry.

```cpp
        std::string id(hardware_id);
        if (id.find("VID_0403&PID_6001") == std::string::npos) continue;   // not an FTDI adapter
```
Wraps the raw C string in `std::string` for easy substring search — filters out every device that
isn't an FTDI `0403:6001` adapter (the bath/temp-controller's current USB-to-serial chip).

```cpp
        char friendly_name[256] = { 0 };
        if (!SetupDiGetDeviceRegistryPropertyA(device_info, &device_data, SPDRP_FRIENDLYNAME,
                nullptr, reinterpret_cast<PBYTE>(friendly_name), sizeof(friendly_name), nullptr)) {
            continue;
        }
```
Same API call again, but with `SPDRP_FRIENDLYNAME` instead — this time fetching the human-readable
device name Windows shows in Device Manager, e.g. `"USB Serial Port (COM10)"`.

```cpp
        std::string name(friendly_name);
        size_t open_paren = name.rfind("(COM");
        size_t close_paren = name.rfind(")");
        if (open_paren == std::string::npos || close_paren == std::string::npos) continue;

        found_ports.push_back(name.substr(open_paren + 1, close_paren - open_paren - 1));  // e.g. "COM5"
```
Pure standard-library string work, no Win32 involved: `rfind` locates the last `"(COM"` and last
`")"` in the friendly name, and `substr` slices out just the `COMx` token between them (skipping
the `(`). `rfind` (search from the end) rather than `find` guards against a device name that
happens to contain an earlier stray `(` or `)`.

```cpp
    }

    SetupDiDestroyDeviceInfoList(device_info);
    return found_ports;
}
```
`SetupDiDestroyDeviceInfoList` — `setupapi.h`. Frees the device info set `SetupDiGetClassDevsA`
allocated at the top — every `SetupDiGetClassDevs*` call must be paired with this, or it leaks.
Then the collected list of matching COM port names is returned to the caller
(`find_bath_port`/`find_temp_controller_port`, which will protocol-probe each one).

## Also changed to support this

- `CMakeLists.txt` — links `Setupapi.lib` (needed for `SetupDiGetClassDevsA` and friends).
- `include/controls/lab.h` — `init_bath()`/`init_temp_controller()` dropped their `std::string COMM`
  parameter.
- `src/wolfram_api.cpp` — `winitialize_bath`/`winitialize_temperature_control` dropped the
  `MArgument_getUTF8String` argument read, since there's no port string to unpack anymore.
- `paclet/Kernel/WolframMachineControl.wl` — `BathInit`/`TempCtrlInit`'s
  `LibraryFunctionLoad` argument template changed from `{UTF8String}` to `{}` to match.
- `README.md` and `paclet/Tests/Test_WolframMachineControl.wlt` — updated to call
  `BathInit[]`/`TempCtrlInit[]` with no arguments.

Verified with `cmake --build . --config Release` — compiles clean, DLL copied into the paclet
directory as expected (see [`librarylink.md`](librarylink.md) for how that copy step works).
