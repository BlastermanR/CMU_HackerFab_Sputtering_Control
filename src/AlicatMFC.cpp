/**
 * Implements the Alicat Mass Flow Controller (MFC).
 * - Utilizes UART --> RS232 converter (MAX3232)
 * - Interrupt driven reciever handler
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <stdio.h>
#include "AlicatMFC.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"

AlicatMFC::AlicatMFC(IUart* uart) : serialPort(uart)
{
    // bufferIndex initialized by IDevice base class
}

AlicatMFC::~AlicatMFC()
{
    // Intentionally Empty
}

void AlicatMFC::init()
{
    serialPort->setCallback(std::bind(&IDevice::onDataReceived, this, std::placeholders::_1));
    serialPort->begin();
    return;
}

void AlicatMFC::printRecieved()
{
    printf("Alicat Message Received: %s\n", recieveBuffer);
}

void AlicatMFC::update()
{
    // Routine checks or background tasks can go here
}

void AlicatMFC::sendMessage(const char* message)
{
    printf("Alicat: Sending Message: %s\n", message);
    serialPort->print(message);
}