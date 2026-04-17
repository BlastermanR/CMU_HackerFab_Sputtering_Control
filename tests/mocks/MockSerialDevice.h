/**
 * @file MockSerialDevice.h
 * @brief GMock mock for the ISerialDevice interface.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#ifndef MOCK_SERIAL_DEVICE_H
#define MOCK_SERIAL_DEVICE_H

#include "ISerialDevice.h"
#include <gmock/gmock.h>

class MockSerialDevice : public ISerialDevice
{
  public:
    MOCK_METHOD(void, begin, (), (override));
    MOCK_METHOD(void, send, (const std::string &msg), (override));
    MOCK_METHOD(bool, hasMessage, (), (override));
    MOCK_METHOD(std::string, popMessage, (), (override));
};

#endif // MOCK_SERIAL_DEVICE_H
