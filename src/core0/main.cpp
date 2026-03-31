/**
 * main.cpp
 * @brief Main entry point for the Sputtering system on Core 0.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <stdio.h>
#include "AlicatMFC.h"
#include "Core1Main.h"
#include "Intercore.h"
#include "PIO_UART.h"
#include "PfiefferGauge.h"
#include "PfiefferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "USBSerial.h"
#include "HardwareUART.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include "GlobalDevices.h"

void executeNormalShutdown();
void executeEmergencyShutdown();

int main()
{
    // Enable IO
    stdio_init_all();

    // Launch Core 1
    multicore_launch_core1(core1_entry);

    /**
     * Define Devices
     */

    // Setup the serial communication
    USBSerial pcTerminal;
    pcTerminal.begin();

    // Set up parsing callback
    pcTerminal.setCallback(
        [&pcTerminal](const std::string &command)
        {
            // Echo the command back to the PC
            pcTerminal.println(command.c_str());

            // Future: Pass the command to the Sputtering Manager on Core 1
        });

    /**
     * Initialize Devices
     */
    mfc1.init();
    mfc2.init();
    gauge.init();
    pump.init();

    /**
     * Loop through until program exit and update devices
     */
    bool run{true};

    uint64_t currentTime;
    uint64_t previousTime;

    while (run)
    {
        // Device Updates
        pcTerminal.update();
        mfc1.update();
        mfc2.update();
        gauge.update();
        pump.update();

        currentTime = get_absolute_time();

        /** TODO PID Control Loop */

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

    /**
     * Shutdown Procedure
     */

    if (getStatus(Status_Core1Err))
    {

    }
}

void executeNormalShutdown()
{
    uint64_t currentTime = get_absolute_time();
    uint64_t previousTime;

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
            // TODO Error Message
            setStatus(Status_Core0Err);
        }

        previousTime = currentTime;
    }

    // TODO mfc done message

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
    sleep_ms(50); // Time to ensure send message
    setStatus(Status_Exit);
}