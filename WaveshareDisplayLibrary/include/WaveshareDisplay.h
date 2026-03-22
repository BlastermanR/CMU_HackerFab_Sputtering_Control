/**
 * @file WaveshareDisplay.h
 * @brief Implementation for the 2" Waveshare LCD module (ST7789VW) using SPI. 
 *
 * @author Ryan Massie (rmassie)
 * @date 3/22/26
 */
#ifndef WAVESHARE_DISPLAY_H
#define WAVESHARE_DISPLAY_H

#include "IDisplay.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

/**
 * @name RGB565 Basic Colors
 * @brief Predefined 16-bit color constants for convenience.
 * @{
 */
#define COLOR_BLACK   0x0000
#define COLOR_BLUE    0x001F
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F
#define COLOR_YELLOW  0xFFE0
#define COLOR_WHITE   0xFFFF
/** @} */

/**
 * @class WaveshareDisplay
 * @brief Driver for the ST7789VW 320x240 IPS display using hardware SPI.
 */
class WaveshareDisplay : public IDisplay {
public:
    /**
     * @brief Construct a new Waveshare Display object.
     * 
     * @param spiPort The hardware SPI instance to use (spi0 or spi1).
     * @param pinCs Chip Select GPIO pin.
     * @param pinDc Data/Command GPIO pin.
     * @param pinRst Reset GPIO pin.
     * @param pinBl Backlight Control (PWM-capable) GPIO pin.
     * @param pinSck SPI Clock GPIO pin.
     * @param pinDin SPI MOSI GPIO pin.
     */
    WaveshareDisplay(spi_inst_t* spiPort, 
                     uint pinCs, uint pinDc, uint pinRst, uint pinBl, 
                     uint pinSck, uint pinDin);

    /** @brief Initializes the display hardware, GPIOs, SPI protocol, and ST7789 controller. */
    void init() override;

    /** @brief Clears the screen with a specific color. */
    void clear(uint16_t color) override;

    /** @brief Draws a single pixel at (x, y). */
    void drawPixel(uint16_t x, uint16_t y, uint16_t color) override;

    /** @brief Fills a rectangle from (x, y) with dimensions (w, h). */
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) override;

    /** @brief Sets the PWM duty cycle for the backlight (0-255). */
    void setBrightness(uint8_t brightness) override;

    /** @return The display width in pixels. */
    uint16_t getWidth() const { return width; }
    /** @return The display height in pixels. */
    uint16_t getHeight() const { return height; }

private:
    spi_inst_t* spiPort;
    uint csPin;
    uint dcPin;
    uint rstPin;
    uint blPin;
    uint sckPin;
    uint mosiPin;

    static constexpr uint16_t width = 320;
    static constexpr uint16_t height = 240;

    /** @brief Sends a command byte to the ST7789. */
    void writeCommand(uint8_t cmd);
    /** @brief Sends a data byte to the ST7789. */
    void writeData(uint8_t data);
    /** @brief Sends a buffer of data to the ST7789 over SPI. */
    void writeDataBuffer(const uint8_t* data, size_t len);
    /** @brief Sets the drawing window on the ST7789. */
    void setWindows(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
};

#endif // WAVESHARE_DISPLAY_H