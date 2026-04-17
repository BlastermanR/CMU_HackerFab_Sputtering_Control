/**
 * @file ProcessPhases.cpp
 * @brief Implementations of all sputtering process phase state machines.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "ProcessPhases.h"
#include "GlobalDevices.h"
#include "SputteringApp.h"

// ===========================================================================
// PumpDownPhase
// ===========================================================================

PumpDownPhase::PumpDownPhase(SputteringApp *app, double targetPressureHPa, uint64_t timeoutUs)
    : m_app(app), m_targetPressureHPa(targetPressureHPa), m_timeoutUs(timeoutUs)
{
}

void PumpDownPhase::onEnter()
{
    m_timer.reset();
    pump.signalPumpOn();
}

void PumpDownPhase::execute(SputterOS::SputterMicros systemTimeMicros)
{
    if (!m_timer.isRunning())
    {
        m_timer.start(systemTimeMicros);
    }

    if (gauge.getPressure() <= m_targetPressureHPa)
    {
        m_app->transitionTo(SputteringCfg::State::PURGE);
        return;
    }

    if (m_timer.hasExpired(systemTimeMicros, m_timeoutUs))
    {
        // Pump-down timed out — abort to fault state.
        m_app->transitionTo(SputteringCfg::State::FAULT);
    }
}

void PumpDownPhase::onExit() { m_timer.reset(); }

// ===========================================================================
// PurgePhase
// ===========================================================================

PurgePhase::PurgePhase(SputteringApp *app, double argonFlowSccm, uint64_t durationUs)
    : m_app(app), m_argonFlowSccm(argonFlowSccm), m_durationUs(durationUs)
{
}

void PurgePhase::onEnter()
{
    m_timer.reset();
    mfc2.setSetpoint(m_argonFlowSccm);
}

void PurgePhase::execute(SputterOS::SputterMicros systemTimeMicros)
{
    if (!m_timer.isRunning())
    {
        m_timer.start(systemTimeMicros);
    }

    if (m_timer.hasExpired(systemTimeMicros, m_durationUs))
    {
        m_app->transitionTo(SputteringCfg::State::IGNITION);
    }
}

void PurgePhase::onExit() { m_timer.reset(); }

// ===========================================================================
// IgnitionPhase
// ===========================================================================

IgnitionPhase::IgnitionPhase(SputteringApp *app, double argonFlowSccm, uint64_t durationUs)
    : m_app(app), m_argonFlowSccm(argonFlowSccm), m_durationUs(durationUs)
{
}

void IgnitionPhase::onEnter()
{
    m_timer.reset();
    mfc2.setSetpoint(m_argonFlowSccm);
}

void IgnitionPhase::execute(SputterOS::SputterMicros systemTimeMicros)
{
    if (!m_timer.isRunning())
    {
        m_timer.start(systemTimeMicros);
    }

    if (m_timer.hasExpired(systemTimeMicros, m_durationUs))
    {
        m_app->transitionTo(SputteringCfg::State::DEPOSITING);
    }
}

void IgnitionPhase::onExit() { m_timer.reset(); }

// ===========================================================================
// DepositingPhase
// ===========================================================================

DepositingPhase::DepositingPhase(SputteringApp *app, double argonFlowSccm, uint64_t durationUs)
    : m_app(app), m_argonFlowSccm(argonFlowSccm), m_durationUs(durationUs)
{
}

void DepositingPhase::onEnter()
{
    m_timer.reset();
    mfc2.setSetpoint(m_argonFlowSccm);
}

void DepositingPhase::execute(SputterOS::SputterMicros systemTimeMicros)
{
    if (!m_timer.isRunning())
    {
        m_timer.start(systemTimeMicros);
    }

    if (m_timer.hasExpired(systemTimeMicros, m_durationUs))
    {
        m_app->transitionTo(SputteringCfg::State::VENTING);
    }
}

void DepositingPhase::onExit()
{
    m_timer.reset();
    // Shut off gas flow when deposition ends.
    mfc1.setSetpoint(0.0);
    mfc2.setSetpoint(0.0);
}

// ===========================================================================
// VentingPhase
// ===========================================================================

VentingPhase::VentingPhase(SputteringApp *app, double ventPressureHPa, uint64_t timeoutUs)
    : m_app(app), m_ventPressureHPa(ventPressureHPa), m_timeoutUs(timeoutUs)
{
}

void VentingPhase::onEnter()
{
    m_timer.reset();
    mfc1.setSetpoint(0.0);
    mfc2.setSetpoint(0.0);
    pump.signalPumpOff();
    pump.ventPump();
}

void VentingPhase::execute(SputterOS::SputterMicros systemTimeMicros)
{
    if (!m_timer.isRunning())
    {
        m_timer.start(systemTimeMicros);
    }

    if (gauge.getPressure() >= m_ventPressureHPa || m_timer.hasExpired(systemTimeMicros, m_timeoutUs))
    {
        m_app->transitionTo(SputteringCfg::State::IDLE);
    }
}

void VentingPhase::onExit() { m_timer.reset(); }
