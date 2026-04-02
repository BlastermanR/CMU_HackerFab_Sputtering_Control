/**
 * Core1Main.cpp
 * @brief Implementation of the main loop for Core 1.
 *
 * Core 1 owns the USB serial interface. It reads incoming commands,
 * dispatches them via the status register or shared data, and drains
 * the output queues from both cores to print to the terminal.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */

#include "Core1Main.h"
#include "Intercore.h"
#include "USBSerial.h"
#include <cstdio>
#include <cstring>

/**
 * @brief Dispatches a command message by setting the appropriate status
 *        register flags or writing to shared data.
 */
static void dispatchCommand(const CommandMessage &cmd)
{
    switch (cmd.id)
    {
        case Cmd_StartProcess:
            setStatus(ExecuteSputteringProcess);
            USBSerial::log(Source_Core1, "Starting sputtering process");
            break;
        case Cmd_StopProcess:
            clearStatus(ExecuteSputteringProcess);
            USBSerial::log(Source_Core1, "Stopping sputtering process");
            break;
        case Cmd_PressurizeChamber:
            setStatus(PressurizeChamber);
            USBSerial::log(Source_Core1, "Pressurizing chamber");
            break;
        case Cmd_VentChamber:
            setStatus(VentChamber);
            USBSerial::log(Source_Core1, "Venting chamber");
            break;
        case Cmd_ShutOffGas:
            setStatus(ShutOffGasFlow);
            USBSerial::log(Source_Core1, "Shutting off gas flow");
            break;
        case Cmd_PollDevices:
            setStatus(PollDevices);
            USBSerial::log(Source_Core1, "Polling devices");
            break;
        case Cmd_SetArgonFlow:
            sharedData.Core1Out.setArgonFlow = cmd.param1;
            setStatus(SetArgonFlow);
            USBSerial::log(Source_Core1, "Argon flow setpoint updated");
            break;
        case Cmd_SetOxygenFlow:
            sharedData.Core1Out.setOxygenFlow = cmd.param1;
            setStatus(SetOxygenFlow);
            USBSerial::log(Source_Core1, "Oxygen flow setpoint updated");
            break;
        case Cmd_SetPumpSpeed:
            sharedData.Core1Out.setPumpSpeed = cmd.param1;
            setStatus(SetPumpSpeed);
            USBSerial::log(Source_Core1, "Pump speed setpoint updated");
            break;
        case Cmd_EnablePump:
            sharedData.Core1Out.enablePump = true;
            setStatus(EnablePump);
            USBSerial::log(Source_Core1, "Pump enabled");
            break;
        case Cmd_DisablePump:
            sharedData.Core1Out.enablePump = false;
            setStatus(DisablePump);
            USBSerial::log(Source_Core1, "Pump disabled");
            break;
        case Cmd_Exit:
            setStatus(Status_Exit);
            USBSerial::log(Source_Core1, "Exit requested", V_CRITICAL);
            break;
        case Cmd_SetVerbosity:
        {
            uint8_t v = (uint8_t)cmd.param1;
            if (v > V_DEBUG) v = V_DEBUG;
            verbosityLevel.store(v, std::memory_order_release);
            char buf[OUTPUT_MSG_TEXT_LEN];
            snprintf(buf, sizeof(buf), "Verbosity set to %u", v);
            USBSerial::log(Source_Core1, buf, V_CRITICAL);
            break;
        }
        default:
            break;
    }
}

// Entry point for Core 1
void core1_entry()
{
    // Core 1 owns USB I/O
    USBSerial pcTerminal;
    pcTerminal.begin();
    pcTerminal.drainOutputQueues();

    // Wait for Core 0 to finish initialization
    {
        uint64_t handshakeStart = get_absolute_time();
        while (!getStatus(Core0_Begin))
        {
            if ((get_absolute_time() - handshakeStart) >= (uint64_t)HANDSHAKE_TIMEOUT_MS * 1000)
            {
                printf("Handshake Error: Core 0 handshake timeout!\n"); // Ensure Print to terminal
                setStatus(Status_Core0Err);
                return;
            }
            pcTerminal.drainOutputQueues(); // Keep the queue clear while waiting
            sleep_ms(1);
        }
    }

    // Signal Core 1 initialization complete
    setStatus(Core1_Begin);
    USBSerial::log(Source_Core1, "Core 1 initialized", V_INFO);
    USBSerial::log(Source_Core1, "Waiting for commands", V_INFO);

    bool run{true};

    while (run)
    {
        // Read USB input and push parsed commands to commandQueue
        pcTerminal.readInput();

        // Drain and dispatch all pending commands
        CommandMessage cmd;
        while (queue_try_remove(&commandQueue, &cmd))
        {
            dispatchCommand(cmd);
        }

        // Print queued output from both cores to the terminal
        pcTerminal.drainOutputQueues();

        // Check for exit
        run = !(isError() || getStatus(Status_Exit));

        if (!run)
        {
            USBSerial::log(Source_Core1, "Core 1 exiting main loop", V_STATUS);
            pcTerminal.drainOutputQueues();
        }

        sleep_ms(10);
    }
}
