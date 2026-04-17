/**
 * @file MockDisplay.h
 * @brief GMock mock for the IDisplay interface.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#ifndef MOCK_DISPLAY_H
#define MOCK_DISPLAY_H

#include "IDisplay.h"
#include <gmock/gmock.h>

class MockDisplay : public IDisplay
{
  public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, clear, (uint16_t color), (override));
    MOCK_METHOD(void, drawPixel, (uint16_t x, uint16_t y, uint16_t color), (override));
    MOCK_METHOD(void, fillRect, (uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color), (override));
    MOCK_METHOD(void, setBrightness, (uint8_t brightness), (override));
};

#endif // MOCK_DISPLAY_H
