#ifndef RS485_DEVICE_H
#define RS485_DEVICE_H

#include "SerialDeviceBase.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

class RS485Device : public SerialDeviceBase {
private:
    uint rtsPin;

public:
    RS485Device(IUart* uartInstance, uint rts)
        : SerialDeviceBase(uartInstance), rtsPin(rts) {
        
        gpio_init(rtsPin);
        gpio_set_dir(rtsPin, GPIO_OUT);
        gpio_put(rtsPin, 0); // Default to listening
    }

    void send(const std::string& msg) override {
        // Toggle RTS high for transmit
        gpio_put(rtsPin, 1);
        
        // Give a tiny amount of time for the hardware to switch states if necessary
        sleep_us(10);
        
        // Send actual data
        uart->print(msg.c_str());
        
        // Wait until transmission actually finishes before dropping the RTS pin
        uart->waitTxComplete();
        
        // Return to receiving mode
        gpio_put(rtsPin, 0);
    }
};

#endif // RS485_DEVICE_H
