#ifndef PIO_UART
#define PIO_UART

/**
 * Implements the UART interface for PIO
 *
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "UARTInterface.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "uart_rx.pio.h"
#include "uart_tx.pio.h"

class PioUart : public IUart
{
  private:
    PIO          pioInstance;
    uint         txSm, rxSm;
    uint         txPin, rxPin;
    uint         baudRate;
    UartCallback rxCallback;

    // Static map for Pico 2: 3 PIO blocks, 4 State Machines each
    static PioUart *instances[3][4];

    // Static IRQ Handlers
    static void pio0_irq_handler()
    {
        for (int i = 0; i < 4; i++)
        {
            if (instances[0][i] && !pio_sm_is_rx_fifo_empty(pio0, i))
            {
                instances[0][i]->handleRxIrq();
            }
        }
    }

    static void pio1_irq_handler()
    {
        for (int i = 0; i < 4; i++)
        {
            if (instances[1][i] && !pio_sm_is_rx_fifo_empty(pio1, i))
            {
                instances[1][i]->handleRxIrq();
            }
        }
    }

    static void pio2_irq_handler()
    {
        for (int i = 0; i < 4; i++)
        {
            if (instances[2][i] && !pio_sm_is_rx_fifo_empty(pio2, i))
            {
                instances[2][i]->handleRxIrq();
            }
        }
    }

    void handleRxIrq()
    {
        while (!pio_sm_is_rx_fifo_empty(pioInstance, rxSm))
        {
            char c = uart_rx_program_getc(pioInstance, rxSm);
            if (rxCallback)
                rxCallback(c);
        }
    }

  public:
    PioUart(PIO pio, uint tSm, uint rSm, uint tx, uint rx, uint baud)
        : pioInstance(pio), txSm(tSm), rxSm(rSm), txPin(tx), rxPin(rx), baudRate(baud)
    {

        int pioIndex              = (pio == pio0) ? 0 : (pio == pio1) ? 1 : 2;
        instances[pioIndex][rxSm] = this;
    }

    ~PioUart()
    {
        int pioIndex = (pioInstance == pio0) ? 0 : (pioInstance == pio1) ? 1 : 2;
        if (instances[pioIndex][rxSm] == this)
        {
            instances[pioIndex][rxSm] = nullptr;
        }

        // Optional: disable IRQs and remove PIO programs if completely cleaning up
        // hardware
        pio_set_irq0_source_enabled(pioInstance, (enum pio_interrupt_source)(pis_sm0_rx_fifo_not_empty + rxSm), false);
        pio_sm_set_enabled(pioInstance, txSm, false);
        pio_sm_set_enabled(pioInstance, rxSm, false);
    }

    void setCallback(UartCallback cb) override { rxCallback = cb; }

    void begin() override
    {
        // Load the compiled PIO programs for TX and RX into the chosen PIO block's
        // instruction memory
        uint txOffset = pio_add_program(pioInstance, &uart_tx_program);
        uint rxOffset = pio_add_program(pioInstance, &uart_rx_program);

        // Initialize both State Machines
        uart_tx_program_init(pioInstance, txSm, txOffset, txPin, baudRate);
        uart_rx_program_init(pioInstance, rxSm, rxOffset, rxPin, baudRate);

        // Enable interrupt whenever data enters the RX FIFO on the RX state machine
        pio_set_irq0_source_enabled(pioInstance, (enum pio_interrupt_source)(pis_sm0_rx_fifo_not_empty + rxSm), true);

        // Map PIO instance to the correct Pico architecture IRQ handler
        uint irqNumber;
        void (*handler)();
        if (pioInstance == pio0)
        {
            irqNumber = PIO0_IRQ_0;
            handler   = pio0_irq_handler;
        }
        else if (pioInstance == pio1)
        {
            irqNumber = PIO1_IRQ_0;
            handler   = pio1_irq_handler;
        }
        else
        {
            irqNumber = PIO2_IRQ_0;
            handler   = pio2_irq_handler;
        }

        // Apply IRQ settings to the processor logic
        irq_set_exclusive_handler(irqNumber, handler);
        irq_set_enabled(irqNumber, true);
    }

    void write(char c) override { pio_sm_put_blocking(pioInstance, txSm, (uint32_t)c); }

    void print(const char *str) override
    {
        while (*str)
            write(*str++);
    }

    void waitTxComplete() override
    {
        while (!pio_sm_is_tx_fifo_empty(pioInstance, txSm))
        {
            tight_loop_contents();
        }
        uint32_t delay_us = (10000000 + baudRate - 1) / baudRate;
        sleep_us(delay_us);
    }
};

// Initialize static array for 3 PIO blocks
inline PioUart *PioUart::instances[3][4] = {{nullptr}, {nullptr}, {nullptr}};

#endif // PIO_UART