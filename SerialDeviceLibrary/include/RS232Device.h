#ifndef RS232_DEVICE_H
#define RS232_DEVICE_H

#include "SerialDeviceBase.h"

class RS232Device : public SerialDeviceBase {
public:
    /**
     * @brief Constructs an RS232Device.
     * @param uartInstance Pointer to an IUart implementation.
     */
    RS232Device(IUart* uartInstance) : SerialDeviceBase(uartInstance) {}

    /**
     * @brief Sends a message over RS232.
     * @param msg The string message to send.
     */
    void send(const std::string& msg) override {
        uart->print(msg.c_str());
    }
};

#endif // RS232_DEVICE_H
