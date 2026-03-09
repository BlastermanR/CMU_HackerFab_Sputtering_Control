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
    // Set the callback
    serialPort->setCallback(std::bind(&IDevice::onDataReceived, this, std::placeholders::_1));

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
   serialPort->print("\r");
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
#ifdef DEBUG
    printf("Alicat: Sending Message: %s\n", message);
#endif
    serialPort->print(message);
}