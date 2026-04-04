/**
 * main.cpp
 * @brief Main entry point for the Sputtering system on Core 0.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "AlicatMFC.h"
#include "Core1Main.h"
#include "GlobalDevices.h"
#include "HardwareUART.h"
#include "Intercore.h"
#include "PIO_UART.h"
#include "PfeifferGauge.h"
#include "PfeifferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "USBSerial.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "picoDefinitions.h"
#include <cstring>
#include <stdio.h>

// Enable for serial testing
#define SERIAL_DEBUG

volatile bool processUpdateFlag = false;

// Interrupt based timer callback
bool update_timer_callback(struct repeating_timer *t)
{
    processUpdateFlag = true;
    return true; // Return true to keep the timer repeating
}

int main()
{
    /*********** Initialzation Space ***********/

    // Enable IO
    stdio_init_all();

    // Wait for USB connection to be active before printing
    // This ensures the user sees the banner in the terminal
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }

    printf("========================================\n");
    printf("  CMU HackerFab Sputtering Control v0.1\n");
    printf("========================================\n");
    printf("   ____ __  __ _    _  _ \n");
    printf("  / ___|  \\/  | |  | || |\n");
    printf(" | |   | |\\/| | |  | || |\n");
    printf(" | |___| |  | | |__| ||_|\n");
    printf("  \\____|_|  |_|\\____/ (_) \n");
    printf("========================================\n");
    printf("System IO Intialiazed!\n");

    // Initialize inter-core queues before launching Core 1
    initQueues();
    printf("Intercore Queues Initialized!\n");

    // Launch Core 1
    printf("Launching Core 1; Handing off control. buh bye! \n");

    /*********** Multicore Barrier ***********/
    multicore_launch_core1(core1_entry);

    /**
     * Initialize Devices
     */
    USBSerial::log(Source_Core0, "Initializing devices", V_INFO);
    mfc1.init();
    mfc1.setGas(ALICAT_GAS_O2);
    USBSerial::log(Source_Core0, "MFC1 initialized to Oxygen", V_DEBUG);
    mfc2.init();
    mfc2.setGas(ALICAT_GAS_AR);
    USBSerial::log(Source_Core0, "MFC2 initialized to Argon", V_DEBUG);
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
                printf("Handshake Error: Core 1 handshake timeout!\n"); // Ensure Print to terminal
                setStatus(Status_Core1Err);
                break;
            }
            tight_loop_contents();
        }
    }

    if (isError())
    {
        USBSerial::log(Source_Core0, "Core 1 error, emergency shutdown (Not implemented)[]", V_CRITICAL);
        return 1;
    }

    USBSerial::log(Source_Core0, "Core 1 ready, entering main loop", V_INFO);

    // Setup an interrupt based timer update for non-sleep polling (5ms)
    struct repeating_timer timer;
    add_repeating_timer_ms(5, update_timer_callback, NULL, &timer);

    {
        bool run{true};

        while (run)
        {
            StatusMask activeCmd = isCommand();

            if (activeCmd != Status_None)
            {
                switch (activeCmd)
                {
                case ExecuteSputteringProcess:
                {
                    USBSerial::log(Source_Core0, "Entering control loop (Not Implemented)", V_INFO);
                    clearStatus(ExecuteSputteringProcess);
                    USBSerial::log(Source_Core0, "Exited control loop", V_INFO);
                    break;
                }

                case ExecuteCleaningProcess:
                {
                    USBSerial::log(Source_Core0, "Entering cleaning loop (Not Implemented)", V_INFO);
                    clearStatus(ExecuteCleaningProcess);
                    USBSerial::log(Source_Core0, "Exited cleaning loop", V_INFO);
                    break;
                }

                case PressurizeChamber:
                {
                    USBSerial::log(Source_Core0, "Activating pump", V_INFO);
                    pump.activatePump();
                    clearStatus(PressurizeChamber);
                    break;
                }

                case VentChamber:
                {
                    USBSerial::log(Source_Core0, "Venting chamber", V_INFO);
                    pump.deactivatePump();
                    pump.ventPump();
                    clearStatus(VentChamber);
                    break;
                }

                case ShutOffGasFlow:
                {
                    USBSerial::log(Source_Core0, "Shutting off gas flow", V_INFO);
                    mfc1.setSetpoint(0);
                    mfc2.setSetpoint(0);
                    clearStatus(ShutOffGasFlow);
                    break;
                }

                case PollDevices:
                {
                    USBSerial::log(Source_Core0, "Polling devices (Not Implemented)", V_INFO);
                    clearStatus(PollDevices);
                    break;
                }

                case PollArgon:
                {
                    USBSerial::log(Source_Core0, "Polling Argon MFC (Not Implemented)", V_INFO);
                    mfc2.pollData();
                    clearStatus(PollArgon);
                    break;
                }

                case PollOxygen:
                {
                    USBSerial::log(Source_Core0, "Polling Oxygen MFC (Not Implemented)", V_INFO);
                    mfc1.pollData();
                    clearStatus(PollOxygen);
                    break;
                }

                case PollPump:
                {
                    USBSerial::log(Source_Core0, "Polling Pump (Not Implemented)", V_INFO);
                    pump.pollDevice();
                    clearStatus(PollPump);
                    break;
                }

                case PollGauge:
                {
                    USBSerial::log(Source_Core0, "Polling Gauge (Not Implemented)", V_INFO);
                    gauge.pollDevice();
                    clearStatus(PollGauge);
                    break;
                }

                case SetArgonFlow:
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Set Argon Flow to %.2f", sharedData.Core1Out.setArgonFlow);
                    USBSerial::log(Source_Core0, buf, V_INFO);
                    mfc2.setSetpoint(sharedData.Core1Out.setArgonFlow);
                    clearStatus(SetArgonFlow);
                    break;
                }

                case SetOxygenFlow:
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Set Oxygen Flow to %.2f", sharedData.Core1Out.setOxygenFlow);
                    USBSerial::log(Source_Core0, buf, V_INFO);
                    mfc1.setSetpoint(sharedData.Core1Out.setOxygenFlow);
                    clearStatus(SetOxygenFlow);
                    break;
                }

                case SetPumpSpeed:
                {
                    USBSerial::log(Source_Core0, "SetPumpSpeed command received (Not implemented)", V_INFO);
                    clearStatus(SetPumpSpeed);
                    break;
                }

                case EnablePump:
                {
                    USBSerial::log(Source_Core0, "EnablePump command received", V_INFO);
                    pump.activatePump();
                    clearStatus(EnablePump);
                    break;
                }

                case DisablePump:
                {
                    USBSerial::log(Source_Core0, "DisablePump command received", V_INFO);
                    pump.deactivatePump();
                    clearStatus(DisablePump);
                    break;
                }

                default:
                    break;
                }
            }

            // Check for exit
            run = !(getStatus(Status_Core1Err) || getStatus(Status_Exit));

            if (processUpdateFlag)
            {
                processUpdateFlag = false;

                // Continually process incoming hardware serial bytes and send telemetry
                mfc1.update();
                if (mfc1.hasNewData())
                {
                    sharedData.Core0Out.oxygenFlow = (float)mfc1.getMassFlow();
                    USBSerial::sendData(Source_Core0, Data_OxygenFlow, (float)mfc1.getMassFlow());
                }
                
                mfc2.update();
                if (mfc2.hasNewData())
                {
                    sharedData.Core0Out.argonFlow = (float)mfc2.getMassFlow();
                    USBSerial::sendData(Source_Core0, Data_ArgonFlow, (float)mfc2.getMassFlow());
                }

                gauge.update();
                if (gauge.hasNewData())
                {
                    sharedData.Core0Out.chamberPressure = (float)gauge.getPressure();
                    USBSerial::sendData(Source_Core0, Data_ChamberPressure, (float)gauge.getPressure());
                }

                pump.update();
                if (pump.hasNewSpeedData())
                {
                    sharedData.Core0Out.actualPumpSpeed = (float)pump.getActualPumpSpeed_hz();
                    USBSerial::sendData(Source_Core0, Data_PumpSpeed, (float)pump.getActualPumpSpeed_hz());
                }
            }

            // Yield slightly without sleeping the core, allowing interrupts to process
            tight_loop_contents();
        }
    }

    /**
     * Shutdown Procedure
     */

    if (getStatus(Status_Core1Err))
    {
        USBSerial::log(Source_Core0, "Core 1 error, emergency shutdown (Not implemented)[]", V_CRITICAL);
    }
    else
    {
        USBSerial::log(Source_Core0, "Normal shutdown initiated (Not implemented)", V_STATUS);
    }
}
