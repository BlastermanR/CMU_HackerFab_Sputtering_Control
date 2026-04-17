/**
 * @file main.cpp
 * @brief SputterOS entry point for the CMU HackerFab sputtering system.
 *
 * Wires the SputterOS kernel using `SystemBuilder<SputteringCfg>` and
 * launches the dual-core scheduler via `multicore_launch_core1` + `System::run`.
 *
 * Core 0 — `ScheduledControlTask`: safety monitors → device polling → process logic.
 * Core 1 — `ScheduledCommsTask`: USB byte ingestion → command parsing → queue push.
 *           `BackgroundDiagnosticsTask`: watchdog kick, timing budgets, memory profiling.
 *
 * Serial command wire format (TEXT mode):
 *   `<CmdID> <targetDevice> <value>\n`
 *
 * CmdID values are defined in `SputteringCfg::CmdID`.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "GlobalDevices.h"
#include "SafetyMonitors.h"
#include "SputteringApp.h"
#include "SputteringCfg.h"
#include "USBStream.h"
#include "osal/PicoMutex.h"
#include "sputteros/builder/SystemBuilder.h"
#include "sputteros/kernel/System.h"
#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <array>
#include <cstdio>

// ---------------------------------------------------------------------------
// Telemetry drain — writes bytes from TelemetryLogger to USB stdout.
// ---------------------------------------------------------------------------

static void usbTelemetryWrite(const uint8_t *data, std::size_t len, void * /*ctx*/)
{
    fwrite(data, 1, len, stdout);
}

// ---------------------------------------------------------------------------
// Platform clock source
// ---------------------------------------------------------------------------

static uint64_t picoGetTimeMicros()
{
    return to_us_since_boot(get_absolute_time());
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    /****** Platform Initialisation ******/

    stdio_init_all();

    // Wait for USB host to connect so the startup banner is visible.
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    printf("========================================\n");
    printf("  CMU HackerFab Sputtering Control v0.2\n");
    printf("         Powered by SputterOS\n");
    printf("========================================\n");

    /****** Hardware Watchdog ******/

    // 2-second timeout, pause watchdog during debug.
    watchdog_enable(2000, true);

    /****** HAL Construction ******/

    USBStream usbStream;

    /****** Safety Monitor Construction ******/

    // gauge and pump are the global device instances from GlobalDevices.cpp.
    OverPressureMonitor pressureMonitor(&gauge);
    PumpHealthMonitor   pumpMonitor(&pump);

    std::array<SputterOS::ISafetyMonitor *, 2> monitors = {&pressureMonitor, &pumpMonitor};

    /****** Application Construction ******/

    SputteringApp app;

    /****** OSAL Mutex for shared TelemetryLogger ******/

    PicoMutex telemetryMutex;

    /****** SystemBuilder Wiring ******/

    SputterOS::SystemBuilder<SputteringCfg> builder(&app, monitors.data(), monitors.size());

    builder.setStream(&usbStream);
    builder.setWatchdogKick([]() { watchdog_update(); });
    builder.setClockSource(picoGetTimeMicros);
    builder.setTelemetryDrain(usbTelemetryWrite, nullptr);
    builder.setTelemetryMutex(&telemetryMutex);

    const SputterOS::BuildResult result = builder.build();
    if (!result)
    {
        printf("[FATAL] SputterOS build failed: %s\n", result.error);
        while (true)
        {
            tight_loop_contents();
        }
    }

    printf("SputterOS kernel built. Launching Core 1...\n");

    /****** Dual-Core Launch ******/

    // Core 1: ScheduledCommsTask + BackgroundDiagnosticsTask.
    multicore_launch_core1([]() { SputterOS::System<SputteringCfg>::run(1); });

    // Core 0: ScheduledControlTask (blocks until a stop condition fires).
    SputterOS::System<SputteringCfg>::run(0);
}
