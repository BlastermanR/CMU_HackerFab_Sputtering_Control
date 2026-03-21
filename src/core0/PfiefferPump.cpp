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
#include <stdio.h>

PfiefferPump::PfiefferPump(ISerialDevice *dev) : serialPort(dev) {}

PfiefferPump::~PfiefferPump() {}

void PfiefferPump::init() { serialPort->begin(); }

void PfiefferPump::update()
{
    while (serialPort->hasMessage())
    {
        std::string response = serialPort->popMessage();

        // Convert the received buffer to a response reading if we have a complete
        // line
        if (response.empty())
        {
            printf("Error: Received empty response from Pfieffer Pump.\n");
            continue;
        }

#ifdef DEBUG
        printf("PfiefferPump received message: %s\n", response.c_str());
#endif

        // Handle message here
    }
}

void PfiefferPump::sendMessage(const char *message)
{
#ifdef DEBUG
    printf("Vacuum Pump: Sending Message: %s\n", message);
#endif
    serialPort->send(message);
}
