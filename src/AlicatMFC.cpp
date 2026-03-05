
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
            printf("MessageReceived: %s\n", recieveBuffer);
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
    
    // For debug, Print every character
    // TODO Remove
    printf("CharRecieved: %c\n", c);
}

void AlicatMFC::sendMessage(const char* message)
{
    serialPort->print(message);
}