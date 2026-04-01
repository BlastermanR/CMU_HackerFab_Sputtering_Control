/**
 * @file MockUart.h
 * @brief GMock mock for the IUart interface.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#ifndef MOCK_UART_H
#define MOCK_UART_H

#include "UARTInterface.h"
#include <gmock/gmock.h>

class MockUart : public IUart
{
  public:
    MOCK_METHOD(void, begin, (), (override));
    MOCK_METHOD(void, write, (char c), (override));
    MOCK_METHOD(void, print, (const char *str), (override));
    MOCK_METHOD(void, waitTxComplete, (), (override));
    MOCK_METHOD(void, setCallback, (UartCallback cb), (override));
};

#endif // MOCK_UART_H
