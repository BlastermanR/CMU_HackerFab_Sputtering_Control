/**
 * @file IDisplay.h
 * @brief Interface for graphical display drivers.
 *
 * This interface defines the contract for basic display operations.
 */
#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <cstdint>

/**
 * @class IDisplay
 * @brief Abstract base class for display implementations.
 */
class IDisplay
{
  public:
    virtual ~IDisplay() = default;

    /**
     * @brief Initializes the display hardware and controller.
     */
    virtual void init() = 0;

    /**
     * @brief Clears the entire screen with a specific color.
     * @param color 16-bit RGB565 color value.
     */
    virtual void clear(uint16_t color) = 0;

    /**
     * @brief Draws a single pixel.
     * @param x X-coordinate.
     * @param y Y-coordinate.
     * @param color 16-bit RGB565 color value.
     */
    virtual void drawPixel(uint16_t x, uint16_t y, uint16_t color) = 0;

    /**
     * @brief Fills a rectangular area with a color.
     * @param x X-coordinate of the top-left corner.
     * @param y Y-coordinate of the top-left corner.
     * @param w Width of the rectangle.
     * @param h Height of the rectangle.
     * @param color 16-bit RGB565 color value.
     */
    virtual void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) = 0;

    /**
     * @brief Sets the display backlight brightness.
     * @param brightness Brightness level (0-255).
     */
    virtual void setBrightness(uint8_t brightness) = 0;
};

#endif // IDISPLAY_H