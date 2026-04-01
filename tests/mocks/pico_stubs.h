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

inline uint32_t to_ms_since_boot(absolute_time_t t) { (void)t; return fake_time_ms; }
inline absolute_time_t get_absolute_time() { return static_cast<absolute_time_t>(fake_time_ms) * 1000; }

/* ── pico/stdlib.h stubs ─────────────────────────────────────────────── */

inline void sleep_ms(uint32_t ms) { (void)ms; }
inline void sleep_us(uint64_t us) { (void)us; }
inline void stdio_init_all() {}

/* ── hardware/gpio.h stubs ───────────────────────────────────────────── */

#define GPIO_OUT 1
#define GPIO_IN  0
#define GPIO_FUNC_GPIO 0
#define GPIO_FUNC_UART 0

inline void gpio_init(unsigned int pin) { (void)pin; }
inline void gpio_set_dir(unsigned int pin, bool out) { (void)pin; (void)out; }
inline void gpio_put(unsigned int pin, bool value) { (void)pin; (void)value; }
inline void gpio_set_function(unsigned int pin, unsigned int fn) { (void)pin; (void)fn; }

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
bool queue_is_empty(queue_t *q);

/* ── hardware/pio.h stubs (minimal — just the types) ─────────────────── */

typedef struct pio_hw {} *PIO;

// Provide placeholder PIO instances referenced by picoDefinitions.h
extern PIO pio0;
extern PIO pio1;
extern PIO pio2;

#endif // PICO_STUBS_H
