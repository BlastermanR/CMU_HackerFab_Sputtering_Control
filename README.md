# Source Directory Overview

This directory contains the core implementation for the CMU HackerFab Sputtering Control System.

## Serial Interface Commands

The system is controlled via the USB Serial interface (115200 baud). Commands are case-sensitive and must be followed by a newline (`\n` or `\r`).

### Basic Operations
| Command | Action |
| :--- | :--- |
| `START` | Begins the sputtering process sequencer. |
| `STOP` | Immediately stops the sputtering process sequencer. |
| `PRESSURIZE` | Triggers the chamber pressurization sequence. |
| `VENT` | Triggers the chamber venting sequence. |
| `GASOFF` | Immediately shuts off all gas flow (Argon and Oxygen). |
| `POLL` | Manually triggers a one-time poll of all connected hardware devices. |
| `POLLARGON` | Triggers a one-time poll of the Argon MFC. |
| `POLLOXYGEN` | Triggers a one-time poll of the Oxygen MFC. |
| `POLLPUMP` | Triggers a one-time poll of the Vacuum Pump. |
| `POLLGAUGE` | Triggers a one-time poll of the Pressure Gauge. |
| `EXIT` | Signals an emergency stop and sets the system to an exit state. |

### Manual Hardware Control
| Command | Action |
| :--- | :--- |
| `PUMPON` | Enables the vacuum pump. |
| `PUMPOFF` | Disables the vacuum pump. |
| `SETARGON <val>` | Sets the Argon Mass Flow Controller setpoint (e.g., `SETARGON 15.5`). |
| `SETOXYGEN <val>` | Sets the Oxygen Mass Flow Controller setpoint (e.g., `SETOXYGEN 5.0`). |
| `SETPUMP <val>` | Sets the Vacuum Pump speed setpoint (range depends on hardware). |

### System Configuration
| Command | Action |
| :--- | :--- |
| `VERBOSE <level>` | Sets the logging verbosity level. |
| | `0`: Critical errors only |
| | `1`: Status updates |
| | `2`: Informational (Info) |
| | `3`: Debug (All logs) |

## Telemetry Format
The system streams telemetry data in the following format:
`$<DataType>:<Value>`

Examples:
- `$Pressure:1.2345`
- `$ArgonFlow:0.0000`
- `$PumpSpeed:700.0000`
