/**
 * AlicatMFC.cpp
 * @brief Implementation of the Alicat Mass Flow Controller (MFC).
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "AlicatMFC.h"
#include "USBSerial.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <sstream>
#include <stdio.h>

AlicatMFC::AlicatMFC(ISerialDevice *dev, char id) : serialPort(dev), deviceId(id) {}

AlicatMFC::~AlicatMFC() {}

void AlicatMFC::init()
{
    // Start the UART communication
    serialPort->begin();

    /**
     * Clear buffer to ensure no garbage data from hardware startup
     * is poluting the Alicat Message Space. this is a common issue with
     * the MAX3232 where it can send random bytes on powerup. Sending a
     * carriage return is a simple way to trigger the buffer to clear
     * if any garbage data is present. The callback will handle clearing
     * the buffer and printing any garbage data if DEBUG is enabled.
     */
    serialPort->send("\r");
}

void AlicatMFC::update()
{
    while (serialPort->hasMessage())
    {
        std::string msg = serialPort->popMessage();

        {
            char _dbg[OUTPUT_MSG_TEXT_LEN];
            snprintf(_dbg, sizeof(_dbg), "MFC RX: %s", msg.c_str());
            USBSerial::log(Source_Core0, _dbg, V_DEBUG);
        }

        // Handle message here
        bool            isValid = false;
        AlicatDataFrame frame;
        AlicatLib::parseResponse(msg, &frame, &isValid);

        if (isValid && frame.id == deviceId)
        {
            lastData = frame;

            // Log any status/error codes present in the frame
            for (const std::string &code : frame.statusCodes)
            {
                char _dbg[OUTPUT_MSG_TEXT_LEN];
                snprintf(_dbg, sizeof(_dbg), "MFC status: %s - %s", code.c_str(),
                         AlicatLib::getStatusDescription(code.c_str()));
                USBSerial::log(Source_Core0, _dbg, V_STATUS);
            }
        }
    }
}

void AlicatMFC::sendCommand(const AlicatCommand &cmd)
{
    bool        valid        = false;
    std::string formattedStr = AlicatLib::formatCommand(&cmd, &valid);

    if (valid)
    {
        sendMessage(formattedStr.c_str());
    }
    else
    {
        USBSerial::log(Source_Core0, "MFC: Error formatting command", V_DEBUG);
    }
}

void AlicatMFC::sendMessage(const char *message)
{
    {
        char _dbg[OUTPUT_MSG_TEXT_LEN];
        snprintf(_dbg, sizeof(_dbg), "MFC TX: %s", message);
        USBSerial::log(Source_Core0, _dbg, V_DEBUG);
    }
    serialPort->send(message);
}

void AlicatMFC::pollData()
{
    AlicatCommand cmd;
    cmd.id     = deviceId;
    cmd.action = ALICAT_POLL;
    sendCommand(cmd);
}

void AlicatMFC::setSetpoint(double setpoint)
{
    AlicatCommand cmd;
    cmd.id     = deviceId;
    cmd.action = ALICAT_CHANGE_SETPOINT;

    // Convert float to string. We could use std::to_string but it adds trailing zeros.
    // std::ostringstream provides cleaner output for normal uses.
    std::ostringstream ss;
    ss << setpoint;
    cmd.data = ss.str();

    sendCommand(cmd);
}

void AlicatMFC::setGas(uint8_t gasId)
{
    if (!AlicatLib::isValidGasId(gasId))
    {
        char _dbg[OUTPUT_MSG_TEXT_LEN];
        snprintf(_dbg, sizeof(_dbg), "MFC: Unknown gas ID %u", static_cast<unsigned>(gasId));
        USBSerial::log(Source_Core0, _dbg, V_STATUS);
        return;
    }

    {
        char _dbg[OUTPUT_MSG_TEXT_LEN];
        snprintf(_dbg, sizeof(_dbg), "MFC: Setting gas %u - %s (%s)", static_cast<unsigned>(gasId),
                 AlicatLib::getGasShortName(gasId), AlicatLib::getGasLongName(gasId));
        USBSerial::log(Source_Core0, _dbg, V_STATUS);
    }

    AlicatCommand cmd;
    cmd.id     = deviceId;
    cmd.action = ALICAT_SET_GAS;
    cmd.data   = std::to_string(gasId);
    sendCommand(cmd);
}

void AlicatMFC::tareFlow()
{
    AlicatCommand cmd;
    cmd.id     = deviceId;
    cmd.action = ALICAT_TARE_FLOW;
    sendCommand(cmd);
}

const AlicatDataFrame &AlicatMFC::getLastData() const { return lastData; }

double AlicatMFC::getMassFlow() const { return lastData.massFlow; }

double AlicatMFC::getVolumetricFlow() const { return lastData.volumetricFlow; }

double AlicatMFC::getPressure() const { return lastData.pressure; }

double AlicatMFC::getTemperature() const { return lastData.temperature; }

double AlicatMFC::getSetpoint() const { return lastData.setpoint; }

std::string AlicatMFC::getGasType() const { return lastData.gasType; }

const std::vector<std::string> &AlicatMFC::getStatusCodes() const { return lastData.statusCodes; }