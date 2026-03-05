
#include "AlicatMFC.h"
#include "pico/stdlib.h"
#include "picoDefinitions.h"

AlicatMFC::AlicatMFC(IUart* uart) : serialPort(uart)
{
    // Intentionally Empty
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

void AlicatMFC::sendMessage(const char* message)
{
    serialPort->print(message);
}