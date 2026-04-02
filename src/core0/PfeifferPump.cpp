/**
 * PfeifferPump.cpp
 * @brief Implementation of the Pfeiffer vacuum pump communication.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include "PfeifferPump.h"
#include "USBSerial.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <cstdlib>
#include <stdio.h>

void PfeifferPump::sendMessage(const char *message)
{
    { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Pump TX: %s", message); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }
    serialPort->send(message);
}

PfeifferPump::PfeifferPump(ISerialDevice *dev) : serialPort(dev), pumpDef(PFEIFFER_PUMP_ADDRESS) {}

PfeifferPump::~PfeifferPump() {}

void PfeifferPump::init() { serialPort->begin(); }

void PfeifferPump::update()
{
    // 1. Time to poll?
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if (currentTime - lastPollTime >= pollingInterval_ms)
    {
        lastPollTime = currentTime;
        pollDevice();
    }

    // 2. Process incoming serial data
    while (serialPort->hasMessage())
    {
        std::string response = serialPort->popMessage();

        if (response.empty())
        {
            continue;
        }

        { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Pump RX: %s", response.c_str()); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }

        bool            valid = false;
        PfeifferCommand command;
        PfeifferLib::decryptResponse(response, &command, &valid);

        std::string speedHzParamStr = std::to_string(static_cast<uint16_t>(Pfeiffer::TC110Cmd::ActualSpd_Hz));

        if (valid && command.action == DATA_RESPONSE)
        {
            if (command.paramNum == speedHzParamStr)
            {
                char  *endPtr;
                double speed = std::strtod(command.data.c_str(), &endPtr);
                if (endPtr != command.data.c_str())
                {
                    actualPumpSpeed_hz = speed;
                    { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Pump speed: %.2f Hz", actualPumpSpeed_hz); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }
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
