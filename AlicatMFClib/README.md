# Alicat MFC Library (AlicatMFClib)

A lightweight, headers-only C++ utility for forming and parsing Alicat serial protocol commands. Supports Alicat Mass Flow Controllers (MFCs) and sensors over RS-232/RS-485.

## Features

- **Command Formatting** — Builds correctly delimited query and setpoint strings for a given device ID (e.g. `AS10.0\r`).
- **Response Parsing** — Strict whitespace tokenizer for Alicat's standard 7-token response frame; populates an `AlicatDataFrame` struct.
- **Safe Float Conversion** — Uses `std::strtod()` to cast flow/pressure/temperature tokens to `double`, avoiding C++ exceptions.
- **Gas Table** — Definitions for all 130 Alicat-numbered gases (`AlicatGases.h`): numeric ID, short name, and long name. Named `ALICAT_GAS_*` constants are provided for every gas.
- **Status / Error Codes** — Definitions for all 11 Alicat status codes (`AlicatErrors.h`): code string and human-readable description. Any status tokens present at the end of a response frame are automatically collected into `AlicatDataFrame::statusCodes`.

## Files

| File | Purpose |
|------|---------|
| `include/AlicatLib.h` | Core protocol library — `AlicatCommand`, `AlicatDataFrame`, `AlicatLib` static methods |
| `include/AlicatGases.h` | Gas ID table, `lookupAlicatGasById()`, and `ALICAT_GAS_*` constants |
| `include/AlicatErrors.h` | Status code table, `lookupAlicatStatusByCode()`, and `ALICAT_STATUS_*` constants |

## CMake Integration

```cmake
include(AlicatMFClib/AlicatMFClib.cmake)
target_link_libraries(your_project PRIVATE AlicatMFClib)
```

## Usage

### Polling a device and parsing the response

```cpp
#include "AlicatLib.h"
#include <string>
#include <cstdio>

// 1. Format a poll command for device 'A'
AlicatCommand cmd;
cmd.id     = 'A';
cmd.action = ALICAT_POLL;
bool valid = false;
std::string raw = AlicatLib::formatCommand(&cmd, &valid);
// Send raw to device over serial...

// 2. Parse the response
std::string reply = "A +014.24 +024.11 +004.99 +004.99 +005.00 Ar";
AlicatDataFrame frame;
bool ok = false;
AlicatLib::parseResponse(reply, &frame, &ok);

if (ok) {
    printf("ID:          %c\n",   frame.id);
    printf("Pressure:    %.2f\n", frame.pressure);
    printf("Temperature: %.2f\n", frame.temperature);
    printf("Mass Flow:   %.2f\n", frame.massFlow);
    printf("Setpoint:    %.2f\n", frame.setpoint);
    printf("Gas:         %s\n",   frame.gasType.c_str());
}
```

### Setting the gas by named constant

```cpp
#include "AlicatLib.h"
#include "AlicatGases.h"

AlicatCommand cmd;
cmd.id     = 'A';
cmd.action = ALICAT_SET_GAS;
cmd.data   = std::to_string(ALICAT_GAS_AR);   // Argon = gas ID 2
bool valid = false;
std::string raw = AlicatLib::formatCommand(&cmd, &valid);
```

All 130 gas constants follow the pattern `ALICAT_GAS_<SHORTNAME>`, e.g.:

| Constant | Gas |
|----------|-----|
| `ALICAT_GAS_AR` | Argon |
| `ALICAT_GAS_N2` | Nitrogen |
| `ALICAT_GAS_O2` | Oxygen |
| `ALICAT_GAS_HE` | Helium |
| `ALICAT_GAS_AIR` | Air |

Use `AlicatLib::getGasShortName(id)` and `AlicatLib::getGasLongName(id)` to look up names at runtime.

### Checking for status / error codes

Status codes appearing after the gas token in a response frame are automatically collected:

```cpp
AlicatDataFrame frame;
bool ok = false;
AlicatLib::parseResponse("A +014.24 +024.11 +004.99 +004.99 +005.00 Ar MOV POV", &frame, &ok);

for (const std::string &code : frame.statusCodes) {
    const char *desc = AlicatLib::getStatusDescription(code.c_str());
    printf("Status: %s — %s\n", code.c_str(), desc ? desc : "unknown");
}
```

Known status codes:

| Code | Meaning |
|------|---------|
| `ADC` | ADC error |
| `EXH` | Exhaust pressure warning |
| `HLD` | Setpoint hold active |
| `LCK` | Front panel locked |
| `MOV` | Over pressure / flow moving |
| `OPL` | Open loop control |
| `OVR` | Overdrive |
| `POV` | Pressure over range |
| `TMF` | Too many flow ranges |
| `TOV` | Temperature over range |
| `VOV` | Volumetric flow over range |

## Protocol Notes

Alicat devices stream whitespace-delimited frames terminated by a carriage return (`\r`) in their default serial mode. The standard response frame has exactly 7 fixed tokens followed by zero or more status code tokens:

```
<ID> <pressure> <temperature> <vol_flow> <mass_flow> <setpoint> <gas> [status...]
```

The parser validates token count (minimum 7) before populating `AlicatDataFrame` and rejects malformed frames.

## Author
**Ryan Massie (rmassie)**  
**Date:** March 2026