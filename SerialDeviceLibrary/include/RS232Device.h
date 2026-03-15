#ifndef RS232_DEVICE_H
#define RS232_DEVICE_H

#include "SerialDeviceBase.h"

class RS232Device : public SerialDeviceBase {
public:
    RS232Device(IUart* uartInstance) : SerialDeviceBase(uartInstance) {}

    void send(const std::string& msg) override {
        uart->print(msg.c_str());
    }
};

#endif // RS232_DEVICE_H
