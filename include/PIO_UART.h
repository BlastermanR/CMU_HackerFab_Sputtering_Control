#ifndef PIO_UART
#define PIO_UART

/**
 * Implements the UART interface for PIO
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "uart_tx.pio.h"
#include "uart_rx.pio.h"
#include "IUart.h"

class PioUart : public IUart {
private:
    PIO pioInstance;
    uint txSm, rxSm;
    uint txPin, rxPin;
    uint baudRate;
    UartCallback rxCallback;

    // Static map for Pico 2: 3 PIO blocks, 4 State Machines each
    static PioUart* instances[3][4];

    // (Static handlers and routing logic go here, identical to our previous discussion 
    // but expanded to check pio0, pio1, and pio2)
    
    void handleRxIrq() {
        while (!pio_sm_is_rx_fifo_empty(pioInstance, rxSm)) {
            char c = uart_rx_program_getc(pioInstance, rxSm);
            if (rxCallback) rxCallback(c);
        }
    }

public:
    PioUart(PIO pio, uint tSm, uint rSm, uint tx, uint rx, uint baud)
        : pioInstance(pio), txSm(tSm), rxSm(rSm), txPin(tx), rxPin(rx), baudRate(baud) {
        
        int pioIndex = (pio == pio0) ? 0 : (pio == pio1) ? 1 : 2;
        instances[pioIndex][rxSm] = this;
    }

    void setCallback(UartCallback cb) override {
        rxCallback = cb;
    }

    void begin() override {
        // (Initialization, pio_add_program, and IRQ setup go here)
    }

    void write(char c) override {
        pio_sm_put_blocking(pioInstance, txSm, (uint32_t)c);
    }

    void print(const char* str) override {
        while (*str) write(*str++);
    }
};

// Initialize static array for 3 PIO blocks
PioUart* PioUart::instances[3][4] = { {nullptr}, {nullptr}, {nullptr} };

#endif //PIO_UART