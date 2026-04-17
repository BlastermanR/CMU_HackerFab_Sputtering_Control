# CMU HackerFab Sputtering Control System

Firmware for the CMU HackerFab magnetron sputtering system, targeting the
RP2350 (Pico 2) dual-core microcontroller. Built on the
[SputterOS](https://github.com/BlastermanR/SputterOS) v0.5 deterministic
AMP control framework.

## Architecture

| Core | SputterOS Task | Responsibility |
|:-----|:---------------|:---------------|
| 0 | `ScheduledControlTask` | Safety monitors → device polling → process state machine (100 Hz) |
| 1 | `ScheduledCommsTask` | USB byte ingestion → command parsing → lock-free queue push |
| 1 | `BackgroundDiagnosticsTask` | Watchdog kick, timing budgets, telemetry drain |

### Process State Machine

```
IDLE → PUMP_DOWN → PURGE → IGNITION → DEPOSITING → VENTING → IDLE
                         ↘ FAULT (from any state)
```

Each phase is an `IProcessState` implementation in `src/logic/ProcessPhases.cpp`.
`SputteringApp` (`src/SputteringApp.cpp`) owns the phase objects and implements
`IUserApplication<SputteringCfg>`.

### Key Modules

| Module | Location | Description |
|:-------|:---------|:------------|
| `SputteringCfg` | `include/config/` | Compile-time config (states, commands, queue depth) |
| `SputteringApp` | `include/` + `src/` | Process engine driven by ControlTask |
| `ProcessPhases` | `include/logic/` + `src/logic/` | Per-phase state machines |
| `SafetyMonitors` | `include/safety/` | Over-pressure and pump-health monitors |
| `USBStream` | `include/hal/` + `src/hal/` | `IStream` wrapper for Pico USB stdio |
| `PicoMutex` | `include/osal/` | `IMutex` wrapper for Pico SDK mutex |
| Device drivers | `src/core0/` + `include/core0/` | AlicatMFC, PfeifferGauge, PfeifferPump |

## Serial Interface

The system is controlled via USB serial (115200 baud). Under SputterOS, the
`CommsTask` on Core 1 parses inbound text into `SputteringCfg::Command` packets.

### Command Format

```
<CmdID> <targetDevice> <value>\n
```

| CmdID | Name | Example | Action |
|:------|:-----|:--------|:-------|
| 0 | SET_STATE | `0 0 1.0` | Transition to PUMP_DOWN (state 1) |
| 1 | ABORT | `1 0 0.0` | Force safe abort → FAULT |
| 2 | SET_ARGON_FLOW | `2 0 20.0` | Set argon MFC to 20 sccm |
| 3 | SET_OXYGEN_FLOW | `3 0 5.0` | Set oxygen MFC to 5 sccm |
| 4 | SET_PUMP_SPEED | `4 0 700.0` | Set pump speed (not yet implemented) |
| 5 | SET_VERBOSITY | `5 0 3.0` | Set telemetry verbosity (0–3) |

### Telemetry Output

The `TelemetryLogger` drains formatted log entries to USB:
```
[<timestamp_ms>][<TaskName>] <text>
```

## Building

### Firmware (Pico SDK cross-compile)
```bash
cmake -B build -G Ninja
ninja -C build
```
Produces `build/SputteringACS.uf2` for flashing via picotool or drag-and-drop.

### Unit Tests (host-native)
```bash
cmake -B build_tests -S tests -G Ninja
ninja -C build_tests
ctest --test-dir build_tests --output-on-failure
```

## Test Coverage

| Test suite | File | Covers |
|:-----------|:-----|:-------|
| `test_sputtering_cfg` | `tests/shared/` | Config struct, enums, constants |
| `test_safety_monitors` | `tests/shared/` | OverPressureMonitor, PumpHealthMonitor |
| `test_sputtering_app` | `tests/shared/` | SputteringApp state machine, commands, abort |
| `test_process_phases` | `tests/shared/` | All 5 process phase transitions |
| `pico_memory_usage` | `Tools/` | ELF heap size check (≥ 2048 bytes) |
