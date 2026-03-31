#ifndef USB_SERIAL
#define USB_SERIAL

/**
 * Implements a USB Serial interface for inter-core communication.
 * Core 1 owns the USB I/O. Incoming text commands are parsed into
 * CommandMessage structs and pushed to the command queue. Outbound
 * messages from both cores are drained from their respective output
 * queues and printed to the terminal.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <string>
#include "Messages.h"

class USBSerial
{
  private:
    std::string inputBuffer;

    /**
     * @brief Parses a raw text command into a CommandMessage.
     * @param input The raw string from the terminal.
     * @param msg   Output CommandMessage to populate.
     * @return True if the command was recognized, false otherwise.
     */
    bool parseCommand(const std::string &input, CommandMessage &msg);

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
     * @brief Non-blocking input reader. Accumulates characters and on Enter,
     * parses the string into a CommandMessage and pushes it to the command queue.
     */
    void readInput();

    /**
     * @brief Drains both core output queues and prints their messages to USB.
     */
    void drainOutputQueues();

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