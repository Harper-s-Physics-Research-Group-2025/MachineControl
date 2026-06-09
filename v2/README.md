# Josh 2.0 Lab Equipment Controller - Mathematica DLL

A Wolfram Language (Mathematica) dynamic library for controlling automated lipid-laser experimentation equipment.

## Project Overview

This library provides Mathematica with control over the Josh 2.0 automated system, which includes:
- **Temperature Control**: Neslab RTE7 water bath and Oven Industries 5R6-900 thermoelectric controller
- **Servo Motors**: Teknic ClearPath SC motors for sample positioning
- **Data Acquisition**: LabJack UV-L3 analog-to-digital converter
- **Measurement**: Newport 1815-C optical power meter and Thor Labs laser

## Project Structure

```
Josh_Controller/
├── CMakeLists.txt              # Build configuration
├── cmake/                       # Custom CMake modules
├── README.md                    # This file
└── .gitignore                   # Git ignore rules

../Mathematica/
├── include/
│   └── Josh_Controller/         # Header files for all equipment classes
│       ├── API.h                # Main API class declaration
│       ├── RTE7.h               # Water bath controller
│       ├── Oven5R6900.h         # Temperature controller
│       ├── recorder.h           # Data recording interface
│       ├── sm_homer.h           # Servo motor homing
│       └── sm_manual_controller.h  # Manual motor control

../src/
├── API.cpp                      # API class implementation
├── RTE7.cpp                     # Water bath implementation
└── Oven5R6900.cpp              # Temperature controller implementation

../notebooks/
└── test_interface.nb            # Mathematica notebook for testing the library

../algorithms/
└── tests/                       # Unit tests
```

## Dependencies

### System Requirements
- Windows (7 or later)
- Mathematica 12.0 or later
- Visual Studio 2017 or later (for compilation)
- CMake 3.15 or later

### External Libraries
- **Wolfram Language Development Kit**: Included with Mathematica
  - Path: `{Mathematica Installation}/SystemFiles/IncludeFiles/C`
  
- **LabJack UD Driver**: For LabJack U3-LV communication
  - Download from: https://labjack.com/support/software/installers/ud
  - Default install path: `C:/Program Files (x86)/LabJack/Drivers`

- **Teknic ClearPath Libraries**: `sFoundation20.dll` (included in Mathematica/include)

## Building

### Prerequisites
1. Ensure Mathematica is installed
2. Install LabJack UD driver
3. Install CMake
4. Install Visual Studio with C++ build tools

### Build Instructions

```bash
# Navigate to Josh_Controller directory
cd Josh_Controller

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "Visual Studio 16 2019" -A x64 ..

# Build the DLL
cmake --build . --config Release

# DLL will be generated in:
# build/bin/LabController.dll
```

### Configuration Notes
- Update `WOLFRAM_INSTALL_DIR` in CMakeLists.txt if Mathematica is not in the default location
- Adjust `LabJack/Drivers` path if installed elsewhere
- Ensure all external DLL dependencies are in the same directory or system PATH

## API Functions

### Bath Control Functions
- `bath_on(port)` - Turn on water bath
- `bath_off(port)` - Turn off water bath
- `bath_dump(port)` - Query current state and setpoint
- `bath_read_temp(port)` - Read current temperature
- `bath_set_temp(port, temp)` - Set target temperature

### Servo Motor Functions
- `servo_motor_home()` - Home the motors (sets reference position)
- `servo_motor_home(milliseconds)` - Home with timeout
- `servo_motor_read_position()` - Get current X,Z position in mm
- `servo_motor_set_position(x, z, velocity)` - Move to position
- `servo_motor_manual_control()` - Interactive manual control

### Temperature Controller Functions
- `temperature_control_on(port)` - Enable controller
- `temperature_control_off(port)` - Disable controller
- `temperature_control_read_temp(port)` - Read current temperature
- `temperature_control_set_temp(port, temp)` - Set setpoint
- `temperature_control_dump(port)` - Query all parameters
- `temperature_control_ramp_soak(...)` - Run ramp/soak sequence

### Data Acquisition
- `read_labjack_ain0()` - Read analog input from LabJack
- `record(filename, bath_port, temp_port)` - Record data to CSV

## Using in Mathematica

After building the DLL and placing it in the Mathematica path:

```mathematica
(* Load the library *)
link = NETLink`LoadNETAssembly["LabController.dll"];

(* Call functions *)
LabJack`LabEquipment`bath_on["COM3"];
temp = LabJack`LabEquipment`bath_read_temp["COM3"];

(* Or using LibraryLink *)
lib = LibraryLink`LibraryLoad["<path>/LabController.dll"];
result = LibraryLink`LibraryFunction[lib, "bath_on", 
    {LibraryLink`"UTF8String"}, LibraryLink`"Void"][port];
```

## Testing

A Mathematica notebook is provided to test library functionality:

```bash
# From Mathematica, open:
../notebooks/test_interface.nb
```

## Development Notes

- All functions return error codes (0 = success)
- COM ports are specified as strings (e.g., "COM3", "/dev/ttyUSB0")
- Positions are in millimeters, temperatures in Celsius
- The library is thread-safe for independent device operations

## Troubleshooting

| Issue | Solution |
|-------|----------|
| DLL not found in Mathematica | Ensure DLL is in system PATH or Mathematica's LibraryPath |
| Device communication fails | Check COM port, ensure device drivers installed, verify cable connection |
| Missing WolframLibrary.h | Ensure WOLFRAM_INSTALL_DIR is correctly set in CMakeLists.txt |
| LabJackUD.lib not found | Install LabJack UD driver or update path in CMakeLists.txt |

## Authors

- Josh Darrow
- Samuel Ntadom

## License

See LICENSE file in project root.
