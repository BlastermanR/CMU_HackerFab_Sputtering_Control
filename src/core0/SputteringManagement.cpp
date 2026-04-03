/**
 * SputteringManagement.cpp
 * @brief Implementation of the Management namespace for the Sputtering automation system.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/2/26
 */

#include "SputteringManagement.h"
#include "GlobalDevices.h"
#include "Intercore.h"
#include "USBSerial.h"
#include "pico/stdlib.h"
#include <cmath> // For abs()

namespace SputteringManagement
{

/**
 * @brief Main control loop for Core 0.
 *
 * Continually updates all hardware devices (MFCs, pump, gauge, PC terminal).
 * At regular intervals, it synchronizes device telemetry with the inter-core
 * shared memory.
 *
 * TODO: Implement PID control functionality.
 */
void controlLoop()
{
    bool     run{true};
    uint64_t currentTime;
    uint64_t previousTime;

    while (run)
    {
        // Device Updates
        mfc1.update();
        mfc2.update();
        gauge.update();
        pump.update();

        currentTime = get_absolute_time();

        /**
         * TODO: Execute PID Control algorithm.
         */

        if ((currentTime - previousTime) >= CORE0_UPDATE_INTERVAL_MS)
        {
            sharedData.Core0Out.actualPumpSpeed = pump.getPumpSpeed();
            sharedData.Core0Out.chamberPressure = gauge.getPressure();
            sharedData.Core0Out.oxygenFlow      = mfc1.getVolumetricFlow();
            sharedData.Core0Out.argonFlow       = mfc2.getVolumetricFlow();
        }

        previousTime = currentTime;
        run          = getStatus(Status_Core1Err) || getStatus(Status_Exit);
    }
}

/**
 * @brief Performs a controlled shutdown of the Sputtering system.
 *
 * Safely ramps down the Mass Flow Controllers (MFCs) by setting flow to zero,
 * waits for the flow to cease, deactivates the vacuum pump, and prepares
 * the system for a safe exit state.
 */
void executeNormalShutdown()
{
    uint64_t currentTime = get_absolute_time();
    uint64_t previousTime;

    USBSerial::log(Source_Core0, "Ramping down MFCs", V_INFO);
    mfc1.setSetpoint(0);
    mfc2.setSetpoint(0);

    bool           mfcFlow{true};
    const uint64_t MFC_RESENT_INTERVAL_MS   = 20;
    const uint64_t MFC_MAX_RAMPDOWN_TIME_MS = 2000;
    const uint64_t RAMP_DOWN_START_TIME     = currentTime;

    while (mfcFlow)
    {
        currentTime = get_absolute_time();

        mfc1.update();
        mfc2.update();

        mfcFlow || !(std::abs(mfc1.getVolumetricFlow()) > 0);
        mfcFlow || !(std::abs(mfc1.getVolumetricFlow()) > 0);

        if ((currentTime - previousTime) >= MFC_RESENT_INTERVAL_MS * 1000)
        {
            mfc1.setSetpoint(0);
            mfc2.setSetpoint(0);
        }

        if ((currentTime - RAMP_DOWN_START_TIME) >= MFC_MAX_RAMPDOWN_TIME_MS * 1000)
        {
            USBSerial::log(Source_Core0, "MFC rampdown timeout", V_CRITICAL);
            setStatus(Status_Core0Err);
        }

        previousTime = currentTime;
    }

    USBSerial::log(Source_Core0, "MFCs stopped, deactivating pump", V_INFO);
    pump.deactivatePump();

    USBSerial::log(Source_Core0, "Normal shutdown complete", V_STATUS);
    sleep_ms(50);
    setStatus(Status_Exit);
}

/**
 * @brief Immediately halts all operations during an error state.
 *
 * Sets the MFCs to zero flow immediately without waiting, deactivates the
 * vacuum pump, and opens the vent valve to bring the chamber to atmospheric
 * pressure as safely and quickly as possible.
 */
void executeEmergencyShutdown()
{
    USBSerial::log(Source_Core0, "Emergency shutdown: stopping all devices", V_CRITICAL);
    mfc1.setSetpoint(0);
    mfc2.setSetpoint(0);
    pump.deactivatePump();
    pump.ventPump();
}

/**
 * @brief Manually polls all devices and updates shared memory.
 *
 * Triggers an update request to all peripheral devices, waits briefly for
 * responses to return over serial, then parses the received data and
 * updates the intercore shared memory structure.
 */
void executePollDevices()
{
    mfc1.pollData();
    mfc2.pollData();
    gauge.pollDevice();
    pump.pollDevice();

    // Hot loop to process incoming serial data instead of sleeping, safely waiting for 50ms
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < 50)
    {
        mfc1.update();
        mfc2.update();
        gauge.update();
        pump.update();
    }

    sharedData.Core0Out.actualPumpSpeed = pump.getPumpSpeed();
    sharedData.Core0Out.chamberPressure = gauge.getPressure();
    sharedData.Core0Out.oxygenFlow      = mfc1.getVolumetricFlow();
    sharedData.Core0Out.argonFlow       = mfc2.getVolumetricFlow();

    // Stream telemetry to the terminal
    USBSerial::sendData(Source_Core0, Data_PumpSpeed, sharedData.Core0Out.actualPumpSpeed);
    USBSerial::sendData(Source_Core0, Data_ChamberPressure, sharedData.Core0Out.chamberPressure);
    USBSerial::sendData(Source_Core0, Data_OxygenFlow, sharedData.Core0Out.oxygenFlow);
    USBSerial::sendData(Source_Core0, Data_ArgonFlow, sharedData.Core0Out.argonFlow);
}

void executePollArgon()
{
    mfc2.pollData();

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < 50)
    {
        mfc2.update();
    }

    sharedData.Core0Out.argonFlow = mfc2.getVolumetricFlow();
    USBSerial::sendData(Source_Core0, Data_ArgonFlow, sharedData.Core0Out.argonFlow);
}

void executePollOxygen()
{
    mfc1.pollData();

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < 50)
    {
        mfc1.update();
    }

    sharedData.Core0Out.oxygenFlow = mfc1.getVolumetricFlow();
    USBSerial::sendData(Source_Core0, Data_OxygenFlow, sharedData.Core0Out.oxygenFlow);
}

void executePollPump()
{
    pump.pollDevice();

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < 50)
    {
        pump.update();
    }

    sharedData.Core0Out.actualPumpSpeed = pump.getPumpSpeed();
    USBSerial::sendData(Source_Core0, Data_PumpSpeed, sharedData.Core0Out.actualPumpSpeed);
}

void executePollGauge()
{
    gauge.pollDevice();

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while ((to_ms_since_boot(get_absolute_time()) - start_time) < 50)
    {
        gauge.update();
    }

    sharedData.Core0Out.chamberPressure = gauge.getPressure();
    USBSerial::sendData(Source_Core0, Data_ChamberPressure, sharedData.Core0Out.chamberPressure);
}
} // namespace SputteringManagement
