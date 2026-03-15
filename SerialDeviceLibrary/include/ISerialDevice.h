#ifndef ISERIALDEVICE_H
#define ISERIALDEVICE_H

/**
 * Interface for High-Level Serial Devices
 */

#include <string>

class ISerialDevice {
public:
    virtual ~ISerialDevice() = default;

    virtual void begin() = 0;
    virtual void send(const std::string& msg) = 0;
    virtual bool hasMessage() const = 0;
    virtual std::string popMessage() = 0;
};

#endif // ISERIALDEVICE_H
