/**
 * @file PfeifferPump.cpp
 * @brief Implementation of the Pfeiffer vacuum pump communication.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include "PfeifferPump.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <cstdio>
#include <cstdlib>

static constexpr int kDbgBufLen = 64;

void PfeifferPump::sendMessage(const char *message)
{
    {
        char _dbg[kDbgBufLen];
        snprintf(_dbg, sizeof(_dbg), "Pump TX: %s", message);
        printf("%s\n", _dbg);
    }
    serialPort->send(message);
}

PfeifferPump::PfeifferPump(ISerialDevice *dev) : serialPort(dev), pumpDef(PFEIFFER_PUMP_ADDRESS) {}

PfeifferPump::~PfeifferPump() {}

void PfeifferPump::init() { serialPort->begin(); }

void PfeifferPump::update()
{
    // 1. Check if it's time to poll the device based on the polling interval
    /*
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if (currentTime - lastPollTime >= pollingInterval_ms)
    {
        lastPollTime = currentTime;
        pollDevice();
    }
    */

    // 2. Process all messages currently in the serial device's incoming buffer
    while (serialPort->hasMessage())
    {
        std::string response = serialPort->popMessage();

        if (response.empty())
        {
            continue;
        }

        {
            char _dbg[kDbgBufLen];
            snprintf(_dbg, sizeof(_dbg), "Pump RX: %s", response.c_str());
            printf("%s\n", _dbg);
        }

        // Parse and validate the Pfeiffer protocol frame
        bool            valid = false;
        PfeifferCommand command;
        PfeifferLib::decryptResponse(response, &command, &valid);

        if (!valid)
        {
            char _dbg[kDbgBufLen];
            snprintf(_dbg, sizeof(_dbg), "Pump RX parse failed: %s", response.c_str());
            printf("%s\n", _dbg);
            continue;
        }

        // Log error responses from the pump (e.g., _RANGE, _LOGIC, NO_DEF)
        if (command.action == ERROR_RESPONSE || (command.action == DATA_RESPONSE && command.data.size() == 6 &&
            (command.data == "NO_DEF" || command.data == "_RANGE" || command.data == "_LOGIC")))
        {
            char _dbg[kDbgBufLen];
            snprintf(_dbg, sizeof(_dbg), "Pump RX error (param %s): %s", command.paramNum.c_str(), command.data.c_str());
            printf("%s\n", _dbg);
            continue;
        }

        // Define the parameter ID for actual speed (Hz) for comparison
        std::string speedHzParamStr = std::to_string(static_cast<uint16_t>(Pfeiffer::TC110Cmd::ActualSpd_Hz));

        // If the response is valid and contains data, check if it's the speed parameter we requested
        if (valid && command.action == DATA_RESPONSE)
        {
            if (command.paramNum == speedHzParamStr)
            {
                char  *endPtr;
                double speed = std::strtod(command.data.c_str(), &endPtr);
                
                // If conversion was successful, update the internal state and set the new data flag
                if (endPtr != command.data.c_str())
                {
                    actualPumpSpeed_hz = speed;
                    newDataFlag = true;
                    {
                        char _dbg[kDbgBufLen];
                        snprintf(_dbg, sizeof(_dbg), "Pump speed: %.2f Hz", actualPumpSpeed_hz);
                        printf("%s\n", _dbg);
                    }
                }
            }
        }
    }
}

void PfeifferPump::pollDevice()
{
    PfeifferCommand cmd;

    if (pumpDef.createReadCommand(static_cast<uint16_t>(Pfeiffer::TC110Cmd::ActualSpd_Hz), cmd))
    {
        bool        valid        = false;
        std::string formattedCmd = PfeifferLib::formatCommand(&cmd, &valid);

        if (valid)
        {
            sendMessage(formattedCmd.c_str());
        }
    }
}

bool PfeifferPump::hasNewSpeedData()
{
    bool ret = newDataFlag;
    newDataFlag = false;
    return ret;
}

void PfeifferPump::signalPumpOn()
{
    PfeifferCommand cmd;
    if (pumpDef.createWriteCommand(static_cast<uint16_t>(Pfeiffer::TC110Cmd::PumpgStatn), 1, cmd))
    {
        bool        valid        = false;
        std::string formattedCmd = PfeifferLib::formatCommand(&cmd, &valid);
        if (valid)
            sendMessage(formattedCmd.c_str());
    }
}

bool PfeifferPump::activatePump()
{
    signalPumpOn();
    // Implementation for blocking wait could be added here if needed,
    // assuming non-blocking returns true immediately for now.
    pumpActivated = true;
    return true;
}

bool PfeifferPump::signalPumpOff()
{
    PfeifferCommand cmd;
    if (pumpDef.createWriteCommand(static_cast<uint16_t>(Pfeiffer::TC110Cmd::PumpgStatn), 0, cmd))
    {
        bool        valid        = false;
        std::string formattedCmd = PfeifferLib::formatCommand(&cmd, &valid);
        if (valid)
        {
            sendMessage(formattedCmd.c_str());
            return true;
        }
    }
    return false;
}

bool PfeifferPump::deactivatePump()
{
    bool success = signalPumpOff();
    if (success)
    {
        pumpActivated = false;
    }
    return success;
}

bool PfeifferPump::ventPump()
{
    PfeifferCommand cmd;
    if (pumpDef.createWriteCommand(static_cast<uint16_t>(Pfeiffer::TC110Cmd::EnableVent), 1, cmd))
    {
        bool        valid        = false;
        std::string formattedCmd = PfeifferLib::formatCommand(&cmd, &valid);
        if (valid)
        {
            sendMessage(formattedCmd.c_str());
            return true;
        }
    }
    return false;
}
