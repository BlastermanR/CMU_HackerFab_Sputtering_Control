/**
 * @file SputteringApp.cpp
 * @brief Implementation of SputteringApp, the IUserApplication for the
 * CMU HackerFab sputtering system.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "SputteringApp.h"
#include "GlobalDevices.h"
#include "sputteros/kernel/System.h"
#include "sputteros/utils/logging/TelemetryLogger.h"
#include <cstdio>
#include <cstring>

// ===========================================================================
// Helpers
// ===========================================================================

static const char *stateToString(SputteringCfg::State s)
{
    switch (s)
    {
    case SputteringCfg::State::IDLE:       return "IDLE";
    case SputteringCfg::State::PUMP_DOWN:  return "PUMP_DOWN";
    case SputteringCfg::State::PURGE:      return "PURGE";
    case SputteringCfg::State::IGNITION:   return "IGNITION";
    case SputteringCfg::State::DEPOSITING: return "DEPOSITING";
    case SputteringCfg::State::VENTING:    return "VENTING";
    case SputteringCfg::State::FAULT:      return "FAULT";
    default:                               return "UNKNOWN";
    }
}

// ===========================================================================
// Constructor
// ===========================================================================

SputteringApp::SputteringApp()
    : m_pumpDownPhase(this),
      m_purgePhase(this),
      m_ignitionPhase(this),
      m_depositingPhase(this),
      m_ventingPhase(this)
{
}

// ===========================================================================
// IUserApplication overrides
// ===========================================================================

void SputteringApp::init()
{
    mfc1.init();
    mfc1.setGas(ALICAT_GAS_O2);

    mfc2.init();
    mfc2.setGas(ALICAT_GAS_AR);

    gauge.init();
    pump.init();

    m_state       = SputteringCfg::State::IDLE;
    m_activePhase = nullptr;
}

void SputteringApp::tick(SputterOS::SputterMicros systemTimeMicros)
{
    pollDevices(systemTimeMicros);

    if (m_activePhase != nullptr)
    {
        m_activePhase->execute(systemTimeMicros);
    }
}

void SputteringApp::handleCommand(const SputteringCfg::Command &cmd)
{
    switch (cmd.id)
    {
    case SputteringCfg::CmdID::SET_STATE:
    {
        const auto targetState = static_cast<SputteringCfg::State>(static_cast<uint8_t>(cmd.value));
        transitionTo(targetState);
        break;
    }

    case SputteringCfg::CmdID::ABORT:
    {
        forceSafeAbort();
        break;
    }

    case SputteringCfg::CmdID::SET_ARGON_FLOW:
    {
        mfc2.setSetpoint(static_cast<double>(cmd.value));
        break;
    }

    case SputteringCfg::CmdID::SET_OXYGEN_FLOW:
    {
        mfc1.setSetpoint(static_cast<double>(cmd.value));
        break;
    }

    case SputteringCfg::CmdID::SET_PUMP_SPEED:
        // Not yet implemented in PfeifferPump driver.
        break;

    case SputteringCfg::CmdID::SET_VERBOSITY:
    {
        const auto level = static_cast<SputterOS::TelemetryLogger::Verbosity>(
            static_cast<uint8_t>(cmd.value));
        SputterOS::System<SputteringCfg>::telemetryLogger().setVerbosity(level);
        break;
    }

    default:
        break;
    }
}

void SputteringApp::forceSafeAbort()
{
    // Shut off all gas flows immediately.
    mfc1.setSetpoint(0.0);
    mfc2.setSetpoint(0.0);

    // Deactivate the pump without blocking.
    pump.signalPumpOff();

    // Exit the current phase without calling onExit on the active phase
    // to avoid any blocking operations during an abort.
    m_activePhase = nullptr;
    m_state       = SputteringCfg::State::FAULT;

    auto &logger = SputterOS::System<SputteringCfg>::telemetryLogger();
    logger.log(SputterOS::TelemetryLogger::TaskID::CONTROL,
               "ABORT -> FAULT",
               SputterOS::TelemetryLogger::Verbosity::CRITICAL, 0);
}

// ===========================================================================
// State machine
// ===========================================================================

void SputteringApp::transitionTo(SputteringCfg::State next)
{
    const SputteringCfg::State prev = m_state;

    if (m_activePhase != nullptr)
    {
        m_activePhase->onExit();
    }

    m_state       = next;
    m_activePhase = phaseFor(next);

    if (m_activePhase != nullptr)
    {
        m_activePhase->onEnter();
    }

    // Log state transition via TelemetryLogger.
    char buf[48];
    snprintf(buf, sizeof(buf), "%s -> %s", stateToString(prev), stateToString(next));
    auto &logger = SputterOS::System<SputteringCfg>::telemetryLogger();
    logger.log(SputterOS::TelemetryLogger::TaskID::CONTROL,
               buf,
               SputterOS::TelemetryLogger::Verbosity::STATUS, 0);
}

SputterOS::IProcessState *SputteringApp::phaseFor(SputteringCfg::State state)
{
    switch (state)
    {
    case SputteringCfg::State::PUMP_DOWN:  return &m_pumpDownPhase;
    case SputteringCfg::State::PURGE:      return &m_purgePhase;
    case SputteringCfg::State::IGNITION:   return &m_ignitionPhase;
    case SputteringCfg::State::DEPOSITING: return &m_depositingPhase;
    case SputteringCfg::State::VENTING:    return &m_ventingPhase;
    default:                               return nullptr;
    }
}

// ===========================================================================
// Device polling
// ===========================================================================

void SputteringApp::pollDevices(SputterOS::SputterMicros systemTimeMicros)
{
    (void)systemTimeMicros;

    mfc1.update();
    mfc2.update();
    gauge.update();
    pump.update();
}
