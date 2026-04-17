/**
 * @file PicoMutex.h
 * @brief SputterOS `IMutex` implementation backed by the Pico SDK `mutex_t`.
 *
 * Used to guard the shared `TelemetryLogger` across Core 0 and Core 1.
 * Wraps `mutex_enter_timeout_ms` and `mutex_exit` to satisfy the
 * `SputterOS::IMutex` bounded-wait contract.
 *
 * @note `mutex_init` must be called before passing this object to
 *       `SystemBuilder::setTelemetryMutex()`. The constructor calls
 *       `mutex_init` automatically.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef PICO_MUTEX_H
#define PICO_MUTEX_H

#include "pico/mutex.h"
#include "sputteros/osal/sync/IMutex.h"
#include <chrono>

/**
 * @brief Pico SDK `mutex_t` wrapper satisfying `SputterOS::IMutex`.
 *
 * Provides bounded-wait lock/unlock semantics required by the
 * `TelemetryLogger` multi-core guard mechanism. All methods delegate
 * directly to Pico SDK mutex primitives.
 */
class PicoMutex : public SputterOS::IMutex
{
  public:
    /**
     * @brief Constructor. Initialises the underlying Pico SDK mutex.
     */
    PicoMutex() { mutex_init(&m_mutex); }

    /**
     * @brief Acquire the mutex, blocking up to `timeout`.
     * @param timeout Maximum time to wait. 0 ms means a single try-lock attempt.
     * @return true if the mutex was acquired within the timeout, false otherwise.
     */
    bool lock(std::chrono::milliseconds timeout) override
    {
        return mutex_enter_timeout_ms(&m_mutex, static_cast<uint32_t>(timeout.count()));
    }

    /**
     * @brief Release the mutex.
     */
    void unlock() override { mutex_exit(&m_mutex); }

  private:
    mutex_t m_mutex;
};

#endif // PICO_MUTEX_H
