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
#include "SputteringManager.h"
#include "USBSerial.h"
#include <cstring>

/**
 * @brief Helper to push a formatted string to the Core 1 output queue.
 */
static void core1Print(const char *text)
{
    OutputMessage msg{};
    msg.source = Source_Core1;
    strncpy(msg.text, text, OUTPUT_MSG_TEXT_LEN - 1);
    msg.text[OUTPUT_MSG_TEXT_LEN - 1] = '\0';
    queue_try_add(&core1OutQueue, &msg);
}

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
            core1Print("Starting sputtering process");
            break;
        case Cmd_StopProcess:
            clearStatus(ExecuteSputteringProcess);
            core1Print("Stopping sputtering process");
            break;
        case Cmd_PressurizeChamber:
            setStatus(PressurizeChamber);
            core1Print("Pressurizing chamber");
            break;
        case Cmd_VentChamber:
            setStatus(VentChamber);
            core1Print("Venting chamber");
            break;
        case Cmd_ShutOffGas:
            setStatus(ShutOffGasFlow);
            core1Print("Shutting off gas flow");
            break;
        case Cmd_PollDevices:
            setStatus(PollDevices);
            core1Print("Polling devices");
            break;
        case Cmd_SetArgonFlow:
            sharedData.Core1Out.setArgonFlow = cmd.param1;
            core1Print("Argon flow setpoint updated");
            break;
        case Cmd_SetOxygenFlow:
            sharedData.Core1Out.setOxygenFlow = cmd.param1;
            core1Print("Oxygen flow setpoint updated");
            break;
        case Cmd_SetPumpSpeed:
            sharedData.Core1Out.setPumpSpeed = cmd.param1;
            core1Print("Pump speed setpoint updated");
            break;
        case Cmd_EnablePump:
            sharedData.Core1Out.enablePump = true;
            core1Print("Pump enabled");
            break;
        case Cmd_DisablePump:
            sharedData.Core1Out.enablePump = false;
            core1Print("Pump disabled");
            break;
        case Cmd_Exit:
            setStatus(Status_Exit);
            core1Print("Exit requested");
            break;
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

    SputteringManager manager;

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

        sleep_ms(10);
    }
}
