/**
 * @file SputteringApp.h
 * @brief IUserApplication implementation for the CMU HackerFab sputtering system.
 *
 * `SputteringApp` is the process engine driven by the SputterOS `ControlTask`
 * at 100 Hz. It owns the five process-phase objects and manages the state
 * machine that sequences them. Device I/O uses the global instances declared
 * in `GlobalDevices.h`.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef SPUTTERING_APP_H
#define SPUTTERING_APP_H

#include "ProcessPhases.h"
#include "SputteringCfg.h"
#include "sputteros/interfaces/IProcessState.h"
#include "sputteros/kernel/interfaces/IUserApplication.h"
#include "sputteros/osal/SputterTime.h"

/**
 * @brief Concrete IUserApplication that runs the sputtering chamber control loop.
 *
 * Responsibilities:
 * - Polling all hardware devices (MFCs, gauge, pump) every control tick.
 * - Delegating per-tick process logic to the active `IProcessState` phase.
 * - Routing operator commands from the lock-free queue to state transitions
 *   or direct device setpoints.
 * - Forcing a safe abort (gas off, pump off, FAULT state) on any safety event.
 */
class SputteringApp : public SputterOS::IUserApplication<SputteringCfg>
{
  public:
    SputteringApp();

    /**
     * @brief Initialise all hardware devices and enter IDLE state.
     * Called once by `ControlTask::init()` before the first tick.
     */
    void init() override;

    /**
     * @brief Advance the active process phase and poll device telemetry.
     * @param systemTimeMicros Monotonic system time in microseconds.
     */
    void tick(SputterOS::SputterMicros systemTimeMicros) override;

    /**
     * @brief Route an operator command to a state transition or direct
     * device setpoint change.
     * @param cmd Command packet from the lock-free queue.
     */
    void handleCommand(const SputteringCfg::Command &cmd) override;

    /**
     * @brief Immediately de-energise all actuators and enter FAULT state.
     *
     * Called by `ControlTask` when any `ISafetyMonitor` reports a failsafe.
     * Must complete in bounded time and must not block.
     */
    void forceSafeAbort() override;

    /**
     * @brief Return the current process state.
     * @return Current state enum value.
     */
    SputteringCfg::State getState() const { return m_state; }

    /**
     * @brief Transition the state machine to a new state.
     *
     * Calls `onExit()` on the current phase, updates `m_state`, and calls
     * `onEnter()` on the new phase. Transitions to IDLE clear the active phase.
     *
     * @param next Target state.
     */
    void transitionTo(SputteringCfg::State next);

  private:
    /**
     * @brief Return a pointer to the phase object associated with `state`,
     * or `nullptr` for IDLE and FAULT.
     */
    SputterOS::IProcessState *phaseFor(SputteringCfg::State state);

    /**
     * @brief Poll each hardware device and log updated sensor values
     * to the TelemetryLogger.
     * @param systemTimeMicros Current system time (for log timestamps).
     */
    void pollDevices(SputterOS::SputterMicros systemTimeMicros);

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------

    SputteringCfg::State      m_state{SputteringCfg::State::IDLE};
    SputterOS::IProcessState *m_activePhase{nullptr};

    // -----------------------------------------------------------------------
    // Process phases — owned by this application instance.
    // -----------------------------------------------------------------------

    PumpDownPhase  m_pumpDownPhase;
    PurgePhase     m_purgePhase;
    IgnitionPhase  m_ignitionPhase;
    DepositingPhase m_depositingPhase;
    VentingPhase   m_ventingPhase;
};

#endif // SPUTTERING_APP_H
