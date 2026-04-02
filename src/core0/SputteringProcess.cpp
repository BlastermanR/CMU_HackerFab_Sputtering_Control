#include "core0/SputteringProcess.h"
#include "shared/Intercore.h"

#include "pico/stdlib.h"

using namespace SputteringConstants;

// ============================================================================
// Construction
// ============================================================================

SputteringProcess::SputteringProcess(AlicatMFC&     mfc_argon,
                                     AlicatMFC&     mfc_oxygen,
                                     PfeifferGauge& gauge,
                                     PfeifferPump&  pump)
    : _mfcArgon(mfc_argon),
      _mfcOxygen(mfc_oxygen),
      _gauge(gauge),
      _pump(pump)
{}

// ============================================================================
// Process Control
// ============================================================================

void SputteringProcess::startProcess() {
    if (_state != ProcessState::IDLE) return;
    transitionTo(ProcessState::PUMPING_DOWN);
}

void SputteringProcess::stopProcess() {
    if (_state == ProcessState::IDLE || _state == ProcessState::FAULT) return;
    transitionTo(ProcessState::SHUTTING_DOWN);
}

void SputteringProcess::emergencyShutdown() {
    zeroAllGasFlows();
    _pump.deactivatePump();
    _pump.ventPump();
    transitionTo(ProcessState::FAULT);
}

// ============================================================================
// Main Update — call every CORE0_UPDATE_INTERVAL_MS
// ============================================================================

void SputteringProcess::update() {
    // Run periodic safety watchdog in active states.
    if (_state == ProcessState::SPUTTERING ||
        _state == ProcessState::GAS_STABILIZING ||
        _state == ProcessState::PUMPING_DOWN)
    {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - _lastSafetyCheck_ms >= SAFETY_WATCHDOG_INTERVAL_MS) {
            _lastSafetyCheck_ms = now;
            if (runSafetyChecks()) {
                emergencyShutdown();
                return;
            }
        }
    }

    switch (_state) {
        case ProcessState::PUMPING_DOWN:    handlePumpingDown();    break;
        case ProcessState::GAS_STABILIZING: handleGasStabilizing(); break;
        case ProcessState::SPUTTERING:      handleSputtering();     break;
        case ProcessState::SHUTTING_DOWN:   handleShuttingDown();   break;
        case ProcessState::IDLE:
        case ProcessState::FAULT:
        default:
            break;
    }
}

// ============================================================================
// Setpoints
// ============================================================================

void SputteringProcess::setArgonFlowSetpoint(double sccm) {
    if (sccm < 0.0) sccm = 0.0;
    if (sccm > MAX_ARGON_FLOW_SCCM) sccm = MAX_ARGON_FLOW_SCCM;
    _argonSetpoint_sccm = sccm;

    // Apply immediately if in an active process phase.
    if (_state == ProcessState::GAS_STABILIZING ||
        _state == ProcessState::SPUTTERING)
    {
        applyArgonSetpoint();
    }
}

void SputteringProcess::setOxygenFlowSetpoint(double sccm) {
    if (sccm < 0.0) sccm = 0.0;
    if (sccm > MAX_OXYGEN_FLOW_SCCM) sccm = MAX_OXYGEN_FLOW_SCCM;
    _oxygenSetpoint_sccm = sccm;

    if (_state == ProcessState::GAS_STABILIZING ||
        _state == ProcessState::SPUTTERING)
    {
        applyOxygenSetpoint();
    }
}

// ============================================================================
// Phase Handlers
// ============================================================================

void SputteringProcess::handlePumpingDown() {
    uint32_t elapsed = getTimeInState_ms();

    // TODO: Add a pre-check: if chamber pressure > PUMP_SAFE_SPINUP_PRESSURE_hPa
    //       when this phase begins, wait (or add a rough-pump phase) before
    //       enabling the turbo.

    if (!_pump.getPumpSpeed()) {
        // TODO: Signal pump on once pump is not yet running.
        _pump.activatePump();
    }

    double pressure = _gauge.getPressure();

    if (pressure <= BASE_PRESSURE_THRESHOLD_hPa &&
        _pump.getPumpSpeed() >= PUMP_MIN_OPERATIONAL_SPEED_HZ)
    {
        // Base pressure achieved — begin gas introduction.
        transitionTo(ProcessState::GAS_STABILIZING);
        return;
    }

    if (elapsed >= PUMP_DOWN_TIMEOUT_MS) {
        // TODO: Log the actual pressure vs threshold for diagnostics.
        _activeFaults = _activeFaults | SafetyFault::BASE_PRESSURE_MISS;
        emergencyShutdown();
    }
}

void SputteringProcess::handleGasStabilizing() {
    // TODO: On first entry (detect via a flag or check MFC setpoints == 0),
    //       call applyArgonSetpoint() and applyOxygenSetpoint() to open gas.

    double pressure = _gauge.getPressure();
    double target   = WORKING_PRESSURE_TARGET_hPa;
    double tol      = WORKING_PRESSURE_TOLERANCE_hPa;

    bool pressureInBand = (pressure >= target - tol) && (pressure <= target + tol);

    if (pressureInBand && getTimeInState_ms() >= GAS_STABILIZATION_TIME_MS) {
        // Pressure is stable — ready for sputtering.
        transitionTo(ProcessState::SPUTTERING);
        return;
    }

    // TODO: Implement a PID or feed-forward controller here to adjust
    //       _argonSetpoint_sccm to servo onto WORKING_PRESSURE_TARGET_hPa.
    //       Call applyArgonSetpoint() after updating _argonSetpoint_sccm.
}

void SputteringProcess::handleSputtering() {
    // TODO: Run the PID pressure control loop.
    //       Update _argonSetpoint_sccm based on gauge feedback and apply.

    // TODO: Log periodic data snapshots to the output queue for the display.
}

void SputteringProcess::handleShuttingDown() {
    // Ramp gas flows toward zero.
    // TODO: Implement a gradual ramp instead of an immediate zero.
    zeroAllGasFlows();

    uint32_t elapsed = getTimeInState_ms();

    // Wait for MFCs to confirm near-zero flow before stopping the pump.
    bool mfcsAtZero = (_mfcArgon.getMassFlow()  < MIN_DETECTABLE_FLOW_SCCM) &&
                      (_mfcOxygen.getMassFlow() < MIN_DETECTABLE_FLOW_SCCM);

    if (mfcsAtZero || elapsed >= MFC_RAMPDOWN_TIMEOUT_MS) {
        _pump.deactivatePump();
        // NOTE: Intentionally NOT venting; chamber stays under vacuum.
        //       Remove this note and call _pump.ventPump() when venting is desired.
        transitionTo(ProcessState::IDLE);
    }
}

// ============================================================================
// Safety Checks
// ============================================================================

bool SputteringProcess::runSafetyChecks() {
    SafetyFault newFaults = SafetyFault::NONE;

    if (!checkPressureSafe())  newFaults = newFaults | SafetyFault::OVERPRESSURE;
    if (!checkPumpSpeed())     newFaults = newFaults | SafetyFault::LOW_PUMP_SPEED;
    if (!checkMFCFlows())      newFaults = newFaults | SafetyFault::MFC_FLOW_OVERRANGE;
    if (!checkGaugeHealth())   newFaults = newFaults | SafetyFault::GAUGE_ERROR;

    _activeFaults = newFaults;
    return newFaults != SafetyFault::NONE;
}

bool SputteringProcess::checkPressureSafe() {
    return _gauge.getPressure() < OVERPRESSURE_LIMIT_hPa;
}

bool SputteringProcess::checkPumpSpeed() {
    // Only enforce speed once the pump has had time to spin up.
    if (_state == ProcessState::PUMPING_DOWN &&
        getTimeInState_ms() < 10000)  // give 10 s grace on startup
    {
        return true;
    }
    return _pump.getPumpSpeed() >= PUMP_MIN_OPERATIONAL_SPEED_HZ;
}

bool SputteringProcess::checkMFCFlows() {
    // TODO: Also check AlicatMFC::getStatusCodes() for error codes (e.g. "OVR", "POV").
    bool arOk = _mfcArgon.getMassFlow()  <= MAX_ARGON_FLOW_SCCM  * 1.1;  // 10% headroom
    bool oxOk = _mfcOxygen.getMassFlow() <= MAX_OXYGEN_FLOW_SCCM * 1.1;
    return arOk && oxOk;
}

bool SputteringProcess::checkGaugeHealth() {
    // A reading of exactly 0.0 after init almost certainly means the gauge is not
    // communicating. Reject it to avoid false "base pressure reached" transitions.
    // TODO: Add a staleness check — if pressure hasn't updated in N seconds, fault.
    return _gauge.getPressure() > 0.0;
}

// ============================================================================
// Helpers
// ============================================================================

void SputteringProcess::transitionTo(ProcessState next) {
    _state = next;
    _stateEnteredAt_ms = to_ms_since_boot(get_absolute_time());

    // TODO: Log the transition to the output queue.
}

void SputteringProcess::applyArgonSetpoint() {
    _mfcArgon.setSetpoint(_argonSetpoint_sccm);
}

void SputteringProcess::applyOxygenSetpoint() {
    _mfcOxygen.setSetpoint(_oxygenSetpoint_sccm);
}

void SputteringProcess::zeroAllGasFlows() {
    _mfcArgon.setSetpoint(0.0);
    _mfcOxygen.setSetpoint(0.0);
}

uint32_t SputteringProcess::getTimeInState_ms() const {
    return to_ms_since_boot(get_absolute_time()) - _stateEnteredAt_ms;
}

// ============================================================================
// Utility
// ============================================================================

const char* processStateToString(ProcessState state) {
    switch (state) {
        case ProcessState::IDLE:             return "IDLE";
        case ProcessState::PUMPING_DOWN:     return "PUMPING_DOWN";
        case ProcessState::GAS_STABILIZING:  return "GAS_STABILIZING";
        case ProcessState::SPUTTERING:       return "SPUTTERING";
        case ProcessState::SHUTTING_DOWN:    return "SHUTTING_DOWN";
        case ProcessState::FAULT:            return "FAULT";
        default:                             return "UNKNOWN";
    }
}
