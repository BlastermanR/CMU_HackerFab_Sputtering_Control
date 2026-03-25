# Raspberry Pi Pico UART Library (UARTLibrary)

A unified C++ interface for the Raspberry Pi Pico (RP2040/RP2350) that supports both **Hardware UART** and **PIO-based UART** (Programmable I/O).

## Features

- **Unified Interface:** Both `HardUart` and `PioUart` inherit from the `IUart` base class, allowing for hot-swapping or generic UART handling code.
- **Interrupt Driven (Async RX):** Supports asynchronous data reception via `std::function` callbacks. Interrupt handlers are mapped internally to ensure minimal latency.
- **Flexible Pin Mapping:** 
  - `HardUart` uses the Pico's dedicated UART peripherals (`uart0`, `uart1`).
  - `PioUart` leverages the PIO state machines, allowing UART functionality on any GPIO pins.
- **Support for RP2350:** Includes support for the 3 PIO blocks available on the newer RP2350/Pico 2 chips.

## Project Structure

```text
UARTLibrary/
├── UARTLibrary.cmake   # CMake integration
├── include/
│   ├── IUart.h         # Virtual interface (was UARTInterface.h)
│   ├── HardwareUART.h  # Native hardware UART implementation
│   └── PIO_UART.h      # PIO-based UART implementation
└── pio/                # PIO assembly files
    ├── uart_rx.pio     # PIO RX program
    └── uart_tx.pio     # PIO TX program
```

## Usage

### Integrating with CMake

```cmake
include(UARTLibrary/UARTLibrary.cmake)
target_link_libraries(your_project PRIVATE UARTLibrary)
```

### Hardware UART Example

```cpp
#include "HardwareUART.h"

// Define a callback for receiving data
void onData(char c) {
    printf("Received: %c\n", c);
}

// Initialize on UART0 (Pins GP0/GP1) at 115200 baud
HardUart rs485(uart0, 0, 1, 115200);

void setup() {
    rs485.setRxCallback(onData);
    rs485.begin();
    rs485.print("Hardware UART Ready\r\n");
}
```

### PIO UART Example

```cpp
#include "PIO_UART.h"

// Initialize on PIO0, using any GPIOs (e.g., GP4/GP5)
PioUart trace(pio0, 4, 5, 115200);

void setup() {
    trace.begin();
    trace.write('A');
    trace.print("PIO UART Initialized\r\n");
}
```

## Abstracting UART Usage

Since both implementations derive from `IUart`, you can write code that doesn't care which peripheral is being used:

```cpp
void sendHeartbeat(IUart* uart) {
    uart->print("Ping\r\n");
}

// Works with either type
sendHeartbeat(&rs485);
sendHeartbeat(&trace);
```

## Author
**Ryan Massie (rmassie)**  
**Date:** March 2026
