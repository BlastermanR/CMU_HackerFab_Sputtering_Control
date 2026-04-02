#ifndef RS485_DEVICE_H
#define RS485_DEVICE_H

#include "SerialDeviceBase.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

class RS485Device : public SerialDeviceBase
{
  private:
    uint rtsPin;

  public:
    /**
     * @brief Constructs an RS485Device.
     * @param uartInstance Pointer to an IUart implementation.
     * @param rts The GPIO pin number used for Request-to-Send (RTS) flow control.
     */
    RS485Device(IUart *uartInstance, uint rts) : SerialDeviceBase(uartInstance), rtsPin(rts)
    {

        gpio_init(rtsPin);
        gpio_set_dir(rtsPin, GPIO_OUT);
        gpio_put(rtsPin, 0); // Default to listening
    }

    /**
     * @brief Sends a message over RS485, handling the RTS pin for half-duplex
     * communication.
     * @param msg The string message to send.
     */
    void send(const std::string &msg) override
    {
        SERIAL_PRINT("[RS485 Tx] %s\n", msg.c_str());

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
