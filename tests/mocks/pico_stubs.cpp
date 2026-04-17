/**
 * @file pico_stubs.cpp
 * @brief Implementation of Pico SDK stub functions for host-based unit testing.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#include "pico_stubs.h"
#include <cstdlib>
#include <cstring>

/* ── Fake clock ──────────────────────────────────────────────────────── */
uint32_t fake_time_ms = 0;

/* ── PIO placeholder instances ───────────────────────────────────────── */
static pio_hw pio0_hw_stub;
static pio_hw pio1_hw_stub;
static pio_hw pio2_hw_stub;
PIO           pio0 = &pio0_hw_stub;
PIO           pio1 = &pio1_hw_stub;
PIO           pio2 = &pio2_hw_stub;

/* ── Queue stubs (simple ring buffer) ────────────────────────────────── */

void queue_init(queue_t *q, unsigned int element_size, unsigned int element_count)
{
    q->element_size  = static_cast<uint16_t>(element_size);
    q->element_count = static_cast<uint16_t>(element_count);
    q->wptr          = 0;
    q->rptr          = 0;
    q->buffer        = static_cast<uint8_t *>(calloc(element_count, element_size));
}

void queue_free(queue_t *q)
{
    free(q->buffer);
    q->buffer = nullptr;
}

bool queue_try_add(queue_t *q, const void *data)
{
    uint16_t next = (q->wptr + 1) % q->element_count;
    if (next == q->rptr)
        return false; // full
    memcpy(q->buffer + q->wptr * q->element_size, data, q->element_size);
    q->wptr = next;
    return true;
}

bool queue_try_remove(queue_t *q, void *data)
{
    if (q->rptr == q->wptr)
        return false; // empty
    memcpy(data, q->buffer + q->rptr * q->element_size, q->element_size);
    q->rptr = (q->rptr + 1) % q->element_count;
    return true;
}

bool queue_try_peek(queue_t *q, void *data)
{
    if (q->rptr == q->wptr)
        return false; // empty
    memcpy(data, q->buffer + q->rptr * q->element_size, q->element_size);
    return true;
}

bool queue_is_empty(queue_t *q) { return q->rptr == q->wptr; }

/* ── UART placeholder instances ──────────────────────────────────────── */
uart_inst_t uart0_inst;
uart_inst_t uart1_inst;
