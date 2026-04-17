/**
 * @file pico_stubs.h
 * @brief Lightweight stubs for Pico SDK types and functions used by project code.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 *
 * These stubs allow compiling and running unit tests on the host (Windows/Linux/Mac)
 * without pulling in the real Pico SDK.  Only the types and functions that are
 * actually referenced by testable code need to be present here.
 */
#ifndef PICO_STUBS_H
#define PICO_STUBS_H

#include <cstdint>
#include <cstdio>
#include <cstring>

/* ── pico/types.h stubs ──────────────────────────────────────────────── */

typedef uint64_t absolute_time_t;

/* ── Controllable fake clock ─────────────────────────────────────────── */

/// Tests can set this to control what to_ms_since_boot() returns.
extern uint32_t fake_time_ms;

inline uint32_t to_ms_since_boot(absolute_time_t t)
{
    (void)t;
    return fake_time_ms;
}
inline absolute_time_t get_absolute_time() { return static_cast<absolute_time_t>(fake_time_ms) * 1000; }

/* ── pico/stdlib.h stubs ─────────────────────────────────────────────── */

inline void sleep_ms(uint32_t ms) { (void)ms; }
inline void sleep_us(uint64_t us) { (void)us; }
inline void stdio_init_all() {}

/* ── hardware/gpio.h stubs ───────────────────────────────────────────── */

#define GPIO_OUT 1
#define GPIO_IN 0
#define GPIO_FUNC_GPIO 0
#define GPIO_FUNC_UART 0

inline void gpio_init(unsigned int pin) { (void)pin; }
inline void gpio_set_dir(unsigned int pin, bool out)
{
    (void)pin;
    (void)out;
}
inline void gpio_put(unsigned int pin, bool value)
{
    (void)pin;
    (void)value;
}
inline void gpio_set_function(unsigned int pin, unsigned int fn)
{
    (void)pin;
    (void)fn;
}

/* ── pico/util/queue.h stubs ─────────────────────────────────────────── */

struct queue_t
{
    uint8_t *buffer;
    uint16_t element_size;
    uint16_t element_count;
    uint16_t wptr;
    uint16_t rptr;
};

void queue_init(queue_t *q, unsigned int element_size, unsigned int element_count);
void queue_free(queue_t *q);
bool queue_try_add(queue_t *q, const void *data);
bool queue_try_remove(queue_t *q, void *data);
bool queue_try_peek(queue_t *q, void *data);
bool queue_is_empty(queue_t *q);

/* ── hardware/pio.h stubs (minimal — just the types) ─────────────────── */

typedef struct pio_hw
{
} *PIO;

// Provide placeholder PIO instances referenced by picoDefinitions.h
extern PIO pio0;
extern PIO pio1;
extern PIO pio2;

struct pio_program
{
    const uint16_t *instructions;
    uint8_t         length;
    int8_t          origin;
    uint8_t         pio_version;
};

// Stub PIO programs referenced by PIO_UART
static const struct pio_program uart_tx_program = {nullptr, 0, -1, 0};
static const struct pio_program uart_rx_program = {nullptr, 0, -1, 0};

typedef struct
{
    uint32_t dummy;
} pio_sm_config;

enum pio_interrupt_source
{
    pis_sm0_rx_fifo_not_empty = 0,
};

enum pio_fifo_join
{
    PIO_FIFO_JOIN_RX = 0,
};

#define PIO0_IRQ_0 7
#define PIO1_IRQ_0 9
#define PIO2_IRQ_0 11

inline bool pio_sm_is_rx_fifo_empty(PIO pio, unsigned int sm)
{
    (void)pio;
    (void)sm;
    return true;
}
inline void pio_sm_set_enabled(PIO pio, unsigned int sm, bool en)
{
    (void)pio;
    (void)sm;
    (void)en;
}
inline unsigned int pio_add_program(PIO pio, const struct pio_program *p)
{
    (void)pio;
    (void)p;
    return 0;
}
inline void pio_sm_put_blocking(PIO pio, unsigned int sm, uint32_t data)
{
    (void)pio;
    (void)sm;
    (void)data;
}
inline void pio_set_irq0_source_enabled(PIO pio, enum pio_interrupt_source src, bool en)
{
    (void)pio;
    (void)src;
    (void)en;
}
inline void pio_sm_set_consecutive_pindirs(PIO pio, unsigned int sm, unsigned int pin,
                                           unsigned int count, bool out)
{
    (void)pio;
    (void)sm;
    (void)pin;
    (void)count;
    (void)out;
}
inline void pio_gpio_init(PIO pio, unsigned int pin)
{
    (void)pio;
    (void)pin;
}
inline pio_sm_config pio_get_default_sm_config() { return pio_sm_config{}; }
inline void sm_config_set_wrap(pio_sm_config *c, unsigned int a, unsigned int b)
{
    (void)c;
    (void)a;
    (void)b;
}
inline void sm_config_set_in_pins(pio_sm_config *c, unsigned int pin)
{
    (void)c;
    (void)pin;
}
inline void sm_config_set_in_shift(pio_sm_config *c, bool right, bool autopush, unsigned int bits)
{
    (void)c;
    (void)right;
    (void)autopush;
    (void)bits;
}
inline void sm_config_set_fifo_join(pio_sm_config *c, enum pio_fifo_join join)
{
    (void)c;
    (void)join;
}
inline void sm_config_set_sideset_pins(pio_sm_config *c, unsigned int pin)
{
    (void)c;
    (void)pin;
}
inline void sm_config_set_out_shift(pio_sm_config *c, bool right, bool autopull, unsigned int bits)
{
    (void)c;
    (void)right;
    (void)autopull;
    (void)bits;
}
inline void sm_config_set_sideset(pio_sm_config *c, unsigned int count, bool opt, bool pindirs)
{
    (void)c;
    (void)count;
    (void)opt;
    (void)pindirs;
}
inline void sm_config_set_clkdiv(pio_sm_config *c, float div)
{
    (void)c;
    (void)div;
}
inline void pio_sm_init(PIO pio, unsigned int sm, unsigned int offset, pio_sm_config *c)
{
    (void)pio;
    (void)sm;
    (void)offset;
    (void)c;
}

/* Stub PIO UART init/getc functions (from generated .pio.h) */
inline void uart_tx_program_init(PIO pio, unsigned int sm, unsigned int offset,
                                 unsigned int pin, unsigned int baud)
{
    (void)pio;
    (void)sm;
    (void)offset;
    (void)pin;
    (void)baud;
}
inline void uart_rx_program_init(PIO pio, unsigned int sm, unsigned int offset,
                                 unsigned int pin, unsigned int baud)
{
    (void)pio;
    (void)sm;
    (void)offset;
    (void)pin;
    (void)baud;
}
inline char uart_rx_program_getc(PIO pio, unsigned int sm)
{
    (void)pio;
    (void)sm;
    return 0;
}
inline void gpio_pull_up(unsigned int pin) { (void)pin; }
inline bool pio_sm_is_tx_fifo_empty(PIO pio, unsigned int sm)
{
    (void)pio;
    (void)sm;
    return true;
}
inline void tight_loop_contents() {}

/* ── hardware/uart.h stubs ───────────────────────────────────────────── */

typedef struct uart_inst
{
} uart_inst_t;

extern uart_inst_t uart0_inst;
extern uart_inst_t uart1_inst;
#define uart0 (&uart0_inst)
#define uart1 (&uart1_inst)

typedef unsigned int uint;

#define UART0_IRQ 20
#define UART1_IRQ 21

inline bool uart_is_readable(uart_inst_t *uart)
{
    (void)uart;
    return false;
}
inline void uart_set_baudrate(uart_inst_t *uart, unsigned int baud)
{
    (void)uart;
    (void)baud;
}
inline void uart_init(uart_inst_t *uart, unsigned int baud)
{
    (void)uart;
    (void)baud;
}
inline void uart_set_hw_flow(uart_inst_t *uart, bool cts, bool rts)
{
    (void)uart;
    (void)cts;
    (void)rts;
}
inline void uart_set_format(uart_inst_t *uart, unsigned int data, unsigned int stop, unsigned int parity)
{
    (void)uart;
    (void)data;
    (void)stop;
    (void)parity;
}
inline void uart_set_fifo_enabled(uart_inst_t *uart, bool en)
{
    (void)uart;
    (void)en;
}
inline void uart_set_translate_crlf(uart_inst_t *uart, bool en)
{
    (void)uart;
    (void)en;
}
inline void uart_puts(uart_inst_t *uart, const char *s)
{
    (void)uart;
    (void)s;
}
inline void uart_putc(uart_inst_t *uart, char c)
{
    (void)uart;
    (void)c;
}
inline void uart_putc_raw(uart_inst_t *uart, char c)
{
    (void)uart;
    (void)c;
}
inline char uart_getc(uart_inst_t *uart)
{
    (void)uart;
    return 0;
}
inline bool uart_is_writable(uart_inst_t *uart)
{
    (void)uart;
    return true;
}
inline void uart_tx_wait_blocking(uart_inst_t *uart)
{
    (void)uart;
}
inline int uart_get_index(uart_inst_t *uart)
{
    (void)uart;
    return (uart == uart0) ? 0 : 1;
}
inline void uart_deinit(uart_inst_t *uart)
{
    (void)uart;
}
inline void uart_set_irq_enables(uart_inst_t *uart, bool rx, bool tx)
{
    (void)uart;
    (void)rx;
    (void)tx;
}

/* ── hardware/irq.h stubs ────────────────────────────────────────────── */

typedef void (*irq_handler_t)(void);
inline void irq_set_enabled(unsigned int irq, bool en)
{
    (void)irq;
    (void)en;
}
inline void irq_set_exclusive_handler(unsigned int irq, irq_handler_t handler)
{
    (void)irq;
    (void)handler;
}

/* ── pico/stdio stubs ────────────────────────────────────────────────── */

inline bool stdio_usb_connected() { return true; }
inline int getchar_timeout_us(uint32_t us)
{
    (void)us;
    return -1; // PICO_ERROR_TIMEOUT
}

/* ── hardware/watchdog.h stubs ───────────────────────────────────────── */

inline void watchdog_enable(uint32_t delay_ms, bool pause_on_debug)
{
    (void)delay_ms;
    (void)pause_on_debug;
}
inline void watchdog_update() {}

#endif // PICO_STUBS_H
