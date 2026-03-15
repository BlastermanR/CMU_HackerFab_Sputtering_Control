/**
 * TODO
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include <stdio.h>
#include "PfiefferPump.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"

PfiefferPump::PfiefferPump(ISerialDevice* dev) : serialPort(dev)
{
}

PfiefferPump::~PfiefferPump()
{
}

void PfiefferPump::init()
{
    serialPort->begin();
}

void PfiefferPump::update()
{
    while (serialPort->hasMessage()) {
        std::string msg = serialPort->popMessage();
        #ifdef DEBUG
        printf("Vacuum Pump: Message Received: %s\n", msg.c_str());
        #endif
        // Handle message here
    }
}

void PfiefferPump::sendMessage(const char* message)
{
#ifdef DEBUG
    printf("Vacuum Pump: Sending Message: %s\n", message);
#endif
    serialPort->send(message);
}
