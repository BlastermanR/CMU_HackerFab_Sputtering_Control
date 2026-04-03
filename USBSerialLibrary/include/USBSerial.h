#ifndef USB_SERIAL
#define USB_SERIAL

/**
 * Implements a USB Serial interface for inter-core communication.
 * Core 1 owns the USB I/O. Incoming text commands are parsed into
 * CommandMessage structs and pushed to the command queue. Outbound
 * messages from both cores are drained from their respective output
 * queues and printed to the terminal.
 *
 * The static log() and sendData() methods may be called from either
 * core. They push messages into the spinlock-protected Pico SDK
 * queues, making cross-core calls safe without additional locking.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "Messages.h"
#include <string>

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
     * Log messages print as "[CoreN] text". Data messages print as "$DataName:value".
     */
    void drainOutputQueues();

    /**
     * @brief Pushes a human-readable log message to the output queue.
     * Safe to call from either core.
     * @param source Which core is sending (Source_Core0 or Source_Core1).
     * @param text   The log text (truncated to OUTPUT_MSG_TEXT_LEN-1).
     * @param level  Verbosity level (default V_STATUS).
     */
    static void log(MessageSource source, const char *text, Verbosity level = V_STATUS);

    /**
     * @brief Pushes a structured data packet to the output queue.
     * Safe to call from either core.
     * @param source Which core is sending.
     * @param id     The telemetry data identifier.
     * @param value  The telemetry value.
     */
    static void sendData(MessageSource source, DataId id, float value);
};

#endif // USB_SERIAL