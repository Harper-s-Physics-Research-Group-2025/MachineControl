# API Function Reference: What Each Function Returns

A function-by-function reference for exactly what value each public Wolfram symbol returns, and
what that value means on both success and failure. Cross-references
[README.md's Known Confusing Error Messages](../README.md#known-confusing-error-messages) section
and `BUGS.md` #23-24, which cover the two systemic issues that make some of the "on failure"
columns below look the way they do.

**How to read the "On failure" column:**
- **Throws, no value** — the call raises a `LibraryFunctionError`; there's no return value to use.
- **`1`-as-error** — returns the integer `1`, which Mathematica may print as
  `LibraryFunctionError[LIBRARY_TYPE_ERROR, 1]` ("inconsistent types") rather than a clean `1` you
  can check (bug #23). Still check for this — don't assume a thrown message means it isn't `1`.
  it means the call failed, whatever the message says.
- **Garbage** — the returned *value* itself is meaningless uninitialized memory, in addition to
  whatever status/error accompanies it (bug #24). Don't use the value without checking success
  first.
- **Echoes input** — on failure, returns the *requested* input back unchanged, not the actual
  resulting state (a milder version of the "garbage" problem, only affects `ServoSetPos[]`).

## Logging

| Function | Returns (success) | On failure |
|---|---|---|
| `GetLogStatus[]` | `Integer` — `1` if logging is enabled, `0` if disabled | Always succeeds |
| `GetLogFile[]` | `String` — the current log file path | Always succeeds |
| `SetLogSettings[status, logfile]` | `Integer` — echoes back `status` | `1`-as-error if `logfile`'s parent directory doesn't exist |

## Fluid Bath

| Function | Returns (success) | On failure |
|---|---|---|
| `BathInit[]` | `Integer` `0` | `1`-as-error (no matching port found, or found but didn't connect) |
| `BathOn[]` / `BathOff[]` / `BathManual[]` | `Integer` `0` | `1`-as-error (not initialized, or the hardware didn't acknowledge) |
| `BathGetTemp[]` | `Real` — temperature in °C | `1`-as-error **and Garbage** — the returned number is meaningless, not just the error |
| `BathGetSetpoint[]` | `Real` — setpoint in °C | Same as `BathGetTemp[]` |
| `BathSetSetpoint[temp]` | `Real` — the bath's *confirmed* setpoint after the write (bug #26's fix: no longer a blind echo of `temp`) | `1`-as-error. A returned value that differs from `temp` means the bath silently clamped the request to its own configured Hi/Lo Temperature Limit — check the bath's physical Setup Loop (`Hit`/`Lot`) |

## Temperature Controller

| Function | Returns (success) | On failure |
|---|---|---|
| `TempCtrlInit[]` | `Integer` `0` | `1`-as-error |
| `TempCtrlOn[]` / `TempCtrlOff[]` | `Integer` `0` | `1`-as-error |
| `TempCtrlGetMode[]` | `Integer` — `0` = normal, `2` = ramp/soak | `1`-as-error **and Garbage** |
| `TempCtrlSetMode[mode]` | `Integer` — echoes back `mode` | `1`-as-error (safe echo — it's your own input, not an uninitialized local) |
| `TempCtrlGetTemp[]` | `Real` — thermistor temperature in °C | `1`-as-error **and Garbage** |
| `TempCtrlGetSetpoint[]` | `Real` — setpoint in °C | `1`-as-error **and Garbage** |
| `TempCtrlSetSetpoint[temp]` | `Real` — echoes back `temp` | `1`-as-error (safe echo) |

## Data Acquisition

| Function | Returns (success) | On failure |
|---|---|---|
| `ReadLabjack[channel]` | `Real` — voltage reading | Throws with a raw LabJack UD error code **and Garbage** — the voltage value is meaningless |

### What `channel` actually is

`channel` is a physical pin number on the LabJack U3, not an arbitrary index. The U3 exposes its
first 8 "Flexible I/O" lines (FIO0-FIO7) on built-in screw terminals — this is what you're
actually wiring a sensor into:

```
LabJack U3 screw terminals (front panel)

  GND    VS    FIO0   FIO1   FIO2   FIO3   FIO4   FIO5   FIO6   FIO7
  |-|    |-|   |-|    |-|    |-|    |-|    |-|    |-|    |-|    |-|
                ch 0   ch 1   ch 2   ch 3   ch 4   ch 5   ch 6   ch 7
                └──────────────────┘ └──────────────────────────────┘
                 fixed analog-only    switchable digital/analog
                 (U3-HV only)         (this is what ANALOG_ENABLE_BIT is for)
```

- **`channel` 0-7** = `FIO0`-`FIO7`, the screw terminals above.
- **`channel` 8-15** = `EIO0`-`EIO7`, 8 more analog-capable lines on the DB15 connector (not on
  the screw terminals — a ribbon/breakout cable is needed to reach these).
- **`channel` 16-19** = `CIO0`-`CIO3`, also on the DB15 connector, but **digital-only** — not
  usable with `ReadLabjack[]` at all.

On the **U3-HV** variant, `FIO0`-`FIO3` (channels 0-3) are permanently hardwired as analog input
only, with a ±10V range — any digital/analog configuration on those 4 is ignored by the device.
Every other analog-capable channel (4-15) starts out configured as *digital* I/O by default and
has to be explicitly switched into analog mode before it'll give a meaningful voltage reading —
this is exactly what the `LJ_ioPUT_ANALOG_ENABLE_BIT` call in `Lab::read_labjack_ain`
(`src/lab.cpp`) does, immediately before every read:

```cpp
ePut(h, LJ_ioPUT_ANALOG_ENABLE_BIT, channel, 1, 0);  // switch this channel into analog mode
errorcode = eGet(h, LJ_ioGET_AIN, channel, &voltage, 0);  // then read its voltage
```

So `ReadLabjack[4]`, for example, reads whatever's wired into the `FIO4` screw terminal, after
first flipping that pin from its default digital mode into analog mode.

Source: LabJack's own U3 datasheet —
[2.6.1 Channel Numbers](https://support.labjack.com/docs/2-6-1-channel-numbers-u3-datasheet),
[2.5 Flexible I/O](https://support.labjack.com/docs/2-5-flexible-i-o-fio-eio-u3-datasheet).

### LabJack Data-Collection Suite

Pure Wolfram Language, defined directly in `paclet/Kernel/WolframMachineControl.wl` — no
LibraryLink/DLL involved. Built on top of `ReadLabjack[]`/`BathGetTemp[]`, so their failure modes
above still apply underneath these.

| Function | Returns (success) | On failure |
|---|---|---|
| `LabJackRecordData[filename, finalTemp, interval]` | `String` — full path to the saved `v2/data/<filename>.csv` | `$Failed` **and** `LabJackRecordData::bathfail`/`::tempctrlfail` if `BathGetTemp[]`/`TempCtrlGetTemp[]` doesn't return a real number before the loop even starts (e.g. `BathInit[]`/`TempCtrlInit[]` was never called) |
| `LabJackListCSVs[]` | `Grid` — file name + creation date for every CSV in `v2/data`, newest first | `{}` (and a `Print[]`) if `v2/data` has no CSVs — never throws |
| `LabJackPlotData[filename]` | `Legended[Graphics, ...]` — one line per channel, plotted against the bath-temperature column | `$Failed` **and** `LabJackPlotData::nofile` if `<filename>.csv` doesn't exist in `v2/data` |

**Assumptions baked into `LabJackRecordData`, worth knowing if your setup differs:**
- Calls `BathOn[]`/`BathSetSetpoint[finalTemp]` **and** `TempCtrlOn[]`/`TempCtrlSetSetpoint[finalTemp]`
  itself before recording — it actively drives both devices toward `finalTemp`, it doesn't just
  passively wait for something else to get them there. Both devices must actually be initialized
  (`BathInit[]`/`TempCtrlInit[]`) first, or it fails fast with `LabJackRecordData::bathfail`/
  `::tempctrlfail` before turning anything on.
- The loop only stops once **both** devices have reached their target — whichever gets there first
  just keeps being read every interval while waiting on the other.
- The bath's side of the stop condition uses its *confirmed* setpoint (`BathSetSetpoint[]`'s return
  value per bug #26's fix), not the raw `finalTemp` argument — if the bath silently clamps an
  out-of-range request to its own Hi/Lo Temperature Limit, `LabJackRecordData::clamped` fires and
  the loop waits for the clamped value instead of looping forever for a temperature the bath will
  never actually reach. **The temp controller has no equivalent protection** — `TempCtrlSetSetpoint[]`
  only echoes back whatever was requested (bug #24), so there's no way to detect a silent clamp on
  that side; an out-of-range `finalTemp` for the temp controller could still loop forever.
- Reads exactly channels 0-7 (`FIO0`-`FIO7`) every interval, not a caller-specified list.
- The stop direction (wait for temperature to rise vs. fall) is inferred once per device, from its
  temperature at the moment the function is called vs. its target — it isn't re-evaluated mid-run.
- Blocks the kernel for the entire run (a plain `While` loop with `Pause[interval]`) — there's no
  background/async mode.

## Servo Motors

| Function | Returns (success) | On failure |
|---|---|---|
| `ServoEnable[]` | `Integer` `0` | `1`-as-error, **or `-1`** on a Teknic SDK exception (an unusual value matching no named LibraryLink error — its on-screen presentation is unpredictable) |
| `ServoDisable[]` | `Integer` `0` | Always succeeds |
| `ServoGetAlerts[]` | `String` — `"X: <alert text> \| Z: <alert text>"` | `1`-as-error, but the string comes back as `"X:  \| Z: "` (empty, not garbage — the buffers are pre-zeroed) |
| `ServoHome[milliseconds]` | **Undefined, always** — see below | Same: undefined, always |
| `ServoHomed[]` | `Integer` — `1` = `True` (homed), `0` = `False` | N/A — always returns a clean answer, never throws (bug #3's fix) |
| `ServoReady[]` | `Integer` — `1` = `True` (enabled + no alerts), `0` = `False` | N/A — same, always clean |
| `ServoGetPos[]` | `{Real, Real}` — `{x_mm, z_mm}` | `1`-as-error **and Garbage** — both coordinates are meaningless |
| `ServoSetPos[x_mm, z_mm, rpm]` | `{Real, Real, Real}` — the *actual* resulting `{x_mm, z_mm, rpm}` after the move | `1`-as-error, and **Echoes input** — you get back the position you asked for, not confirmation it was reached |
| `ServoManualControl[]` | `Integer` `0` | Not currently reachable — always returns `0` in practice |

`ServoHome[milliseconds]`'s wrapper (`wservos_home` in `src/wolfram_api.cpp`) never calls any
`MArgument_set*(Res, ...)` on any code path — success or failure. Its `.wl` binding still declares
an `Integer` return type, so **the returned value carries no information at all, ever.** The only
thing worth checking after calling it is whether it throws a `LibraryFunctionError` — the actual
returned integer (thrown or not) should not be used for anything.

## The two systemic caveats behind most of the "On failure" column

1. **Status-code collision** (`BUGS.md` #23): most of these functions' underlying `Lab::`
   functions return a plain `1` for "didn't work," which is also the numeric value of
   `LIBRARY_TYPE_ERROR`. Mathematica has no way to distinguish "a real type mismatch" from
   "the function just failed," so it prints "inconsistent types" either way. Don't take that
   message literally — check whether you called the right `*Init[]`/`ServoEnable[]` first.
2. **Uninitialized-on-failure locals** (`BUGS.md` #24): several wrappers declare their output
   variable, pass it to the underlying `Lab::` call, and hand it to Mathematica regardless of
   whether that call actually filled it in. On failure, these functions return a real-looking but
   meaningless number *in addition to* an error/status — always check for success before trusting
   a numeric result from the functions marked **Garbage** above.

Neither of these is fixed yet as of this writing — see `docs/BUGS.md` #23 and #24 for the
tracked fix plan.
