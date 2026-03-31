/**
 * main.cpp
 * @brief Main entry point for the Sputtering system on Core 0.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <stdio.h>
#include <cstring>
#include "AlicatMFC.h"
#include "Core1Main.h"
#include "Intercore.h"
#include "PIO_UART.h"
#include "PfiefferGauge.h"
#include "PfiefferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "HardwareUART.h"
#include "USBSerial.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include "GlobalDevices.h"

void controlLoop();
void executeNormalShutdown();
void executeEmergencyShutdown();
void executePollDevices();

int main()
{
    // Enable IO
    stdio_init_all();

    // Initialize inter-core queues before launching Core 1
    initQueues();

    // Launch Core 1
    multicore_launch_core1(core1_entry);

    /**
     * Initialize Devices
     */
    USBSerial::log(Source_Core0, "Initializing devices", V_INFO);
    mfc1.init();
    USBSerial::log(Source_Core0, "MFC1 initialized", V_DEBUG);
    mfc2.init();
    USBSerial::log(Source_Core0, "MFC2 initialized", V_DEBUG);
    gauge.init();
    USBSerial::log(Source_Core0, "Gauge initialized", V_DEBUG);
    pump.init();
    USBSerial::log(Source_Core0, "Pump initialized", V_DEBUG);

    // Signal Core 0 initialization complete
    setStatus(Core0_Begin);
    USBSerial::log(Source_Core0, "Core 0 initialized, waiting for Core 1", V_INFO);

    // Wait for Core 1 to signal ready
    {
        uint64_t handshakeStart = get_absolute_time();
        while (!getStatus(Core1_Begin))
        {
            if ((get_absolute_time() - handshakeStart) >= (uint64_t)HANDSHAKE_TIMEOUT_MS * 1000)
            {
                USBSerial::log(Source_Core0, "Core 1 handshake timeout", V_CRITICAL);
                setStatus(Status_Core1Err);
                break;
            }
            sleep_ms(1);
        }
    }

    if (isError())
    {
        executeEmergencyShutdown();
        return 1;
    }

    USBSerial::log(Source_Core0, "Core 1 ready, entering main loop", V_INFO);

    {
        bool run{true};

        uint64_t currentTime;
        uint64_t previousTime;

        while(run)
        {
            if (getStatus(ExecuteSputteringProcess))
            {
                USBSerial::log(Source_Core0, "Entering control loop", V_INFO);
                controlLoop();
                clearStatus(ExecuteSputteringProcess);
                USBSerial::log(Source_Core0, "Exited control loop", V_INFO);
            }

            if (getStatus(PressurizeChamber))
            {
                USBSerial::log(Source_Core0, "Activating pump", V_INFO);
                pump.activatePump();
                clearStatus(PressurizeChamber);
            }

            if (getStatus(VentChamber))
            {
                USBSerial::log(Source_Core0, "Venting chamber", V_INFO);
                pump.deactivatePump();
                pump.ventPump();
                clearStatus(VentChamber);
            }

            if (getStatus(ShutOffGasFlow))
            {
                USBSerial::log(Source_Core0, "Shutting off gas flow", V_INFO);
                mfc1.setSetpoint(0);
                mfc2.setSetpoint(0);
                clearStatus(ShutOffGasFlow);
            }

            if (getStatus(PollDevices))
            {
                USBSerial::log(Source_Core0, "Polling devices", V_INFO);
                executePollDevices();
                clearStatus(PollDevices);
            }

            // Check for exit
            run = !(getStatus(Status_Core1Err) || getStatus(Status_Exit));

            sleep_ms(5); // Short delay to wait for something to change
        }
    }

    /**
     * Shutdown Procedure
     */

    if (getStatus(Status_Core1Err))
    {
        USBSerial::log(Source_Core0, "Core 1 error, emergency shutdown", V_CRITICAL);
        executeEmergencyShutdown();
    }
    else
    {
        USBSerial::log(Source_Core0, "Normal shutdown initiated", V_STATUS);
        executeNormalShutdown();
    }
}

/**
 * @brief Main control loop for Core 0.
 * 
 * Continually updates all hardware devices (MFCs, pump, gauge, PC terminal).
 * At regular intervals, it synchronizes device telemetry with the inter-core 
 * shared memory.
 * 
 * TODO: Implement PID control functionality. This loop will be responsible for 
 * evaluating pressure/flow setpoints from Core 1 and running the PID algorithm 
 * to adjust the vacuum pump and MFC setpoints accordingly.
 */
void controlLoop()
{
    bool run{true};

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
         * Calculate and update hardware setpoints based on the desired 
         * target values from Core 1 and the current device readings.
         */

        // Update info at regular intervals 
        if ((currentTime - previousTime) >= CORE0_UPDATE_INTERVAL_MS)
        {
            sharedData.Core0Out.actualPumpSpeed = pump.getPumpSpeed();
            sharedData.Core0Out.chamberPressure = gauge.getPressure();

            /**
             * Currenty uses volumentric flow as actual measure isn't too important
             */
            sharedData.Core0Out.oxygenFlow = mfc1.getVolumetricFlow();
            sharedData.Core0Out.argonFlow = mfc2.getVolumetricFlow();   
        }

        // Update time
        previousTime = currentTime;

        // Check for exit
        run = getStatus(Status_Core1Err) || getStatus(Status_Exit);
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
    // Shut down gas flow
    mfc1.setSetpoint(0);
    mfc2.setSetpoint(0);

    bool mfcFlow{true};
    const uint64_t MFC_RESENT_INTERVAL_MS = 20;
    const uint64_t MFC_MAX_RAMPDOWN_TIME_MS = 2000;
    const uint64_t RAMP_DOWN_START_TIME = currentTime;

    while(mfcFlow)
    {
        currentTime = get_absolute_time();

        mfc1.update();
        mfc2.update();

        // Loop will break once flow is zero
        mfcFlow || !(abs(mfc1.getVolumetricFlow()) > 0);
        mfcFlow || !(abs(mfc1.getVolumetricFlow()) > 0);

        // Check resend interval
        if ((currentTime - previousTime) >= MFC_RESENT_INTERVAL_MS)
        {
            mfc1.setSetpoint(0);
            mfc2.setSetpoint(0);
        }

        // Check max time
        if ((currentTime - RAMP_DOWN_START_TIME) >= MFC_MAX_RAMPDOWN_TIME_MS)
        {
            USBSerial::log(Source_Core0, "MFC rampdown timeout", V_CRITICAL);
            setStatus(Status_Core0Err);
        }

        previousTime = currentTime;
    }

    // TODO mfc done message
    USBSerial::log(Source_Core0, "MFCs stopped, deactivating pump", V_INFO);

    // Deactivate pump
    pump.deactivatePump();

    /*
    TODO: Don't vent for testing

    // Wait for it to stop spinning
    while (pump.getPumpSpeed() > 0)
    {
        sleep_ms(10);
    }

    // Vent chamber
    pump.ventPump();
    */

    // TODO Message
    USBSerial::log(Source_Core0, "Normal shutdown complete", V_STATUS);
    sleep_ms(50); // Time to ensure send message
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

    // Shutdown MFCs
    mfc1.setSetpoint(0);
    mfc2.setSetpoint(0);

    // Shutdown Pump & Vent
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
    // Trigger devices to send requests
    mfc1.update();
    mfc2.update();
    gauge.update();
    pump.update();

    // Wait to ensure devices have time to respond over serial
    sleep_ms(50);

    // Process received data
    mfc1.update();
    mfc2.update();
    gauge.update();
    pump.update();

    // Update intercore shared values
    sharedData.Core0Out.actualPumpSpeed = pump.getPumpSpeed();
    sharedData.Core0Out.chamberPressure = gauge.getPressure();
    sharedData.Core0Out.oxygenFlow      = mfc1.getVolumetricFlow();
    sharedData.Core0Out.argonFlow       = mfc2.getVolumetricFlow();
}
