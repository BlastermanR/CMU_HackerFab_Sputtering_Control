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

    void onDataReceived(char c);

public:
    SerialDeviceBase(IUart* uartInstance);
    virtual ~SerialDeviceBase();

    void begin() override;
    bool hasMessage() override;
    std::string popMessage() override;
};

#endif // SERIAL_DEVICE_BASE_H
