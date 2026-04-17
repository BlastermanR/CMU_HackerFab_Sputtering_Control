/**
 * @file ProcessPhases.h
 * @brief IProcessState implementations for the sputtering chamber process sequence.
 *
 * Each class represents one discrete phase in the sputtering process state machine:
 * PUMP_DOWN → PURGE → IGNITION → DEPOSITING → VENTING.
 *
 * Phase objects hold a back-pointer to `SputteringApp` to trigger state transitions
 * via `SputteringApp::transitionTo()` when exit conditions are met. Device access
 * uses the global device instances from `GlobalDevices.h`.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef PROCESS_PHASES_H
#define PROCESS_PHASES_H

#include "SputteringCfg.h"
#include "sputteros/interfaces/IProcessState.h"
#include "sputteros/osal/SputterTime.h"
#include "sputteros/utils/NonBlockingStopwatch.h"
#include <cstdint>

// Forward declaration — avoids circular include with SputteringApp.h.
class SputteringApp;

// ---------------------------------------------------------------------------
// PumpDownPhase
// ---------------------------------------------------------------------------

/**
 * @brief Activates the vacuum pump and waits until the chamber reaches
 * the target base pressure or a timeout elapses.
 *
 * Default target: < 0.1 hPa within 120 seconds.
 * On completion, transitions the application to PURGE state.
 */
class PumpDownPhase : public SputterOS::IProcessState
{
  public:
    /**
     * @brief Constructor.
     * @param app Back-pointer used to trigger state transitions.
     * @param targetPressureHPa Pressure threshold to exit pump-down (hPa).
     * @param timeoutUs Maximum time to spend in this phase (microseconds).
     */
    explicit PumpDownPhase(SputteringApp *app,
                           double         targetPressureHPa = 0.1,
                           uint64_t       timeoutUs         = 120'000'000ULL);

    /** @brief Signals the pump on and latches entry time. */
    void onEnter() override;

    /**
     * @brief Polls chamber pressure; transitions to PURGE when at target
     * or transitions to FAULT on timeout.
     */
    void execute(SputterOS::SputterMicros systemTimeMicros) override;

    /** @brief No actuator changes on exit. */
    void onExit() override;

  private:
    SputteringApp *m_app;
    double         m_targetPressureHPa;
    uint64_t       m_timeoutUs;
    SputterOS::NonBlockingStopwatch m_timer;
};

// ---------------------------------------------------------------------------
// PurgePhase
// ---------------------------------------------------------------------------

/**
 * @brief Opens the argon gas flow to purge residual atmosphere from the chamber.
 *
 * Default flow: 50 sccm argon for 30 seconds.
 * On completion, transitions to IGNITION state.
 */
class PurgePhase : public SputterOS::IProcessState
{
  public:
    /**
     * @brief Constructor.
     * @param app Back-pointer used to trigger state transitions.
     * @param argonFlowSccm Argon MFC setpoint in sccm.
     * @param durationUs Purge duration in microseconds.
     */
    explicit PurgePhase(SputteringApp *app,
                        double         argonFlowSccm = 50.0,
                        uint64_t       durationUs    = 30'000'000ULL);

    /** @brief Sets argon flow setpoint and latches entry time. */
    void onEnter() override;

    /** @brief Transitions to IGNITION after the purge duration. */
    void execute(SputterOS::SputterMicros systemTimeMicros) override;

    /** @brief No actuator changes on exit (gas flow maintained for ignition). */
    void onExit() override;

  private:
    SputteringApp *m_app;
    double         m_argonFlowSccm;
    uint64_t       m_durationUs;
    SputterOS::NonBlockingStopwatch m_timer;
};

// ---------------------------------------------------------------------------
// IgnitionPhase
// ---------------------------------------------------------------------------

/**
 * @brief Reduces gas flow to the ignition setpoint and waits for RF plasma
 * ignition.
 *
 * RF power control is not yet implemented. This phase runs for a fixed
 * dwell time before transitioning to DEPOSITING.
 *
 * Default flow: 20 sccm argon for 15 seconds.
 *
 * @todo Integrate RF power supply driver when available.
 */
class IgnitionPhase : public SputterOS::IProcessState
{
  public:
    /**
     * @brief Constructor.
     * @param app Back-pointer used to trigger state transitions.
     * @param argonFlowSccm Argon MFC setpoint for ignition in sccm.
     * @param durationUs Ignition dwell time in microseconds.
     */
    explicit IgnitionPhase(SputteringApp *app,
                           double         argonFlowSccm = 20.0,
                           uint64_t       durationUs    = 15'000'000ULL);

    /** @brief Reduces argon flow to ignition setpoint and latches entry time. */
    void onEnter() override;

    /** @brief Transitions to DEPOSITING after the ignition dwell. */
    void execute(SputterOS::SputterMicros systemTimeMicros) override;

    /** @brief No actuator changes on exit. */
    void onExit() override;

  private:
    SputteringApp *m_app;
    double         m_argonFlowSccm;
    uint64_t       m_durationUs;
    SputterOS::NonBlockingStopwatch m_timer;
};

// ---------------------------------------------------------------------------
// DepositingPhase
// ---------------------------------------------------------------------------

/**
 * @brief Runs the steady-state deposition process for a configurable duration.
 *
 * Maintains stable argon flow while the target material is sputtered onto
 * the substrate. On completion, transitions to VENTING.
 *
 * Default: 15 sccm argon for 60 seconds.
 */
class DepositingPhase : public SputterOS::IProcessState
{
  public:
    /**
     * @brief Constructor.
     * @param app Back-pointer used to trigger state transitions.
     * @param argonFlowSccm Argon MFC setpoint during deposition in sccm.
     * @param durationUs Deposition duration in microseconds.
     */
    explicit DepositingPhase(SputteringApp *app,
                             double         argonFlowSccm = 15.0,
                             uint64_t       durationUs    = 60'000'000ULL);

    /** @brief Sets deposition argon flow and latches entry time. */
    void onEnter() override;

    /** @brief Transitions to VENTING when the deposition timer expires. */
    void execute(SputterOS::SputterMicros systemTimeMicros) override;

    /** @brief Shuts off gas flow on exit. */
    void onExit() override;

  private:
    SputteringApp *m_app;
    double         m_argonFlowSccm;
    uint64_t       m_durationUs;
    SputterOS::NonBlockingStopwatch m_timer;
};

// ---------------------------------------------------------------------------
// VentingPhase
// ---------------------------------------------------------------------------

/**
 * @brief Deactivates the pump and vents the chamber back to atmosphere.
 *
 * Calls the pump vent routine and waits for the chamber to re-pressurize
 * above the vent threshold before transitioning to IDLE.
 *
 * Default: vent to > 900 hPa within 120 seconds.
 */
class VentingPhase : public SputterOS::IProcessState
{
  public:
    /**
     * @brief Constructor.
     * @param app Back-pointer used to trigger state transitions.
     * @param ventPressureHPa Pressure threshold indicating vent complete (hPa).
     * @param timeoutUs Maximum venting time in microseconds.
     */
    explicit VentingPhase(SputteringApp *app,
                          double         ventPressureHPa = 900.0,
                          uint64_t       timeoutUs       = 120'000'000ULL);

    /** @brief Shuts off all gas flows and signals the pump to vent. */
    void onEnter() override;

    /**
     * @brief Transitions to IDLE when pressure reaches the vent threshold
     * or when the timeout elapses.
     */
    void execute(SputterOS::SputterMicros systemTimeMicros) override;

    /** @brief No actuator changes on exit. */
    void onExit() override;

  private:
    SputteringApp *m_app;
    double         m_ventPressureHPa;
    uint64_t       m_timeoutUs;
    SputterOS::NonBlockingStopwatch m_timer;
};

#endif // PROCESS_PHASES_H
