/**
 * PfiefferPump.cpp
 * @brief Implementation of the Pfeiffer vacuum pump communication.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include "PfiefferPump.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include "Debug.h"
#include <stdio.h>
#include <cstdlib>

void PfiefferPump::sendMessage(const char *message)
{
    DEBUG_PRINT("Vacuum Pump: Sending Message: %s\n", message);
    serialPort->send(message);
}

PfiefferPump::PfiefferPump(ISerialDevice *dev) : serialPort(dev), pumpDef(PFIEFFER_PUMP_ADDRESS) {}

PfiefferPump::~PfiefferPump() {}

void PfiefferPump::init() { serialPort->begin(); }

void PfiefferPump::update()
{
    // 1. Time to poll?
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if (currentTime - lastPollTime >= pollingInterval_ms) {
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

        DEBUG_PRINT("PfiefferPump received message: %s\n", response.c_str());

        bool valid = false;
        PfiefferCommand command;
        PfieifferLib::decryptResponse(response, &command, &valid);

        std::string speedHzParamStr = std::to_string(static_cast<uint16_t>(Pfieffer::TC110Cmd::ActualSpd_Hz));

        if (valid && command.action == DATA_RESPONSE) 
        {
            if (command.paramNum == speedHzParamStr) {
                char* endPtr;
                double speed = std::strtod(command.data.c_str(), &endPtr);
                if (endPtr != command.data.c_str()) {
                    actualPumpSpeed_hz = speed;
                    DEBUG_PRINT("PfiefferPump: Parsed Speed Reading: %f Hz\n", actualPumpSpeed_hz);
                }
            }
        }
    }
}

void PfiefferPump::pollDevice()
{
    PfiefferCommand cmd;
    
    if (pumpDef.createReadCommand(static_cast<uint16_t>(Pfieffer::TC110Cmd::ActualSpd_Hz), cmd)) {
        bool valid = false;
        std::string formattedCmd = PfieifferLib::formatCommand(&cmd, &valid);
        
        if (valid) {
            sendMessage(formattedCmd.c_str());
        }
    }
}

void PfiefferPump::signalPumpOn()
{
    PfiefferCommand cmd;
    if (pumpDef.createWriteCommand(static_cast<uint16_t>(Pfieffer::TC110Cmd::PumpgStatn), 1, cmd)) {
        bool valid = false;
        std::string formattedCmd = PfieifferLib::formatCommand(&cmd, &valid);
        if (valid) sendMessage(formattedCmd.c_str());
    }
}

bool PfiefferPump::activatePump()
{
    signalPumpOn();
    // Implementation for blocking wait could be added here if needed,
    // assuming non-blocking returns true immediately for now.
    pumpActivated = true;
    return true;
}

bool PfiefferPump::signalPumpOff()
{
    PfiefferCommand cmd;
    if (pumpDef.createWriteCommand(static_cast<uint16_t>(Pfieffer::TC110Cmd::PumpgStatn), 0, cmd)) {
        bool valid = false;
        std::string formattedCmd = PfieifferLib::formatCommand(&cmd, &valid);
        if (valid) {
            sendMessage(formattedCmd.c_str());
            return true;
        }
    }
    return false;
}

bool PfiefferPump::deactivatePump()
{
    bool success = signalPumpOff();
    if (success) {
        pumpActivated = false;
    }
    return success;
}

bool PfiefferPump::ventPump()
{
    PfiefferCommand cmd;
    if (pumpDef.createWriteCommand(static_cast<uint16_t>(Pfieffer::TC110Cmd::EnableVent), 1, cmd)) {
        bool valid = false;
        std::string formattedCmd = PfieifferLib::formatCommand(&cmd, &valid);
        if (valid) {
            sendMessage(formattedCmd.c_str());
            return true;
        }
    }
    return false;
}
