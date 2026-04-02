#ifndef HARD_UART_CLASS_H
#define HARD_UART_CLASS_H

/**
 * Implements the UART interface for native hardware implementation.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "UARTInterface.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

class HardUart : public IUart
{
  private:
    uart_inst_t *uartInstance;
    uint         txPin;
    uint         rxPin;
    uint         baudRate;
    UartCallback rxCallback;

    // Static router map for the 2 hardware UARTs (uart0, uart1)
    static HardUart *instances[2];

    static void irqHandlerWrapper()
    {
        // Check uart0
        if (instances[0] && uart_is_readable(uart0))
        {
            instances[0]->handleRxIrq();
        }
        // Check uart1
        if (instances[1] && uart_is_readable(uart1))
        {
            instances[1]->handleRxIrq();
        }
    }

    void handleRxIrq()
    {
        while (uart_is_readable(uartInstance))
        {
            char c = uart_getc(uartInstance);
            if (rxCallback)
                rxCallback(c);
        }
    }

  public:
    /**
     * @brief Constructs a HardUart object.
     * @param uart The hardware UART instance (uart0 or uart1).
     * @param tx The GPIO pin used for transmission.
     * @param rx The GPIO pin used for reception.
     * @param baud The desired baud rate.
     */
    HardUart(uart_inst_t *uart, uint tx, uint rx, uint baud) : uartInstance(uart), txPin(tx), rxPin(rx), baudRate(baud)
    {

        // Register in the static map (0 for uart0, 1 for uart1)
        int index        = (uartInstance == uart0) ? 0 : 1;
        instances[index] = this;
    }

    /**
     * @brief Destructor for HardUart. Cleans up IRQs and de-initializes UART.
     */
    ~HardUart()
    {
        int index = (uartInstance == uart0) ? 0 : 1;
        if (instances[index] == this)
        {
            instances[index] = nullptr;
        }

        // Optional: disable IRQ and de-init UART hardware to leave cleanly
        uint irqNumber = (uartInstance == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_enabled(irqNumber, false);
        uart_deinit(uartInstance);
    }

    /**
     * @brief Sets the callback for received characters.
     * @param cb Function to call when a character is received.
     */
    void setCallback(UartCallback cb) override { rxCallback = cb; }

    /**
     * @brief Initializes the hardware UART, configures pins, and enables interrupts.
     */
    void begin() override
    {
        uart_init(uartInstance, baudRate);
        gpio_set_function(txPin, GPIO_FUNC_UART);
        gpio_set_function(rxPin, GPIO_FUNC_UART);

        // Turn off FIFO so we interrupt on every single character
        uart_set_fifo_enabled(uartInstance, false);

        // Set up the interrupt
        uint irqNumber = (uartInstance == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_exclusive_handler(irqNumber, irqHandlerWrapper);
        irq_set_enabled(irqNumber, true);
        uart_set_irq_enables(uartInstance, true,
                             false); // Enable RX IRQ, disable TX IRQ
    }

    /**
     * @brief Writes a single character to the hardware UART.
     * @param c Character to write.
     */
    void write(char c) override { uart_putc_raw(uartInstance, c); }

    /**
     * @brief Prints a null-terminated string to the hardware UART.
     * @param str String to print.
     */
    void print(const char *str) override { uart_puts(uartInstance, str); }

    /**
     * @brief Blocks until all characters in the TX FIFO have been transmitted.
     */
    void waitTxComplete() override { uart_tx_wait_blocking(uartInstance); }

    /**
     * @brief Gets the configured TX pin.
     * @return TX GPIO pin number.
     */
    unsigned int getTxPin() const override { return txPin; }

    /**
     * @brief Gets the configured RX pin.
     * @return RX GPIO pin number.
     */
    unsigned int getRxPin() const override { return rxPin; }
};

// Initialize static array
inline HardUart *HardUart::instances[2] = {nullptr, nullptr};

#endif // HARDWARE_UART_CLASS_H