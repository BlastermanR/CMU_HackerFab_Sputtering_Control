# Pfeiffer Vacuum Library (Pfeifferlib)

A lightweight, headers-only C++17 library for forming and parsing Pfeiffer Vacuum serial protocol commands. Supports Pfeiffer Turbo Pumps, Gauges, and other RS-485/RS-232 devices using the standard Pfeiffer Vacuum serial protocol.

## Features

- **Command Formatting** — Builds correctly structured query and setpoint strings, including automatic data-length and Modulo-256 checksum fields.
- **Response Parsing** — Validates and decrypts incoming Pfeiffer response frames, populating a `PfeifferCommand` struct.
- **Zero-Overhead Abstraction** — Uses the **Curiously Recurring Template Pattern (CRTP)** to provide a generic device interface without `vtable` or dynamic allocation overhead.
- **Parameter Guarding** — Each device defines a static dictionary of parameters including:
  - **Access Control:** Prevents writing to Read-Only parameters or reading Write-Only ones.
  - **Bounds Checking:** Automatically validates inputs against `min`/`max` limits defined in the manufacturer manual.
  - **Data Typing:** Tracks Pfeiffer data types (Boolean, Integer, Real, String, Expo) for correct serialization.

## Files

| File | Purpose |
|------|---------|
| `include/PfeifferLib.h` | Core protocol library — `PfeifferCommand`, `PfeifferLib` static methods (format, parse, checksum) |
| `include/PfeifferDevice.h` | CRTP base class `PfeifferDevice<Derived>` — access control, bounds checking, command creation |
| `include/Devices/TC110DriveUnit.h` | TC 110 Electronic Drive Unit (Turbo Pump) — full parameter dictionary and `TC110Cmd` enum |
| `include/Devices/MPT200.h` | MPT 200 AR Cold Cathode Gauge — parameter dictionary and `MPT200Cmd` enum |

## CMake Integration

```cmake
include(Pfeifferlib/PfeifferLibrary.cmake)
target_link_libraries(your_project PRIVATE Pfeifferlib)
```

## Usage

### Sending a write command (TC110 Turbo Pump)

```cpp
#include "Devices/TC110DriveUnit.h"

using namespace Pfeiffer;

// 1. Initialize a device with its RS-485 address (default is 1)
TC110DriveUnit turboPump(1);

// 2. Create a validated write command (e.g., turn on the pump motor)
// Checks that MotorPump supports write and that 1.0 is within the allowed range.
PfeifferCommand cmd;
if (turboPump.createWriteCommand((uint16_t)TC110Cmd::MotorPump, 1.0, cmd)) {
    bool valid = false;
    std::string raw = PfeifferLib::formatCommand(&cmd, &valid);
    // Send raw over your RS-485/UART interface...
}
```

### Sending a read command

```cpp
// Read the actual spindle speed in Hz
PfeifferCommand cmd;
if (turboPump.createReadCommand((uint16_t)TC110Cmd::ActualSpd_Hz, cmd)) {
    bool valid = false;
    std::string raw = PfeifferLib::formatCommand(&cmd, &valid);
    // Send raw over your RS-485/UART interface...
}
```

### Parsing a response

```cpp
// Example raw DATA_RESPONSE frame from the pump (ActualSpd_Hz = 500 Hz)
std::string response = "0011030906000500132\r";
PfeifferCommand reply;
bool isValid = false;

PfeifferLib::decryptResponse(response, &reply, &isValid);

if (isValid) {
    printf("Param %s = %s\n", reply.paramNum.c_str(), reply.data.c_str());
}
```

### Reading pressure from the MPT 200 gauge

```cpp
#include "Devices/MPT200.h"

using namespace Pfeiffer;

MPT200 gauge(2); // RS-485 address 2
PfeifferCommand cmd;

if (gauge.createReadCommand((uint16_t)MPT200Cmd::Pressure, cmd)) {
    bool valid = false;
    std::string raw = PfeifferLib::formatCommand(&cmd, &valid);
    // Send raw and await the response...
}
```

## Adding New Devices

Create a new header in `include/Devices/` and inherit from `PfeifferDevice<YourClass>`. Implement a static `getParamDef(uint16_t paramNum)` method and a `DICT[]` constexpr array populated with `PfeifferParamDef` entries from the device manual.

```cpp
class MyDevice : public PfeifferDevice<MyDevice> {
public:
    explicit MyDevice(uint8_t address = 1) : PfeifferDevice<MyDevice>(address) {}

    static constexpr PfeifferParamDef DICT[] = {
        {303, "Error", 4, AccessType::READ_ONLY, 0, 0, 0, false},
        // ...
    };

    static const PfeifferParamDef *getParamDef(uint16_t parameterNumber) {
        for (const auto &def : DICT)
            if (def.number == parameterNumber) return &def;
        return nullptr;
    }
};
```

## Protocol Notes

Pfeiffer devices communicate over RS-485 using a fixed-field binary-ASCII frame terminated by a carriage return (`\r`). Every frame — command or response — follows the same layout:

```
<address(3)><action(2)><paramNum(3)><dataLen(2)><data><checksum(3)>\r
```

| Field | Length | Description |
|-------|--------|-------------|
| `address` | 3 chars | RS-485 device address, zero-padded (e.g. `001`) |
| `action` | 2 chars | `00` = read request, `10` = data response, `20` = error response |
| `paramNum` | 3 chars | Parameter number, zero-padded (e.g. `309`) |
| `dataLen` | 2 chars | Length of the data field in characters, zero-padded |
| `data` | variable | Parameter value or query string (`=?` for read requests) |
| `checksum` | 3 chars | Modulo-256 sum of all preceding ASCII characters, zero-padded |

Known action codes:

| Code | Meaning |
|------|---------|
| `00` | Read/query request (`READ_PARAMETER`) |
| `10` | Successful data response (`DATA_RESPONSE`) |
| `20` | Error response (`ERROR_RESPONSE`) |

## Author
**Ryan Massie (rmassie)**  
**Date:** March 2026
