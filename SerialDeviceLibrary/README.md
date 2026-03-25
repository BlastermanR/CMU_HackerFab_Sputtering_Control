# Serial Device Library (SerialDeviceLibrary)

A high-level C++ abstraction layer for serial communication on the Raspberry Pi Pico. This library converts raw UART byte streams into structured, message-based communication, with specific support for **RS-232** and **RS-485** (half-duplex) protocols.

## Features

- **Message Queuing:** Automatically buffers incoming characters into complete messages using a thread-safe `pico/util/queue`.
- **Protocol Abstraction:** Provides a unified `ISerialDevice` interface for both RS-232 and RS-485.
- **RS-485 Flow Control:** Handles the Request-to-Send (RTS) pin automatically during transmission, ensuring proper switching between transmit and receive modes in half-duplex setups.
- **Buffer Management:** Includes built-in protection against buffer overflows and handles message delimiting (Carriage Return `\r`).

## Project Structure

```text
SerialDeviceLibrary/
├── SerialDeviceLibrary.cmake  # CMake integration
├── include/
│   ├── ISerialDevice.h       # High-level interface for all serial devices
│   ├── SerialDeviceBase.h    # Base class handling buffering and queuing
│   ├── RS232Device.h         # Standard full-duplex implementation
│   └── RS485Device.h         # Half-duplex implementation with RTS control
└── src/
    └── SerialDeviceBase.cpp  # Implementation of buffering logic
```

## Usage

### Integrating with CMake

```cmake
include(SerialDeviceLibrary/SerialDeviceLibrary.cmake)
target_link_libraries(your_project PRIVATE SerialDeviceLibrary)
```

### RS-485 Example (e.g., Pfeiffer Vacuum Gauge)

RS-485 requires an RTS pin to control the transceiver direction.

```cpp
#include "HardwareUART.h"
#include "RS485Device.h"

// 1. Setup the underlying UART
HardUart uart(uart0, 0, 1, 9600);

// 2. Wrap it in an RS485 device (using GPIO 2 as RTS)
RS485Device gauge(&uart, 2);

void setup() {
    gauge.begin();
}

void loop() {
    // Send a message (RTS is handled automatically)
    gauge.send("0010070102=?\r");

    // Check for a reply
    if (gauge.hasMessage()) {
        std::string reply = gauge.popMessage();
        printf("Received: %s\n", reply.c_str());
    }
}
```

### RS-232 Example (e.g., Alicat MFC)

RS-232 is simpler as it doesn't require direction switching.

```cpp
#include "PIO_UART.h"
#include "RS232Device.h"

PioUart pioUart(pio0, 4, 5, 19200);
RS232Device mfc(&pioUart);

void setup() {
    mfc.begin();
    mfc.send("A\r"); // Basic Alicat identification
}
```

## How It Works

1. **Reception:** The `SerialDeviceBase` registers a callback with the underlying `IUart`. As bytes arrive, they are added to an internal `receiveBuffer`.
2. **Delimiting:** When a Carriage Return (`\r`) is detected, the buffer is copied into a `SerialMessage` struct and pushed onto the `msgQueue`.
3. **Consumption:** The application calls `hasMessage()` and `popMessage()` to retrieve fully formed strings from the queue, decoupling the timing of serial reception from the main application logic.

## Author
**Ryan Massie (rmassie)**  
**Date:** March 2026
