#ifndef ISERIALDEVICE_H
#define ISERIALDEVICE_H

/**
 * Interface for High-Level Serial Devices
 */

#include <string>

// Uncomment the following line to enable serial debug prints
// #define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
#include <stdio.h>
#define SERIAL_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define SERIAL_PRINT(fmt, ...)
#endif

class ISerialDevice
{
  public:
    /**
     * @brief Virtual destructor for ISerialDevice.
     */
    virtual ~ISerialDevice() = default;

    /**
     * @brief Initializes the serial device.
     */
    virtual void begin() = 0;

    /**
     * @brief Sends a message through the serial device.
     * @param msg The string message to send.
     */
    virtual void send(const std::string &msg) = 0;

    /**
     * @brief Checks if there are any pending received messages.
     * @return true if a message is available, false otherwise.
     */
    virtual bool hasMessage() = 0;

    /**
     * @brief Retrieves and removes the oldest message from the receive queue.
     * @return The received message as a string.
     */
    virtual std::string popMessage() = 0;
};

#endif // ISERIALDEVICE_H
