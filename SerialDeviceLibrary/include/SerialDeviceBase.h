/**
 * @file SerialDeviceBase.h
 * @brief Declarations for the SerialDeviceBase component.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/4/26
 */

#ifndef SERIAL_DEVICE_BASE_H
#define SERIAL_DEVICE_BASE_H

#include "ISerialDevice.h"
#include "UARTInterface.h"
#include "pico/util/queue.h"
#include <cstring>
#include <string>

#define RECEIVE_BUFFER_SIZE 256
#define SERIAL_QUEUE_SIZE 10

struct SerialMessage
{
    char data[RECEIVE_BUFFER_SIZE];
};

class SerialDeviceBase : public ISerialDevice
{
  protected:
    IUart   *uart;
    queue_t  msgQueue;
    char     receiveBuffer[RECEIVE_BUFFER_SIZE];
    uint16_t receiveIndex;

    /**
     * @brief Internal callback for handling raw data characters from UART.
     * @param c The received character.
     */
    void onDataReceived(char c);

  public:
    /**
     * @brief Constructs a SerialDeviceBase with a specific UART interface.
     * @param uartInstance Pointer to an IUart implementation.
     */
    SerialDeviceBase(IUart *uartInstance);

    /**
     * @brief Destructor for SerialDeviceBase. Cleans up internal message queue.
     */
    virtual ~SerialDeviceBase();

    /**
     * @brief Initializes the serial device and sets up UART callbacks.
     */
    void begin() override;

    /**
     * @brief Checks if there are any complete messages in the queue.
     * @return true if at least one message is available, false otherwise.
     */
    bool hasMessage() override;

    /**
     * @brief Pops the next available message from the queue.
     * @return The message string.
     */
    std::string popMessage() override;
};

#endif // SERIAL_DEVICE_BASE_H
