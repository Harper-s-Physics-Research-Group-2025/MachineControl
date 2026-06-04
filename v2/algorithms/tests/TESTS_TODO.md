# Test Suite Development Plan
**Project**: Josh 2.0 Lab Equipment Controller  
**Date Created**: May 28, 2026

## Test Organization

Tests should be organized into the following categories based on dependencies and purpose.

---

## 1. Unit Tests (No Hardware Required)

### API Class Tests
- [ ] **Verify function signatures** match Wolfram library expectations
- [ ] **Parameter validation** - COM ports format
- [ ] **Parameter validation** - temperature ranges (min/max)
- [ ] **Parameter validation** - velocity limits
- [ ] **Error code returns** - correct error codes for each failure case
- [ ] **Function return types** - void vs integer returns

### Mock Device Tests
- [ ] **Mock RTE7 bath controller** - simulate communication
- [ ] **Mock Oven5R6900 temperature controller** - simulate communication
- [ ] **Test device response parsing** - verify correct interpretation of device messages
- [ ] **Test command formatting** - verify correct command structure sent to devices

---

## 2. Communication Tests (Requires Hardware)

### Bath Control (RTE7)
- [ ] **Turn on** - verify bath powers on
- [ ] **Turn off** - verify bath powers off
- [ ] **Read current temperature** - verify reading accuracy
- [ ] **Set temperature** - verify setpoint updates
- [ ] **Set temperature validation** - reject temperatures outside valid range
- [ ] **Invalid COM port** - handle gracefully
- [ ] **Device timeout** - handle non-responsive device

### Servo Motors (Teknic ClearPath)
- [ ] **Homing sequence** - motors reach home position
- [ ] **Homing with timeout** - completes within specified time
- [ ] **Read position accuracy** - position readings are consistent
- [ ] **Set position** - motors move to target position
- [ ] **Velocity limits** - reject velocities exceeding 1000 RPM
- [ ] **Boundary conditions** - detect when motors reach limits
- [ ] **Error recovery** - disable motors on error

### Temperature Controller (Oven 5R6-900)
- [ ] **Read mode** - correctly identify control mode
- [ ] **Set mode** - switch between control modes
- [ ] **Read setpoint** - verify current setpoint
- [ ] **Write setpoint** - update temperature setpoint
- [ ] **Ramp/Soak sequence initiation** - start sequence successfully
- [ ] **Ramp/Soak parameter verification** - parameters stored correctly
- [ ] **Query all parameters** - dump returns all device state

---

## 3. Data Acquisition Tests

### LabJack Analog Input
- [ ] **Read AIN0** - successfully read analog voltage
- [ ] **Voltage range** - values within expected 0-2.4V range
- [ ] **Reading consistency** - multiple reads return stable values
- [ ] **Error handling** - handle LabJack not found

### Data Recording
- [ ] **Create CSV file** - file generated successfully
- [ ] **CSV headers** - correct column headers written
- [ ] **CSV data format** - timestamp, temperature, position recorded
- [ ] **Multiple recordings** - append or overwrite behavior correct
- [ ] **File permissions** - handle write errors gracefully

---

## 4. Failure & Edge Cases

### Invalid Parameters
- [ ] **Empty COM port string** - reject empty string
- [ ] **Invalid port format** - reject non-standard port names
- [ ] **Negative temperature** - reject invalid temperature
- [ ] **Temperature exceeds max** - reject out-of-range values
- [ ] **Negative velocity** - reject negative RPM
- [ ] **Velocity exceeds limit** - reject > 1000 RPM
- [ ] **Negative position** - reject negative coordinates

### Device Errors
- [ ] **Device not found** - return appropriate error code
- [ ] **Device not responding** - timeout after reasonable delay
- [ ] **Corrupted response** - handle malformed device messages
- [ ] **Serial communication error** - handle transmission failures
- [ ] **Partial response** - handle incomplete device responses

### Recovery
- [ ] **Re-establish connection** - reconnect after timeout
- [ ] **Motor disable on error** - motors disabled for safety
- [ ] **State cleanup** - library state consistent after error
- [ ] **Resource cleanup** - no memory leaks or dangling connections

---

## 5. Mathematica Integration Tests

### Library Loading
- [ ] **DLL loads successfully** in Mathematica
- [ ] **GetVersion** returns correct version
- [ ] **Initialize** function called on load
- [ ] **Uninitialize** function called on unload

### Function Wrappers
- [ ] **bath_on** callable from Mathematica
- [ ] **servo_motor_home** callable from Mathematica
- [ ] **temperature_control_dump** callable from Mathematica
- [ ] **read_labjack_ain0** callable from Mathematica
- [ ] **record** callable from Mathematica

### Return Values
- [ ] **Integer returns** - correctly marshaled to Mathematica
- [ ] **Void functions** - return cleanly to Mathematica
- [ ] **Error codes** - propagate to Mathematica
- [ ] **String parameters** - passed correctly to C++

---

## 6. Performance & Stress Tests

- [ ] **Rapid consecutive calls** - library handles fast operation
- [ ] **Long-running sequences** - no memory leaks over time
- [ ] **Multiple device connections** - can control bath + motors + temperature simultaneously
- [ ] **Shutdown sequence** - all devices properly disabled on exit

---

## Implementation Priority

**Phase 1** (Critical - Start Here)
- Unit tests: API validation, mock devices
- Communication: Basic bath/motor/temperature control

**Phase 2** (Important)
- Edge cases and error handling
- Data acquisition tests

**Phase 3** (Nice to Have)
- Mathematica integration tests
- Performance/stress tests

---

## Testing Framework

Currently using **Google Test (GTest)**. Alternative option: Add simple assertion macros for no-dependency testing.

### Running Tests
```bash
cd V2/algorithms/tests/build
cmake ..
cmake --build . --config Release
ctest --verbose
```

---

## Notes

- Mark hardware-dependent tests with `[HW_REQUIRED]` comment
- Use mocks for offline testing
- Create test devices/COM port stubs for CI/CD environments
- Document any test-specific dependencies or setup required
