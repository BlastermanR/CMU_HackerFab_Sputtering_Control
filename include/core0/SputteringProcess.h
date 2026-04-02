/**
 * SputteringProcess.h
 * @brief Defines the core sputtering process state machine for Core 0.
 *        Owns all process-level constants, phase sequencing, and safety
 *        interlocks. Device references are injected at construction.
 *
 *        Typical sequence:
 *          IDLE -> PUMPING_DOWN -> GAS_STABILIZING -> SPUTTERING -> SHUTTING_DOWN -> IDLE
 *          Any active phase -> FAULT (on safety violation)
 *
 *  TODO: THIS IS JUST AN AI PROTOYPE, WILL REQUIRE MASSIVE REVISIONS
 * 
 * @author Ryan Massie (rmassie)
 * @date 4/1/26
 */
#ifndef SPUTTERING_PROCESS_H
#define SPUTTERING_PROCESS_H

#include "AlicatMFC.h"
#include "PfeifferGauge.h"
#include "PfeifferPump.h"


// ----------------------------------------------------------------------------
// Process Constants
// ----------------------------------------------------------------------------

namespace SputteringConstants {

    // --- Pressure thresholds (hPa) ---

    // Chamber must reach this base pressure before sputtering can begin.
    // 1e-4 hPa ≈ 7.5e-5 Torr (high-vacuum regime for clean sputtering).
    constexpr double BASE_PRESSURE_THRESHOLD_hPa    = 1e-4;

    // Target working pressure during Ar sputtering (hPa).
    // 6.67e-3 hPa ≈ 5 mTorr — typical DC/RF magnetron working pressure.
    constexpr double WORKING_PRESSURE_TARGET_hPa    = 6.67e-3;

    // Acceptable band around working pressure (±, hPa).
    constexpr double WORKING_PRESSURE_TOLERANCE_hPa = 1e-3;

    // Immediate emergency shutdown if pressure exceeds this value during sputtering.
    constexpr double OVERPRESSURE_LIMIT_hPa         = 1e-1;

    // Maximum pressure at which the turbo pump is safe to spin up.
    // Spinning up under too high a pressure can damage the turbo bearings.
    constexpr double PUMP_SAFE_SPINUP_PRESSURE_hPa  = 1.0;


    // --- Pump speed thresholds (Hz) ---

    // Minimum turbo pump speed before allowing gas flow or power on.
    // TC110 rated maximum is typically ~1500 Hz — adjust to actual nameplate.
    constexpr double PUMP_MIN_OPERATIONAL_SPEED_HZ  = 1350.0;

    // Speed considered "full speed" (used in status reporting and sequencing).
    constexpr double PUMP_FULL_SPEED_HZ             = 1500.0;


    // --- MFC flow limits (sccm) ---

    // Hard upper limit for Argon setpoint. Prevents excessive working pressure.
    constexpr double MAX_ARGON_FLOW_SCCM            = 50.0;

    // Hard upper limit for Oxygen setpoint. O2 reacts with target; must be limited.
    constexpr double MAX_OXYGEN_FLOW_SCCM           = 10.0;

    // Minimum non-zero flow considered "on" (used to detect MFC errors / no flow).
    constexpr double MIN_DETECTABLE_FLOW_SCCM       = 0.1;


    // --- Timing constants (milliseconds) ---

    // Maximum time to wait for the chamber to reach BASE_PRESSURE_THRESHOLD_hPa.
    constexpr uint32_t PUMP_DOWN_TIMEOUT_MS          = 300000;  // 5 minutes

    // Time to hold stable gas flow before enabling power after reaching
    // working pressure — allows pressure to equilibrate.
    constexpr uint32_t GAS_STABILIZATION_TIME_MS     = 5000;

    // Period between safety-check evaluations during active sputtering.
    constexpr uint32_t SAFETY_WATCHDOG_INTERVAL_MS   = 500;

    // Maximum time allowed for MFC ramp-down during normal shutdown.
    constexpr uint32_t MFC_RAMPDOWN_TIMEOUT_MS       = 2000;

    // Pump speed polling interval during pump-down phase.
    constexpr uint32_t PUMP_POLL_INTERVAL_MS         = 1000;

}  // namespace SputteringConstants


// ----------------------------------------------------------------------------
// Process State Machine
// ----------------------------------------------------------------------------

enum class ProcessState : uint8_t {
    IDLE,               // No active process; devices at rest.
    PUMPING_DOWN,       // Turbo pump running; waiting for base pressure.
    GAS_STABILIZING,    // Ar flow introduced; waiting for working pressure.
    SPUTTERING,         // Active sputtering; all safety watchdogs running.
    SHUTTING_DOWN,      // Normal shutdown sequence in progress.
    FAULT               // Safety violation detected; emergency stop issued.
};

// Human-readable label for logging / display.
const char* processStateToString(ProcessState state);


// ----------------------------------------------------------------------------
// Safety Status
// ----------------------------------------------------------------------------

// Bitmask of active safety faults. Multiple faults can be present at once.
enum class SafetyFault : uint16_t {
    NONE                = 0x0000,
    OVERPRESSURE        = 0x0001,   // Chamber pressure exceeded OVERPRESSURE_LIMIT
    LOW_PUMP_SPEED      = 0x0002,   // Turbo fell below MIN_OPERATIONAL_SPEED
    MFC_FLOW_OVERRANGE  = 0x0004,   // An MFC reported flow above its hard limit
    BASE_PRESSURE_MISS  = 0x0008,   // Pump-down timed out before reaching base pressure
    GAUGE_ERROR         = 0x0010,   // Pressure gauge not returning valid readings
    MFC_ERROR           = 0x0020,   // MFC device reported an error status code
    PUMP_ERROR          = 0x0040,   // Pump controller reported fault
};

inline SafetyFault operator|(SafetyFault a, SafetyFault b) {
    return static_cast<SafetyFault>(
        static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}
inline bool hasFault(SafetyFault flags, SafetyFault check) {
    return (static_cast<uint16_t>(flags) & static_cast<uint16_t>(check)) != 0;
}


// ----------------------------------------------------------------------------
// SputteringProcess
// ----------------------------------------------------------------------------

class SputteringProcess {
public:
    // Inject device references. Does not take ownership.
    SputteringProcess(AlicatMFC& mfc_argon,
                      AlicatMFC& mfc_oxygen,
                      PfeifferGauge& gauge,
                      PfeifferPump& pump);

    // ---- Process control -----------------------------------------------

    // Request a transition to PUMPING_DOWN.
    // Safe to call only when IDLE; ignored otherwise.
    void startProcess();

    // Request a graceful shutdown (SPUTTERING → SHUTTING_DOWN → IDLE).
    void stopProcess();

    // Immediately zero gas and deactivate pump. Transitions to FAULT.
    // Called by safety watchdog or external emergency source.
    void emergencyShutdown();

    // Must be called every CORE0_UPDATE_INTERVAL_MS from the core 0 loop.
    // Advances the state machine and runs periodic safety checks.
    void update();

    // ---- Setpoints (validated against hard limits before applying) ------

    // Set Argon flow target (sccm). Clamped to MAX_ARGON_FLOW_SCCM.
    void setArgonFlowSetpoint(double sccm);

    // Set Oxygen flow target (sccm). Clamped to MAX_OXYGEN_FLOW_SCCM.
    void setOxygenFlowSetpoint(double sccm);

    // ---- Status queries ------------------------------------------------

    ProcessState getState() const  { return _state; }
    SafetyFault  getActiveFaults() const { return _activeFaults; }

    double getArgonFlowSetpoint()  const { return _argonSetpoint_sccm; }
    double getOxygenFlowSetpoint() const { return _oxygenSetpoint_sccm; }

    // Returns elapsed time in current state (ms).
    uint32_t getTimeInState_ms() const;

private:
    // ---- Device references ---------------------------------------------
    AlicatMFC&     _mfcArgon;
    AlicatMFC&     _mfcOxygen;
    PfeifferGauge& _gauge;
    PfeifferPump&  _pump;

    // ---- State ---------------------------------------------------------
    ProcessState _state       = ProcessState::IDLE;
    SafetyFault  _activeFaults = SafetyFault::NONE;
    uint32_t     _stateEnteredAt_ms = 0;  // absolute time (ms_since_boot) of last transition
    uint32_t     _lastSafetyCheck_ms = 0;

    // ---- Setpoints -----------------------------------------------------
    double _argonSetpoint_sccm  = 0.0;
    double _oxygenSetpoint_sccm = 0.0;

    // ---- Internal phase handlers (called from update()) ----------------
    void handlePumpingDown();
    void handleGasStabilizing();
    void handleSputtering();
    void handleShuttingDown();

    // ---- Safety checks -------------------------------------------------

    // Run all checks; accumulates faults into _activeFaults.
    // Returns true if any new fault was detected.
    bool runSafetyChecks();

    bool checkPressureSafe();    // pressure < OVERPRESSURE_LIMIT
    bool checkPumpSpeed();       // speed >= MIN_OPERATIONAL_SPEED
    bool checkMFCFlows();        // flows within hard limits & no error codes
    bool checkGaugeHealth();     // gauge returning plausible readings

    // ---- Helpers -------------------------------------------------------
    void transitionTo(ProcessState next);
    void applyArgonSetpoint();
    void applyOxygenSetpoint();
    void zeroAllGasFlows();
};

#endif  // SPUTTERING_PROCESS_H
