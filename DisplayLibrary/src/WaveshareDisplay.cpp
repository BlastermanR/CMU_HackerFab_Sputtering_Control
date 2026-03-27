/**
 * @file WaveshareDisplay.cpp
 * @brief Implementation of ST7789VW display operations.
 */
#include "WaveshareDisplay.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"
#include <cstdio>
#include <cstring>
#include "font5x7.h"

/** @name ST7789 Commands
 *  @brief ST7789 controller specific command set.
 *  @{
 */
#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_COLMOD 0x3A
#define ST7789_MADCTL 0x36
#define ST7789_INVON 0x21
#define ST7789_NORON 0x13
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
/** @} */

WaveshareDisplay::WaveshareDisplay(spi_inst_t *spiPortIn, uint pinCs, uint pinDc, uint pinRst, uint pinBl, uint pinSck,
                                   uint pinDin)
    : spiPort(spiPortIn), csPin(pinCs), dcPin(pinDc), rstPin(pinRst), blPin(pinBl), sckPin(pinSck), mosiPin(pinDin)
{
}

void WaveshareDisplay::writeCommand(uint8_t cmd)
{
    gpio_put(dcPin, 0); // Command mode
    gpio_put(csPin, 0);
    spi_write_blocking(spiPort, &cmd, 1);
    gpio_put(csPin, 1);
}

void WaveshareDisplay::writeData(uint8_t data)
{
    gpio_put(dcPin, 1); // Data mode
    gpio_put(csPin, 0);
    spi_write_blocking(spiPort, &data, 1);
    gpio_put(csPin, 1);
}

void WaveshareDisplay::writeDataBuffer(const uint8_t *data, size_t len)
{
    gpio_put(dcPin, 1);
    gpio_put(csPin, 0);
    spi_write_blocking(spiPort, data, len);
    gpio_put(csPin, 1);
}

void WaveshareDisplay::init()
{
    // 1. Init SPI
    spi_init(spiPort, 40 * 1000 * 1000); // 40 MHz SPI
    gpio_set_function(sckPin, GPIO_FUNC_SPI);
    gpio_set_function(mosiPin, GPIO_FUNC_SPI);

    // 2. Init GPIOs
    gpio_init(csPin);
    gpio_set_dir(csPin, GPIO_OUT);
    gpio_put(csPin, 1);

    gpio_init(dcPin);
    gpio_set_dir(dcPin, GPIO_OUT);
    gpio_put(dcPin, 0);

    gpio_init(rstPin);
    gpio_set_dir(rstPin, GPIO_OUT);
    gpio_put(rstPin, 1);

    // 3. Init PWM for Backlight
    gpio_set_function(blPin, GPIO_FUNC_PWM);
    uint       sliceNum = pwm_gpio_to_slice_num(blPin);
    pwm_config config   = pwm_get_default_config();
    pwm_config_set_wrap(&config, 255);
    pwm_init(sliceNum, &config, true);
    setBrightness(255); // Full brightness

    // 4. Reset sequence
    gpio_put(rstPin, 1);
    sleep_ms(100);
    gpio_put(rstPin, 0);
    sleep_ms(100);
    gpio_put(rstPin, 1);
    sleep_ms(100);

    // 5. ST7789 Init Commands
    writeCommand(ST7789_SWRESET);
    sleep_ms(150);

    writeCommand(ST7789_SLPOUT);
    sleep_ms(150);

    writeCommand(ST7789_COLMOD); // Interface Pixel Format
    writeData(0x55);             // 16-bit/pixel (RGB565)

    writeCommand(ST7789_MADCTL);
    // Memory Data Access Control (0x70) configures rotation & mirroring
    // Typical combination for 240x320 converting to 320x240 landscape setup
    writeData(0x70);

    writeCommand(ST7789_INVON); // Inversion on is often needed for IPS panels
    sleep_ms(10);
    writeCommand(ST7789_NORON); // Normal Display On
    sleep_ms(10);
    writeCommand(ST7789_DISPON); // Display On
    sleep_ms(100);
}

void WaveshareDisplay::setBrightness(uint8_t brightness) { pwm_set_gpio_level(blPin, brightness); }

void WaveshareDisplay::setWindows(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    // Set X
    writeCommand(ST7789_CASET);
    writeData(xStart >> 8);
    writeData(xStart & 0xFF);
    writeData(xEnd >> 8);
    writeData(xEnd & 0xFF);

    // Set Y
    writeCommand(ST7789_RASET);
    writeData(yStart >> 8);
    writeData(yStart & 0xFF);
    writeData(yEnd >> 8);
    writeData(yEnd & 0xFF);

    writeCommand(ST7789_RAMWR);
}

void WaveshareDisplay::drawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= width || y >= height)
        return;
    setWindows(x, y, x, y);
    uint8_t buf[2] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
    writeDataBuffer(buf, 2);
}

void WaveshareDisplay::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (x >= width || y >= height)
        return;
    if ((x + w - 1) >= width)
        w = width - x;
    if ((y + h - 1) >= height)
        h = height - y;

    setWindows(x, y, x + w - 1, y + h - 1);

    // Allocate a small buffer to speed up SPI transfers (2 bytes per pixel)
    const size_t buf_size = 512;
    uint8_t      buf[buf_size * 2];
    uint8_t      hi = color >> 8;
    uint8_t      lo = color & 0xFF;

    for (size_t i = 0; i < buf_size * 2; i += 2)
    {
        buf[i]     = hi;
        buf[i + 1] = lo;
    }

    uint32_t totalPixels = w * h;
    while (totalPixels > 0)
    {
        size_t toWrite = (totalPixels > buf_size) ? buf_size : totalPixels;
        writeDataBuffer(buf, toWrite * 2);
        totalPixels -= toWrite;
    }
}

void WaveshareDisplay::clear(uint16_t color) { fillRect(0, 0, width, height, color); }

// --- Basic UI/Graphics Implementation ---
void WaveshareDisplay::drawFastHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color)
{
    fillRect(x, y, w, 1, color);
}

void WaveshareDisplay::drawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size)
{
    if((x >= width)            || // Clip right
       (y >= height)           || // Clip bottom
       ((x + 6 * size - 1) < 0) || // Clip left
       ((y + 8 * size - 1) < 0))   // Clip top
        return;

    for (int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
        uint8_t line = font[c * 5 + i];
        for (int8_t j = 0; j<8; j++) {
            if (line & 0x1) {
                if (size == 1) // default size
                    drawPixel(x+i, y+j, color);
                else {  // big size
                    fillRect(x+(i*size), y+(j*size), size, size, color);
                } 
            } else if (bg != color) {
                if (size == 1) // default size
                    drawPixel(x+i, y+j, bg);
                else {  // big size
                    fillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
            line >>= 1;
        }
    }
}

void WaveshareDisplay::drawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size)
{
    int cursor_x = x;
    while (*str)
    {
        drawChar(cursor_x, y, *str, color, bg, size);
        cursor_x += 6 * size;
        str++;
    }
}

void WaveshareDisplay::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0 - r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
}

void WaveshareDisplay::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    drawFastHLine(x0 - r, y0, 2 * r + 1, color);
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawFastHLine(x0 - x, y0 + y, 2 * x + 1, color);
        drawFastHLine(x0 - x, y0 - y, 2 * x + 1, color);
        drawFastHLine(x0 - y, y0 + x, 2 * y + 1, color);
        drawFastHLine(x0 - y, y0 - x, 2 * y + 1, color);
    }
}

// --- Sputtering UI Methods ---
void WaveshareDisplay::initSputteringUI()
{
    clear(COLOR_WHITE);
    // Draw Header
    drawString(10, 10, "CMU Hacker Fab Sputtering", COLOR_BLACK, COLOR_WHITE, 2);
    // Header Line
    drawFastHLine(10, 30, 300, COLOR_BLACK);

    // Draw Labels
    drawString(10, 45, "Status:", COLOR_BLACK, COLOR_WHITE, 2);
    drawString(10, 75, "Pump Speed:", COLOR_BLACK, COLOR_WHITE, 2);
    drawString(10, 105, "Chamber Pressure:", COLOR_BLACK, COLOR_WHITE, 2);
    drawString(10, 135, "Argon Flow:", COLOR_BLACK, COLOR_WHITE, 2);
    drawString(10, 165, "Oxygen Flow:", COLOR_BLACK, COLOR_WHITE, 2);

    // Default Values
    setStatus("Idle", 0xCE79); // Light Blue-ish
    setPumpSpeed(2500);
    setChamberPressure(10.0f);
    setArgonFlow(10.0f);
    setOxygenFlow(10.0f);
}

void WaveshareDisplay::setStatus(const char *status, uint16_t indicatorColor)
{
    fillRect(100, 45, 150, 20, COLOR_WHITE); // clear old status text
    drawString(100, 45, status, COLOR_BLACK, COLOR_WHITE, 2);

    // Status circle indicator
    fillCircle(280, 50, 15, indicatorColor);
    drawCircle(280, 50, 15, COLOR_BLACK); // outline
}

void WaveshareDisplay::setPumpSpeed(int speed)
{
    char buf[16];
    std::sprintf(buf, "%d", speed);
    fillRect(150, 75, 100, 20, COLOR_WHITE);
    drawString(150, 75, buf, COLOR_BLACK, COLOR_WHITE, 2);
}

void WaveshareDisplay::setChamberPressure(float pressure)
{
    char buf[32];
    std::sprintf(buf, "%.1f hPa", pressure);
    fillRect(210, 105, 100, 20, COLOR_WHITE);
    drawString(210, 105, buf, COLOR_BLACK, COLOR_WHITE, 2);
}

void WaveshareDisplay::setArgonFlow(float flow)
{
    char buf[16];
    std::sprintf(buf, "%.1f", flow);
    fillRect(150, 135, 100, 20, COLOR_WHITE);
    drawString(150, 135, buf, COLOR_BLACK, COLOR_WHITE, 2);
}

void WaveshareDisplay::setOxygenFlow(float flow)
{
    char buf[16];
    std::sprintf(buf, "%.1f", flow);
    fillRect(160, 165, 100, 20, COLOR_WHITE);
    drawString(160, 165, buf, COLOR_BLACK, COLOR_WHITE, 2);
}
