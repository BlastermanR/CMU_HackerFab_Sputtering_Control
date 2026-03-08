/**
 * Implements the Alicat Mass Flow Controller (MFC).
 * - Utilizes UART --> RS232 converter (MAX3232)
 * - Interrupt driven reciever handler
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "AlicatMFC.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"

AlicatMFC::AlicatMFC(IUart* uart) : serialPort(uart)
{
    bufferIndex = 0; // Set recieve buffer index to zero
}

AlicatMFC::~AlicatMFC()
{
    // Intentionally Empty
}

void AlicatMFC::init()
{
    serialPort->setCallback(std::bind(&AlicatMFC::onDataReceived, this, std::placeholders::_1));
    serialPort->begin();
    return;
}

void AlicatMFC::onDataReceived(char c) 
{
    if (bufferIndex < BUFFER_SIZE)
    {
        recieveBuffer[bufferIndex] = c;

        // Check if the character just added is a line terminator
        if (c == '\r' || c == '\n') 
        {
            recieveBuffer[bufferIndex] = '\0';
            printf("Alicat Message Received: %s\n", recieveBuffer);
            bufferIndex = 0;
        } else 
        {
            bufferIndex++; // Move to next slot
        }
    }
    else
    {
        // Handle overflow: buffer is full without finding a newline
        recieveBuffer[BUFFER_SIZE] = '\0'; 
        printf("Buffer Full: %s\n", recieveBuffer);
        bufferIndex = 0;
    }
}

void AlicatMFC::sendMessage(const char* message)
{
    printf("Alicat: Sending Message: %s\n", message);
    serialPort->print(message);
}