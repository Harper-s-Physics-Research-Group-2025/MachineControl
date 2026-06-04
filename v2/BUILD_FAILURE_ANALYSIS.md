# Build Failure Analysis
**Date**: June 4, 2026
**Project**: MachineControl v2 — Josh_Controller
**Status**: RESOLVED — DLL built successfully

**DLL output**: `C:\Projects\MachineControl\v2\Josh_Controller\build\bin\Debug\LabController.dll`

---

## Errors and Fixes (in order encountered)

---

### 1. C++17 not enabled — `<variant>` rejected

**Error**:
```
warning STL4038: The contents of <variant> are available only with C++17 or later.
API.cpp(16): error C2187: syntax error: 'variant' was unexpected here
API.cpp(468): error C2065: 'generic_type': undeclared identifier
API.cpp(473): error C4430: missing type specifier - int assumed
API.cpp(474): error C3861: 'visit': identifier not found
```

**Cause**: The original manual `cl.exe` command lacked `/std:c++17`. Without it, `std::variant`, `std::visit`, `std::is_same_v`, and `generic_type` (a `variant` typedef) all fail. Every downstream error in the file cascaded from this one.

**Fix**: Stopped using the manual `cl.exe` command. The existing `CMakeLists.txt` already had `set(CMAKE_CXX_STANDARD 17)`, so switching to `cmake --build` resolved this automatically. Also created `v2/Josh_Controller/build.ps1` as a manual fallback with `/std:c++17` included.

---

### 2. Wrong include path in CMakeLists.txt

**Error**:
```
error C1083: Cannot open include file: 'API.h': No such file or directory
error C1083: Cannot open include file: 'Oven5R6900.h': No such file or directory
(same for RTE7.h, recorder.h, sm_homer.h, sm_manual_controller.h)
```

**Cause**: `CMakeLists.txt` had:
```cmake
include_directories(
    ../Mathematica/include/
    ../Mathematica/include/Josh_Controller/
    ...
)
```
That path does not exist. The project headers live in `v2/include/`.

**Fix** (`CMakeLists.txt`):
```cmake
include_directories(
    ../include/
    ...
)
```

---

### 3. Namespace qualification missing in `sm_manual_controller.h`

**Errors**:
```
sm_manual_controller.cpp(64): error C2065: 'keyQueue': undeclared identifier
sm_manual_controller.cpp(68): error C2065: 'keys': undeclared identifier
sm_manual_controller.cpp(80): error C2065: 'motorX': undeclared identifier
sm_manual_controller.cpp(81): error C2065: 'motorZ': undeclared identifier
sm_manual_controller.cpp(96): error C2065: 'Mgr': undeclared identifier
sm_manual_controller.cpp(149): error C2511: 'void ManualController::home(sFnd::SysManager*,sFnd::INode*,...)': overloaded member function not found in 'ManualController'
sm_manual_controller.cpp(179): error C2511: 'void ManualController::print_motor_info(sFnd::INode*)': overloaded member function not found in 'ManualController'
```

**Cause**: `sm_manual_controller.h` declared member variables and method parameters using bare `SysManager`, `IPort`, `INode`, `queue`, `vector`, `unordered_set`, `string` — all without namespace qualifiers. The header has no `using namespace` directives, so:
- `SysManager`, `IPort`, `INode` (which live in `namespace sFnd`) were unresolved, making those member declarations silently fail.
- The `.cpp` file has `using namespace sFnd`, so it resolved them as `sFnd::SysManager*` etc. — a different type than the header's unresolved version. Method definitions in the `.cpp` therefore didn't match the header declarations.

**Fix** (`include/sm_manual_controller.h`): Fully qualified all types:
```cpp
// Before
queue<KeyEvent> keyQueue;
SysManager* Mgr = nullptr;
IPort* Port = nullptr;
INode* motorX = nullptr;
INode* motorZ = nullptr;
vector<string> comHubPorts;
unordered_set<DWORD> keys;
void home(SysManager* manager, INode* motor, const char* name, const int timeout);
void print_motor_info(INode* motor) const;

// After
std::queue<KeyEvent> keyQueue;
sFnd::SysManager* Mgr = nullptr;
sFnd::IPort* Port = nullptr;
sFnd::INode* motorX = nullptr;
sFnd::INode* motorZ = nullptr;
std::vector<std::string> comHubPorts;
std::unordered_set<DWORD> keys;
void home(sFnd::SysManager* manager, sFnd::INode* motor, const char* name, const int timeout);
void print_motor_info(sFnd::INode* motor) const;
```

---

### 4. `std::variant` has no `.toString()` method

**Error**:
```
API.cpp(727): error C2039: 'toString': is not a member of 'std::variant<int,float,std::string,bool>'
API.cpp(728): error C2039: 'toString': is not a member of 'std::variant<int,float,std::string,bool>'
```

**Cause**: Three `cout` lines called `.toString()` on `generic_type` (which is `variant<int, float, string, bool>`). `std::variant` has no such method.

**Fix** (`src/API.cpp`): Replaced the calls with a local `std::visit` lambda:
```cpp
auto gts = [](const generic_type& v) -> string {
    return visit([](auto&& x) -> string {
        if constexpr (is_same_v<decay_t<decltype(x)>, string>) return x;
        else if constexpr (is_same_v<decay_t<decltype(x)>, bool>) return x ? "true" : "false";
        else return to_string(x);
    }, v);
};
cout << "Sequence: " << gts(args[0]) << "\nRamp to (C): " << gts(args[1]) << ...
```

---

### 5. `bath_read_temp` declared `void` but assigned to `double`

**Error**:
```
API.cpp(821): error C2440: 'initializing': cannot convert from 'void' to 'double'
```

**Cause**: The DLL wrapper at line 821 did:
```cpp
double temp = equipment.bath_read_temp(comm_str);
```
But `LabEquipment::bath_read_temp` was declared and defined as `void`. It read the temperature into a local variable, printed it, and discarded it — never returning a value and never setting `Res` for Mathematica.

**Fix** (`src/API.cpp` and `include/API.h`): Changed return type to `double`, returned the measured temperature:
```cpp
// API.h
double bath_read_temp(std::string COMM) const;

// API.cpp implementation
double LabEquipment::bath_read_temp(std::string COMM) const {
    float temp = 0.0f;
    RTE7 bath = RTE7(COMM);
    if (!bath.get_temp(temp)) {
        cerr << "Read Failed." << endl;
    }
    return static_cast<double>(temp);
}

// DLL wrapper — result now wired back to Mathematica
double temp = equipment.bath_read_temp(comm_str);
MArgument_setReal(Res, temp);
```

---

### 6. LabJack library was x86, build target is x64

**Error**:
```
warning LNK4272: library machine type 'x86' conflicts with target machine type 'x64'
error LNK2019: unresolved external symbol Close
error LNK2019: unresolved external symbol OpenLabJack
error LNK2019: unresolved external symbol eGet
error LNK2019: unresolved external symbol ePut
fatal error LNK1120: 4 unresolved externals
```

**Cause**: `FindLabJack.cmake` searched `C:/Program Files (x86)/LabJack/Drivers/` first and found the 32-bit `LabJackUD.lib`. The build targets x64, so the linker rejected it and could not resolve any LabJack symbols.

A 64-bit version exists at `C:/Program Files (x86)/LabJack/Drivers/64bit/LabJackUD.lib`.

**Fix** (`cmake/FindLabJack.cmake`): Added the `64bit/` subdirectory as the first search hint:
```cmake
find_library(LABJACK_LIBRARY
    NAMES LabJackUD
    HINTS
        "C:/Program Files (x86)/LabJack/Drivers/64bit"
        "C:/Program Files/LabJack/Drivers/64bit"
        "C:/Program Files (x86)/LabJack/Drivers"
        "C:/Program Files/LabJack/Drivers"
)
```
CMake cache was cleared with `-DLABJACK_LIBRARY=...` to force the new path to take effect.

---

## Files Changed

| File | Change |
|------|--------|
| `Josh_Controller/CMakeLists.txt` | Fixed include path from `../Mathematica/include/` to `../include/` |
| `Josh_Controller/cmake/FindLabJack.cmake` | Added `64bit/` search path ahead of x86 path |
| `include/sm_manual_controller.h` | Qualified all Teknic (`sFnd::`) and STL (`std::`) types |
| `include/API.h` | Changed `bath_read_temp` return type from `void` to `double` |
| `src/API.cpp` | Fixed `bath_read_temp` implementation; replaced `.toString()` calls with `std::visit` lambda |
| `Josh_Controller/build.ps1` | Created manual build script with `/std:c++17` flag as fallback |
