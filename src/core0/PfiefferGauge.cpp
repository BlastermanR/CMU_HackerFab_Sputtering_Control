/**
 * PfiefferGauge.cpp
 * @brief Implementation of the Pfeiffer pressure gauge communication.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include "PfiefferGauge.h"
#include "USBSerial.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "math.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"
#include <stdio.h>
#include <string>

PfiefferGauge::PfiefferGauge(ISerialDevice *dev) : serialPort(dev), gaugeDef(PFIEFFER_GAUGE_ADDRESS) {}

PfiefferGauge::~PfiefferGauge()
{
    // Intentionally Empty
}

void PfiefferGauge::init() { serialPort->begin(); }

void PfiefferGauge::update()
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

        // Convert the received buffer to a pressure reading if we have a complete
        // line
        if (response.empty())
        {
            printf("Error: Received empty response from Pfieffer Gauge.\n");
            continue;
        }

        { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Gauge RX: %s", response.c_str()); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }

        bool            valid = false;
        PfiefferCommand command;
        PfieifferLib::decryptResponse(response, &command, &valid);

        std::string pressureParamStr = std::to_string(static_cast<uint16_t>(Pfieffer::MPT200Cmd::Pressure));

        if (valid && command.paramNum == pressureParamStr && command.action == DATA_RESPONSE)
        {
            // Read Pressure response is param 740, data is: aaaabb where a is mantissa and b is exponent (10^b, offset
            // = 20)
            const unsigned int EXPECTED_DATA_LENGTH = 6;
            if (command.data.size() >= EXPECTED_DATA_LENGTH)
            {
                const unsigned int EXPONENT_BIAS = 20;

                // Use standard string to numeric conversion functions compatible with -fno-exceptions
                char              *endPtr;
                const unsigned int mantissa = std::strtoul(command.data.substr(0, 4).c_str(), &endPtr, 10);

                if (endPtr != command.data.substr(0, 4).c_str())
                {
                    const int exponent = std::strtol(command.data.substr(4, 2).c_str(), &endPtr, 10) - EXPONENT_BIAS;

                    chamberPressure_hPa = static_cast<double>(mantissa) * pow(10, exponent);

                    { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Gauge pressure: %.4e hPa", chamberPressure_hPa); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }
                }
                else
                {
                    { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Gauge: Bad pressure data: %s", command.data.c_str()); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }
                }
            }
        }
    }
}

void PfiefferGauge::pollDevice()
{
    PfiefferCommand cmd;

    // Address defaults to "001"
    char addrStr[4];
    snprintf(addrStr, sizeof(addrStr), "%03d", gaugeDef.getAddress());
    cmd.address = addrStr;

    // Action Request value
    cmd.action = READ_PARAMETER;

    // Request Parameter 740 (Pressure)
    cmd.paramNum = std::to_string(static_cast<uint16_t>(Pfieffer::MPT200Cmd::Pressure));
    cmd.data     = QUERY_DATA_STR;

    bool        valid        = false;
    std::string formattedCmd = PfieifferLib::formatCommand(&cmd, &valid);

    if (valid)
    {
        sendMessage(formattedCmd.c_str());
    }
}

void PfiefferGauge::sendMessage(const char *message)
{
    { char _dbg[OUTPUT_MSG_TEXT_LEN]; snprintf(_dbg, sizeof(_dbg), "Gauge TX: %s", message); USBSerial::log(Source_Core0, _dbg, V_DEBUG); }
    serialPort->send(message);
}