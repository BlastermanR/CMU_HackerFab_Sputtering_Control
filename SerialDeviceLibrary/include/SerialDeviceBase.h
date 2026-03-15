#ifndef SERIAL_DEVICE_BASE_H
#define SERIAL_DEVICE_BASE_H

#include "ISerialDevice.h"
#include "UARTInterface.h"
#include "pico/util/queue.h"
#include <string>
#include <cstring>

struct SerialMessage {
    char data[256];
};

class SerialDeviceBase : public ISerialDevice {
protected:
    IUart* uart;
    queue_t msgQueue;
    std::string receiveBuffer;

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
    SerialDeviceBase(IUart* uartInstance);

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
