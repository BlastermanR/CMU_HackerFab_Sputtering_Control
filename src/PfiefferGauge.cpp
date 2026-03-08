/**
 * TODO
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/7/26
 */

#include <stdio.h>
#include "PfiefferGauge.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"

PfiefferGauge::PfiefferGauge(IUart* uart) : serialPort(uart)
{
    // bufferIndex initialized by IDevice base class
}

PfiefferGauge::~PfiefferGauge()
{
    // Intentionally Empty
}

void PfiefferGauge::init()
{
    serialPort->setCallback(std::bind(&IDevice::onDataReceived, this, std::placeholders::_1));
    serialPort->begin();
}

void PfiefferGauge::printRecieved()
{
    printf("Pressure Gauge: Message Received: %s\n", recieveBuffer);
}

void PfiefferGauge::update()
{
    // Routine checks or background tasks can go here
}

void PfiefferGauge::sendMessage(const char* message)
{
#ifdef DEBUG
    printf("Pressure Gauge: Sending Message: %s\n", message);
#endif
    serialPort->print(message);
}