#ifndef HARDWARE_UART
#define HARDWARE_UART

/**
 * Implements the UART interface for native hardware implementation.
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "UARTInterface.h"

class HardUart : public IUart {
private:
    uart_inst_t* uartInstance;
    uint txPin;
    uint rxPin;
    uint baudRate;
    UartCallback rxCallback;

    // Static router map for the 2 hardware UARTs (uart0, uart1)
    static HardUart* instances[2];

    static void irqHandlerWrapper() {
        // Check uart0
        if (instances[0] && uart_is_readable(uart0)) {
            instances[0]->handleRxIrq();
        }
        // Check uart1
        if (instances[1] && uart_is_readable(uart1)) {
            instances[1]->handleRxIrq();
        }
    }

    void handleRxIrq() {
        while (uart_is_readable(uartInstance)) {
            char c = uart_getc(uartInstance);
            if (rxCallback) rxCallback(c);
        }
    }

public:
    HardUart(uart_inst_t* uart, uint tx, uint rx, uint baud)
        : uartInstance(uart), txPin(tx), rxPin(rx), baudRate(baud) {
        
        // Register in the static map (0 for uart0, 1 for uart1)
        int index = (uartInstance == uart0) ? 0 : 1;
        instances[index] = this;
    }

    void setCallback(UartCallback cb) override {
        rxCallback = cb;
    }

    void begin() override {
        uart_init(uartInstance, baudRate);
        gpio_set_function(txPin, GPIO_FUNC_UART);
        gpio_set_function(rxPin, GPIO_FUNC_UART);

        // Turn off FIFO so we interrupt on every single character
        uart_set_fifo_enabled(uartInstance, false);

        // Set up the interrupt
        uint irqNumber = (uartInstance == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_exclusive_handler(irqNumber, irqHandlerWrapper);
        irq_set_enabled(irqNumber, true);
        uart_set_irq_enables(uartInstance, true, false); // Enable RX IRQ, disable TX IRQ
    }

    void write(char c) override {
        uart_putc_raw(uartInstance, c);
    }

    void print(const char* str) override {
        uart_puts(uartInstance, str);
    }
};

// Initialize static array
HardUart* HardUart::instances[2] = {nullptr, nullptr};

#endif