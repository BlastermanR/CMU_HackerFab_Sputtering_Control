#ifndef USB_SERIAL
#define USB_SERIAL

/**
 * Implements a Serial interface for recieving input from a host PC.
 * To be used for simple tests, not final serial implementation
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <functional>
#include <string>

// Callback passes a full string command once the user hits 'Enter'
typedef std::function<void(const std::string &)> UsbCommandCallback;

class USBSerial
{
  private:
    std::string        inputBuffer;
    UsbCommandCallback commandCallback;

  public:
    /**
     * @brief Constructs a new USBSerial object.
     */
    USBSerial();

    /**
     * @brief Initializes the USB serial interface.
     */
    void begin();

    /**
     * @brief Main loop function to process USB serial input.
     *
     * This should be called frequently to check for new data and trigger callbacks.
     */
    void update();

    /**
     * @brief Sets the callback function for received commands.
     * @param cb The callback function taking a const std::string reference.
     */
    void setCallback(UsbCommandCallback cb);

    /**
     * @brief Prints a null-terminated string to the USB serial output.
     * @param str The string to print.
     */
    void print(const char *str);

    /**
     * @brief Prints a null-terminated string followed by a newline to the USB serial output.
     * @param str The string to print.
     */
    void println(const char *str);
};

#endif // USB_SERIAL