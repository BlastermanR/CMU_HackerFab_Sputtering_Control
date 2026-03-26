/**
 * AlicatMFC.cpp
 * @brief Implementation of the Alicat Mass Flow Controller (MFC).
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "AlicatMFC.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <stdio.h>
#include <sstream>

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
#ifdef DEBUG
        printf("Alicat Message Received: %s\n", msg.c_str());
#endif
        // Handle message here
        bool isValid = false;
        AlicatDataFrame frame;
        AlicatLib::parseResponse(msg, &frame, &isValid);

        if (isValid && frame.id == deviceId)
        {
            lastData = frame;
        }
    }
}

void AlicatMFC::sendCommand(const AlicatCommand& cmd)
{
    bool valid = false;
    std::string formattedStr = AlicatLib::formatCommand(&cmd, &valid);
    
    if (valid)
    {
        sendMessage(formattedStr.c_str());
    }
    else
    {
#ifdef DEBUG
        printf("Alicat: Error formatting command\n");
#endif
    }
}

void AlicatMFC::sendMessage(const char *message)
{
#ifdef DEBUG
    printf("Alicat: Sending Message: %s\n", message);
#endif
    serialPort->send(message);
}

void AlicatMFC::pollData()
{
    AlicatCommand cmd;
    cmd.id = deviceId;
    cmd.action = ALICAT_POLL;
    sendCommand(cmd);
}

void AlicatMFC::setSetpoint(double setpoint)
{
    AlicatCommand cmd;
    cmd.id = deviceId;
    cmd.action = ALICAT_CHANGE_SETPOINT;
    
    // Convert float to string. We could use std::to_string but it adds trailing zeros.
    // std::ostringstream provides cleaner output for normal uses.
    std::ostringstream ss;
    ss << setpoint;
    cmd.data = ss.str();
    
    sendCommand(cmd);
}

void AlicatMFC::setGas(int gasNumber)
{
    AlicatCommand cmd;
    cmd.id = deviceId;
    cmd.action = ALICAT_SET_GAS;
    cmd.data = std::to_string(gasNumber);
    sendCommand(cmd);
}

void AlicatMFC::tareFlow()
{
    AlicatCommand cmd;
    cmd.id = deviceId;
    cmd.action = ALICAT_TARE_FLOW;
    sendCommand(cmd);
}

const AlicatDataFrame& AlicatMFC::getLastData() const
{
    return lastData;
}

double AlicatMFC::getMassFlow() const
{
    return lastData.massFlow;
}

double AlicatMFC::getVolumetricFlow() const
{
    return lastData.volumetricFlow;
}

double AlicatMFC::getPressure() const
{
    return lastData.pressure;
}

double AlicatMFC::getTemperature() const
{
    return lastData.temperature;
}

double AlicatMFC::getSetpoint() const
{
    return lastData.setpoint;
}

std::string AlicatMFC::getGasType() const
{
    return lastData.gasType;
}