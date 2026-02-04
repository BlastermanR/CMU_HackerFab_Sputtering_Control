# Arduino Vacuum System Controller

Arduino-based control system for automated vacuum pumping and gas flow management in a sputtering deposition setup.

## Overview

This project uses an Arduino Uno to interface with and control:

- **Pfeiffer HiCube 300 Eco** turbomolecular pumping station
- **Pfeiffer MPT 200** pressure gauge
- **Alicat mass flow controllers (MFCs)**

The system enables programmatic control of vacuum pressure, pump speed, and process gas flow rates via serial communication protocols.

## Hardware Components

### Vacuum System

- **Pump**: [Pfeiffer HiCube 300 Eco Benchtop Turbomolecular Pump Station](https://www.idealvac.com/en-us/Pfeiffer-HiCube-300-Eco-Benchtop-Turbomolecular-Pump-Station-ISO-100K-Inlet-110-240-VAC-PN:-PM-025-019-AT/pp/P1011608)
  - ISO 100K inlet flange
  - Integrated HiPace 300 turbo pump
  - MVP 030-3 DC diaphragm backing pump
  - TC 110 electronic drive unit (RS485 control)

- **Pressure Gauge**: [Pfeiffer MPT 200 Pirani/Cold Cathode Combination Gauge](https://www.idealvac.com/en-us/Pfeiffer-PiraniCold-Cathode-Combination-MPT-200-Gauge-RS-485-analog-0-10V-KF25-PN:-PT-R40-141/pp/P107321?srsltid=AfmBOooRa2jCMva1dUtlUuyUUF6zqvpxA4Uad86e-j1NAPchPNNiyPaS)
  - RS485 digital interface
  - KF25 flange
  - Range: 5×10⁻⁹ to 1500 mbar

### Gas Flow Control

- **Mass Flow Controllers**: [Alicat Laminar DP Mass Flow Controllers](https://www.alicat.com/products/gas-flow/mass-flow-controller/laminar-dp-mass-flow-controllers/)
  - Serial (RS232/TTL) communication
  - Programmable gas flow setpoints

### Controller

- **Arduino Uno** (ATmega328P)
- RS485 transceiver modules with DE/RE control pins
- SoftwareSerial for multiple serial ports

## Communication Architecture

### RS485 (Pfeiffer Devices)

The Pfeiffer HiCube pump/controller and MPT 200 gauge communicate via RS485 using Pfeiffer's ASCII-based telegram protocol:

- **Baud rate**: 9600
- **Protocol**: Pfeiffer Vacuum Protocol (ASCII telegrams with checksums)
- **Addressing**: 
  - Pump: Address 001
  - Gauge: Address 002

This is handled by [`ArduinoPfeiffer.h/cpp`](SputteringControlLoop/ArduinoPfeiffer.h)

**Key Parameters**:
- `P:010` - Pumping station ON/OFF
- `P:026` - Speed setting mode enable
- `P:707` - Speed setpoint (% of nominal)
- `P:340` - Pressure reading
- See [`PfeifferPumpCommands.h`](SputteringControlLoop/PfeifferPumpCommands.h) for complete parameter definitions

### Serial (Alicat MFCs)

Alicat devices use the [RS-232](https://www.alicat.com/support/rs-232-communication-with-an-alicat/) ASCII serial protocol:

- **Baud rate**: 9600
- **Commands**: Simple ASCII strings for setpoint control and readback

## Project Status

**Current Phase**: Module Testing & Integration

The main project code is located in the [**`SputteringControlLoop`**](SputteringControlLoop) directory. We are currently:

1. ✅ Testing individual module communication (pump, gauge, MFCs)
2. ✅ Validating RS485 protocol implementation
3. ✅ Verifying parameter read/write functionality
4. 🔄 Developing control routines for vacuum evacuation and speed management
5. ⏳ Planning automated sputtering deposition control loop

## Project Structure

```text
.
├── SputteringControlLoop/ # Main project directory
│ ├── SputteringControlLoop.ino # Main Arduino sketch
│ ├── config.h # Global configuration and debug flags
│ ├── ArduinoPfeiffer.h/.cpp # Pfeiffer protocol command formatter
│ ├── SerialComms.h/.cpp # RS485/serial communication layer
│ ├── PfeifferPumpCommands.h/.cpp # Pump control parameter definitions
│ ├── AlicatMfcCommands.h/.cpp # MFC control functions
│ └── Pressure.h/.cpp # Pressure data structures and conversion
│
├── README.md # This file
|
|   # Everything below is legacy
├───Alicat_tests
├───brendanSweeney_tutorial
├───Control_loops_Tests
├───DisplayPressureTest
├───PfiefferLib
├───PfiefferWorking
├───PressureControl
└── SetPoint_Alicat
```

## Key Features

- **Modular communication layer**: Separate abstractions for protocol formatting and hardware transport
- **Parameter-based control**: Comprehensive Pfeiffer parameter definitions as C macros
- **Error handling**: Checksum verification and telegram parsing with error detection
- **Debug mode**: Conditional verbose logging via `#define VERBOSE` in [`config.h`](SputteringControlLoop/config.h)

## Getting Started

### Prerequisites

- Arduino IDE 1.8+ or compatible (VS Code with Arduino extension)
- SoftwareSerial library (included with Arduino)
- RS485 transceiver hardware (MAX485 or similar)
- RS232 transceiver hardware

### Hardware Connections

**RS485 (Pump)**:
- RX: Pin 6
- TX: Pin 7
- DE: Pin 5
- RE: Pin 4

**RS485 (Gauge)**:
- RX: Pin 10
- TX: Pin 11
- DE: Pin 9
- RE: Pin 8

**Argon Alicat MFC (Serial)**:
- RX: Pin 13
- TX: Pin 12

### Uploading

1. Open `SputteringControlLoop/SputteringControlLoop.ino` in Arduino IDE
2. Select **Board**: Arduino Uno
3. Select appropriate **Port**
4. Click **Upload**

### Testing

The current sketch runs basic tests on startup:

- Turns pump ON and ramps to operational speed
- Reads pressure from gauge
- Varies pump speed through setpoints
- Turn pump OFF
- Tests MFC setpoint control

Monitor via Serial at **9600 baud** to observe test output.

## Documentation

### Pfeiffer Vacuum Protocol

Consult the following manuals (available from [Pfeiffer Vacuum download center](https://www.pfeiffer-vacuum.com)):

- **HiCube Eco Operating Instructions** - [Station overview and specifications](https://www.idealvac.com/files/manuals/Pfeiffer_HiCube_ECO_30_80_300_Turbo_Pumping_System_Operating_Instructions.pdf), [station purchase link](https://www.idealvac.com/en-us/Pfeiffer-HiCube-300-Eco-Benchtop-Turbomolecular-Pump-Station-ISO-100K-Inlet-110-240-VAC-PN:-PM-025-019-AT/pp/P1011608)
- **TC 110 Electronic Drive Unit Manual** - [RS485 parameter reference](https://www.idealvac.com/files/brochures/Pfeiffer_TC_110_Operating_Instructions.pdf)
- **HiPace 300 Operating Instructions** - [Turbo pump specifications](https://www.idealvac.com/files/manuals/Pfeiffer_HiPace300_Manaul.pdf)
- **MPT 200 Operating Instructions** - [Vacuum gauge specifications](https://www.idealvac.com/files/manuals/Pfeiffer_DigiLine_MPT_200_Digital_Pirani_and_Cold_Cathode_Gauge_Operating_Instructions.pdf)
- **Pfeiffer Vacuum RS485 Protocol Specification** - [Telegram format details](https://www.idealvac.com/files/brochures/Pfeiffer_TC_110_Operating_Instructions.pdf#G6923033)


## Future Development

Planned features for automated sputtering control:

- **Pressure-based staging**: Evacuate → ignition → sputter pressure setpoints
- **Gas mixing control**: Multi-MFC coordination for Ar/O₂ mixtures
- **Control GUI**: Graphical interface for controlling setpoints and stages

## Sputtering Control Flow (WIP)

### 1. Evacuation
- Evacuate chamber to **1×10⁻⁷ hPa**
- **Wait for user input** - Reminder: Open gas cylinder valve

### 2. Ignition
- Reduce pump speed to **25%** (250Hz)
- Ramp argon flow up to **~1 SCCM**, later maybe as high as **~10 SCCM**
- Wait until pump speed reaches **250 Hz**
- Spike argon flow to **~90 SCCM** (or set target pressure)
- **Wait for user input** - Reminder: Power on plasma source and do impedance match

### 3. Sputtering
- Lower argon flow to specified setpoint (target pressure)
- **Wait for user input** - Reminder: Wait for target cleaning, then open shutter
- Run deposition for desired time

### 4. Completion
- Shut off RF power
- Cut off argon flow
- Turn off pumping station
- Wait until turbo speed reaches **0 Hz**
- **User message** - Safe to vent chamber

## Authors

- Kyle Minihan, Ryan Massie - System revision and upgrading (2026)
- Shayaan Gandhi - Original Pfeiffer protocol library (2025)

## Acknowledgments

- Pfeiffer Vacuum for comprehensive RS485 protocol documentation
- Alicat Scientific for serial communication specifications

