#include "core0/SputteringManagement.h"
#include "core0/GlobalDevices.h"
#include "shared/Intercore.h"
#include "USBSerial.h"
#include "pico/stdlib.h"
#include <cstdio>

namespace SputteringManagement {

static uint32_t lastArgonPoll = 0;
static uint32_t lastOxygenPoll = 0;
static uint32_t lastPumpPoll = 0;
static uint32_t lastGaugePoll = 0;
static const uint32_t POLL_INTERVAL_MS = 100;

void controlLoop() {
    USBSerial::log(Source_Core0, "SputteringManagement::controlLoop() - Placeholder running", V_INFO);
    while (!(getStatus(Status_Exit) || getStatus(Status_Core1Err))) {
        executePollDevices();
        sleep_ms(10);
    }
}

void executeNormalShutdown() {
    USBSerial::log(Source_Core0, "SputteringManagement::executeNormalShutdown()", V_INFO);
    mfc1.setSetpoint(0);
    mfc2.setSetpoint(0);
    pump.deactivatePump();
}

void executeEmergencyShutdown() {
    USBSerial::log(Source_Core0, "SputteringManagement::executeEmergencyShutdown()", V_CRITICAL);
    mfc1.setSetpoint(0);
    mfc2.setSetpoint(0);
    // Add logic to quickly depressurize safely if needed
}

void executePollDevices() {
    executePollArgon();
    executePollOxygen();
    executePollPump();
    executePollGauge();
}

void executePollArgon() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - lastArgonPoll >= POLL_INTERVAL_MS) {
        mfc2.update();
        sharedData.Core0Out.argonFlow = (float)mfc2.getMassFlow();
        lastArgonPoll = now;
    }
}

void executePollOxygen() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - lastOxygenPoll >= POLL_INTERVAL_MS) {
        mfc1.update();
        sharedData.Core0Out.oxygenFlow = (float)mfc1.getMassFlow();
        lastOxygenPoll = now;
    }
}

void executePollPump() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - lastPumpPoll >= POLL_INTERVAL_MS) {
        pump.update();
        lastPumpPoll = now;
    }
}

void executePollGauge() {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - lastGaugePoll >= POLL_INTERVAL_MS) {
        gauge.update();
        sharedData.Core0Out.chamberPressure = (float)gauge.getPressure();
        lastGaugePoll = now;
    }
}

} // namespace SputteringManagement
