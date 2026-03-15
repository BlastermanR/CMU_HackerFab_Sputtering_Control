/**
 * TODO
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include <stdio.h>
#include "PfiefferGauge.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "picoDefinitions.h"
#include "math.h"

PfiefferGauge::PfiefferGauge(ISerialDevice* dev) : serialPort(dev)
{
}

PfiefferGauge::~PfiefferGauge()
{
    // Intentionally Empty
}

void PfiefferGauge::init()
{
    serialPort->begin();
}

void PfiefferGauge::update()
{
    while (serialPort->hasMessage()) {
        std::string response = serialPort->popMessage();

        // Convert the received buffer to a pressure reading if we have a complete line
        if (response.empty()) 
        {
            printf("Error: Received empty response from Pfieffer Gauge.\n");
            continue;
        }
        
        /*
        // Decrypt the response using the PfieifferLib to extract the command fields
        PfiefferCommand command;
        PfieifferLib::decryptResponse(response, &command);

        // Temporary conversion of data field to double for pressure reading
        PfieifferLib::printCommand(&command); // debug print the parsed command

        // Read Preassure response is param 740, data is: aaaabb where a is mantissa and 
        // b is exponent (10^b, offset = 20)
        // Also data is 6
        const unsigned int EXPECTED_DATA_LENGTH = 6;
        const unsigned int EXPONENT_BIAS = 20;
        const unsigned int mantissa = std::stoi(command.data.substr(0, 4));
        const unsigned int exponent = std::stoi(command.data.substr(4, 2)) - EXPONENT_BIAS;
        chamberPressure_hPa = static_cast<double>(mantissa) * pow(10, exponent);
        printf("PfiefferGauge: Parsed Pressure Reading: %f hPa\n", chamberPressure_hPa);
        
        // State machine logic vars will be set on command type
        */
    }
}

void PfiefferGauge::sendMessage(const char* message)
{
#ifdef DEBUG
    printf("PfiefferGauge: Sending Message: %s\n", message);
#endif
    serialPort->send(message);
}