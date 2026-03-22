/**
 * Intercore.h
 * @brief Definitions for inter-core shared memory and atomic variables.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */
#ifndef INTERCORE_H
#define INTERCORE_H

#include <stdint.h>
#include <atomic>

/**
 * Define enum for status register bit masks to ensure type safety 
 * and avoid preprocessor macro pitfalls.
 */
enum StatusMask : uint8_t {
    Status_Core0Err    = (1 << 0),
    Status_Core1Err    = (1 << 1),
    Status_AlicatOxErr = (1 << 2),
    Status_AlicatArErr = (1 << 3),
    Status_PumpErr     = (1 << 4),
    Status_GaugeErr    = (1 << 5),
    // Unused (6)
    Status_Exit        = (1 << 7)
};

extern std::atomic<uint8_t> statusReg;

inline bool getStatus(StatusMask mask)
{
    return (statusReg.load(std::memory_order_acquire) & mask) != 0;
}

inline void setStatus(StatusMask mask)
{
    statusReg.fetch_or(mask, std::memory_order_release);
}

inline void clearStatus(StatusMask mask)
{
    statusReg.fetch_and(~mask, std::memory_order_release);
}

/**
 * @brief Structure for inter-core data sharing. This structure is designed to
 * hold all the shared variables that need to be accessed by both cores.
 */
typedef struct 
{
    struct
    {
        /* Vacuum Pump */
        float actualPumpSpeed{0.0f};

        /* Vacuum Gauge */
        float chamberPressure{0.0f};

        /* Argon MFC */
        float argonFlow{0.0f};

        /* Oxygen MFC */
        float oxygenFlow{0.0f};
    } Core0Out;
    
    struct
    {
        /* Vacuum Pump */
        float setPumpSpeed{0.0f};
        bool enablePump{false};

        /* Argon MFC */
        float setArgonFlow{0.0f};

        /* Oxygen MFC */
        float setOxygenFlow{0.0f};
    } Core1Out;
} intercoreData;

/**
 * @brief Volatile static structure for inter-core data sharing.
 * Marking as volatile to prevent compiler optimizations that could
 * cache values across cores.
 */
static volatile intercoreData sharedData;

#endif
