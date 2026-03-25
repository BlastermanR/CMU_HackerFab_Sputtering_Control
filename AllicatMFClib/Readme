# Alicat MFC Library (AllicatMFClib)

A lightweight C++ headers-only utility for forming and parsing commands to communicate with Alicat Mass Flow Controllers and sensors over the serial protocol.

## Features

- **String Compilation:** Easily formats the query or setpoint strings to target the exact ID with the required syntax (`A` + `S` + `10.0` + `\r`).
- **Response Extractor:** Includes a strict whitespace tokenizing parser designed for Alicat's standard 7-token response frame.
- **Float Conversion:** Automatically casts the tokens (Pressure, Mass Flow, Volumetric Flow, Temperature, Setpoint) into doubles safely using `std::strtod()`, avoiding C++ exceptions.

## Usage

### Connecting in CMakeLists.txt

```cmake
include(AllicatMFClib/AllicatMFClib.cmake)
target_link_libraries(your_project PRIVATE AllicatMFClib)
```

### Basic Output Example

```cpp
#include "AlicatLib.h"
#include <string>
#include <cstdio>

// 1. Ask MFC ID 'A' for data
AlicatCommand cmd = {'A', "", ""}; // Blank action usually defaults to Polling
std::string rawStr = AlicatLib::formatCommand(&cmd);
// Send rawStr to device...

// 2. Setting Setpoint to 5.0
AlicatCommand setCmd = {'A', "S", "5.0"};
std::string rawSetStr = AlicatLib::formatCommand(&setCmd);
// Send rawSetStr to Device...

// 3. Extracting incoming data
std::string reply = "A +014.24 +024.11 +004.99 +004.99 +005.00 Air";
AlicatDataFrame frame;
bool isGood = false;

AlicatLib::parseResponse(reply, &frame, &isGood);

if (isGood) {
   printf("Controller ID: %c\n", frame.id);
   printf("Current Mass Flow: %f\n", frame.massFlow);
   printf("Identified Gas: %s\n", frame.gasType.c_str());
}
```

## Protocol Context 

*Alicat devices usually stream whitespace delimited string frames terminating in a carriage return in their default serial mode.* This parser relies on that strict form and avoids interface patterns.

## Author
**Ryan Massie (rmassie)**  
**Date:** March 2026