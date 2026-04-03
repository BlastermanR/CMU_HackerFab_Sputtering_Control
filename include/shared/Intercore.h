/**
 * Intercore.h
 * @brief Definitions for inter-core shared memory and atomic variables.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */
#ifndef INTERCORE_H
#define INTERCORE_H

#include "Messages.h"
#include "pico/util/queue.h"
#include <atomic>
#include <stdint.h>

#define CORE0_UPDATE_INTERVAL_MS 20
#define HANDSHAKE_TIMEOUT_MS 5000

/**
 * Define enum for status register bit masks to ensure type safety
 * and avoid preprocessor macro pitfalls.
 */
enum StatusMask : uint32_t
{
    // Errors and Exit Signals
    Status_None        = 0,
    Status_Core0Err    = (1U << 0),
    Status_Core1Err    = (1U << 1),
    Status_AlicatOxErr = (1U << 2),
    Status_AlicatArErr = (1U << 3),
    Status_PumpErr     = (1U << 4),
    Status_GaugeErr    = (1U << 5),
    Status_Exit        = (1U << 6),

    // Core 0 Instructions
    ExecuteSputteringProcess = (1U << 7),
    ExecuteCleaningProcess   = (1U << 8),
    PressurizeChamber        = (1U << 9),
    VentChamber              = (1U << 10),
    ShutOffGasFlow           = (1U << 11),
    PollDevices              = (1U << 12),

    SetArgonFlow  = (1U << 13),
    SetOxygenFlow = (1U << 14),
    SetPumpSpeed  = (1U << 15),
    EnablePump    = (1U << 16),
    DisablePump   = (1U << 17),
    PollArgon     = (1U << 18),
    PollOxygen    = (1U << 19),
    PollPump      = (1U << 20),
    PollGauge     = (1U << 21),

    // Startup
    Core0_Begin = (1U << 30),
    Core1_Begin = (1U << 31)
};

extern std::atomic<uint32_t> statusReg;
extern std::atomic<uint8_t>  verbosityLevel;

extern queue_t commandQueue;
extern queue_t core0OutQueue;
extern queue_t core1OutQueue;

/**
 * @brief Initializes the inter-core communication queues.
 * Must be called before launching Core 1.
 */
void initQueues();

/**
 * @brief Gets the current status of the specified mask.
 * @param mask The bitmask of the status to check.
 * @return True if the specified status bit is set, false otherwise.
 */
inline bool getStatus(StatusMask mask) { return (statusReg.load(std::memory_order_acquire) & mask) != 0; }

/**
 * @brief Sets the specified status bit(s).
 * @param mask The bitmask of the status to set.
 */
inline void setStatus(StatusMask mask) { statusReg.fetch_or(mask, std::memory_order_release); }

/**
 * @brief Clears the specified status bit(s).
 * @param mask The bitmask of the status to clear.
 */
inline void clearStatus(StatusMask mask) { statusReg.fetch_and(~mask, std::memory_order_release); }

/**
 * @brief Checks if any error status bits are currently set.
 * @note This ignores the Status_Exit bit and any execution command bits.
 * @return True if at least one error bit is set, false otherwise.
 */
inline bool isError()
{
    const uint32_t ERROR_MASK =
        Status_Core0Err | Status_Core1Err | Status_AlicatOxErr | Status_AlicatArErr | Status_PumpErr | Status_GaugeErr;
    return (statusReg.load(std::memory_order_acquire) & ERROR_MASK) != 0;
}

/**
 * @brief Gets the first currently active error status mask.
 * @note Error checks are prioritized in sequential order from Core0 down to Gauge errors.
 * @return The highest-priority active error StatusMask, or Status_None if no errors exist.
 */
inline StatusMask getError()
{
    const uint32_t ERROR_MASK =
        Status_Core0Err | Status_Core1Err | Status_AlicatOxErr | Status_AlicatArErr | Status_PumpErr | Status_GaugeErr;
    uint32_t status = statusReg.load(std::memory_order_acquire) & ERROR_MASK;
    if (status & Status_Core0Err)
        return Status_Core0Err;
    if (status & Status_Core1Err)
        return Status_Core1Err;
    if (status & Status_AlicatOxErr)
        return Status_AlicatOxErr;
    if (status & Status_AlicatArErr)
        return Status_AlicatArErr;
    if (status & Status_PumpErr)
        return Status_PumpErr;
    if (status & Status_GaugeErr)
        return Status_GaugeErr;
    return Status_None;
}

/**
 * @brief Checks for active commands and returns the lowest set command bit.
 * @return The first active command StatusMask, or Status_None if no commands exist.
 */
inline StatusMask isCommand()
{
    const uint32_t COMMAND_MASK = ExecuteSputteringProcess | ExecuteCleaningProcess | PressurizeChamber |
                                  VentChamber | ShutOffGasFlow | PollDevices | PollArgon | PollOxygen | PollPump |
                                  PollGauge | SetArgonFlow | SetOxygenFlow | SetPumpSpeed | EnablePump | DisablePump;

    uint32_t commandBits = statusReg.load(std::memory_order_acquire) & COMMAND_MASK;
    if (commandBits)
    {
        return static_cast<StatusMask>(commandBits & -commandBits);
    }
    return Status_None;
}

/**
 * @brief Structure for inter-core data sharing. This structure is designed to
 * hold all the shared variables that need to be accessed by both cores.
 */
struct intercoreData
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
        bool  enablePump{false};

        /* Argon MFC */
        float setArgonFlow{0.0f};

        /* Oxygen MFC */
        float setOxygenFlow{0.0f};
    } Core1Out;
};

/**
 * @brief Volatile structure for inter-core data sharing.
 * Marking as volatile to prevent compiler optimizations that could
 * cache values across cores.
 */
extern volatile intercoreData sharedData;

#endif
