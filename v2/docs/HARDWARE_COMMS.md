# How This App Talks to the Hardware

One page explaining how each piece of lab equipment is wired into the app, and whether any of it
works outside Windows. (Merges what used to be `CHANGES_DETAILS.md`, `HARDWARE_PROTOCOLS.md`, and
`notes`.)

## Quick reference

| Device | Talks over | Who provides the driver | Needs a COM port? |
|---|---|---|---|
| Fluid bath (Neslab RTE7) | Windows serial API (raw bytes) | We wrote it (`RTE7.cpp`) | Yes — auto-detected |
| Temp controller (Oven 5R6-900) | Windows serial API (ASCII text) | We wrote it (`Oven5R6900.cpp`) | Yes — auto-detected |
| Servo motors (Teknic ClearPath) | Teknic's own SDK | Teknic (`sFoundation20.dll`) | No — SDK finds its own hub |
| Data acquisition (LabJack) | LabJack's own SDK | LabJack (`LabJackUD.dll`) | No — SDK finds the device over USB |
| Mathematica ↔ this DLL | Wolfram LibraryLink | Wolfram | N/A (see [specs/librarylink.md](specs/librarylink.md)) |

The short version: **the bath and temp controller are the only two devices where our code talks
directly to a serial port** — everything else hides its connection details behind a
manufacturer-provided SDK.

## Fluid bath & temp controller: why they need special handling

Both devices plug in through the exact same model of USB-to-serial adapter chip. That causes two
problems, in order of how badly they used to bite:

1. **The COM number isn't stable.** Windows reassigns COM numbers on replug/reboot, so a
   hardcoded `"COM5"` breaks the next time someone unplugs a cable.
2. **The two adapters look identical to Windows.** Even matching by hardware ID or friendly name
   can't tell "the bath's adapter" apart from "the temp controller's adapter" — they report the
   same VID/PID.

The fix (see [specs/port-autodetect.md](specs/port-autodetect.md) for the full spec) is: find
every port that *could* be one of these adapters, then send each one a real command in the
device's own protocol and see which one answers correctly. Whichever port replies is that device
— implemented in `src/lab.cpp` (`find_serial_adapter_ports`, `find_bath_port`,
`find_temp_controller_port`).

The adapters' current chipset is FTDI FT232 (`VID_0403&PID_6001`) — matched in
`find_serial_adapter_ports()`. The rig previously used Prolific `PL2303` adapters
(`VID_067B&PID_2303`); those were swapped out due to a Windows driver compatibility problem (see
"One hardware caveat worth knowing" below). If the hardware ever changes chipset again, that
hardcoded VID/PID string is the one line to update.

### Fluid bath (RTE7) — binary protocol
`include/controls/RTE7.h`, `src/RTE7.cpp`

- Opens the port at 19200 baud, 8 data bits, 1 stop bit, no parity.
- Every message is raw bytes starting with `0xCA` and ending with a checksum byte (sum of all
  bytes after `0xCA`, low byte, XOR `0xFF`).
- Example: turning the bath on sends the fixed byte string
  `CA 00 01 81 08 01 02 02 02 02 02 02 02 66`.
- Reading a value (temperature, setpoint) means sending a short "give me X" message and parsing a
  reply that encodes a precision exponent plus a raw number.

### Temp controller (Oven 5R6-900) — text protocol
`include/controls/Oven5R6900.h`, `src/Oven5R6900.cpp`

- Same serial settings (19200, 8N1), but the protocol is ASCII text, not raw binary.
- Every message looks like `*00<2-char command><8 hex digits of value><2-char checksum>\r`.
- Example: reading the temperature sends command `01`; reading the setpoint sends `40`. Values are
  scaled integers (e.g. setpoint is sent as `temp * 100` so the ASCII stays integer-only).

Both devices' checksum failures, malformed replies, or a closed port all come back as a plain
`false`/`0` — see `BUGS.md` for known gaps in how those failures get surfaced to Mathematica.

## Servo motors — Teknic SDK

`src/lab.cpp` (`initialize_servos`, `shutdown_servos`, etc.)

The Teknic ClearPath motors connect through Teknic's own `sFoundation20` SDK
(`Mgr->FindComHubPorts(...)`), which finds its USB hub itself. Our code never touches a COM port
string for this device — we just ask the SDK for "the one connected hub" and it hands back a
manager object. If it finds zero or more than one hub, we bail out rather than guess.

## LabJack — LabJack UD SDK

`src/lab.cpp` (`read_labjack_ain`)

Same idea: `OpenLabJack(LJ_dtU3, LJ_ctUSB, "0", 1, &h)` asks LabJack's own driver for "the first U3
found over USB." No COM port, no manual configuration.

## Mathematica ↔ our C++ DLL

Mathematica never touches the hardware directly — it calls into our compiled DLL through Wolfram's
**LibraryLink** interface, and our DLL calls the device code above. This is its own separate
concept (not device communication, but *Mathematica-to-C++* communication), documented fully in
[specs/librarylink.md](specs/librarylink.md).

## Does any of this work on Mac or Linux?

**No, not today.** `CMakeLists.txt` has scaffolding that picks a different output folder name per
OS (`Windows-x86-64` / `MacOSX-ARM64` / `MacOSX-x86-64` / `Linux-x86-64`) — that's just Wolfram's
convention for where a paclet expects to find a platform's compiled binary, so a paclet *can* ship
binaries for multiple platforms.

But the actual code that would need to run on that binary is Windows-only:

- `RTE7.cpp` and `Oven5R6900.cpp` both talk to the serial port through the raw Win32 API
  (`CreateFileA`, `ReadFile`, `WriteFile`, the `DCB` struct) — there's no POSIX (`termios`)
  equivalent written yet.
- Auto-detecting the bath/temp-controller ports uses Windows' `SetupAPI`
  (`SetupDiGetClassDevsA` and friends, in `src/lab.cpp`) — Linux would use something like `udev`
  or scanning `/dev/serial/by-id/`, and Mac would use IOKit. Neither is implemented.
  See [specs/port-autodetect.md](specs/port-autodetect.md).
- Manual (keyboard-driven) servo jogging installs a low-level Windows keyboard hook
  (`SetWindowsHookEx` in `src/lab.cpp`) — this has no direct equivalent on other platforms.
- `CMakeLists.txt`'s `find_path`/`find_library` calls for Wolfram, LabJack, and Teknic all use
  hardcoded Windows install paths (`C:/Program Files/...`) as search hints.

So: porting to Mac or Linux would mean rewriting the serial I/O layer, the port-autodetect logic,
and the manual-control keyboard handling, plus pointing CMake at each SDK's install location on
that platform — it's a real port, not a recompile.

## One hardware caveat worth knowing

The rig previously used Prolific `PL2303`-family adapters, which have spotty driver support on
modern Windows — Prolific's official stance for older `TA`-series chips is "buy a newer adapter"
rather than releasing an updated driver. That's exactly what happened: one adapter started
showing up in Device Manager as `PL2303TA DO NOT SUPPORT WINDOWS 11 OR LATER, PLEASE CONTACT YOUR
SUPPLIER` instead of getting a COM port at all. The bath and temp controller now use FTDI FT232
adapters instead (`VID_0403&PID_6001`, matched in `find_serial_adapter_ports()`), which don't have
this problem. If a port ever stops responding after a Windows update again, check whether it's a
driver problem before assuming it's a wiring or code issue.
