/**
 * @file SputteringCfg.h
 * @brief SputterOS compile-time configuration for the CMU HackerFab sputtering system.
 *
 * Defines the `SputteringCfg` struct satisfying the SputterOS `ConfigTraits`
 * contract. Enumerates process states, operator command IDs, and queue
 * capacity constraints for the dual-core RP2350 deployment.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef SPUTTERING_CFG_H
#define SPUTTERING_CFG_H

#include <cstddef>
#include <cstdint>

/**
 * @brief SputterOS configuration struct for the sputtering control system.
 *
 * This struct satisfies the SputterOS `ConfigTraits` contract. It defines the
 * process-state machine, command vocabulary, and kernel tuning parameters for
 * a dual-core RP2350 deployment.
 */
struct SputteringCfg
{
    /**
     * @brief High-level process states for the sputtering chamber.
     *
     * Transitions follow the order: IDLE → PUMP_DOWN → PURGE → IGNITION
     * → DEPOSITING → VENTING → IDLE. FAULT can be entered from any state.
     */
    enum class State : uint8_t
    {
        IDLE       = 0,
        PUMP_DOWN  = 1,
        PURGE      = 2,
        IGNITION   = 3,
        DEPOSITING = 4,
        VENTING    = 5,
        FAULT      = 6,
    };

    /**
     * @brief Operator command identifiers received over USB serial.
     *
     * Wire format (TEXT mode): `<CmdID> <targetDevice> <value>\n`
     *
     * Examples:
     *   - Start pump-down:     `0 0 1.0\n`   (SET_STATE → PUMP_DOWN)
     *   - Set argon flow:      `2 0 20.0\n`  (SET_ARGON_FLOW to 20 sccm)
     *   - Abort:               `1 0 0.0\n`   (ABORT)
     */
    enum class CmdID : uint8_t
    {
        SET_STATE       = 0,
        ABORT           = 1,
        SET_ARGON_FLOW  = 2,
        SET_OXYGEN_FLOW = 3,
        SET_PUMP_SPEED  = 4,
        SET_VERBOSITY   = 5,
    };

    /**
     * @brief Inter-core command packet transferred via the lock-free queue.
     */
    struct Command
    {
        CmdID   id;           /**< @brief Command identifier. */
        uint8_t targetDevice; /**< @brief Reserved device index (0 = system). */
        float   value;        /**< @brief Command parameter (setpoint, state ID, etc.). */
    };

    /** @brief Number of CPU cores used: 2 (Core 0 = control, Core 1 = comms). */
    static constexpr std::size_t kCoreCount = 2;

    /** @brief Lock-free command queue depth (producer: Core 1, consumer: Core 0). */
    static constexpr std::size_t kQueueCapacity = 16;

    /** @brief Maximum commands drained per control tick. */
    static constexpr int kMaxCommandsPerTick = 8;

    /** @brief Highest valid CmdID value (SET_VERBOSITY = 5). */
    static constexpr uint8_t kMaxValidCommandID = 5;

    /** @brief Control loop budget in microseconds (100 Hz = 10 ms). */
    static constexpr uint32_t kControlBudgetUs = 10000;
};

#endif // SPUTTERING_CFG_H
