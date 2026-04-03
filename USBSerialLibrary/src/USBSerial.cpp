#include "USBSerial.h"
#include "Intercore.h"
#include "pico/stdlib.h"
#include <cstdlib>
#include <cstring>
#include <stdio.h>

USBSerial::USBSerial() : inputBuffer("") {}

void USBSerial::begin()
{
    sleep_ms(2000);
    printf("USB Serial Interface Initialized.\n");
}

// ---------- Static cross-core API ----------

void USBSerial::log(MessageSource source, const char *text, Verbosity level)
{
    OutputMessage msg{};
    msg.timestamp = to_ms_since_boot(get_absolute_time());
    msg.source    = source;
    msg.level     = level;
    msg.type      = Msg_Log;
    strncpy(msg.text, text, OUTPUT_MSG_TEXT_LEN - 1);
    msg.text[OUTPUT_MSG_TEXT_LEN - 1] = '\0';

    queue_t *q = (source == Source_Core0) ? &core0OutQueue : &core1OutQueue;
    queue_try_add(q, &msg);
}

static const char *dataIdToString(DataId id)
{
    switch (id)
    {
    case Data_PumpSpeed:
        return "PumpSpeed";
    case Data_ChamberPressure:
        return "Pressure";
    case Data_ArgonFlow:
        return "ArgonFlow";
    case Data_OxygenFlow:
        return "OxygenFlow";
    default:
        return "Unknown";
    }
}

void USBSerial::sendData(MessageSource source, DataId id, float value)
{
    OutputMessage msg{};
    msg.timestamp  = to_ms_since_boot(get_absolute_time());
    msg.source     = source;
    msg.level      = V_STATUS;
    msg.type       = Msg_Data;
    msg.data.id    = id;
    msg.data.value = value;

    queue_t *q = (source == Source_Core0) ? &core0OutQueue : &core1OutQueue;
    queue_try_add(q, &msg);
}

// ---------- Instance methods (Core 1 only) ----------

void USBSerial::readInput()
{
    while (true)
    {
        int c = getchar_timeout_us(0);

        if (c == PICO_ERROR_TIMEOUT)
        {
            break;
        }

        if (c == '\n' || c == '\r')
        {
            if (!inputBuffer.empty())
            {
                // Echo the command back
                printf("> %s\n", inputBuffer.c_str());

                CommandMessage msg{};
                if (parseCommand(inputBuffer, msg))
                {
                    queue_try_add(&commandQueue, &msg);
                }
                else
                {
                    printf("Unknown command: %s\n", inputBuffer.c_str());
                }

                inputBuffer.clear();
            }
        }
        else if (c == '\b' || c == 127) // Handle backspace
        {
            if (!inputBuffer.empty())
            {
                inputBuffer.pop_back();
            }
        }
        else if (c >= 32 && c <= 126)
        {
            inputBuffer += (char)c;
        }
    }
}

void USBSerial::drainOutputQueues()
{
    OutputMessage msg0, msg1;
    bool          hasMsg0, hasMsg1;
    uint8_t       currentVerbosity = verbosityLevel.load(std::memory_order_acquire);

    while (true)
    {
        hasMsg0 = queue_try_peek(&core0OutQueue, &msg0);
        hasMsg1 = queue_try_peek(&core1OutQueue, &msg1);

        if (!hasMsg0 && !hasMsg1)
        {
            break;
        }

        OutputMessage *msgToPrint = nullptr;

        if (hasMsg0 && hasMsg1)
        {
            if (msg0.timestamp <= msg1.timestamp)
            {
                queue_try_remove(&core0OutQueue, &msg0);
                msgToPrint = &msg0;
            }
            else
            {
                queue_try_remove(&core1OutQueue, &msg1);
                msgToPrint = &msg1;
            }
        }
        else if (hasMsg0)
        {
            queue_try_remove(&core0OutQueue, &msg0);
            msgToPrint = &msg0;
        }
        else
        {
            queue_try_remove(&core1OutQueue, &msg1);
            msgToPrint = &msg1;
        }

        if (msgToPrint->type == Msg_Data || msgToPrint->level <= currentVerbosity)
        {
            if (msgToPrint->type == Msg_Data)
                printf("[%lu] $%s:%.4f\n", (unsigned long)msgToPrint->timestamp, dataIdToString(msgToPrint->data.id),
                       msgToPrint->data.value);
            else
                printf("[%lu] [Core%u] %s\n", (unsigned long)msgToPrint->timestamp, msgToPrint->source,
                       msgToPrint->text);
        }
    }
}

bool USBSerial::parseCommand(const std::string &input, CommandMessage &msg)
{
    // Echo the exact string being parsed for debugging
    char debugBuf[128];
    snprintf(debugBuf, sizeof(debugBuf), "DEBUG parseCommand received: '%s'", input.c_str());
    USBSerial::log(Source_Core1, debugBuf, V_DEBUG);

    // Simple prefix-based command parsing
    // Commands are case-sensitive, space-separated: COMMAND [param1] [param2]

    if (input == "START")
    {
        msg.id = Cmd_StartProcess;
        return true;
    }
    if (input == "STOP")
    {
        msg.id = Cmd_StopProcess;
        return true;
    }
    if (input == "PRESSURIZE")
    {
        msg.id = Cmd_PressurizeChamber;
        return true;
    }
    if (input == "VENT")
    {
        msg.id = Cmd_VentChamber;
        return true;
    }
    if (input == "GASOFF")
    {
        msg.id = Cmd_ShutOffGas;
        return true;
    }
    if (input == "POLL")
    {
        msg.id = Cmd_PollDevices;
        return true;
    }
    if (input == "POLLARGON" || input == "POLL ARGON")
    {
        msg.id = Cmd_PollArgon;
        return true;
    }
    if (input == "POLLOXYGEN" || input == "POLL OXYGEN")
    {
        msg.id = Cmd_PollOxygen;
        return true;
    }
    if (input == "POLLPUMP" || input == "POLL PUMP")
    {
        msg.id = Cmd_PollPump;
        return true;
    }
    if (input == "POLLGAUGE" || input == "POLL GAUGE")
    {
        msg.id = Cmd_PollGauge;
        return true;
    }
    if (input == "EXIT")
    {
        msg.id = Cmd_Exit;
        return true;
    }
    if (input == "PUMPON")
    {
        msg.id = Cmd_EnablePump;
        return true;
    }
    if (input == "PUMPOFF")
    {
        msg.id = Cmd_DisablePump;
        return true;
    }

    // Parameterized commands: "SETARGON <flow>" / "SETOXYGEN <flow>" / "SETPUMP <speed>"
    if (input.rfind("SETARGON ", 0) == 0)
    {
        msg.id     = Cmd_SetArgonFlow;
        msg.param1 = (float)atof(input.c_str() + 9);
        return true;
    }
    if (input.rfind("SETOXYGEN ", 0) == 0)
    {
        msg.id     = Cmd_SetOxygenFlow;
        msg.param1 = (float)atof(input.c_str() + 10);
        return true;
    }
    if (input.rfind("SETPUMP ", 0) == 0)
    {
        msg.id     = Cmd_SetPumpSpeed;
        msg.param1 = (float)atof(input.c_str() + 8);
        return true;
    }
    if (input.rfind("VERBOSE ", 0) == 0)
    {
        msg.id     = Cmd_SetVerbosity;
        msg.param1 = (float)atof(input.c_str() + 8);
        return true;
    }

    return false;
}