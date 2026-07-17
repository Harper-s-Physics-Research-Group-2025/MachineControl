# Spec: Linking Our C++ Code to Mathematica

**What problem this solves:** Mathematica can't call a plain DLL function directly — it needs a
translation layer that knows how to convert Wolfram values (integers, reals, strings) into C
types and back. Wolfram's **LibraryLink** is that translation layer, and this app is built
entirely around it.

## The chain, end to end

```
Mathematica notebook
      |  calls e.g. BathOn[]
      v
WolframMachineControl.wl   <-- Wolfram-side wrapper, one line per function
      |  LibraryFunctionLoad binds the symbol to a named DLL export
      v
wolfram_machine_controller.dll   <-- our compiled C++
      |  wolfram_api.cpp: extern "C" function, unpacks arguments
      v
Lab:: namespace (lab.cpp)   <-- actual device logic
      |
      v
RTE7 / Oven5R6900 / Teknic SDK / LabJack SDK   <-- talks to real hardware
```

## 1. The DLL side: `src/wolfram_api.cpp`

Every function Mathematica can call is declared like this:

```cpp
DLLEXPORT int wbath_on(WolframLibraryData lp, mint Argc, MArgument *Args, MArgument Res){
    return Lab::bath_on();
}
```

- `DLLEXPORT` — a macro from Wolfram's `WolframLibrary.h` that marks the function for export from
  the DLL (so it shows up as a callable symbol, similar to marking a function `public`).
- Every LibraryLink function has the **same fixed signature**: `WolframLibraryData`, an argument
  count (`Argc`), an array of arguments (`Args`), and a result slot (`Res`) — regardless of how
  many Wolfram-side arguments the function logically takes.
- Arguments are unpacked with helpers like `MArgument_getReal(Args[0])`,
  `MArgument_getUTF8String(Args[1])`, etc. — see `wset_log_settings` for an example that reads two
  arguments of different types.
- Results go out through `Res`, using `MArgument_setInteger(Res, ...)`,
  `MArgument_setReal(Res, ...)`, etc. — never a normal C++ `return` for the actual data.
- The function's actual `return` value is a **status code**, not the answer — `0`
  (`LIBRARY_NO_ERROR`) means success; anything else tells Mathematica the call failed (and which
  way). This is easy to get wrong: see `docs/BUGS.md` #3 for a case where a plain success/fail boolean
  got returned directly as this status code and silently meant something else to Mathematica.
- Two special lifecycle hooks Wolfram calls automatically, not from user code:
  `WolframLibrary_initialize` (runs once when the DLL is first loaded — starts up the servos here)
  and `WolframLibrary_uninitialize` (runs on unload — tears down the bath/temp
  controller/servos).

## 2. The Wolfram side: `paclet/Kernel/WolframMachineControl.wl`

Two things happen in this file for every function:

**A usage string** (public documentation, shown by `?BathOn` in Mathematica):
```wolfram
WolframMachineControl`BathOn::usage = "BathOn[] activates the fluid bath equipment."
```

**A binding** that connects a Wolfram symbol to a named export in the DLL, plus its argument and
return types:
```wolfram
WolframMachineControl`BathOn = LibraryFunctionLoad[$dllPath, "wbath_on", {}, Integer];
```
Reading this left to right: *"the Wolfram function `BathOn`, when called, should run the DLL's
`wbath_on` export, which takes no arguments (`{}`) and returns an `Integer`."* The argument-type
list here (`{}`, `{Real}`, `{Integer, UTF8String}`, etc.) **must exactly match** how many/what type
of arguments the C++ side reads out of `Args[]` — a mismatch is exactly what produced the
`LibraryFunction::cfct` argument-count error seen earlier while testing `BathOn`.

`$dllPath` is found automatically via `FindLibrary["wolfram_machine_controller"]` — nobody needs to
hardcode a path to the compiled DLL.

## 3. Telling Mathematica this is a LibraryLink paclet: `paclet/PacletInfo.wl`

```wolfram
"Extensions" -> {
    {"Kernel", "Root" -> "Kernel", "Context" -> {"WolframMachineControl`"}},
    {"LibraryLink"}
}
```
The `"LibraryLink"` extension is what tells Mathematica's paclet loader this package ships a
native library at all, and the `"Kernel"` extension points at `WolframMachineControl.wl` as the
code to load into that context.

## 4. Getting the compiled DLL into the right folder: `CMakeLists.txt`

Wolfram expects a paclet's native binary to live at a specific, per-platform path inside the
paclet folder (`LibraryResources/<platform>/`). After every build, CMake copies the freshly built
DLL (and Teknic's `sFoundation20.dll`, which it also depends on) there automatically:

```cmake
add_custom_command(TARGET wolfram_machine_controller POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:wolfram_machine_controller>" "${PACLET_DEST_DIR}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${TEKNIC_RUNTIME_DLL}" "${PACLET_DEST_DIR}/"
)
```
Nobody needs to move the built `.dll` by hand — `cmake --build .` leaves it exactly where
`FindLibrary` in step 2 expects to find it.

## Files touched when adding a new function

Adding one new callable function to Mathematica means touching all three of these, in order:
1. `src/lab.cpp` — the actual logic (usually calling into a device class).
2. `src/wolfram_api.cpp` — a new `DLLEXPORT` wrapper that unpacks `Args`/`Res` and calls step 1.
3. `paclet/Kernel/WolframMachineControl.wl` — a `::usage` string plus a
   `LibraryFunctionLoad` binding whose argument/return types match step 2 exactly.
