# `find_prolific_ports()` Walkthrough

Line-by-line breakdown of `src/lab.cpp`'s `find_prolific_ports()`, with the header each Windows
type/constant/function comes from. Everything Windows-specific here is from **SetupAPI**
(`setupapi.h`) and **device class GUIDs** (`devguid.h`) — Windows SDK headers, not the C++
standard library.

```cpp
std::vector<std::string> find_prolific_ports() {
    std::vector<std::string> found_ports;
```
`std::vector`/`std::string` — standard library (`<vector>`/`<string>`, pulled in via `lab.h`). This
will hold every COM port name that matches the Prolific hardware ID.

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
  `USB\VID_067B&PID_2303`).
- The first `nullptr` is `PropertyRegDataType` — we don't care what registry value type it reports
  back (`REG_SZ`, `REG_MULTI_SZ`, etc.), so it's skipped.
- `PBYTE` — `minwindef.h` (via `windows.h`), just `unsigned char*`. The API wants a raw byte
  buffer, so the `char[256]` gets `reinterpret_cast` to that.
- The last `nullptr` is `RequiredSize` — we don't ask how many bytes it actually needed; if our
  fixed 256-byte buffer is too small, the call just fails and `continue` skips that device rather
  than trying to resize and retry.

```cpp
        std::string id(hardware_id);
        if (id.find("VID_067B&PID_2303") == std::string::npos) continue;   // not a Prolific adapter
```
Wraps the raw C string in `std::string` for easy substring search — filters out every device that
isn't a Prolific `067B:2303` adapter (the bath/temp-controller's USB-to-serial chip).

```cpp
        char friendly_name[256] = { 0 };
        if (!SetupDiGetDeviceRegistryPropertyA(device_info, &device_data, SPDRP_FRIENDLYNAME,
                nullptr, reinterpret_cast<PBYTE>(friendly_name), sizeof(friendly_name), nullptr)) {
            continue;
        }
```
Same API call again, but with `SPDRP_FRIENDLYNAME` instead — this time fetching the human-readable
device name Windows shows in Device Manager, e.g. `"Prolific USB-to-Serial Comm Port (COM5)"`.

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
