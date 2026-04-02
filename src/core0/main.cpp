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
#include "PfeifferGauge.h"
#include "PfeifferPump.h"
#include "RS232Device.h"
#include "RS485Device.h"
#include "HardwareUART.h"
#include "USBSerial.h"
#include "SputteringManagement.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include "GlobalDevices.h"

int main()
{
    /*********** Initialzation Space ***********/
    
    // Enable IO
    stdio_init_all();

    // Wait for USB connection to be active before printing
    // This ensures you see the banner in the terminal
    while (!stdio_usb_connected()) {
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
            sleep_ms(1);
        }
    }

    if (isError())
    {
        SputteringManagement::executeEmergencyShutdown();
        return 1;
    }

    USBSerial::log(Source_Core0, "Core 1 ready, entering main loop", V_INFO);
    {
        bool run{true};

        while(run)
        {
            StatusMask activeCmd = isCommand();

            if (activeCmd != Status_None)
            {
                switch (activeCmd)
                {
                    case ExecuteSputteringProcess:
                        USBSerial::log(Source_Core0, "Entering control loop", V_INFO);
                        SputteringManagement::controlLoop();
                        clearStatus(ExecuteSputteringProcess);
                        USBSerial::log(Source_Core0, "Exited control loop", V_INFO);
                        break;
                        
                    case PressurizeChamber:
                        USBSerial::log(Source_Core0, "Activating pump", V_INFO);
                        pump.activatePump();
                        clearStatus(PressurizeChamber);
                        break;
                        
                    case VentChamber:
                        USBSerial::log(Source_Core0, "Venting chamber", V_INFO);
                        pump.deactivatePump();
                        pump.ventPump();
                        clearStatus(VentChamber);
                        break;
                        
                    case ShutOffGasFlow:
                        USBSerial::log(Source_Core0, "Shutting off gas flow", V_INFO);
                        mfc1.setSetpoint(0);
                        mfc2.setSetpoint(0);
                        clearStatus(ShutOffGasFlow);
                        break;
                        
                    case PollDevices:
                        USBSerial::log(Source_Core0, "Polling devices", V_INFO);
                        SputteringManagement::executePollDevices();
                        clearStatus(PollDevices);
                        break;

                    case SetArgonFlow:
                        USBSerial::log(Source_Core0, "SetArgonFlow command received (Not implemented)", V_INFO);
                        clearStatus(SetArgonFlow);
                        break;

                    case SetOxygenFlow:
                        USBSerial::log(Source_Core0, "SetOxygenFlow command received (Not implemented)", V_INFO);
                        clearStatus(SetOxygenFlow);
                        break;

                    case SetPumpSpeed:
                        USBSerial::log(Source_Core0, "SetPumpSpeed command received (Not implemented)", V_INFO);
                        clearStatus(SetPumpSpeed);
                        break;

                    case EnablePump:
                        USBSerial::log(Source_Core0, "EnablePump command received (Not implemented)", V_INFO);
                        clearStatus(EnablePump);
                        break;

                    case DisablePump:
                        USBSerial::log(Source_Core0, "DisablePump command received (Not implemented)", V_INFO);
                        clearStatus(DisablePump);
                        break;

                    default:
                        break;
                }
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
        SputteringManagement::executeEmergencyShutdown();
    }
    else
    {
        USBSerial::log(Source_Core0, "Normal shutdown initiated", V_STATUS);
        SputteringManagement::executeNormalShutdown();
    }
}

