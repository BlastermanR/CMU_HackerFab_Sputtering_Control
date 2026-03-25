# Pfeiffer Vacuum Library (Pfiefferlib)

A C++17 communication library for interfacing with Pfeiffer Vacuum devices (Turbo Pumps, Gauges, etc.) over RS-485 / RS-232 using the Pfeiffer Vacuum Protocol.

## Project Structure

```text
Pfiefferlib/
├── PfiefferLibrary.cmake   # CMake integration
├── include/
│   ├── PfiefferLib.h       # Bare protocol (Checksums, Parsing, Structs)
│   ├── PfiefferDevice.h    # CRTP Base class for typed device handling
│   └── Devices/            # Specific device implementations
│       ├── TC110DriveUnit.h # TC 110 Electronic Drive Unit (Turbo Pump)
│       └── MPT200.h         # MPT 200 AR Cold Cathode Gauge
```

## Features

- **Zero-Overhead Abstracting:** Uses the **Curiously Recurring Template Pattern (CRTP)** to provide a generic interface without `vtable` or dynamic allocation overhead.
- **Protocol Safety:** Automatic checksum calculation (Modulo 256) and character validation for incoming responses.
- **Parameter Guarding:** Each device defines a static dictionary of parameters including:
  - **Access Control:** Prevents writing to Read-Only parameters or reading Write-Only ones.
  - **Bounds Checking:** Automatically validates `double` inputs against `min`/`max` limits defined in the manufacturer manual.
  - **Data Typing:** Tracks Pfeiffer data types (e.g., Integer, Real, String) for correct serialization.

## Usage

### Integrating with CMake

Add the library to your `CMakeLists.txt`:

```cmake
include(Pfiefferlib/PfiefferLibrary.cmake)
target_link_libraries(your_project PRIVATE Pfiefferlib)
```

### Basic Communication Example

```cpp
#include "Devices/TC110DriveUnit.h"

using namespace Pfieffer;

// 1. Initialize a device with its RS-485 address (default is 1)
TC110DriveUnit turboPump(1);

// 2. Prepare a command structure
PfiefferCommand cmd;

// 3. Create a validated Write Command (e.g., Turning on the pump)
// This checks if 'MotorPump' supports write and if '1.0' is within allowed range.
if (turboPump.createWriteCommand((uint16_t)TC110Cmd::MotorPump, 1.0, cmd)) {
    // Get the raw string to send via your UART/Serial library
    std::string raw = PfieifferLib::formatCommand(&cmd);
    // Send(raw);
}

// 4. Create a Read Command
if (turboPump.createReadCommand((uint16_t)TC110Cmd::ActualSpd_Hz, cmd)) {
    std::string raw = PfieifferLib::formatCommand(&cmd);
    // Send(raw);
}
```

### Parsing Responses

```cpp
std::string response = "0011030906000500132\r"; // Example RAW response from pump
PfiefferCommand reply;
bool isValid = false;

PfieifferLib::decryptResponse(response, &reply, &isValid);

if (isValid) {
    printf("Parameter %s value is: %s\n", reply.paramNum.c_str(), reply.data.c_str());
}
```

## Adding New Devices

To add a new Pfeiffer device, create a new header in `include/Devices/` and inherit from `PfiefferDevice<YourClass>`. Define a static `getParamDef(uint16_t paramNum)` method and a `DICT[]` array containing the parameter definitions from the device manual.

## Author
**Ryan Massie (rmassie)**  
**Date:** March 2026
