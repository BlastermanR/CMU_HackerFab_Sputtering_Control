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

PfiefferPump::PfiefferPump(IUart* uart) : serialPort(uart)
{
    // bufferIndex initialized by IDevice base class
}

PfiefferPump::~PfiefferPump()
{
    // Intentionally Empty
}

void PfiefferPump::init()
{
    serialPort->setCallback(std::bind(&IDevice::onDataReceived, this, std::placeholders::_1));
    serialPort->begin();
    return;
}

void PfiefferPump::printRecieved()
{
    printf("Vacuum Pump: Message Received: %s\n", recieveBuffer);
}

void PfiefferPump::update()
{
    // Routine checks or background tasks can go here
}

void PfiefferPump::sendMessage(const char* message)
{
    printf("Vacuum Pump: Sending Message: %s\n", message);
    serialPort->print(message);
}
